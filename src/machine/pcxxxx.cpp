//#include	<string.h>
//#include	<stdlib.h>
#include <QDebug>

#include <QFile>
#include <QFileDialog>
#include <QPainter>
#include <QResizeEvent>

#include "common.h"
#include "pcxxxx.h"
#include "Inter.h"
#include "Lcdc.h"
#include "cpu.h"
#include "Connect.h"
#include "Log.h"
#include "Dasm.h"
#include "Debug.h"
#include "watchpoint.h"
#include "ui/dialogdasm.h"
#include "cextension.h"
#include "Keyb.h"
#include "xmlwriter.h"
#include "ui/cregcpu.h"
#include "breakpoint.h"

#define		bREAD				0

extern bool	UpdateDisplayRunning;
extern int ask(QWidget *parent,QString msg,int nbButton);


CpcXXXX::CpcXXXX(CPObject *parent)	: CPObject(parent)
{								//[constructor]
    Initial_Session_Fname = "TO DO";
    pCPU		= 0;
    pCONNECTOR	= 0;
    pSIOCONNECTOR	= 0;
    KeyMapLenght = 0;
    pBreakpointManager = new CbreakpointManager(this);

    RomBank=RamBank=0;
    fp_log  = 0;
    off			= true;
    setcfgfname(QString("pcXXXX"));
    SessionHeader	= "PCXXXXPKM";

    memset(Log_String,0,sizeof(Log_String));
    memset(Regs_String,0,sizeof(Regs_String));

    SoundOn			= 1;
    IO_A = IO_B = IO_C = IO_F = 0;
    RomBank=RamBank=ProtectMemory=0;

    Japan		= false;
    timerRate   = 1;


    Tape_Base_Freq=4000;

    ext_11pins		= 0;
    ext_MemSlot1	= 0;
    ext_MemSlot2	= 0;
    ext_MemSlot3	= 0;
    ext_Serial		= 0;
    ext_60pins		= 0;

    setPosX(0);
    setPosY(0);

    ioFreq = 0;//24000;
    DasmFlag = false;
    DasmStep = false;
    TraceRange.clear();
    BreakSubLevel = -1;


    connect(this,SIGNAL(showDasm()),this,SLOT(Dasm()));

}

CpcXXXX::~CpcXXXX()
{
    free(mem);
    delete pCONNECTOR;
    delete pSIOCONNECTOR;
    delete pCPU;
//    delete ext_MemSlot1;
//    delete ext_MemSlot2;
//    delete ext_MemSlot3;
}

bool CpcXXXX::UpdateFinalImage(void)
{
#ifdef Q_OS_ANDROID
#define TRANSFORM Qt::FastTransformation
#else
#define TRANSFORM Qt::SmoothTransformation
//#define TRANSFORM Qt::FastTransformation
#endif

    //    qWarning()<<"UpdateFinalImage";

    // Paint FinalImage
    QRect                        destRect,srcRect;
    int x,y,z,t;

    QPainter painter;

    if (pLCDC) {
//        if (!pLCDC->Refresh) return false;
    }
    CPObject::UpdateFinalImage();

    if ( (BackgroundImage) )
    {
        painter.begin(FinalImage);
//        painter.drawImage(QPoint(0,0),*BackgroundImage);

        if (pLCDC && pLCDC->rect.isValid())
        {
            if (pLCDC->symbRect.isValid()) {
                //painter.setRenderHint(QPainter::Antialiasing);
                x = pLCDC->symbRect.x() * internalImageRatio;
                y = pLCDC->symbRect.y() * internalImageRatio;
                z = (int) (pLCDC->symbRect.width() * pLCDC->Lcd_Symb_ratio_X * internalImageRatio);
                t = (int) (pLCDC->symbRect.height()* pLCDC->Lcd_Symb_ratio_Y * internalImageRatio);

                painter.drawImage(QRect(x,y,z,t),pLCDC->SymbImage->scaled(z,t,Qt::IgnoreAspectRatio,TRANSFORM));
            }
            x	= pLCDC->rect.x() * internalImageRatio;
            y	= pLCDC->rect.y() * internalImageRatio;
            z	= (int) (pLCDC->rect.width() * internalImageRatio);
            t	= (int) (pLCDC->rect.height() * internalImageRatio);
            painter.drawImage(QRect(x,y,z,t),pLCDC->LcdImage->scaled(z,t,Qt::IgnoreAspectRatio,TRANSFORM));

        }
        painter.end();

        if (pLCDC) pLCDC->Refresh = false;
    }


    return true;
}

bool CpcXXXX::CompleteDisplay(void)
{
    return true;
}


bool CpcXXXX::InitDisplay(void)
{
    CPObject::InitDisplay();

//    Refresh_Display = true;
    UpdateDisplayRunning = false;

    if (pLCDC) {
        pLCDC->InitDisplay();
    }

    UpdateDisplayRunning = true;
    Refresh_Display = true;
    return(1);
}

void CpcXXXX::TurnOFF(void)
{
#ifdef EMSCRIPTEN
    mainwindow->saveAll=NO;
#endif
    switch (mainwindow->saveAll) {
    case ASK: if (ask(mainwindow,tr( "Do you want to save the session ?"),2)==1)
            {
             Initial_Session_Save();
            }
        break;
    case YES: Initial_Session_Save(); break;
    default : break;

    }


    off = 1;
    Power = false;
    PowerSwitch = PS_OFF;
    if (pLCDC) pLCDC->TurnOFF();
    InitDisplay();

    Refresh_Display = true;
    update();
}


