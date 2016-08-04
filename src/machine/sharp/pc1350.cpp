/*** POCKEMUL [PC1350.CPP] ********************************/
/* PC1350 emulator main class                             */
/**********************************************************/
 
//FIXME: Ce-126p not working with Shift+Enter

//#include	<string.h>
//#include	<stdlib.h>
#include	<QMessageBox>
#include	"common.h"
#include	"pc1350.h"
#include "cextension.h"
#include "Connect.h"
#include "Lcdc_pc1350.h"
#include "sc61860.h"
#include "Inter.h"
#include "Keyb.h"
#include "Keyb1350.h"
#include "Log.h"
#include "dialoganalog.h"
#include "bus.h"
#include "watchpoint.h"
#include "clink.h"

#include "fluidlauncher.h"

#include "modelids.h"
#include "sharp/ce2xxx.h"

Cpc13XX::Cpc13XX(CPObject *parent)	: CpcXXXX(parent)
{								//[constructor]
    setfrequency( (int) 768000/3);
    setcfgfname("pc1350");

    setDXmm(182);
    setDYmm(72);
    setDZmm(16);

    setDX(633);
    setDY(252);

    RightFname = P_RES(":/pc1350/pc1350Right.png");
    LeftFname  = P_RES(":/pc1350/pc1350Left.png");
    TopFname   = P_RES(":/pc1350/pc1350Top.png");
    BottomFname= P_RES(":/pc1350/pc1350Bottom.png");

    BackFname   = P_RES(":/pc1350/pc1350Back.png");

    cnt=0;

    busS1 = new Cbus("RAM Bus S1");
    backdoorS1Open = false;
    m_backdoorS1Angle = 0;
    pS1CONNECTOR = 0;
    backDoorImage=0;
}

Cpc13XX::~Cpc13XX()
{								//[constructor]
    delete busS1;
}

bool Cpc13XX::init(void)
{

#ifndef QT_NO_DEBUG
    //pCPU->logsw = true;
#endif
    initExtension();
    CpcXXXX::init();
    pCONNECTOR	= new Cconnector(this,11,0,Cconnector::Sharp_11,"Connector 11 pins",false,QPoint(0,90));	publish(pCONNECTOR);
    pSIOCONNECTOR	= new Cconnector(this,15,1,Cconnector::Sharp_15,"Connector 15 pins",false,QPoint(633,105));	publish(pSIOCONNECTOR);

	WatchPoint.add(&pSIOCONNECTOR_value,64,15,this,"Serial 15pins connector");

    pS1CONNECTOR = new Cconnector(this,35,2,Cconnector::Sharp_35,"Memory SLOT 1",true,QPoint(0,90));	publish(pS1CONNECTOR);

	return true;
}

Cpc1350::Cpc1350(CPObject *parent)	: Cpc13XX(parent)
{								//[constructor]
    setcfgfname("pc1350");

    SessionHeader	= "PC1350PKM";
    Initial_Session_Fname ="pc1350.pkm";

    BackGroundFname	= P_RES(":/pc1350/pc1350.png");
    memsize			= 0x10000;

    SlotList.clear();
    SlotList.append(CSlot(8 , 0x0000 ,	P_RES(":/pc1350/cpu-1350.rom")	, "pc-1350/cpu-1350.rom"	, CSlot::ROM , "CPU ROM"));
    SlotList.append(CSlot(24, 0x2000 ,	""								, "pc-1350/R1-1350.ram"		, CSlot::RAM , "RAM"));
    SlotList.append(CSlot(32, 0x8000 ,	P_RES(":/pc1350/bas-1350.rom")	, "pc-1350/bas-1350.rom"	, CSlot::ROM , "BASIC ROM"));

    KeyMap		= KeyMap1350;
    KeyMapLenght= KeyMap1350Lenght;

    pLCDC		= new Clcdc_pc1350(this,
                                   QRect(75,48,300,64),
                                   QRect(50,48,30,64));
    pKEYB		= new Ckeyb(this,"pc1350.map",scandef_pc1350);
    pCPU		= new CSC61860(this);
    pTIMER		= new Ctimer(this);

}

