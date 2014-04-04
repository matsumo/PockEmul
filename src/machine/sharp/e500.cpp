#include <QDebug>

#include "e500.h"
#include "sc62015.h"
#include "hd61102.h"
//#include "rp5c01.h"
#include "Connect.h"
#include "Keyb.h"
#include "Lcdc_e500.h"
#include "Inter.h"
#include "Log.h"
#include "init.h"
#include "cextension.h"

//TODO: Connector output for ce-126p and ce-140f

//TODO: UART Emulation
//TODO: Memory cards management

#define TEST_MEMORY_MAPPING 0
/*
00000   -----------------------
        |                     | you can peek ( 00000 - 000FF ) by PEEK ,
        |   None.             | but it is not memory. It is cpu's internal ram.
        |                     |
        |                     |
40000H  +---------------------+---------+---------+8KB CARD+
        |                     |32KB CARD|16KB CARD+--------+
        |   64KB CARD         |         +---------+
        |                     +---------+
        |                     |
        |                     |
50000H  +---------------------+
        |                     | PC-E500 has 32kb ( B8000 - BFFFF )
        |   BUILDIN RAM       | PC-E550 has 64kb ( B0000 - BFFFF )
        |           SLOT 1    | ram slot "S1:" and ram drive "E:"
        |                     |
C0000   -----------------------
        |                     |
        |   SYSTEM ROM        | rom slot "S3:" ( C0000 - DFFFF ) and
        |                     | rom drive "G:"
        |                     |
FFFFF   -----------------------

<CE0> 80000h ~ BFFFFh logique negative RAM integree (S1 :)
<CE1> 40000h ~ 7FFFFh logique positive elargie carte RAM (S2 :)
<CE2> C0000H ~ FFFFFh logique negative integre dans la ROM (S3 :)
<CE3> 20000h ~ 3FFFFh logique negative (comme DELTA)
<CE4>??
<CE5>??
<CE6> 10000h ~ 1FFFFh negative carte graphique logique
<CE7> BC000h reservation du reseau de logique positive BFFFFh ~
*/

Ce500::Ce500(CPObject *parent, Models mod)	: CpcXXXX(parent)
{								//[constructor]
    setfrequency( (int) 3072000/3);
    memsize		= 0x100000;
    InitMemValue	= 0xff;
    /* ROM area(c0000-fffff) S3: */
    SlotList.clear();
    SlotList.append(CSlot(256, 0x40000 , ""             , ""            , CSlot::RAM , "RAM S1"));
    SlotList.append(CSlot(256, 0x80000 , ""             , ""            , CSlot::RAM , "RAM S2"));

    switch (mod) {
    case E500:
        setcfgfname(QString("e500"));
        SessionHeader	= "E500PKM";
        Initial_Session_Fname ="e500.pkm";

        BackGroundFname	= P_RES(":/e500/pc-e500.png");
        LcdFname		= P_RES(":/e500/e500lcd.png");
        SymbFname		= P_RES(":/e500/e500symb.png");
        SlotList.append(CSlot(256, 0xC0000 , P_RES(":/e500/s3.rom"), "e500/s3.rom" , CSlot::ROM , "ROM 7.3"));
        break;
    case E500S:
        setcfgfname(QString("e500s"));
        SessionHeader	= "E500SPKM";
        Initial_Session_Fname ="e500s.pkm";

        BackGroundFname	= P_RES(":/e500/pc-e500s.png");
        LcdFname		= P_RES(":/e500/e500lcd.png");
        SymbFname		= P_RES(":/e500/e500symb.png");
        SlotList.append(CSlot(256, 0xC0000 , P_RES(":/e500/s3-8.3-E500S.rom"), "e500/s3-8.3-E500S.rom" , CSlot::ROM , "ROM 8.3"));
        break;
    default: break;
    }





    PowerSwitch	= 0;
    Pc_Offset_X = Pc_Offset_Y = 0;

    setDXmm(200);
    setDYmm(100);
    setDZmm(14);

    setDX(715);
    setDY(357);

    Lcd_X		= 69;
    Lcd_Y		= 99;
    Lcd_DX		= 240;//168;//144 ;
    Lcd_DY		= 32;
    Lcd_ratio_X	= 348.0/240;
    Lcd_ratio_Y	= 60.0/32;

    Lcd_Symb_X	= 69;//(int) (45 * 1.18);
    Lcd_Symb_Y	= 79;//(int) (35 * 1.18);
    Lcd_Symb_DX	= 348;
    Lcd_Symb_DY	= 20;
    Lcd_Symb_ratio_X	= 1;//1.18;

    pLCDC		= new Clcdc_e500(this);
    pCPU		= new Csc62015(this);
    pTIMER		= new Ctimer(this);
    pKEYB		= new Ckeyb(this,"e500.map");

    pHD61102_1  = new CHD61102(this);
    pHD61102_2  = new CHD61102(this);

//    pRP5C01     = new CRP5C01(this);
    start2khz = 0;
    start4khz = 0;
    Xin=Xout=false;
    tmp_state=0;
    ioFreq=0;

}