void CpcXXXX::TurnON(void)
{

    if (pKEYB->LastKey == 0) pKEYB->LastKey = K_POW_ON;
    qWarning()<<"power1="<<Power<< " k="<<pKEYB->LastKey;
    if ( (pKEYB->LastKey == K_POW_ON) ||
         (!Power && pKEYB->LastKey == K_OF) ||
         (!Power && pKEYB->LastKey == K_BRK))
    {
         qWarning()<<"power ON:";
        AddLog(LOG_MASTER,"Power ON");
        if (!hardreset) {
//            Initial_Session_Load();
        }
        else hardreset = false;
        off = 0;
        Power = true;
        PowerSwitch = PS_RUN;
        if (pLCDC) pLCDC->TurnON();
        pKEYB->LastKey = 0;
    }
}

void CpcXXXX::Reset(void)
{
	pCPU->Reset();
}

BYTE	CpcXXXX::Get_PortA(void) {	return (IO_A); }
BYTE	CpcXXXX::Get_PortB(void) {	return (IO_B); }
BYTE	CpcXXXX::Get_PortC(void) {	return (IO_C); }
BYTE	CpcXXXX::Get_PortF(void) {	return (IO_F); }
BYTE	CpcXXXX::Get_PortT(void) {	return (IO_T); }

BYTE	CpcXXXX::Get_Port(PORTS Port)
{
	switch (Port)
	{
		case PORT_A: return (Get_PortA());
		case PORT_B: return (Get_PortB());
		case PORT_C: return (Get_PortC());
		case PORT_F: return (Get_PortF());
		case PORT_T: return (Get_PortT());
	}
	return (0);
}

bool	CpcXXXX::Get_Port_bit(PORTS Port, int bit)
{
	return ( (Get_Port(Port) >> (bit-1)) & 1);
}

void	CpcXXXX::Set_PortA(BYTE data) {	IO_A = data; }
void	CpcXXXX::Set_PortB(BYTE data) {	IO_B = data; }
void	CpcXXXX::Set_PortF(BYTE data) {	IO_F = data; }
void	CpcXXXX::Set_PortT(BYTE data) {	IO_T = data; }
void	CpcXXXX::Set_PortC(BYTE data) {	IO_C = data; 
//										pLCDC->Refresh = ((IO_C & 0x01)?true:false);
													}

void	CpcXXXX::Set_Port(PORTS Port,BYTE data)
{
	switch(Port)
	{
		case PORT_A: Set_PortA(data);	break;
		case PORT_B: Set_PortB(data);	break;
		case PORT_C: Set_PortC(data);	break;
		case PORT_F: Set_PortF(data);	break;
		case PORT_T: Set_PortT(data);	break;
	}
}


void	CpcXXXX::Set_Port_Bit(PORTS Port, int bit, BYTE data)
{
	int t;

	if (data)
	{
		t=1<<(bit-1);
		switch (Port)
		{
		case PORT_A: IO_A |= t;break;
		case PORT_B: IO_B |= t;break;
		case PORT_C: IO_C |= t;break;
		case PORT_F: IO_F |= t;break;
		case PORT_T: IO_T |= t;break;
		}
	}
	else
	{
		t=0xFF - (1<<(bit-1));
		switch (Port)
		{
		case PORT_A: IO_A &= t;break;
		case PORT_B: IO_B &= t;break;
		case PORT_C: IO_C &= t;break;
		case PORT_F: IO_F &= t;break;
		case PORT_T: IO_T &= t;break;
		}
	}
}



/*****************************************************************************/
/* Get data from mem[]														 */
/*****************************************************************************/
void CpcXXXX::checkBreakRead(UINT32 adr,UINT32 d) {
    if (pBreakpointManager->isBreak(Cbreakpoint::READ,adr,d)) {
        BreakSubLevel = 99999;
        DasmStep = true;
        DasmFlag = false;
        emit showDasm();
//        emit RefreshDasm();
    }
}
void CpcXXXX::checkBreakWrite(UINT32 adr,UINT32 d) {
    if (pBreakpointManager->isBreak(Cbreakpoint::WRITE,adr,d)) {
        BreakSubLevel = 99999;
        DasmStep = true;
        DasmFlag = false;
        emit showDasm();
//        emit RefreshDasm();
    }
}
BYTE CpcXXXX::Get_PC(UINT32 adr)
{
    UINT32 extValue = 0;
    if (Chk_Adr_R(&adr,&extValue)) {
        checkBreakRead(adr,mem[adr]);
        return(mem[adr]);
    }
    else {
        checkBreakRead(adr,extValue);
        return(extValue);
    }
}
BYTE CpcXXXX::Get_8(UINT32 adr)
{
    UINT32 extValue = 0;
    if (Chk_Adr_R(&adr,&extValue)) {
        checkBreakRead(adr,mem[adr]);
        return(mem[adr]);
    }
    else{
        checkBreakRead(adr,extValue);
        return(extValue);
    }
}