void	Cpc1350::initExtension(void)
{
//	// initialise ext_MemSlot1
//	ext_MemSlot1 = new CExtensionArray("Memory Slot 1","Add memory credit card");
//    ext_MemSlot1->setAvailable(ID_CE201M,true);		ext_MemSlot1->setChecked(ID_CE202M,true);
//	ext_MemSlot1->setAvailable(ID_CE202M,true);
//	ext_MemSlot1->setAvailable(ID_CE203M,true);
	
//	addExtMenu(ext_MemSlot1);

//    extensionArray[0] = ext_11pins;
//    extensionArray[1] = ext_MemSlot1;
//    extensionArray[2] = ext_Serial;
}

bool Cpc13XX::CheckUpdateExtension(CExtension *ext)
{
		if (ext)
			if (ext->Id == "ce-125")
			{
				switch(QMessageBox::warning(mainwindow, "PockEmul",
	                                    "This model does not fit correctly into the CE-125.\n"
	                                    "You risk connector damage !!!\n "
	                                    "Are you sure ????\n",
	                                    "Yes",
	                                    "Abort", 0, 0, 1)) { 
	        	case 0: // The user clicked the Continue again button or pressed Enter
	            // try again
	            	return true;
	        	    break;
	        	case 1: // The user clicked the Cancel or pressed Escape
	        	    // exit
	        	    return false;
	        	    break;
	        	}
	       	}
        return true;
}

int Cpc13XX::mapKey(QKeyEvent *event)
{
    int key = CpcXXXX::mapKey(event);

#if 0
    switch (key) {
    case K_UA: return '8';
    case K_DA: return '2';
    case K_LA: return '4';
    case K_RA: return '6';
    case ' ': return K_RET;
    }
#endif

    return key;
}




BYTE	Cpc13XX::Get_PortA(void)
{
    BYTE data = pKEYB->Read(IO_A);

    return (data);
}

BYTE	Cpc1350::Get_PortA(void)
{
    BYTE data = Cpc13XX::Get_PortA();

    if (! Japan) PUT_BIT(data,7,pKEYB->Get_KS() & 0x40);

    return (data);
}

BYTE	Cpc1350::Get_PortB(void)
{
	return (IO_B);
}


void	Cpc13XX::Set_PortA(BYTE data)
{

	if ((IO_A != 0) && (data == 0))
	{
		++cnt;
        if (cnt > 0)
		{
			pKEYB->keyscan();
			cnt = 0;
		}
	}
	IO_A = data;
}

void	Cpc1350::Set_PortB(BYTE data)
{
	IO_B = data;
}

void	Cpc1350::Set_PortF(BYTE data)
{
	IO_F = data;
}

// PIN_MT_OUT2	1
// PIN_GND		2
// PIN_VGG		3
// PIN_BUSY		4
// PIN_D_IN		5
// PIN_MT_IN	6
// PIN_MT_OUT1	7
// PIN_D_OUT	8
// PIN_ACK		9
// PIN_SEL2		10
// PIN_SEL1		11

#define SIO_GNDP	1
#define SIO_SD 		2
#define SIO_RD		3
#define SIO_RS		4
#define SIO_CS		5

#define SIO_GND		7
#define SIO_CD		8
#define SIO_VC1		10
#define SIO_RR		11
#define SIO_PAK		12
#define SIO_VC2		13
#define SIO_ER		14
#define SIO_PRQ		15

bool Cpc1350::Set_Connector(Cbus *_bus)
{
    Q_UNUSED(_bus)

    if (_bus == busS1) {
        pS1CONNECTOR->Set_values(busS1->toUInt64());
        return true;
    }

#if 1
	// MANAGE STANDARD CONNECTOR
	pCONNECTOR->Set_pin(PIN_MT_OUT2	,0);
	pCONNECTOR->Set_pin(PIN_GND		,0);
	pCONNECTOR->Set_pin(PIN_VGG		,1);
	pCONNECTOR->Set_pin(PIN_BUSY	,GET_PORT_BIT(PORT_F,3));		// F03
	pCONNECTOR->Set_pin(PIN_D_OUT	,GET_PORT_BIT(PORT_F,2));		// F02
	pCONNECTOR->Set_pin(PIN_MT_OUT1	,pCPU->Get_Xout());
	pCONNECTOR->Set_pin(PIN_SEL2	,0);
	pCONNECTOR->Set_pin(PIN_SEL1	,0);
#endif

	// MANAGE SERIAL CONNECTOR
	// TO DO 
	pSIOCONNECTOR->Set_pin(SIO_SD	,GET_PORT_BIT(PORT_F,4));	//	pSIO->Set_SD((IO_F & 0x08) ? 1 : 0);
	pSIOCONNECTOR->Set_pin(SIO_RR	,GET_PORT_BIT(PORT_B,3));	//( READ_BIT(IO_B,2) );
	pSIOCONNECTOR->Set_pin(SIO_RS	,GET_PORT_BIT(PORT_B,2));	//( READ_BIT(IO_B,1) );
	pSIOCONNECTOR->Set_pin(SIO_ER	,GET_PORT_BIT(PORT_B,1));	//( READ_BIT(IO_B,0) );
	
	return(1);
}