void	Ce500::initExtension(void)
{
    AddLog(LOG_MASTER,"INIT EXT PC-E500");
    // initialise ext_MemSlot1
    ext_MemSlot1 = new CExtensionArray("RAM Slot","Add RAM Module");
    ext_MemSlot1->setAvailable(ID_CE210M,true);
    ext_MemSlot1->setAvailable(ID_CE211M,true);
    ext_MemSlot1->setAvailable(ID_CE212M,true);
    ext_MemSlot1->setAvailable(ID_CE2H16M,true);
    ext_MemSlot1->setAvailable(ID_CE2H32M,true);
    ext_MemSlot1->setAvailable(ID_CE2H64M,true);

//    ext_MemSlot1->setChecked(ID_CE2H64M,true);
    addExtMenu(ext_MemSlot1);

    extensionArray[0] = ext_MemSlot1;

}

bool Ce500::init(void) {
//    pCPU->logsw = true;
#ifndef QT_NO_DEBUG
    pCPU->logsw = true;
#endif
    initExtension();

    CpcXXXX::init();

//    pRP5C01->init();
    pCONNECTOR	= new Cconnector(this,11,0,Cconnector::Sharp_11,"Connector 11 pins",false,QPoint(1,87));		publish(pCONNECTOR);

    WatchPoint.remove(this);
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

//	if(emsmode!=0) EMS_Load();

    return true;
}

#define GET_IMEM_BIT(adr,Bit) ((pCPU->imem[adr] & (1<<((Bit)-1))) ? 1:0)

bool Ce500::run(void) {

#if 0
    //HACK : puul down D_OUT after 5ms

    if (pCPU->imem[IMEM_EOL]&0x10) {
        if (tmp_state==0) tmp_state = pTIMER->state;
        if (pTIMER->msElapsed(tmp_state)>5) {
            AddLog(LOG_PRINTER,"init");
            pCPU->setImemBit(IMEM_EOL,5,0);
            tmp_state=0;
        }
    }
#endif

    CpcXXXX::run();



    getKey();
    Csc62015 * sc = (Csc62015*)pCPU;

    if(sc->get_imem(IMEM_ISR)) sc->halt=false;

    if((sc->get_imem(IMEM_IMR)&0x80)&&(sc->get_imem(IMEM_IMR) & sc->get_imem(IMEM_ISR))) {
        sc->set_reg(REG_S,sc->get_reg(REG_S)-SIZE_20-2);								//go interrupt routine
        sc->set_mem(sc->get_reg(REG_S)+2,SIZE_20,sc->get_reg(REG_P));
        sc->set_mem(sc->get_reg(REG_S)+1,SIZE_8,sc->get_reg(REG_F));
        sc->set_mem(sc->get_reg(REG_S),SIZE_8,sc->get_imem(IMEM_IMR));
        sc->opr_imem(IMEM_IMR,OPR_AND,0x7f);
        sc->set_reg(REG_P,sc->get_mem(VECT_IR,SIZE_20));
    }

    // SOUND
    //
    computeSound();

//    pRP5C01->step();

    return true;
}