WORD CpcXXXX::Get_16(UINT32 adr)
{
    UINT32 extValue1 = 0;
    UINT32 extValue2 = 0;
	UINT32	a;
	a=adr+1;
    bool c1 = Chk_Adr_R(&adr,&extValue1);
    bool c2 = Chk_Adr_R(&a,&extValue2);
    if (c1 && c2) {
        checkBreakRead(adr,mem[adr]);
        checkBreakRead(a,mem[a]<<8);
        return(mem[adr]+(mem[a]<<8));
    }
    else {
        checkBreakRead(adr,extValue1);
        checkBreakRead(a,extValue2<<8);
        return(extValue1+(extValue2<<8));
    }
}

WORD CpcXXXX::Get_16r(UINT32 adr)
{
    UINT32 extValue1 = 0;
    UINT32 extValue2 = 0;
	UINT32	a;
	a=adr+1;
    bool c1 = Chk_Adr_R(&adr,&extValue1);
    bool c2 = Chk_Adr_R(&a,&extValue2);
    if (c1 && c2) {
        checkBreakRead(adr,mem[adr]<<8);
        checkBreakRead(a,mem[a]);
        return((mem[adr]<<8)+mem[a]);
    }
    else {
        checkBreakRead(adr,extValue1<<8);
        checkBreakRead(a,extValue2);
        return((extValue1<<8)+extValue2);
    }
}

WORD CpcXXXX::Get_16rPC(UINT32 adr)
{
#if 0
    UINT32	a;
    a=adr+1;
    if (Chk_Adr_R(&adr,bREAD) && Chk_Adr_R(&a,bREAD)) {
        checkBreakRead(adr,mem[adr]<<8);
        checkBreakRead(a,mem[a]);
        return((mem[adr]<<8)+mem[a]);
    }
    else
        return(0);
#else
    UINT32 extValue1 = 0;
    UINT32 extValue2 = 0;
    UINT32	a;
    a=adr+1;
    bool c1 = Chk_Adr_R(&adr,&extValue1);
    bool c2 = Chk_Adr_R(&a,&extValue2);
    if (c1 && c2) {
        checkBreakRead(adr,mem[adr]<<8);
        checkBreakRead(a,mem[a]);
        return((mem[adr]<<8)+mem[a]);
    }
    else {
        checkBreakRead(adr,extValue1<<8);
        checkBreakRead(a,extValue2);
        return((extValue1<<8)+extValue2);
    }
#endif
}

UINT32 CpcXXXX::Get_20(UINT32 adr)
{
    Chk_Adr_R(&adr,bREAD);
    UINT32 data = (mem[adr]+(mem[adr+1]<<8)+(mem[adr+2]<<16))&MASK_20;
    return(data);
}

UINT32 CpcXXXX::Get_24(UINT32 adr)
{
    Chk_Adr_R(&adr,bREAD);
    return((mem[adr]+(mem[adr+1]<<8)+(mem[adr+2]<<16))&MASK_24);
}

UINT32 CpcXXXX::get_mem(UINT32 adr,int size)
{
    switch(size)
    {
    case SIZE_8 :return(Get_PC(adr));
    case SIZE_16:return(Get_PC(adr)+(Get_PC(adr+1)<<8));
    case SIZE_20:return((Get_PC(adr)+(Get_PC(adr+1)<<8)+(Get_PC(adr+2)<<16))&MASK_20);
    case SIZE_24:return((Get_PC(adr)+(Get_PC(adr+1)<<8)+(Get_PC(adr+2)<<16))&MASK_24);
    }
    return(0);
}
/*****************************************************************************/
/* Set data to mem[]														 */
/*  ENTRY :DWORD adr=RAM address, BYTE(8),WORD(16),DWORD(20,24) d=data		 */
/*****************************************************************************/
void CpcXXXX::Set_8(UINT32 adr,BYTE d)
{
    if(Chk_Adr(&adr,d)) {
        checkBreakWrite(adr,d);
        if (adr > memsize) {
            qWarning()<<"ERROR MEMORY WRITE OUT RANGE";
        }
		mem[adr]=d;
    }
    else checkBreakWrite(adr,d);
}

void CpcXXXX::Set_16(UINT32 adr,WORD d)
{
	UINT32	a;
	a=adr;
    if(Chk_Adr(&a,d)) {
        checkBreakWrite(a,d);
        mem[a]=(BYTE) d;
    }
    else checkBreakWrite(adr,d);
    a=adr+1;
    if(Chk_Adr(&a,(d>>8))) {
        checkBreakWrite(a,d>>8);
        mem[a]=(BYTE) (d>>8);
    }
    else checkBreakWrite(adr,d);
}
 
void CpcXXXX::Set_16r(UINT32 adr,WORD d)
{
    UINT32	a;
    a=adr;
    if(Chk_Adr(&a,(d>>8))) {
        checkBreakWrite(a,d>>8);
        mem[a]=(BYTE) (d>>8);
    }
    else checkBreakWrite(adr,d);
    a=adr+1;
    if(Chk_Adr(&a,d)) {
        checkBreakWrite(a,d);
        mem[a]=(BYTE) d;
    }
    else checkBreakWrite(adr,d);
}

void CpcXXXX::Set_20(UINT32 adr, UINT32 d)
{
    UINT32	a;
    a=adr;
    if(Chk_Adr(&a,d)) mem[a]=d;
    a=++adr;
    if(Chk_Adr(&a,(d>>8))) mem[a]=(d>>8);
    a=++adr;
    if(Chk_Adr(&a,(d>>16)&MASK_4)) mem[a]=(d>>16)&MASK_4;
}