bool Cpc1350::Get_Connector(Cbus *_bus)
{
    Q_UNUSED(_bus)

    if (_bus == busS1) {
        busS1->fromUInt64(pS1CONNECTOR->Get_values());
        busS1->setEnable(false);
        return true;
    }

	// MANAGE STANDARD CONNECTOR
	Set_Port_Bit(PORT_B,8,pCONNECTOR->Get_pin(PIN_D_IN));	// DIN	:	IB8
	Set_Port_Bit(PORT_B,7,pCONNECTOR->Get_pin(PIN_ACK));	// ACK	:	IB7
	pCPU->Set_Xin(pCONNECTOR->Get_pin(PIN_MT_IN));

	// MANAGE SERIAL CONNECTOR
	// TO DO 
	Set_Port_Bit(PORT_B,4,pSIOCONNECTOR->Get_pin(SIO_RD));
	Set_Port_Bit(PORT_B,5,pSIOCONNECTOR->Get_pin(SIO_CS));
	Set_Port_Bit(PORT_B,6,pSIOCONNECTOR->Get_pin(SIO_CD));
	// PAK
	return(1);
}




/*****************************************************************************/
/* RETURN: 0=error, 1=success												 */
/*****************************************************************************/

bool Cpc1350::run(void) 
{ 
	pSIOCONNECTOR_value = pSIOCONNECTOR->Get_values();

    CpcXXXX::run();

#if 1
	// HACK Program Counter
#define FUNC_CALL(ADDR,LIB) case ADDR: AddLog(LOG_ROM,tr(LIB).arg(pCPU->get_PC(),5,16,QChar('0')));

	switch (pCPU->get_PC())
	{
    FUNC_CALL(0x8758,QT_TR_NOOP("Console Call [%1] - SIO Input 1 byte"));
//            pCPU->logsw = true;
//            pCPU->Check_Log();
            break;
	//FUNC_CALL(0xF343,QT_TR_NOOP("Function Call [%1] - LOAD"));						break;
	//FUNC_CALL(0xF267,QT_TR_NOOP("Function Call [%1] - SAVE"));						break;

//	FUNC_CALL(0x9D1D,QT_TR_NOOP("Function Call [%1] - Header Input"));				;
//											pCPU->logsw=1;
//											pCPU->Check_Log();
//											break;
	}
#endif
	return(1); 
}

/*****************************************************************************/
/* Check Address ROM or RAM ?												 */
/*  ENTRY :DWORD d=Address													 */
/*  RETURN:bool (1=RAM,0=ROM)												 */
/*****************************************************************************/
// Virtual Fonction

