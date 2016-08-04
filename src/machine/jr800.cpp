//FIXME: issue with turnoff turnon and reset.... initialyse memory


#include <QDebug>

#include "jr800.h"
#include "mc6800/mc6800.h"
//#include "upd16434.h"
#include "Log.h"
#include "Keyb.h"
#include "Inter.h"
#include "Lcdc_jr800.h"
#include "Connect.h"
#include "watchpoint.h"
#include "hd44102.h"

/**
 * @brief
 *
 * @param parent
 */
Cjr800::Cjr800(CPObject *parent)	: CpcXXXX(parent)
{								//[constructor]
    setfrequency( (int) 4915200/4);
    setcfgfname(QString("jr800"));

    SessionHeader	= "JR800PKM";
    Initial_Session_Fname ="jr800.pkm";

    BackGroundFname	= P_RES(":/jr800/jr800.png");

    RightFname = P_RES(":/jr800/jr800Right.png");
    LeftFname = P_RES(":/jr800/jr800Left.png");
    TopFname = P_RES(":/jr800/jr800Top.png");
//    BackFname = P_RES(":/jr800/jr800Back.png");

    memsize		= 0xFFFF;
    InitMemValue	= 0xFF;

    SlotList.clear();
    SlotList.append(CSlot(32 ,0x0000 ,	""	, ""	, CSlot::RAM , "RAM"));
    SlotList.append(CSlot(32, 0x8000 ,	P_RES(":/jr800/rom-8000-FFFF.bin"), ""	, CSlot::ROM , "ROM"));

    PowerSwitch	= 0;

    setDXmm(255);
    setDYmm(140);
    setDZmm(32);

    setDX(911);
    setDY(501);

    PowerSwitch = 0;

    pLCDC		= new Clcdc_jr800(this,
                                   QRect(98,94,340,115),
                                   QRect(86,94,364,115));
    pCPU		= new Cmc6800(this);
    for (int i=0;i<8;i++) {
        hd44102[i]  = new CHD44102(this);
        hd44102[i]->setObjectName(QString("%1").arg(i));
        qWarning()<<hd44102[i];
    }
    pTIMER		= new Ctimer(this);
    pKEYB		= new Ckeyb(this,"jr800.map");

    ioFreq = 0;
}

/**
 * @brief
 *
 */
Cjr800::~Cjr800() {

}

/**
 * @brief
 *
 * @return bool
 */
bool Cjr800::init(void)				// initialize
{

//pCPU->logsw = true;
#ifndef QT_NO_DEBUG
//    pCPU->logsw = true;
//    if (!fp_log) fp_log=fopen("jr800.log","wt");	// Open log file
#endif

    for (int i=0;i<8;i++) {
        hd44102[i]->init();
    }

    CpcXXXX::init();

    pTIMER->resetTimer(1);

    pTAPECONNECTOR	= new Cconnector(this,3,0,Cconnector::Jack,"Line in / Rec / Rmt",false,
                                     QPoint(804,0),Cconnector::NORTH);
    publish(pTAPECONNECTOR);
    pPRINTERCONNECTOR	= new Cconnector(this,9,1,Cconnector::DIN_8,"Printer",false,
                                         QPoint(402,0),Cconnector::NORTH);
    publish(pPRINTERCONNECTOR);
    WatchPoint.add(&pTAPECONNECTOR_value,64,2,this,"Line In / Rec");
    WatchPoint.add(&pPRINTERCONNECTOR_value,64,9,this,"Printer");


    // Just to be sure, in case of an old saved session
    ((Cmc6800*)pCPU)->regs.opmode = 4;

    return true;
}

// PORT 0:
//    bit 0 : TAPE OUT
//    bit 1 : TAPE IN ??
//    bit 2 :
//    bit 3 : Audio out enable ?
//    bit 4 : Audio out
//    bit 5 : Auto off when 1. CPU halt ? PC in F159
//    bit 6 :
//    bit 7 : Audio out enable ?


/**
 * @brief
 *
 * @return bool
 */
bool Cjr800::run() {

    CpcXXXX::run();

    BYTE _soundData = 0;

// Temporary direct access to port 0 to avois constant warning during port 0 analyse
#if 1
    if( READ_BIT(((Cmc6800*)pCPU)->regs.port[0].wreg,7)) {
        _soundData = READ_BIT(((Cmc6800*)pCPU)->regs.port[0].wreg , 4) ? 0xff : 0x00;
        fillSoundBuffer(_soundData);
    }
#else
    if (BIT(pCPU->get_mem(0x02,SIZE_8),7)) {
        _soundData = BIT(pCPU->get_mem(0x02,SIZE_8) , 4) ? 0xff : 0x00;
        fillSoundBuffer(_soundData);
    }
#endif

    pTAPECONNECTOR_value   = pTAPECONNECTOR->Get_values();
    pPRINTERCONNECTOR_value = pPRINTERCONNECTOR->Get_values();
    return true;
}