void CpcXXXX::Set_24(UINT32 adr, UINT32 d)
{
    UINT32	a;
    a=adr;
    if(Chk_Adr(&a,d)) mem[a]=d;
    a=++adr;
    if(Chk_Adr(&a,(d>>8))) mem[a]=(d>>8);
    a=++adr;
    if(Chk_Adr(&a,(d>>16))) mem[a]=(d>>16);
}

void CpcXXXX::set_mem(UINT32 adr,int size,UINT32 data)
{
    switch(size)
    {
    case SIZE_8 :
        Set_8(adr , (BYTE) data);
        break;
    case SIZE_16:
        Set_8(adr , (BYTE) data);
        Set_8(adr+1 , (BYTE) (data>>8));
        break;
    case SIZE_20:
        Set_8(adr , (BYTE) data);
        Set_8(adr+1 , (BYTE) (data>>8));
        Set_8(adr+2 , (BYTE) ((data>>16)&MASK_4));
        break;
    case SIZE_24:
        Set_8(adr , (BYTE) data);
        Set_8(adr+1 , (BYTE) (data>>8));
        Set_8(adr+2 , (BYTE) (data>>16));
        break;
    }
}
/*****************************************************************************/
/* RETURN: 0=error, 1=success												 */
/*****************************************************************************/
//FILE *_loclog;
bool CpcXXXX::init(void)
{
//    pCPU->logsw = true;
#ifndef QT_NO_DEBUG
//    pCPU->logsw = true;
#endif
	CPObject::init();
	
	AddLog(LOG_FUNC,"CpcXXXX::init");
	


	AddLog(LOG_MASTER,tr("LCD init"));
    if(pLCDC && !(pLCDC->init())) return(0);


    AddLog(LOG_MASTER,tr("CPU init"));
    if (!pCPU) return 0;
    if(!(pCPU->init())) return(0);
	if(pKEYB)	pKEYB->init();
	if(pTIMER)	pTIMER->init();
	if(pLCDC)	pLCDC->init_screen();
    AddLog(LOG_MASTER,"Success");

    QHash<int,QString> lbl;
    lbl[1]="MT_OUT2";
    lbl[2]="GND";
    lbl[3]="VGG";
    lbl[4]="BUSY";
    lbl[5]="D_OUT";
    lbl[6]="MT_IN";
    lbl[7]="MT_OUT1";
    lbl[8]="D_IN";
    lbl[9]="ACK";
    lbl[10]="SEL2";
    lbl[11]="SEL1";
    WatchPoint.add(&pCONNECTOR_value,64,11,this,"Standard 11pins connector",lbl);
    WatchPoint.add((qint64 *) &IO_A,8,8,this,"Port A");
    WatchPoint.add((qint64 *) &IO_B,8,8,this,"Port B");
    WatchPoint.add((qint64 *) &IO_C,8,8,this,"Port C");
    WatchPoint.add((qint64 *) &IO_F,8,8,this,"Port F");
	
	initsound();

//    _loclog=fopen("toto.log","wt");

    if (pCPU->pDEBUG) {
        dialogdasm = new DialogDasm(this);
        dialogdasm->hide();
    }

    Initial_Session_Load();
	return(1);
}

/*****************************************************************************/
/* RETURN: 0=error, 1=success												 */
/*****************************************************************************/
bool CpcXXXX::exit(void)
{
	TurnOFF();

	if (pLCDC)		pLCDC->exit();
	if (pCPU)		pCPU->exit();
	if (pTIMER)		pTIMER->exit();
	if (pCONNECTOR) pCONNECTOR->exit();
	if (pKEYB)		pKEYB->exit();

	exitsound();
	return(1);
}

INLINE bool CpcXXXX::checkTraceRange(UINT32 adr) {
    if (TraceRange.isEmpty()) return true;

    QMapIterator<QPair<UINT32,UINT32>, Qt::CheckState> i(TraceRange);
    while (i.hasNext()) {
        i.next();
//        cout << i.key() << ": " << i.value() << endl;
        if ( (adr >=i.key().first) && (adr <=i.key().second) && (i.value()==Qt::Checked)) return true;
    }

    return false;
}

