//FIXME: issue with turnoff turnon and reset.... initialyse memory


#include <QDebug>

#include "jr800.h"
#include "mc6800/mc6800.h"
#include "upd16434.h"
//#include "pd1990ac.h"
#include "Log.h"
#include "Keyb.h"
#include "Inter.h"
#include "lcdc_jr800.h"
#include "Connect.h"
#include "watchpoint.h"
#include "hd44102.h"


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
    BackFname = P_RES(":/jr800/jr800Back.png");

    memsize		= 0xFFFF;
    InitMemValue	= 0x00;

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
                                   QRect(93,80,192*2,64*2),
                                   QRect());
    pCPU		= new Cmc6800(this);
    for (int i=0;i<8;i++) {
        hd44102[i]  = new CHD44102(this);
        qWarning()<<hd44102[i];
    }
    pTIMER		= new Ctimer(this);
    pKEYB		= new Ckeyb(this,"jr800.map");

    ioFreq = 0;
}

Cjr800::~Cjr800() {

}

bool Cjr800::init(void)				// initialize
{

//pCPU->logsw = true;
#ifndef QT_NO_DEBUG
//    pCPU->logsw = true;
//    if (!fp_log) fp_log=fopen("jr800.log","wt");	// Open log file
#endif
    CpcXXXX::init();

    pTIMER->resetTimer(1);

    pTAPECONNECTOR	= new Cconnector(this,3,0,Cconnector::Jack,"Line in / Rec / Rmt",false,
                                     QPoint(804,231),Cconnector::EAST);
    publish(pTAPECONNECTOR);
    pPRINTERCONNECTOR	= new Cconnector(this,9,1,Cconnector::DIN_8,"Printer",false,
                                         QPoint(402,0),Cconnector::NORTH);
    publish(pPRINTERCONNECTOR);
    WatchPoint.add(&pTAPECONNECTOR_value,64,2,this,"Line In / Rec");
    WatchPoint.add(&pPRINTERCONNECTOR_value,64,9,this,"Printer");

    for (int i=0;i<8;i++) {
        hd44102[i]->init();
        qWarning()<<hd44102[i];
    }
//    portB = 0;

    return true;
}

bool Cjr800::run() {

    CpcXXXX::run();

#if 0
    if (pTIMER->msElapsedId(1)>20) {
        Cupd7907 *upd7810 = (Cupd7907 *)pCPU;
        upd7810->upd7907stat.irr |= 0x08;
        pTIMER->resetTimer(1);
    }


    if (upd7907->upd7907stat.imem[0x00] &0x40)
    {
        UNSET_BIT(portB,1);
    }
    else {
        SET_BIT(portB,1);
    }

    quint8 data = upd7907->upd7907stat.imem[0x08];
    if ( (data > 0) && (data != 0xff))
    {

        switch (upd7907->upd7907stat.imem[0x00]>>6) {
        case 0x00:   // LCD transmission
        {
            // flip flop PB1 0-2-0
            //        SET_BIT(portB,1);
            pTIMER->resetTimer(2);
            quint8 currentLCDctrl = upd7907->upd7907stat.imem[0] & 0x03;
            quint8 cmddata = (upd7907->upd7907stat.imem[0] >> 2) & 0x01;
            switch(cmddata) {
            case 0x01: upd16434[currentLCDctrl]->instruction(data);
                break;
            case 0x00: upd16434[currentLCDctrl]->data(data);
                break;
            }
            upd7907->upd7907stat.imem[0x08] = 0;
        }
            break;
        case 0x01:  // PRINTER PORT
            sendToPrinter = data;
            upd7907->upd7907stat.imem[0x08] = 0;
            break;
        }
    }

    fillSoundBuffer(upd7907->upd7907stat.to ? 0xff : 0x00);
#endif

    pTAPECONNECTOR_value   = pTAPECONNECTOR->Get_values();
    pPRINTERCONNECTOR_value = pPRINTERCONNECTOR->Get_values();
    return true;
}