INLINE void Ce500::computeSound(void)
{
    Csc62015 * sc = (Csc62015*)pCPU;

//    qint64 delta;
    qint64 wait2khz = getfrequency()/1000/4;
    qint64 wait4khz = getfrequency()/1000/8;

    switch ( (sc->get_imem(IMEM_SCR)>>4) & 0x07)
    {
        case 0x00 : Xout = false;
                    start2khz = 0;
                    start4khz = 0;
                    //if (fp_log) fprintf(fp_log,"XOUT LOW\n");
                    break;

        case 0x01 : Xout = true;
                    start2khz = 0;
                    start4khz = 0;
                    //if (fp_log) fprintf(fp_log,"XOUT HIGH\n");
                    break;

        case 0x02 : // 2khz
                    start4khz = 0;
                    //if (fp_log) fprintf(fp_log,"XOUT 2Khz\n");
                    if (start2khz == 0){
                        start2khz = pTIMER->state;
                        if (fp_log) fprintf(fp_log,"XOUT 2Khz INIT\n");
                        Xout = true;
                    }
//                    delta = pTIMER->state - start2khz;
                    //while
                    if ((pTIMER->state - start2khz) >= wait2khz){
                        Xout = !Xout;
                        start2khz += wait2khz;
                        if (fp_log) fprintf(fp_log,"XOUT 2Khz switch\n");
                    }
                    break;

        case 0x03 : // 4khz
                    start2khz = 0;
                    //if (fp_log) fprintf(fp_log,"XOUT 4Khz\n");
                    if (start4khz==0)
                    {
                        start4khz = pTIMER->state;
                        if (fp_log) fprintf(fp_log,"XOUT 4Khz INIT\n");
                        Xout = true;
                    }
//                    delta = pTIMER->state - start4khz;
                    //while
                    if (( pTIMER->state - start4khz) >= wait4khz)
                    {
                        Xout = !Xout;
                        start4khz += wait4khz;
//                        if (fp_tmp) fprintf(fp_tmp,"%s\n",tr("switch XOUT to %1 : wait = %2  -  delta=%3  new:%4 - old:%5 ").arg(Xout).arg(wait4khz).arg(pPC->pTIMER->state - start4khz).arg(pPC->pTIMER->state).arg(start4khz).toLocal8Bit().data());


                        if (fp_log) fprintf(fp_log,"XOUT 4Khz switch\n");
                    }
                    break;

        case 0x04 : Xout = false;
                    start2khz = 0;
                    start4khz = 0;
                    //if (fp_log) fprintf(fp_log,"XOUT LOW");
                    break;

        case 0x05 : Xout = true;
                    start2khz = 0;
                    start4khz = 0;
                    //if (fp_log) fprintf(fp_log,"XOUT HIGH\n");
                    break;

        case 0x06 :
        case 0x07 : // Xin -> Xout
                    Xout = Xin;
                    start2khz = 0;
                    start4khz = 0;
                    break;
    }

    fillSoundBuffer((Xout?0xff:0x00));
}

//********************************************************/
//* Check for E-PORT and Get data						 */
//********************************************************/
// PIN_MT_OUT2	1
// PIN_GND		2
// PIN_VGG		3
// PIN_BUSY		4
// PIN_D_OUT	5
// PIN_MT_IN	6           F4  0x10
// PIN_MT_OUT1	7
// PIN_D_IN		8
// PIN_ACK		9
// PIN_SEL2		10
// PIN_SEL1		11
//********************************************************/



bool Ce500::Set_Connector(void)
{
    int port1 = pCPU->imem[IMEM_EOL];
    int port2 = pCPU->imem[IMEM_EOH];

    pCONNECTOR->Set_pin(PIN_MT_OUT2	,0);
    pCONNECTOR->Set_pin(PIN_VGG		,1);
    pCONNECTOR->Set_pin(PIN_BUSY	,READ_BIT(port1,6));

    if (!pCONNECTOR->Get_pin(PIN_ACK))
    {
        pCONNECTOR->Set_pin(PIN_D_OUT   ,READ_BIT(port2,2));// | READ_BIT(port1,4));
        pCONNECTOR->Set_pin(PIN_D_IN	,READ_BIT(port2,3));
        pCONNECTOR->Set_pin(PIN_SEL2	,READ_BIT(port2,1));
        pCONNECTOR->Set_pin(PIN_SEL1	,READ_BIT(port2,0));
    }
    pCONNECTOR->Set_pin(PIN_MT_OUT1	,pCPU->Get_Xout());

    return(1);
}