/*****************************************************************************/
/* Execute XXXX emulator (while pPC->pCPU->end==0)							 */
/* RETURN: 0=error, 1=success												 */
/*****************************************************************************/
bool CpcXXXX::run(void)
{	
    CPObject::run();

    old_state = pTIMER->state;

    if (DasmFlag) return true;

    Get_Connector();
    // Read the connectors
    if (pCONNECTOR) pCONNECTOR_value = pCONNECTOR->Get_values();


    if(!(pCPU->halt|pCPU->off) && !off)
	{
        memset(Log_String,0,sizeof(Log_String));
#if 0
        if ( (pCPU->logsw) && (pCPU->fp_log) )
#else
        if (  pCPU->logsw && pCPU->fp_log && checkTraceRange(pCPU->get_PC()))
#endif
        {
            fflush(pCPU->fp_log);
            //char	s[2000];
//            sprintf(Log_String," ");
            pCPU->pDEBUG->DisAsm_1(pCPU->get_PC());
//            fprintf(pCPU->fp_log,"[%lld] ",pTIMER->state);
            fprintf(pCPU->fp_log,"[%02i]",pCPU->prevCallSubLevel);
            for (int g=0;g<pCPU->prevCallSubLevel;g++) fprintf(pCPU->fp_log,"\t");

            pCPU->step();
            Regs_Info(1);

            fprintf(pCPU->fp_log,"%-40s   %s  %s\n",pCPU->pDEBUG->Buffer,pCPU->Regs_String,Log_String);
            if (pCPU->prevCallSubLevel < pCPU->CallSubLevel) {
                for (int g=0;g<pCPU->prevCallSubLevel;g++) fprintf(pCPU->fp_log,"\t");
                fprintf(pCPU->fp_log,"{\n");
            }
            if (pCPU->prevCallSubLevel > pCPU->CallSubLevel) {
                for (int g=0;g<(pCPU->prevCallSubLevel-1);g++) fprintf(pCPU->fp_log,"\t");
                fprintf(pCPU->fp_log,"}\n");
            }
            if (pCPU->CallSubLevel <0) pCPU->CallSubLevel=0;
            pCPU->prevCallSubLevel = pCPU->CallSubLevel;

            //fprintf(pCPU->fp_log,s);
//            fflush(pCPU->fp_log);
        }
        else {
//            fprintf(_loclog,"[%lld] %05x",pTIMER->state,pCPU->get_PC());
            if (!off) {
                pCPU->step();
//                qWarning()<<Log_String;
            }
#ifndef QT_NO_DEBUG
            Regs_Info(0);
#endif
        }

//        if (BreakPoints.value(pCPU->get_PC(),Qt::Unchecked) == Qt::Checked)
        if (pBreakpointManager->isBreak(Cbreakpoint::EXEC,pCPU->get_PC(),0))
        {
                DasmStep = true;
                BreakSubLevel = 99999;
        }
        if (DasmStep)
        {
            if ( (pCPU->get_PC() != DasmLastAdr) &&
                 (BreakSubLevel >= pCPU->CallSubLevel)){
                BreakSubLevel = pCPU->CallSubLevel;
                DasmLastAdr = pCPU->get_PC();
                pCPU->pDEBUG->DisAsm_1(DasmLastAdr);
                emit showDasm();
                emit askRefreshDasm();
                DasmFlag = true;
                DasmStep = false;
            }
        }

	}
    else {
        if (!off) {
            pCPU->step();
        }
        pTIMER->state +=20;//= pTIMER->currentState();
    }

    Set_Connector();		//Write the connectors
    if (pCONNECTOR) {
        pCONNECTOR_value = pCONNECTOR->Get_values();
    }

//	if (DasmStep)		// Stop after 1 step processed (DASM)
//	{
//		pCPU->halt = 1;
//		DasmStep = 0;
//	}

	return(1);
}



bool CpcXXXX::SaveSession_File(QXmlStreamWriter *xmlOut) {
    xmlOut->writeStartElement("session");
        xmlOut->writeAttribute("version", "2.0");
        xmlOut->writeAttribute("model", SessionHeader );
        xmlOut->writeAttribute("power",Power?"true":"false");
        SaveConfig(xmlOut);
        SaveExt(xmlOut);
        if (pCPU) pCPU->save_internal(xmlOut);
        xmlOut->writeStartElement("memory");
            for (int s=0; s<SlotList.size(); s++)				// Save Memory
            {
                if (SlotList[s].getType() == CSlot::RAM)	Mem_Save(xmlOut,s);
            }
        xmlOut->writeEndElement();  // memory
    xmlOut->writeEndElement();  // session
//    SaveExtra(&xw);									// Save all other data  (virtual)
    return true;
}

bool CpcXXXX::SaveSession_File(QFile *file)
{
	QDataStream out(file);	

    out.writeRawData( (char*)(SessionHeader.toLocal8Bit().data() ),SessionHeader.length());	// Write Header
	SaveConfig(file);									// Write PC configuration
    if (pCPU) pCPU->save_internal(file);							// Save cpu status
	for (int s=0; s<SlotList.size(); s++)				// Save Memory
	{
		if (SlotList[s].getType() == CSlot::RAM)	Mem_Save(file,s);
	}
	
	SaveExtra(file);									// Save all other data  (virtual)
	return(1);
}

bool CpcXXXX::LoadSession_File(QXmlStreamReader *xmlIn) {

    if ((xmlIn->name()=="session") || (xmlIn->readNextStartElement())) {
        if ( (xmlIn->name() == "session") &&
             (xmlIn->attributes().value("model") == SessionHeader) ) {
//            Power = (xmlIn->attributes().value("power")=="true") ?true:false;
            QString version = xmlIn->attributes().value("version").toString();
            if (!LoadConfig(xmlIn)) {
                emit msgError("ERROR Loading Session Config:"+SessionHeader);
                return false;
            }
            if ( (version == "2.0") && !LoadExt(xmlIn)) {
                MSG_ERROR("ERROR Loading Session Extensions");
                return false;
            }

            if (pCPU) pCPU->Load_Internal(xmlIn);

            AddLog(LOG_MASTER,"Loadmemory:"+xmlIn->name().toString());
            if (xmlIn->readNextStartElement() && xmlIn->name() == "memory" ) {
                AddLog(LOG_MASTER,"Load Memory");
                for (int s=0; s<SlotList.size(); s++)				// Save Memory
                {
                    if (SlotList[s].getType() == CSlot::RAM) {
                        AddLog(LOG_MASTER,"    Load Slot"+SlotList[s].getLabel());
                        Mem_Load(xmlIn,s);
                    }
                }
            }
        }
        if (Power) TurnON();
    }

    updateMenuFromExtension();
    return true;
}