bool Cjr800::Chk_Adr(UINT32 *d, UINT32 data)
{
    Q_UNUSED(data)


    if ((*d>=0x0A00) && (*d<=0x0AFF)) {
        int _chip = *d & 0xff;
        int _id = 0;
        switch (_chip) {
            case 0x01: _id = 0; break;
            case 0x02: _id = 1; break;
            case 0x04: _id = 2; break;
            case 0x08: _id = 3; break;
            case 0x10: _id = 4; break;
            case 0x20: _id = 5; break;
            case 0x40: _id = 6; break;
            case 0x80: _id = 7; break;
        }
//qWarning()<<"Write cmd:"<<data<<" to driver:"<<_id;
        hd44102[_id]->cmd_write(data);
        return false;
    }
    if ((*d>=0x0B00) && (*d<=0x0BFF)) {
        int _chip = *d & 0xff;
        int _id = 0;
        switch (_chip) {
            case 0x01: _id = 0; break;
            case 0x02: _id = 1; break;
            case 0x04: _id = 2; break;
            case 0x08: _id = 3; break;
            case 0x10: _id = 4; break;
            case 0x20: _id = 5; break;
            case 0x40: _id = 6; break;
            case 0x80: _id = 7; break;
        }

        qWarning()<<tr("Write data:%1").arg(data,2,16,QChar('0'))<<" to driver:"<<_id;
        hd44102[_id]->set8(data);
        return false;
    }

    if(*d < 0x8000) return true; /* RAM */

    return false;
}

bool Cjr800::Chk_Adr_R(UINT32 *d, UINT32 *data)
{
    Q_UNUSED(d)
    Q_UNUSED(data)


    if ((*d>=0x0A00) && (*d<=0x0AFF)) {
        int _chip = *d & 0xff;
        int _id = 0;
        switch (_chip) {
            case 0x01: _id = 0; break;
            case 0x02: _id = 1; break;
            case 0x04: _id = 2; break;
            case 0x08: _id = 3; break;
            case 0x10: _id = 4; break;
            case 0x20: _id = 5; break;
            case 0x40: _id = 6; break;
            case 0x80: _id = 7; break;
        }

        *data = hd44102[_id]->cmd_status();
//        qWarning()<<"Read status:"<<*data<<" from driver:"<<_id;
        return false;
    }

    if ((*d>=0x0B00) && (*d<=0x0BFF)) {
        int _chip = *d & 0xff;
        int _id = 0;
        switch (_chip) {
            case 0x01: _id = 0; break;
            case 0x02: _id = 1; break;
            case 0x04: _id = 2; break;
            case 0x08: _id = 3; break;
            case 0x10: _id = 4; break;
            case 0x20: _id = 5; break;
            case 0x40: _id = 6; break;
            case 0x80: _id = 7; break;
        }

        *data = hd44102[_id]->get8();
//        qWarning()<<"Read data:"<<*data<<" from driver:"<<_id;
        return false;
    }

//    if (*d==0x0DFF) {
//        *data = 0x10;
////        qWarning()<<"OK";
//        return false;
//    }

//    if ((*d>=0xC000) & (*d<=0xEFFF)) return false;
    return true;
}

UINT8 Cjr800::in(UINT8 Port)
{
//    switch (Port) {
//    case 0x01 : return portB  | (pTAPECONNECTOR->Get_pin(1) ? 0x80 : 0x00); break;
//    case 0x02 : return (getKey() & 0x3F); break;
//    }

    return 0;
}

UINT8 Cjr800::out(UINT8 Port, UINT8 x)
{
    Q_UNUSED(Port)
    Q_UNUSED(x)

    return 0;
}

UINT16 Cjr800::out16(UINT16 address, UINT16 value)
{
//    if (address == UPD7907_PORTE) {
//        kstrobe = value;
//    }

    return 0;
}