bool Cpc1350::Chk_Adr(UINT32 *d,UINT32 data)
{
if ( (*d>=0x6f00) && (*d<=0x6fff) )	{
    if (pCPU->fp_log) fprintf(pCPU->fp_log,"ECRITURE [%04x]=%02x (%c)\n",*d,data,data);
}

//	if ( (*d>=0x7000) && (*d<=0x79FF) )	{pLCDC->SetDirtyBuf(*d-0x7000);return(1);}
    if ( ( (*d>=0x7000) && (*d<=0x707C) ) ||
         ( (*d>=0x7200) && (*d<=0x727C) ) ||
         ( (*d>=0x7400) && (*d<=0x747C) ) ||
         ( (*d>=0x7600) && (*d<=0x767C) ) ||
         ( (*d>=0x7800) && (*d<=0x787C) ))	{
        if (mem[*d] != data) {
            pLCDC->updated = true;
            pLCDC->SetDirtyBuf(*d-0x7000);
        }
        return(1);
    }
	if ( (*d>=0x7E00) && (*d<=0x7FFF) )	
	{
		pKEYB->Set_KS( (BYTE) data & 0x7F );
		return(1);
	}



//	if ( (*d>=0x2000) && (*d<=0x3FFF) && EXTENSION_CE201M_CHECK ) { *d+=0x2000;	return(1);}										// 8Kb Ram Card Image
//	if ( (*d>=0x2000) && (*d<=0x3FFF) )	{ return( EXTENSION_CE202M_CHECK | EXTENSION_CE203M_CHECK); }							// 16K
//	if ( (*d>=0x4000) && (*d<=0x5FFF) )	{ return( EXTENSION_CE201M_CHECK | EXTENSION_CE202M_CHECK | EXTENSION_CE203M_CHECK); }	// 32K
	if ( (*d>=0x6000) && (*d<=0x7FFF) ) return(1);																				// Internal RAM area(6000-8000)

    if ( (*d>=0x2000) && (*d<=0x5FFF)) {
        UINT32 _addr = *d - 0x2000;
        writeBus(busS1 ,&_addr,data);
    }

	return(0);

}

bool Cpc1350::Chk_Adr_R(UINT32 *d,UINT32 *data) {
    Q_UNUSED(data)

    if ( (*d>=0x6f00) && (*d<=0x6fff) )	{
        if (pCPU->fp_log) fprintf(pCPU->fp_log,"LECTURE [%04x]=%02x (%c)\n",*d,mem[*d],mem[*d]);
    }

// Manage ram bank
    if ( (*d>=0x2000) && (*d<=0x5FFF) )	{

        UINT32 _addr = *d - 0x2000;

        readBus(busS1 ,&_addr,data);
        return false;
    }

    return(1);
}

void Cpc13XX::PostFlip()
{
    manageCardVisibility();
}

void Cpc13XX::manageCardVisibility() {
    if ((currentView == BACKview) || (currentView == BACKviewREV)) {
        // show memory cards
        CPObject * S1PC = pS1CONNECTOR->LinkedToObject();
        if (S1PC){
            if (backdoorS1Open) {
                S1PC->showObject();
            }
            else {
                S1PC->hideObject();
            }
        }
    }
}

void Cpc13XX::animateBackDoorS1(bool _open) {
    qWarning()<<"ANIMATE S1";
    if (backdoorS1Open == _open) return;

    backdoorS1Open = _open;


    QPropertyAnimation *animation1 = new QPropertyAnimation(this, "backdoorS1angle");
     animation1->setDuration(1500);
     if (backdoorS1Open) {
         animation1->setStartValue(m_backdoorS1Angle);
         animation1->setEndValue(80);
     }
     else {
         manageCardVisibility();
         animation1->setStartValue(m_backdoorS1Angle);
         animation1->setEndValue(0);
     }

     QParallelAnimationGroup *group = new QParallelAnimationGroup;
     group->addAnimation(animation1);

     connect(animation1,SIGNAL(valueChanged(QVariant)),this,SLOT(RefreshDisplay()));
     connect(animation1,SIGNAL(finished()),this,SLOT(endbackdoorAnimation()));
     backdoorFlipping = true;
     group->start();

}

void Cpc13XX::endbackdoorAnimation()
{
    backdoorFlipping = false;
    manageCardVisibility();
}

void Cpc13XX::linkObject(QString item,CPObject *pPC)
{
    qWarning()<<"currentslot:"<<currentSlot;
    Cconnector *_conn = 0;
    QRect _rect;
    if (currentSlot==1) {
        _conn = pS1CONNECTOR;
        _rect = pKEYB->getKey(0x241).Rect;
    }

    mainwindow->pdirectLink->addLink(_conn,pPC->ConnList.at(0),true);
    pPC->setPosX(posx()+_rect.left()*mainwindow->zoom);
    pPC->setPosY(posy()+_rect.top()*mainwindow->zoom);
    pPC->raise();
    if (currentSlot==1) {
        pPC->setRotation(180);
    }
    emit stackPosChanged();
}