bool CpcXXXX::LoadSession_File(QFile *file)
{
	char t[20];

	// Read Header
    file->read( t, SessionHeader.length() );
    t[SessionHeader.length()] = '\0';
	//MSG_ERROR(QString(t))
	if(QString(t) != SessionHeader)
	{		//bad image
		MSG_ERROR(tr("Not the correct file format"));
		return false;
	}
	else
	{
		LoadConfig(file);												// Write PC configuration

		// Load cpu status	
        if (pCPU) pCPU->Load_Internal(file);
		// Save Memory
		for (int s=0; s<SlotList.size(); s++)
		{
            if (SlotList[s].getType() == CSlot::RAM)
                Mem_Load(file,s);
		}
		// Close the file
		return(1);
	}

	return false;
}

// Load PC Configuration
bool CpcXXXX::LoadConfig(QFile *file)
{
	QDataStream in(file);
    qint8 ioA,ioB,ioC,ioF,romb,ramb,protect;
		
    in >> ioA >> ioB >> ioC >> ioF >> romb >> ramb >> protect >> Japan;
	IO_A = ioA;
	IO_B = ioB;
	IO_C = ioC;
	IO_F = ioF;
	RomBank = romb;
	RamBank = ramb;
    ProtectMemory = protect;

//    if (pKEYB) {
//        in >> ks;
//        pKEYB->Set_KS(ks);
//    }
	return true;
}

bool CpcXXXX::LoadConfig(QXmlStreamReader *xmlIn)
{
    if (xmlIn->readNextStartElement()) {
        if (xmlIn->name() == "config" && xmlIn->attributes().value("version") == "1.0") {
            if (xmlIn->readNextStartElement() && xmlIn->name() == "internal" ) {
                QString s= xmlIn->attributes().value("IO_A").toString();
                IO_A = s.toInt(0,16);
                IO_B = xmlIn->attributes().value("IO_B").toString().toInt(0,16);
                IO_C = xmlIn->attributes().value("IO_C").toString().toInt(0,16);
                IO_F = xmlIn->attributes().value("IO_F").toString().toInt(0,16);
                RomBank = xmlIn->attributes().value("RomBank").toString().toInt(0,16);
                RamBank = xmlIn->attributes().value("RamBank").toString().toInt(0,16);
                ProtectMemory = xmlIn->attributes().value("ProtectMemory").toString().toInt(0,16);
                //Japan = xml->attributes().value("ProtectMemory").toString().toInt(0,16);
                closed = xmlIn->attributes().value("closed").toString().toInt(0,16);
                xmlIn->skipCurrentElement();
            }

        }
        xmlIn->skipCurrentElement();
    }

    return true;
}

// Save PC Configuration
bool CpcXXXX::SaveConfig(QFile *file) {
	QDataStream out(file);
		
	out << (qint8)IO_A << (qint8)IO_B << (qint8)IO_C << (qint8)IO_F;
    out << (qint8)RomBank << (qint8)RamBank << (qint8)ProtectMemory;
	out << Japan;
//    if (pKEYB) out << (qint8) pKEYB->KStrobe;

	return true;
}	

bool CpcXXXX::SaveConfig(QXmlStreamWriter *xmlOut)
{
    xmlOut->writeStartElement("config");
    xmlOut->writeAttribute("version", "1.0");
        xmlOut->writeStartElement("internal");
            xmlOut->writeAttribute("IO_A",QString("%1").arg(IO_A,2,16));
            xmlOut->writeAttribute("IO_B",QString("%1").arg(IO_B,2,16));
            xmlOut->writeAttribute("IO_C",QString("%1").arg(IO_C,2,16));
            xmlOut->writeAttribute("IO_F",QString("%1").arg(IO_F,2,16));
            xmlOut->writeAttribute("RomBank",QString("%1").arg(RomBank,2,16));
            xmlOut->writeAttribute("RamBank",QString("%1").arg(RamBank,2,16));
            xmlOut->writeAttribute("ProtectMemory",QString("%1").arg(ProtectMemory));
            xmlOut->writeAttribute("Japan",QString("%1").arg(Japan));
            xmlOut->writeAttribute("closed",QString("%1").arg(closed));
        xmlOut->writeEndElement();
        pBreakpointManager->serialize(xmlOut);
    xmlOut->writeEndElement();

    return true;
}

bool CpcXXXX::LoadExt(QXmlStreamReader *xmlIn)
{
    AddLog(LOG_MASTER,"LoadExt");
    if (xmlIn->readNextStartElement()) {
        AddLog(LOG_MASTER,"Loadext name1:"+xmlIn->name().toString());
        if (xmlIn->name() == "extarray" && xmlIn->attributes().value("version") == "1.0") {

            while (xmlIn->readNextStartElement()) {
                AddLog(LOG_MASTER,"Loadext name2:"+xmlIn->name().toString());
                if ( xmlIn->name() == "ext" ) {

                    int i = xmlIn->attributes().value("idarray").toString().toInt(0,10);
                    if (extensionArray[i]) {
                        QString Id = xmlIn->attributes().value("idext").toString();
                        AddLog(LOG_MASTER,"Found : "+Id);
                        for (int j = 0;j<NB_EXT;j++) {
                            if (extensionArray[i]->ExtArray[j]->Id == Id) {
                                extensionArray[i]->ExtArray[j]->IsChecked = true;
                                AddLog(LOG_MASTER,tr("Found : %1").arg(j));
                            }
                        }
                    }
                }
//                else
                    xmlIn->skipCurrentElement();
            }
            AddLog(LOG_MASTER,"Loadext end name:"+xmlIn->name().toString());
//            if (found) xmlIn->skipCurrentElement();
        }

    }

    return true;
}