bool Cjr800::Set_Connector(Cbus *_bus)
{
    Q_UNUSED(_bus)

//    pTAPECONNECTOR->Set_pin(3,true);       // RMT
//    pTAPECONNECTOR->Set_pin(2,upd7907->upd7907stat.imem[0x00] & 0x10 ? 0xff : 0x00);    // Out

//    if (sendToPrinter>0) {
//        pPRINTERCONNECTOR->Set_values(sendToPrinter);
//        AddLog(LOG_PRINTER,QString("Send Char:%1").arg(sendToPrinter,2,16,QChar('0')));
//    }
//    else
//        pPRINTERCONNECTOR->Set_values(0);

    return true;
}

bool Cjr800::Get_Connector(Cbus *_bus)
{
    Q_UNUSED(_bus)

//    if (pPRINTERCONNECTOR->Get_pin(9)) {
//        sendToPrinter = 0;
//    }

    return true;
}

void Cjr800::TurnOFF(void) {
    mainwindow->saveAll = YES;
    CpcXXXX::TurnOFF();
    mainwindow->saveAll = ASK;
    AddLog(LOG_TEMP,"TURN OFF");

}

void Cjr800::TurnON(void){
    CpcXXXX::TurnON();
//    upd7907->Reset();
//    upd7907->upd7907stat.pc.w.l=0;
//qWarning()<<"LCD ON:"<<pLCDC->On;
}

void Cjr800::Reset()
{
    CpcXXXX::Reset();
//    upd7907->upd7907stat.pc.w.l=0;
//    pLCDC->init();
    for (int i=0;i<8;i++) hd44102[i]->Reset();
//    sendToPrinter=0;

}

bool Cjr800::LoadConfig(QXmlStreamReader *xmlIn)
{
    for (int i=0;i<8;i++) hd44102[i]->Load_Internal(xmlIn);
    return true;
}

bool Cjr800::SaveConfig(QXmlStreamWriter *xmlOut)
{
    for (int i=0;i<8;i++) hd44102[i]->save_internal(xmlOut);
    return true;
}



#define KEY(c)	( pKEYB->keyPressedList.contains(TOUPPER(c)) || pKEYB->keyPressedList.contains(c) || pKEYB->keyPressedList.contains(TOLOWER(c)))