void Cpc13XX::setbackdoorS1Angle(int value) {
    this->m_backdoorS1Angle = value;
}



extern int ask(QWidget *parent,QString msg,int nbButton);

void Cpc13XX::ComputeKey(KEYEVENT ke, int scancode, QMouseEvent *event)
{
    Q_UNUSED(ke)
    Q_UNUSED(scancode)

    // Manage left connector click
    if ((currentView==LEFTview) && KEY(0x240) ) {
        FluidLauncher *launcher = new FluidLauncher(mainwindow,
                                     QStringList()<<P_RES(":/pockemul/configExt.xml"),
                                     FluidLauncher::PictureFlowType,QString(),
                                     "Sharp_11");
        launcher->show();
    }

    if ((currentView==BACKview) || (currentView==BACKviewREV)) {
        if ( KEY(0x241) && backdoorS1Open) {
            if (!pS1CONNECTOR->isLinked()) {
                pKEYB->keyPressedList.removeAll(0x241);
                FluidLauncher *launcher = new FluidLauncher(mainwindow,
                                                            QStringList()<<P_RES(":/pockemul/configExt.xml"),
                                                            FluidLauncher::PictureFlowType,QString(),
                                                            "Sharp_35w");
                connect(launcher,SIGNAL(Launched(QString,CPObject *)),this,SLOT(linkObject(QString,CPObject *)));
                currentSlot = 1;
                launcher->show();
            }
            else {
                // unlink card
            }
        }
        if (KEY(0x243)) {
            // open S1
            pKEYB->keyPressedList.removeAll(0x243);

            animateBackDoorS1(true);

        }
        if (KEY(0x244)) {
            // close S1
            pKEYB->keyPressedList.removeAll(0x244);

            animateBackDoorS1(false);

        }

    }
}

bool Cpc13XX::UpdateFinalImage(void) {
//    qWarning()<<"UpdateFinalImage";
    // Draw backdoor when not in frontview
    if (!flipping && (currentView != FRONTview) ) {
//        InitView(BACKview);
        delete BackImage;
        BackImage = new QImage(BackImageBackup);
    }

    CpcXXXX::UpdateFinalImage();

    if ((currentView != FRONTview) ) {
        QPainter painter;
        painter.begin(BackImage);
        painter.translate(19*internalImageRatio,30*internalImageRatio+backDoorImage->height()/2);
        QTransform matrix2;
        matrix2.rotate(m_backdoorS1Angle, Qt::YAxis);
        float sx = 0.55;
        if (m_backdoorS1Angle<45)
            sx = 1 - (float)m_backdoorS1Angle/100;
        else
            sx = .55 - (float)(m_backdoorS1Angle-45)/200;

        matrix2.scale(sx,1);
        painter.setTransform(matrix2,true);
        painter.drawImage(0,-backDoorImage->height()/2,*backDoorImage);
        painter.end();
    }

    return true;
}

bool Cpc13XX::InitDisplay()
{
    CpcXXXX::InitDisplay();
    backDoorImage = new QImage(QImage(QString(P_RES(":/pc1350/backdoor.png"))).scaled(354*internalImageRatio,192*internalImageRatio));

    BackImageBackup = BackImage->copy();
    return true;
}

void Cpc1350::TurnON()
{
    // Check if there is a memory card connected to the PS1Connnector before turning ON
    CPObject * S1PC = pS1CONNECTOR->LinkedToObject();
    if (!S1PC){
        // If not, ask for connecting the default 16Ko card.
//        int result = ask(this,"No memory card in SLOT 1. Do you want to add a 16Ko RAM Card ?",2);
//        if (result == 1)
        {
            // Create ram card PObject
            CPObject *cardPC = mainwindow->LoadPocket(CE202M);
            // link the memory card to PS1Connector
            // and move memory card to correct coordinates
            currentSlot = 1;
            linkObject("",cardPC);

            // Hide memory card
            cardPC->hideObject();
            memcpy((char*) cardPC->mem,(char*) &mem[0x2000],0x4000);
        }
    }

    Cpc13XX::TurnON();
}

bool Cpc1350::LoadSession_File(QXmlStreamReader *xmlIn) {

    CpcXXXX::LoadSession_File(xmlIn);



    return true;
}