bool CpcXXXX::SaveExt(QXmlStreamWriter *xmlOut)
{
    xmlOut->writeStartElement("extarray");
    xmlOut->writeAttribute("version", "1.0");
    for (int i = 0; i<4;i++) {
        if (extensionArray[i]) {
            for (int j = 0; j<NB_EXT;j++) {
                CExtension* e = extensionArray[i]->ExtArray[j];
                if (e && e->IsChecked) {
                    xmlOut->writeStartElement("ext");
                        xmlOut->writeAttribute("idarray",QString("%1").arg(i));
                        xmlOut->writeAttribute("idext",e->Id);
                        xmlOut->writeAttribute("desc",e->Description);
                    xmlOut->writeEndElement();
                }
            }
        }
    }
    xmlOut->writeEndElement();

    return true;
}

bool CpcXXXX::LoadExtra(QFile *file) 	{ Q_UNUSED(file) return true; }
bool CpcXXXX::SaveExtra(QFile *file)	{ Q_UNUSED(file) return true; }

void CpcXXXX::LoadSession(void)
{
	QString fileName = QFileDialog::getOpenFileName(
										mainwindow,
										tr("Choose a file"),
										".",
										tr("PockEmul sessions (*.pkm)"));

    if (fileName.isEmpty()) return;
                    
	QFile file(fileName);
	if (!file.open(QIODevice::ReadOnly)) {
		QMessageBox::warning(mainwindow,tr("PockEmul"),
								tr("Cannot read file %1:\n%2.")
								.arg(file.fileName())
								.arg(file.errorString()));
		return ;
	}

    QXmlStreamReader xmlIn;

    xmlIn.setDevice(&file);
    if (LoadSession_File(&xmlIn) && pLCDC) pLCDC->forceRedraw();

	file.close();							// Close the file

}

void CpcXXXX::SaveSession(void)
{
QString fileName = QFileDialog::getSaveFileName(
                    mainwindow,
                    tr("Choose a file"),
                    ".",
                    tr("PockEmul sessions (*.pkm)"));
    QFile file(fileName);

    if (file.open(QFile::WriteOnly | QFile::Truncate)) {
        QString s;
        QXmlStreamWriter *xmlOut = new QXmlStreamWriter(&s);
        xmlOut->setAutoFormatting(true);
        SaveSession_File(xmlOut);
        QTextStream out(&file);
        out << s;
    }

	file.close();							// Close the file
}



/*****************************************************************************/
/* Check Address ROM or RAM ?												 */
/*  ENTRY :DWORD d=Address													 */
/*  RETURN:bool (1=RAM,0=ROM)												 */
/*****************************************************************************/
// Virtual Fonction
bool CpcXXXX::Chk_Adr(UINT32 *d,UINT32 data) { Q_UNUSED(d) Q_UNUSED(data) return(1); }
bool CpcXXXX::Chk_Adr_R(UINT32 *d,UINT32 *data) { Q_UNUSED(d) Q_UNUSED(data) return(1);}






bool CpcXXXX::Initial_Session_Load()
{
    qWarning()<<"Initial_Session_Load start";
	QFile file(Initial_Session_Fname);

	if (file.open(QIODevice::ReadOnly))
	{

        QXmlStreamReader xmlIn;
        xmlIn.setDevice(&file);
        if (LoadSession_File(&xmlIn) && pLCDC)
            pLCDC->forceRedraw();
		file.close();	
		return true;
	}

	return false;
}

bool CpcXXXX::Initial_Session_Save()
{
	QFile file(Initial_Session_Fname);
	if (file.open(QIODevice::WriteOnly))
	{
        QString s;
        QXmlStreamWriter *xmlOut = new QXmlStreamWriter(&s);
        xmlOut->setAutoFormatting(true);
        SaveSession_File(xmlOut);
        QTextStream out(&file);
        out << s;

        //SaveSession_File(&file);
		file.close();	
		return true;
	}

	return false;
}

void CpcXXXX::Regs_Info(UINT8 Type)
{
	switch(Type)
	{
	case 0:			// Monitor Registers Dialog
			pCPU->Regs_Info(0);
			strcpy(Regs_String,pCPU->Regs_String);
			break;
    case 1:			// Monitor Registers Dialog
            pCPU->Regs_Info(1);
            strcpy(Regs_String,pCPU->Regs_String);
            break;
	}
}

void CpcXXXX::manageEmptyExtensions(void)
{
	emptyExtensionArray((QAction *)sender());
	updateMenuFromExtension();

}
void CpcXXXX::manageLoadExtensions(void)
{
    emptyExtensionArray((QAction *)sender());
    updateMenuFromExtension();

}
void CpcXXXX::manageSaveExtensions(void)
{
    emptyExtensionArray((QAction *)sender());
    updateMenuFromExtension();

}
void CpcXXXX::manageExtensions(QAction * action)
{
	updateExtension(action);
	updateMenuFromExtension();

}