/**
 * @brief
 *
 * @param d
 * @param data
 * @return bool
 */
bool Cjr800::Chk_Adr(UINT32 *d, UINT32 data)
{
    Q_UNUSED(data)


    if ((*d>=0x0A00) && (*d<=0x0AFF)) {
        UINT8 _chip = *d & 0xff;
        UINT8 _id = 0;
        switch (_chip) {
        case 0x01: _id = 0; break;
        case 0x02: _id = 1; break;
        case 0x04: _id = 2; break;
        case 0x08: _id = 3; break;
        case 0x10: _id = 4; break;
        case 0x20: _id = 5; break;
        case 0x40: _id = 6; break;
        case 0x80: _id = 7; break;
        default: qWarning()<<"ERR - Write cmd:"<<data<<" to driver:"<<_chip;
            break;
        }
//        qWarning()<<"Write cmd:"<<data<<" to driver:"<<_id;
        hd44102[_id]->cmd_write(data);
        return false;
    }
    if ((*d>=0x0B00) && (*d<=0x0BFF)) {
        UINT8 _chip = *d & 0xff;
        UINT8 _id = 0;
        switch (_chip) {
        case 0x01: _id = 0; break;
        case 0x02: _id = 1; break;
        case 0x04: _id = 2; break;
        case 0x08: _id = 3; break;
        case 0x10: _id = 4; break;
        case 0x20: _id = 5; break;
        case 0x40: _id = 6; break;
        case 0x80: _id = 7; break;
        default: qWarning()<<tr("ERR - Write data:%1").arg(data,2,16,QChar('0'))<<" to driver:"<<tr("%1").arg(_chip,2,16,QChar('0'));
            break;
        }
        hd44102[_id]->set8(data);
//        qWarning()<<"Write data:"<<data<<" to driver:"<<_id;
        return false;
    }

    if ((*d>=0x2000) && (*d<=0x5FFF)) {
        return true; /* RAM */
    }
    if ((*d>=0x6000) && (*d<=0x7FFF)) {
        return true; /* Extended RAM */
    }
    qWarning()<<"ERR WRITE :"<<tr("[%1]=%2").arg(*d,4,16,QChar('0')).arg(data,2,16,QChar('0'));
    return false;
}

/**
 * @brief
 *
 * @param d
 * @param data
 * @return bool
 */
bool Cjr800::Chk_Adr_R(UINT32 *d, UINT32 *data)
{
    Q_UNUSED(d)
    Q_UNUSED(data)


    if ((*d>=0x0A00) && (*d<=0x0AFF)) {
        UINT8 _chip = *d & 0xff;
        UINT8 _id = 0;
        switch (_chip) {
            case 0x01: _id = 0; break;
            case 0x02: _id = 1; break;
            case 0x04: _id = 2; break;
            case 0x08: _id = 3; break;
            case 0x10: _id = 4; break;
            case 0x20: _id = 5; break;
            case 0x40: _id = 6; break;
            case 0x80: _id = 7; break;
        default: qWarning()<<"ERR - Read status:"<<_chip;
            break;
        }
        *data = hd44102[_id]->cmd_status();
//        qWarning()<<"Read status:"<<*data<<" from driver:"<<_id;
        return false;
    }

    if ((*d>=0x0B00) && (*d<=0x0BFF)) {
        UINT8 _chip = *d & 0xff;
        UINT8 _id = 0;
        switch (_chip) {
            case 0x01: _id = 0; break;
            case 0x02: _id = 1; break;
            case 0x04: _id = 2; break;
            case 0x08: _id = 3; break;
            case 0x10: _id = 4; break;
            case 0x20: _id = 5; break;
            case 0x40: _id = 6; break;
            case 0x80: _id = 7; break;
        default: qWarning()<<"ERR - Read data:"<<*data<<" from driver:"<<_chip;
            break;
        }
        *data = hd44102[_id]->get8();
//        qWarning()<<"Read data:"<<*data<<" from driver:"<<_id;
        return false;
    }

    if ((*d>=0x0D00) && (*d<=0x0FFF)) {
        kstrobe = *d;

        *data = getKey();
//        qWarning()<<"Read data:"<<*data<<" from driver:"<<_id;
        return false;
    }

    if ((*d>=0x2000) && (*d<=0x5FFF)) {
        return true; /* RAM */
    }
    if ((*d>=0x6000) && (*d<=0x7FFF)) {
        return true; /* Extended RAM */
    }

    if ((*d>=0x8000) & (*d<=0xFFFF)) return true;  // ROM

    qWarning()<<"ERR UnCatched Read :"<<tr("%1").arg(*d,4,16,QChar('0'));

    return true;
}