bool Ce500::Get_Connector(void)
{
    pCPU->setImemBit(IMEM_EIL,8,pCONNECTOR->Get_pin(PIN_ACK));

    if (pCONNECTOR->Get_pin(PIN_ACK))
    {
        pCPU->setImemBit(IMEM_EIH,5,pCONNECTOR->Get_pin(PIN_SEL1));
        pCPU->setImemBit(IMEM_EIH,6,pCONNECTOR->Get_pin(PIN_SEL2));
        pCPU->setImemBit(IMEM_EIH,7,pCONNECTOR->Get_pin(PIN_D_OUT));
        pCPU->setImemBit(IMEM_EIH,8,pCONNECTOR->Get_pin(PIN_D_IN));
    }

    pCPU->Set_Xin(pCONNECTOR->Get_pin(PIN_MT_IN));

    return(1);
}

void Ce500::disp(qint8 cmd,UINT32 data)
{
    switch(cmd){
    case   6:							/* LCDC 2 write data */
        pHD61102_2->instruction(0x100 | data);
        pLCDC->Refresh = true;
        break;
    case 0xa:							/* LCDC 1 write data */
        pHD61102_1->instruction(0x100 | data);
        pLCDC->Refresh = true;
        break;
    case   4:							/* LCDC 2 inst */
        pHD61102_2->instruction(data);
        pLCDC->Refresh = true;
        break;
    case   8:							/* LCDC 1 inst */
        pHD61102_1->instruction(data);
        pLCDC->Refresh = true;
        break;
    case   0:							/* LCDC 1&2 inst */
        pHD61102_1->instruction(data);
        pHD61102_2->instruction(data);
        pLCDC->Refresh = true;
        break;
    case   2:							/* LCDC 1&2 write data */
        pHD61102_1->instruction(0x100|data);
        pHD61102_2->instruction(0x100|data);
        pLCDC->Refresh = true;
        break;
    case   7:							/* LCDC 2 read data */
        mem[0x2007] = pHD61102_2->instruction(0x300);
        break;
    case 0xb:							/* LCDC 1 read data */
        mem[0x200b] = pHD61102_1->instruction(0x300);
        break;
    default:
        qWarning()<<"unhandled lcd command:"<< cmd<<data;
        break;
    }
}

/*---------------------------------------------------------------------------*/
/*****************************************************************************/
/* Check Address ROM or RAM ?												 */
/*  ENTRY :UINT32 d=Address													 */
/*  RETURN:bool (1=RAM,0=ROM)												 */
/*****************************************************************************/

void Ce500::MemMirror(UINT32 *d) {

    if ((ext_MemSlot1->ExtArray[ID_CE210M]->IsChecked ||
         ext_MemSlot1->ExtArray[ID_CE211M]->IsChecked ||
         ext_MemSlot1->ExtArray[ID_CE212M]->IsChecked ||
         ext_MemSlot1->ExtArray[ID_CE2H16M]->IsChecked ||
         ext_MemSlot1->ExtArray[ID_CE2H32M]->IsChecked ||
         ext_MemSlot1->ExtArray[ID_CE2H64M]->IsChecked) &&
            (*d>=0x40000) && (*d<=0xB7FFF))
    {
        if (ext_MemSlot1->ExtArray[ID_CE2H64M]->IsChecked) {
            *d = (*d & 0xffff) | 0x40000;
        }
        else
        if (ext_MemSlot1->ExtArray[ID_CE2H32M]->IsChecked) {
            *d = (*d & 0x7fff) | 0x40000;
        }
        else
        if (ext_MemSlot1->ExtArray[ID_CE2H16M]->IsChecked) {
            *d = (*d & 0x3fff) | 0x40000;
        }
        else
        if (ext_MemSlot1->ExtArray[ID_CE212M]->IsChecked) {
            *d = (*d & 0x1fff) | 0x40000;
        }
        else
        if (ext_MemSlot1->ExtArray[ID_CE211M]->IsChecked) {
            *d = (*d & 0xfff) | 0x40000;
        }
        else
        if (ext_MemSlot1->ExtArray[ID_CE210M]->IsChecked) {
            *d = (*d & 0x7ff) | 0x40000;
        }
    }
    // 32Ko internal
    else if ( (*d>=0x80000) && (*d<=0xB7FFF)) {
//        quint32 tmp = *d;
        *d = (*d & 0x7fff) | 0xB8000;
//        AddLog(LOG_MASTER,QString("adr;%1 -> %2").arg(tmp,6,16,QChar('0')).arg(*d,6,16,QChar('0')));
    }
}