/*!
 \brief

 \fn CpcXXXX::loadMem
 \param adr
 \param data
 \return bool
*/


QByteArray CpcXXXX::getmem()
{
    return (QByteArray((const char*) mem,memsize));
}


#define MAXEXT  40
bool CpcXXXX::CheckUpdateExtension(CExtension *ext)
{
    Q_UNUSED(ext)

    return true;
}

void CpcXXXX::updateExtension(CExtensionArray *array,QAction *action)
{
        for (int ind = 0 ; ind < NB_EXT; ind++){
            if (array->ExtArray[ind]->Action == action)
            {
                if (CheckUpdateExtension(array->ExtArray[ind]))
                    array->setChecked(ind,true);
            }
        }

        for (int ind = 0 ; ind < array->ExtList.size(); ind++){
            if (array->ExtList[ind].Action == action)
            {
                if (CheckUpdateExtension(&array->ExtList[ind]))
                    array->setChecked(ind,true);
            }
        }
}

void CpcXXXX::ExtChanged(void) {
    TurnOFF();
    InitDisplay();
}


void CpcXXXX::updateExtension(QAction *action)
{
    for (int ind = 0; ind < 5; ind++){
        if (extensionArray[ind]) updateExtension(extensionArray[ind],action);
    }
    ExtChanged();


}

void CpcXXXX::updateMenuFromExtension(void)
{
    QAction *action;
    bool checked;

    for (int indArray = 0; indArray < 5; indArray++){
        if (extensionArray[indArray]) {
            for (int indExt = 0; indExt < NB_EXT ; indExt++) {
                checked = extensionArray[indArray]->ExtArray[indExt]->IsChecked;
                action = extensionArray[indArray]->ExtArray[indExt]->Action;
                if (action) action->setChecked(false);
                if (action && checked &&  !action->isChecked()) action->setChecked(true);
            }
            for (int indExt = 0; indExt < extensionArray[indArray]->ExtList.size() ; indExt++) {
                checked = extensionArray[indArray]->ExtList[indExt].IsChecked;
                action = extensionArray[indArray]->ExtList[indExt].Action;
                if (action) action->setChecked(false);
                if (action && checked &&  !action->isChecked()) action->setChecked(true);
            }
        }
    }
}


CExtension * CpcXXXX::findExtension(CExtensionArray *array,QAction *action)
{
    for (int ind = 0 ; ind < NB_EXT; ind++){
        if (array->ExtArray[ind]->Action == action)
        {
            return(array->ExtArray[ind]);
        }
    }
    for (int ind = 0 ; ind < array->ExtList.size(); ind++){
        if (array->ExtList[ind].Action == action)
        {
            return &(array->ExtList[ind]);
        }
    }

    return(0);
}


void CpcXXXX::addExtMenu(CExtensionArray *ext)
{
    if (! ext) {
//        MSG_ERROR("ERREUR");
        return;
    }

    delete ext->Menu;

    ext->Menu = new QMenu(ext->Id,this);

    ext->emptyAction = ext->Menu->addAction("Empty");
    connect(ext->emptyAction, SIGNAL(triggered()), this, SLOT(manageEmptyExtensions()));

    ext->Menu->addSeparator();
    ext->actionGroup = new QActionGroup(this);
    connect(ext->actionGroup, SIGNAL(triggered(QAction *)), this, SLOT(manageExtensions(QAction *)));
    for (int ind = 0 ; ind < NB_EXT; ind++){
        if (ext->ExtArray[ind]->IsAvailable){
            ext->ExtArray[ind]->Action = ext->Menu->addAction(ext->ExtArray[ind]->Id + " ("+ext->ExtArray[ind]->Description+")");
            ext->ExtArray[ind]->Action->setToolTip(ext->ExtArray[ind]->Description);
            ext->ExtArray[ind]->Action->setIcon(QIcon(ext->ExtArray[ind]->fname));
            ext->ExtArray[ind]->Action->setCheckable(true);
            ext->ExtArray[ind]->Action->setChecked(ext->ExtArray[ind]->IsChecked);
            ext->actionGroup->addAction(ext->ExtArray[ind]->Action);
        }
    }
    // check if power saved extensions" exist
    ext->Menu->addSeparator();
    ext->loadAction = ext->Menu->addAction("Load...");
    ext->saveAction = ext->Menu->addAction("Save...");
    connect(ext->loadAction, SIGNAL(triggered()), this, SLOT(manageLoadExtensions()));
    connect(ext->saveAction, SIGNAL(triggered()), this, SLOT(manageSaveExtensions()));


}

void CpcXXXX::emptyExtensionArray(QAction *action)
{
    for (int ind = 0; ind < 5; ind++){
        if (extensionArray[ind])
        {
            if (extensionArray[ind]->emptyAction == action)
            {
                // is this array empty or not ?
//				bool isempty=true;
                for (int indExt = 0;indExt<NB_EXT; indExt++)
                {
                    if (extensionArray[ind]->ExtArray[indExt]->IsChecked)
                        {
                            extensionArray[ind]->ExtArray[indExt]->IsChecked = false;
                            TurnOFF();
                            InitDisplay();
                        }
                }
            }
        }
    }

}
void CpcXXXX::loadExtensionArray(QAction *action)
{
    Q_UNUSED(action)
}

void CpcXXXX::saveExtensionArray(QAction *action)
{
    Q_UNUSED(action)
}