/**
 * @brief
 *
 * @param Port
 * @param sender
 * @return UINT8
 */
UINT8 Cjr800::in(UINT8 Port,QString sender)
{
    Q_UNUSED(Port)

    qWarning()<<sender<<tr(": in [%1]").arg(Port,2,16,QChar('0'));

//    switch (Port) {
//    case 0x01 : return portB  | (pTAPECONNECTOR->Get_pin(1) ? 0x80 : 0x00); break;
//    }

    return 0;
}

/**
 * @brief
 *
 * @param Port
 * @param x
 * @param sender
 * @return UINT8
 */
UINT8 Cjr800::out(UINT8 Port, UINT8 x, QString sender)
{
    Q_UNUSED(Port)
    Q_UNUSED(x)
    Q_UNUSED(sender)

//    qWarning()<<sender<<tr(": out [%1]=%2").arg(Port,2,16,QChar('0')).arg(x,2,16,QChar('0'));
    return 0;
}

/**
 * @brief
 *
 * @param address
 * @param value
 * @param sender
 * @return UINT16
 */
UINT16 Cjr800::out16(UINT16 address, UINT16 value, QString sender)
{
    Q_UNUSED(address)
    Q_UNUSED(value)
    Q_UNUSED(sender)

    return 0;
}

/**
 * @brief
 *
 * @param _bus
 * @return bool
 */
bool Cjr800::Set_Connector(Cbus *_bus)
{
    Q_UNUSED(_bus)

    pTAPECONNECTOR->Set_pin(3,true);       // RMT
    pTAPECONNECTOR->Set_pin(2,READ_BIT(pCPU->get_mem(0x02,SIZE_8) , 0) ? 0x00 : 0xff); // TAPE OUT

//    if (sendToPrinter>0) {
//        pPRINTERCONNECTOR->Set_values(sendToPrinter);
//        AddLog(LOG_PRINTER,QString("Send Char:%1").arg(sendToPrinter,2,16,QChar('0')));
//    }
//    else
//        pPRINTERCONNECTOR->Set_values(0);

    return true;
}

/**
 * @brief
 *
 * @param _bus
 * @return bool
 */
bool Cjr800::Get_Connector(Cbus *_bus)
{
    Q_UNUSED(_bus)

    // TAPE IN
    UINT8 _data = pCPU->get_mem(0x02,SIZE_8);
    PUT_BIT(_data,2,pTAPECONNECTOR->Get_pin(1));
    pCPU->set_mem(0x02,SIZE_8,_data);

//    if (pPRINTERCONNECTOR->Get_pin(9)) {
//        sendToPrinter = 0;
//    }

    return true;
}

/**
 * @brief
 *
 */
void Cjr800::TurnOFF(void) {
    mainwindow->saveAll = YES;
    CpcXXXX::TurnOFF();
    mainwindow->saveAll = ASK;
    AddLog(LOG_TEMP,"TURN OFF");

}

/**
 * @brief
 *
 */
void Cjr800::TurnON(void){
    CpcXXXX::TurnON();
//    upd7907->Reset();
//    upd7907->upd7907stat.pc.w.l=0;
//qWarning()<<"LCD ON:"<<pLCDC->On;
}

/**
 * @brief
 *
 */
void Cjr800::Reset()
{
    CpcXXXX::Reset();

//    pLCDC->init();
    for (int i=0;i<8;i++) hd44102[i]->Reset();

}

/**
 * @brief
 *
 * @param xmlIn
 * @return bool
 */
bool Cjr800::LoadConfig(QXmlStreamReader *xmlIn)
{
    for (int i=0;i<8;i++) hd44102[i]->Load_Internal(xmlIn);
    return true;
}

/**
 * @brief
 *
 * @param xmlOut
 * @return bool
 */
bool Cjr800::SaveConfig(QXmlStreamWriter *xmlOut)
{
    for (int i=0;i<8;i++) hd44102[i]->save_internal(xmlOut);
    return true;
}




/**
 * @brief
 *
 * @return UINT16
 */