void Ce550::MemMirror(UINT32 *d)
{
    if ((ext_MemSlot1->ExtArray[ID_CE210M]->IsChecked ||
         ext_MemSlot1->ExtArray[ID_CE211M]->IsChecked ||
         ext_MemSlot1->ExtArray[ID_CE212M]->IsChecked ||
         ext_MemSlot1->ExtArray[ID_CE2H16M]->IsChecked ||
         ext_MemSlot1->ExtArray[ID_CE2H32M]->IsChecked ||
         ext_MemSlot1->ExtArray[ID_CE2H64M]->IsChecked) &&
            (*d>=0x40000) && (*d<=0xB7FFF))
    {
        if (ext_MemSlot1->ExtArray[ID_CE2H64M]->IsChecked) {
            *d = (*d & 0xffff) | 0x40000;
        }
        else
        if (ext_MemSlot1->ExtArray[ID_CE2H32M]->IsChecked) {
            *d = (*d & 0x7fff) | 0x40000;
        }
        else
        if (ext_MemSlot1->ExtArray[ID_CE2H16M]->IsChecked) {
            *d = (*d & 0x3fff) | 0x40000;
        }
        else
        if (ext_MemSlot1->ExtArray[ID_CE212M]->IsChecked) {
            *d = (*d & 0x1fff) | 0x40000;
        }
        else
        if (ext_MemSlot1->ExtArray[ID_CE211M]->IsChecked) {
            *d = (*d & 0xfff) | 0x40000;
        }
        else
        if (ext_MemSlot1->ExtArray[ID_CE210M]->IsChecked) {
            *d = (*d & 0x7ff) | 0x40000;
        }
    }
    else
    // 64Ko internal
    if ( (*d>=0x80000) && (*d<=0xAFFFF)) {
        *d = (*d & 0xffff) | 0xB0000;
    }
}

bool Ce500::Chk_Adr(UINT32 *d,UINT32 data)
{
#if (TEST_MEMORY_MAPPING)
    quint32 tmp = *d;
    MemMirror(d);

    if ( (tmp>=0x40000) && (tmp<=0xBFFFF) && (pCPU->fp_log)) fprintf(pCPU->fp_log,"\nRAM WRITE: [%06X] -> [%06X]=%02X\n",tmp,*d,data);
#endif


    if ( (*d>=0x00000) && (*d<=0x3FFFF)) {

//        if((*d&0x3000)==0x1000){
//            *d&=0x103f; pRP5C01->write(*d&31,data);
//            return((*d&0x10)==0);		/* CLOCK (010xx) */
//        }

        if((*d&0x6000)==0x2000){
            *d&=0x200f; disp(*d&15,data);//lcdc.access=1; lcdc.lcdcadr=*d&15;
            return(1-(*d&1));			/* LCDC (0200x) */
        }
        return 1;
    }
#if (TEST_MEMORY_MAPPING)

    if ( (*d>=0x40000) && (*d<=0x4FFFF)) return (ext_MemSlot1->ExtArray[ID_CE210M]->IsChecked ||
                                                 ext_MemSlot1->ExtArray[ID_CE211M]->IsChecked ||
                                                 ext_MemSlot1->ExtArray[ID_CE212M]->IsChecked ||
                                                 ext_MemSlot1->ExtArray[ID_CE2H16M]->IsChecked ||
                                                 ext_MemSlot1->ExtArray[ID_CE2H32M]->IsChecked ||
                                                 ext_MemSlot1->ExtArray[ID_CE2H64M]->IsChecked);
    if ( (*d>=0x80000) && (*d<=0xB7FFF)) {
        AddLog(LOG_RAM,QString("adr;%1").arg(*d,6,16,QChar('0')));
    }
    if ( (*d>=0xB8000) && (*d<=0xBFFFF)) return 1;
#endif

    if ( (*d>=0x40000) && (*d<=0xBFFFF)) return 1;

    if ( (*d>=0xC0000) && (*d<=0xFFFFF)) return 0;

//    if(*d>0xbffff) return(0);			/* ROM area(c0000-fffff) S3: */
//    if(*d>0x7ffff) return(1);			/* RAM area(80000-bffff) S1: */
//    if(*d>0x3ffff) return(1);			/* RAM area(40000-7ffff) S2: */


#if 0

    if(*d>0x1ffff){
//        if(sc.e6) return(0);			/* ROM area(20000-3ffff) ->E650/U6000 */
//        else{
//            *d=BASE_128[GetBank()]+(*d&0x1ffff);
//            return(1-(sc.emsmode>>4));		/* RAM area(20000-3ffff) EMS */
//        }
    }
    if(*d>0x0ffff){
        *d=BASE_64[GetBank()]+(*d&0xffff);
        return(1-(sc.emsmode>>4));			/* RAM area(10000-1ffff) EMS */
    }
    if((*d&0xf000)==0xe000){
        *d&=0xf005; ssfdc.access=1; ssfdc.adr=*d;
        return(1);					/* SSFDC (0e00x) */
    }

    if((*d&0x3000)==0x1000){
        *d&=0x103f; rtc.access=1; rtc.adr=*d&31;
        return((*d&0x10)==0);		/* CLOCK (010xx) */
    }
#endif
    return true;
}