UINT16 Cjr800::getKey()
{

    UINT16 ks = kstrobe^0xFFFF;
    UINT16 data=0;

    if ((pKEYB->LastKey) && ks )
    {
//        if (fp_log) fprintf(fp_log,"KSTROBE=%04X\n",ks);

        if (ks&0x01) {
//            if (KEY(K_F1))			data|=0x01;
//            if (KEY(K_F2))			data|=0x02;
//            if (KEY(K_SHT))			data|=0x04;
            if (pKEYB->isShift) data|=0x04;
//            if (KEY(K_F4))			data|=0x08;
//            if (KEY(K_F5))			data|=0x10;
//            if (KEY(K_F6))			data|=0x20;
        }

        if (ks&0x02) {
            if (KEY('Q'))			data|=0x01;
            if (KEY('A'))			data|=0x02;
//            if (KEY(K_F1))			data|=0x04;
            if (KEY(K_0))			data|=0x08;
            if (KEY('1'))			data|=0x10;
            if (KEY('Z'))			data|=0x20;
        }
        if (ks&0x04) {
            if (KEY('W'))			data|=0x01;
            if (KEY('S'))			data|=0x02;
            if (KEY('X'))			data|=0x04;
            if (KEY(K_1))			data|=0x08;
            if (KEY('2'))			data|=0x10;
            if (KEY(K_F1))			data|=0x20; // F1
        }

        if (ks&0x08) {
            if (KEY(','))			data|=0x01;
            if (KEY('D'))			data|=0x02;
            if (KEY('C'))			data|=0x04;
            if (KEY(K_2))			data|=0x08;
            if (KEY('3'))			data|=0x10;
            if (KEY(K_F2))			data|=0x20;     //F2
        }

        if (ks&0x10) {
            if (KEY('R'))			data|=0x01;
            if (KEY('F'))			data|=0x02;
            if (KEY('V'))			data|=0x04;
            if (KEY(K_3))			data|=0x08;
            if (KEY('4'))			data|=0x10;
            if (KEY(K_F3))			data|=0x20;     //F3
        }
        if (ks&0x20) {
            if (KEY('T'))			data|=0x01;
            if (KEY('G'))			data|=0x02;
            if (KEY('B'))			data|=0x04;
            if (KEY(K_4))			data|=0x08;
            if (KEY('5'))			data|=0x10;
            if (KEY(K_F4))			data|=0x20;     //F4
        }
        if (ks&0x40) {
            if (KEY('Y'))			data|=0x01;
            if (KEY('H'))			data|=0x02;
            if (KEY('N'))			data|=0x04;
            if (KEY(K_5))			data|=0x08;
            if (KEY('6'))			data|=0x10;
            if (KEY(','))			data|=0x20;
        }
        if (ks&0x80) {
            if (KEY('U'))			data|=0x01;
            if (KEY('J'))			data|=0x02;
            if (KEY('M'))			data|=0x04;
            if (KEY(K_6))			data|=0x08;
            if (KEY('7'))			data|=0x10;
            if (KEY('.'))			data|=0x20;
        }
        if (ks&0x100) {
            if (KEY('I'))			data|=0x01;
            if (KEY('K'))			data|=0x02;
//            if (KEY('/'))			data|=0x04;
            if (KEY(K_7))			data|=0x08;
            if (KEY('8'))			data|=0x10;
            if (KEY('/'))			data|=0x20;
        }
        if (ks&0x200) {
            if (KEY('O'))			data|=0x01;
            if (KEY('L'))			data|=0x02;
            if (KEY('*'))			data|=0x04;
            if (KEY(K_8))			data|=0x08;
            if (KEY('9'))			data|=0x10;
            if (KEY(';'))			data|=0x20;
        }
        if (ks&0x400) {
            if (KEY('P'))			data|=0x01;
//            if (KEY(K_F2))			data|=0x02;
//            if (KEY('-'))			data|=0x04;     // numpad -
            if (KEY(K_9))			data|=0x08;
            if (KEY('0'))			data|=0x10;
            if (KEY(':'))			data|=0x20;
        }
        if (ks&0x800) {
            if (KEY('@'))			data|=0x01;
//            if (KEY(K_F2))			data|=0x02;
            if (KEY('+'))			data|=0x04;
            if (KEY('E'))			data|=0x08;
            if (KEY('-'))			data|=0x10;
            if (KEY(']'))			data|=0x20;
        }
        if (ks&0x1000) {
            if (KEY('^'))			data|=0x01;
            if (KEY(' '))			data|=0x02; //???
//            if (KEY('.'))			data|=0x04; // Numpad .
            if (KEY(K_UA))			data|=0x08;
            if (KEY('['))			data|=0x10;
//            if (KEY(K_F6))			data|=0x20;
        }
        if (ks&0x2000) {
            if (KEY(K_DEL))			data|=0x01;
            if (KEY(K_INS))			data|=0x02;
//            if (KEY(K_UA))			data|=0x04;
            if (KEY(K_DA))			data|=0x08;
            if (KEY(K_LA))			data|=0x10;
            if (KEY(K_RA))			data|=0x20;
        }
        if (ks&0x4000) {
//            if (KEY(K_F1))			data|=0x01;
            if (KEY(K_RET))			data|=0x02;
//            if (KEY(K_F3))			data|=0x04;
            if (KEY(K_SML))			data|=0x08;  // KANA ???
            if (KEY(K_CLR))			data|=0x10; // CLR ???
//            if (KEY(K_F6))			data|=0x20;
        }
//        if (ks&0x8000) {
//            if (KEY(K_F1))			data|=0x01;
//            if (KEY(K_F2))			data|=0x02;
//            if (KEY(K_F3))			data|=0x04;
//            if (KEY(K_F4))			data|=0x08;
//            if (KEY(K_F5))			data|=0x10;
//            if (KEY(K_F6))			data|=0x20;
//        }


//        if (fp_log) fprintf(fp_log,"Read key [%02x]: strobe=%02x result=%02x\n",pKEYB->LastKey,ks,data^0xff);

    }
    return (data^0xff) & 0x3F;

}