UINT16 Cjr800::getKey()
{

    UINT16 ks = kstrobe^0xFFFF;
    UINT16 data=0;

//    if ((pKEYB->LastKey) && ks )
    {
//        if (fp_log) fprintf(fp_log,"KSTROBE=%04X\n",ks);

        if (ks&0x01) {
            if (KEY(K_0))			data|=0x01;
            if (KEY(K_1))			data|=0x02;
            if (KEY(K_2))			data|=0x04;
            if (KEY(K_3))			data|=0x08;
            if (KEY(K_4))			data|=0x10;
            if (KEY(K_5))			data|=0x20;
            if (KEY(K_6))			data|=0x40;
            if (KEY(K_7))			data|=0x80;
        }

        if (ks&0x02) {
            if (KEY(K_8))			data|=0x01;
            if (KEY(K_9))			data|=0x02;
            if (KEY('*'))			data|=0x04;
            if (KEY('+'))			data|=0x08;
            if (KEY('='))			data|=0x10;
            if (KEY('-'))			data|=0x20;
            if (KEY(K_PT))			data|=0x40;
            if (KEY('/'))			data|=0x80;
        }
        if (ks&0x04) {
            if (KEY('0'))			data|=0x01; // OK
            if (KEY('1'))			data|=0x02; // OK
            if (KEY('2'))			data|=0x04; // OK
            if (KEY('3'))			data|=0x08;
            if (KEY('4'))			data|=0x10;
            if (KEY('5'))			data|=0x20;
            if (KEY('6'))			data|=0x40;
            if (KEY('7'))			data|=0x80;
        }

        if (ks&0x08) {
            if (KEY('8'))			data|=0x01;
            if (KEY('9'))			data|=0x02;
            if (KEY(':'))			data|=0x04; // OK
            if (KEY(';'))			data|=0x08;
            if (KEY(','))			data|=0x10;
            if (KEY('^'))			data|=0x20;
            if (KEY('.'))			data|=0x40;
            if (KEY(K_CLR))			data|=0x80;

        }

        if (ks&0x10) {
            if (KEY(' '))			data|=0x01; // OK
            if (KEY('A'))			data|=0x02; // OK
            if (KEY('B'))			data|=0x04; // OK
            if (KEY('C'))			data|=0x08;
            if (KEY('D'))			data|=0x10;
            if (KEY('E'))			data|=0x20;
            if (KEY('F'))			data|=0x40;
            if (KEY('G'))			data|=0x80;
        }
        if (ks&0x20) {
            if (KEY('H'))			data|=0x01; // OK
            if (KEY('I'))			data|=0x02; // OK
            if (KEY('J'))			data|=0x04; // OK
            if (KEY('K'))			data|=0x08;
            if (KEY('L'))			data|=0x10;
            if (KEY('M'))			data|=0x20;
            if (KEY('N'))			data|=0x40;
            if (KEY('O'))			data|=0x80;
        }
        if (ks&0x40) {
            if (KEY('P'))			data|=0x01; // OK
            if (KEY('Q'))			data|=0x02; // OK
            if (KEY('R'))			data|=0x04; // OK
            if (KEY('S'))			data|=0x08;
            if (KEY('T'))			data|=0x10;
            if (KEY('U'))			data|=0x20;
            if (KEY('V'))			data|=0x40;
            if (KEY('W'))			data|=0x80;
        }
        if (ks&0x80) {
            if (KEY('X'))			data|=0x01;
            if (KEY('Y'))			data|=0x02;
            if (KEY('Z'))			data|=0x04;
            if (KEY(K_INS))			data|=0x08;
            if (KEY(K_RA))			data|=0x10;
            if (KEY(K_UA))			data|=0x20;
            if (KEY(K_RET))			data|=0x40;
            if (KEY(K_BRK))			data|=0x80; // HOME CLS
        }

        if (ks&0x100) {
            if (KEY(K_F1))			data|=0x01;
            if (KEY(K_F2))			data|=0x02;
            if (KEY(K_F3))			data|=0x04;
            if (KEY(K_F4))			data|=0x08;
            if (KEY(K_F5))			data|=0x10;
            if (KEY(K_F6))			data|=0x20;
            if (KEY(K_F7))			data|=0x40;
            if (KEY(K_F8))			data|=0x80;
        }
        if (ks&0x200) {
//            if (KEY(K_F1))			data|=0x01;
//            if (KEY(K_F2))			data|=0x02;
            if (KEY(K_MENU))		data|=0x04;
            if (KEY(K_SHT))			data|=0x08;
            if (KEY(K_CTRL))		data|=0x10;
//            if (KEY(K_F6))			data|=0x20;
//            if (KEY(K_F7))			data|=0x40;
//            if (KEY(K_F5))			data|=0x80;
        }


//        if (fp_log) fprintf(fp_log,"Read key [%02x]: strobe=%02x result=%02x\n",pKEYB->LastKey,ks,data^0xff);

    }
    return (data^0xff);

}