/*****************************************************************************/
/* Check Address ROM or RAM ?												 */
/*  ENTRY :UINT32 d=Address													 */
/*  RETURN:bool (1=RAM,0=ROM)												 */
/*****************************************************************************/
bool Ce500::Chk_Adr_R(UINT32 *d,UINT32 *data)
{
#if (TEST_MEMORY_MAPPING)
    quint32 tmp = *d;
    MemMirror(d);
    if ( (tmp>=0x40000) && (tmp<=0xBFFFF) && (pCPU->fp_log)) fprintf(pCPU->fp_log,"\nRAM READ : [%06X] -> [%06X]\n",tmp,*d);
#endif
//    if ( (*d>=0xB0000) && (*d<=0xB7FFF)) { *d += 0x8000;return 1;}


    if(*d>0xbffff) return(1);			/* ROM area(c0000-fffff) S3: */
    if(*d>0x7ffff) return(1);			/* RAM area(80000-bffff) S1: */
    if(*d>0x3ffff) return(1);			/* RAM area(40000-7ffff) S2: */

    if((*d&0x6000)==0x2000){
        *d&=0x200f; disp(*d&15,*data);//pLCDC->SetDirtyBuf(pLCDC->SetDirtyBuf(*d & 15));
        return(1);//-(*d&1));			/* LCDC (0200x) */
    }

//    if((*d&0x3000)==0x1000){
//        *d&=0x103f; mem[*d]= pRP5C01->read(*d&0x1f);
//        return((*d&0x10)==0);		/* CLOCK (010xx) */
//    }

#if 0
    if(*d>0x1ffff){
        if(sc.e6) return(0);			/* ROM area(20000-3ffff) ->E650/U6000 */
        else{
            *d=BASE_128[GetBank()]+(*d&0x1ffff);
            return(1-(sc.emsmode>>4));		/* RAM area(20000-3ffff) EMS */
        }
    }
    if(*d>0x0ffff){
        *d=BASE_64[GetBank()]+(*d&0xffff);
        return(1-(sc.emsmode>>4));			/* RAM area(10000-1ffff) EMS */
    }
    if((*d&0xf000)==0xe000){
        *d&=0xf005; ssfdc.access=1; ssfdc.adr=*d;
        return(1);					/* SSFDC (0e00x) */
    }

    if((*d&0x3000)==0x1000){
        *d&=0x103f; rtc.access=1; rtc.adr=*d&31;
        return((*d&0x10)==0);		/* CLOCK (010xx) */
    }
#endif
    return true;
}



BYTE Ce500::Get_PortA()
{
    return 0;
}

BYTE Ce500::Get_PortB()
{
    return 0;
}

void Ce500::TurnON()
{
    if (!Power && pKEYB->LastKey == K_BRK) {
        AddLog(LOG_MASTER,"TURN ON");
        if (!hardreset) {
            Initial_Session_Load();
        }
        else hardreset = false;

        if (pLCDC) pLCDC->TurnON();
        this->Reset();
        off = 0;
        Power = true;
        PowerSwitch = PS_RUN;
    }
}

void Ce500::TurnOFF()
{
    mainwindow->saveAll = YES;
    CpcXXXX::TurnOFF();
    mainwindow->saveAll = ASK;
}

bool Ce500::LoadExtra(QFile *)
{
    return true;
}

bool Ce500::SaveExtra(QFile *)
{
    return true;
}

UINT8 Ce500::in(UINT8 address)
{
    return 0;
}

UINT8 Ce500::out(UINT8 address, UINT8 value)
{
    return 0;
}

//#define KEY(c)	( pKEYB->keyPressedList.contains(TOUPPER(c)) || pKEYB->keyPressedList.contains(c) || pKEYB->keyPressedList.contains(TOLOWER(c)))
#define KEY(c)	( TOUPPER(pKEYB->LastKey) == TOUPPER(c) )


BYTE Ce500::getKey()
{
    UINT8 data=0;

    UINT32 ks = (pCPU->imem[IMEM_KOH]<<8)+pCPU->imem[IMEM_KOL];

//    if ((pKEYB->LastKey) )
    {
        AddLog(LOG_KEYBOARD,tr("GetKEY : %1").arg(ks,4,16,QChar('0')));
        if (ks&1) {
            if (KEY(K_EXTMENU))     data|=0x01;
            if (KEY('Q'))			data|=0x02;
            if (KEY(K_MENU))		data|=0x04;
            if (KEY('A'))			data|=0x08;
            if (KEY(K_BASIC))		data|=0x10;
            if (KEY('Z'))			data|=0x20;
            if (pKEYB->isShift)		data|=0x40;
            if (KEY(K_CTRL))		data|=0x80;			// UP ARROW
        }
        if (ks&2) {
            if (KEY('W'))			data|=0x01;
            if (KEY('E'))			data|=0x02;
            if (KEY('S'))			data|=0x04;			// OFF
            if (KEY('D'))			data|=0x08;
            if (KEY('X'))			data|=0x10;
            if (KEY('C'))			data|=0x20;
            if (KEY(K_SML))			data|=0x40;
            if (KEY(K_KANA))		data|=0x80;
        }
        if (ks&4) {
            if (KEY('R'))			data|=0x01;
            if (KEY('T'))			data|=0x02;
            if (KEY('F'))			data|=0x04;
            if (KEY('G'))			data|=0x08;
            if (KEY('V'))			data|=0x10;
            if (KEY('B'))			data|=0x20;
            if (KEY(' '))			data|=0x40;
            if (KEY(K_DA))			data|=0x80;
        }
        if (ks&8) {
            if (KEY('Y'))			data|=0x01;
            if (KEY('U'))			data|=0x02;
            if (KEY('H'))			data|=0x04;
            if (KEY('J'))			data|=0x08;
            if (KEY('N'))			data|=0x10;
            if (KEY('M'))			data|=0x20;
            if (KEY(K_UA))			data|=0x40;
            if (KEY(K_LA))			data|=0x80;
        }
        if (ks&0x10) {
            if (KEY('I'))			data|=0x01;			// +
            if (KEY('O'))			data|=0x02;			// *
            if (KEY('K'))			data|=0x04;			// /
            if (KEY('L'))			data|=0x08;
            if (KEY(','))			data|=0x10;			// Key F2
            if (KEY(';'))			data|=0x20;
            if (KEY(K_RA))			data|=0x40;
            if (KEY(K_RET))			data|=0x80;
        }
        if (ks&0x20) {
            if (KEY(K_RCL))			data|=0x01;			// =
            if (KEY(K_HYP))			data|=0x02;			// LEFT ARROW
            if (KEY(K_HEX))			data|=0x04;
            if (KEY(K_EXP))			data|=0x08;
            if (KEY('7'))			data|=0x10;
            if (KEY('4'))			data|=0x20;
            if (KEY('1'))			data|=0x40;
            if (KEY('0'))			data|=0x80;
        }
        if (ks&0x40) {
            if (KEY(K_STO))			data|=0x01;			// R ARROW
            if (KEY(K_SIN))			data|=0x02;			// MODE
            if (KEY(K_DEG))			data|=0x04;			// CLS
            if (KEY(K_POT))			data|=0x08;
            if (KEY('8'))			data|=0x10;
            if (KEY('5'))			data|=0x20;
            if (KEY('2'))			data|=0x40;
            if (KEY(K_SIGN))			data|=0x80;
        }
        if (ks&0x80) {
            if (KEY(K_CCE))			data|=0x01;
            if (KEY(K_COS))			data|=0x02;
            if (KEY(K_LN))			data|=0x04;
            if (KEY(K_ROOT))		data|=0x08;
            if (KEY('9'))			data|=0x10;			// Key F4
            if (KEY('6'))			data|=0x20;
            if (KEY('3'))			data|=0x40;
            if (KEY('.'))			data|=0x80;			// DOWN ARROW
        }
        if (ks&0x100) {
            if (KEY(K_STAT))		data|=0x01;
            if (KEY(K_TAN))			data|=0x02;
            if (KEY(K_LOG))			data|=0x04;
            if (KEY(K_SQR))         data|=0x08;
            if (KEY('/'))			data|=0x10;			// Key F4
            if (KEY('*'))			data|=0x20;
            if (KEY('-'))			data|=0x40;
            if (KEY('+'))			data|=0x80;			// DOWN ARROW
        }
        if (ks&0x200) {
            if (KEY(')'))			data|=0x01;
            if (KEY(K_FSE))			data|=0x02;
            if (KEY(K_1X))			data|=0x04;
            if (KEY('('))   		data|=0x08;
            if (KEY(K_DEL))			data|=0x10;			// Key F4
            if (KEY(K_BS))			data|=0x20;
            if (KEY(K_INS))			data|=0x40;
            if (KEY('='))			data|=0x80;			// DOWN ARROW
        }
        if (ks&0x400) {
            if (KEY('P'))			data|=0x01;
            if (KEY(K_SHT2))		data|=0x02;
            if (KEY(K_F5))			data|=0x04;
            if (KEY(K_F4))			data|=0x08;
            if (KEY(K_F3))			data|=0x10;
            if (KEY(K_F2))			data|=0x20;
            if (KEY(K_F1))			data|=0x40;			// Key F4

        }
        if (ks&0x800) {
            if (KEY(K_POW_OFF))		data|=0x01;
            if (KEY(K_BRK))		data|=0x02;
        }
//        if (fp_log) fprintf(fp_log,"Read key [%02x]: strobe=%02x result=%02x\n",pKEYB->LastKey,ks,data^0xff);

    }

    if (KEY(K_BRK)) {
        ((Csc62015*)pCPU)->opr_imem(IMEM_ISR,OPR_OR,INT_ONKEY);
        pCPU->setImemBit(IMEM_SSR,4,1);
    }
    else
        pCPU->setImemBit(IMEM_SSR,4,0);
//    else
    if(data) {
        ((Csc62015*)pCPU)->opr_imem(IMEM_ISR,OPR_OR,INT_KEY);	// set status to ISR
    }
    pCPU->imem[IMEM_KI] = data;					//set data to ki
    return data^0xff;start2khz = 0;
    start4khz = 0;

}



Ce550::Ce550(CPObject *parent):Ce500(parent)
{
    setcfgfname(QString("e550"));

    SessionHeader	= "E550PKM";
    Initial_Session_Fname ="e550.pkm";

    BackGroundFname	= P_RES(":/e500/pc-e550.png");

    Lcd_X		= 70;
    Lcd_Y		= 96;

    Lcd_Symb_X	= 70;//(int) (45 * 1.18);
    Lcd_Symb_Y	= 76;//(int) (35 * 1.18);

}


bool Ce550::Chk_Adr(UINT32 *d, UINT32 data)
{
    return Ce500::Chk_Adr(d,data);
}

bool Ce550::Chk_Adr_R(UINT32 *d, UINT32 *data)
{
    return Ce500::Chk_Adr_R(d,data);
}



