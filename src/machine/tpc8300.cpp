#include <QDebug>

#include "tpc8300.h"
#include "upd7907/upd7907.h"
#include "upd16434.h"
#include "pd1990ac.h"
#include "Log.h"
#include "Keyb.h"
#include "Inter.h"
#include "Lcdc_tpc8300.h"
#include "Connect.h"
#include "watchpoint.h"

/*

 PA bit 0-1:
        0   LCD controller 0
        1   LCD controller 1
        2   LCD controller 2
        3   LCD controller 3
     bit 2: LCDCommand if 1 or LCDData
     bit 3: TIMER
     bit 6: PRINTER PORT SELECTED ????? 0x40




 PB 0x00 : output to Printer
    0x01 : ACK for serial transmission
    0x02 : output to LCD
    0x04 : read timer chip bit

    0x20 : write cmd to timer chip
    0x40 : write cmd to timer chip

    0x80 : TAPE rmt

 */


Ctpc8300::Ctpc8300(CPObject *parent)	: CpcXXXX(parent)
{								//[constructor]
    setfrequency( (int) 3865000/8);
    setcfgfname(QString("tpc8300"));

    SessionHeader	= "TPC8300PKM";
    Initial_Session_Fname ="tpc8300.pkm";

    BackGroundFname	= P_RES(":/tpc8300/tpc-8300.png");

    memsize		= 0x10000;
    InitMemValue	= 0xFF;

    SlotList.clear();
    SlotList.append(CSlot(4 , 0x0000 ,	P_RES(":/tpc8300/tpc8300-0000-0FFF.bin"), ""	, CSlot::ROM , "ROM"));
    SlotList.append(CSlot(12, 0x1000 ,	"", ""	,                             CSlot::RAM , "RAM"));
    SlotList.append(CSlot(8,  0x4000 ,	P_RES(":/tpc8300/tpc8300-4000-5FFF.bin"), ""	, CSlot::ROM , "ROM"));
    SlotList.append(CSlot(8,  0x6000 ,	P_RES(":/tpc8300/tpc8300-6000-7FFF.bin"), ""	, CSlot::ROM , "ROM"));
    SlotList.append(CSlot(32 ,0x8000 ,	""	, ""	,                         CSlot::RAM , "RAM"));

//    Pc_Offset_X = Pc_Offset_Y = 0;

    setDXmm(199);//Pc_DX_mm 206
    setDYmm(96);//Pc_DY_mm =83)
    setDZmm(26);//Pc_DZ_mm =30)

    setDX(711);//Pc_DX		= 483;//409;
    setDY(343);//Pc_DY		= 252;//213;

    PowerSwitch = 0;

    pLCDC		= new Clcdc_tpc8300(this,
                                    QRect(197,44,144*2.25,20*2.25),
                                    QRect());
    pCPU		= new Cupd7907(this);    upd7907 = (Cupd7907*)pCPU;
    for (int i=0;i<3;i++) upd16434[i]  = new CUPD16434(this);
    pd1990ac    = new CPD1990AC(this);
    pTIMER		= new Ctimer(this);
    pKEYB		= new Ckeyb(this,"tpc8300.map");

    ioFreq = 0;
}

Ctpc8300::~Ctpc8300() {

    for (int i=0;i<3;i++) {
        delete(upd16434[i]);
    }
}

bool Ctpc8300::init(void)				// initialize
{

//pCPU->logsw = true;
#ifndef QT_NO_DEBUG
//    pCPU->logsw = true;
//    if (!fp_log) fp_log=fopen("tpc8300.log","wt");	// Open log file
#endif
    CpcXXXX::init();

    pTIMER->resetTimer(1);
    pCONNECTOR	= new Cconnector(this,20,0,Cconnector::General_20,"20 pins connector",false,
                                     QPoint(0,158));
    publish(pCONNECTOR);

    WatchPoint.remove(this);
    WatchPoint.add(&pCONNECTOR_value,64,20,this,"20 pins connector");

    portB = 0;


initcmd = true;

    return true;
}

#define LCD_PORT    (upd7907->upd7907stat.imem[0x00])

bool Ctpc8300::run() {


    CpcXXXX::run();

    if (pTIMER->msElapsedId(1)>1000) {
        // Check if interrupts are enabled
//        upd7907->upd7907stat.irr |= 0x10;
        pTIMER->resetTimer(1);
    }

    if (upd7907->upd7907stat.imem[0x01] & 0x40)
    {
        UNSET_BIT(portB,1);
    }
    else {
        SET_BIT(portB,1);
    }

    quint8 data = upd7907->upd7907stat.imem[0x08];
//    if ( (data > 0) && (data != 0xff))
    if (upd7907->upd7907stat.serialPending)
    {
        upd7907->upd7907stat.serialPending = false;
        switch (LCD_PORT) {
        case 0x77:  // PRINTER PORT
        case 0x7F:
            sendToPrinter = data;
            AddLog(LOG_CONSOLE,tr("PRINTER:%1=(%2)").arg(data).arg(QChar(data)));
            upd7907->upd7907stat.imem[0x08] = 0;
            break;
        default:   // LCD transmission
        {
            // flip flop PB1 0-2-0
            //        SET_BIT(portB,1);
//            pTIMER->resetTimer(2);
            quint8 currentLCDctrl=0;
            switch (LCD_PORT & 0x07) {
            case 3 : currentLCDctrl = 2; break;
            case 5 : currentLCDctrl = 0; break;
            case 6 : currentLCDctrl = 1; break;
            }

            quint8 cmddata = (LCD_PORT >> 6) & 0x01;
            switch(cmddata) {
            case 0x01: /*qWarning()<<"lcd cmd:"<<currentLCDctrl;*/upd16434[currentLCDctrl]->instruction(data);
                break;
            case 0x00: /*qWarning()<<"lcd data:"<<currentLCDctrl*/;upd16434[currentLCDctrl]->data(data);
                break;
            }
             upd16434[currentLCDctrl]->updated = true;
//            upd7907->upd7907stat.imem[0x08] = 0;
        }
            break;

        }

    }

     fillSoundBuffer(upd7907->upd7907stat.to ? 0xff : 0x00);

     pCONNECTOR_value = pCONNECTOR->Get_values();
//     pTAPECONNECTOR_value   = pTAPECONNECTOR->Get_values();
//     pPRINTERCONNECTOR_value = pPRINTERCONNECTOR->Get_values();
    return true;
}

bool Ctpc8300::Chk_Adr(UINT32 *d, UINT32 data)
{
    Q_UNUSED(data)

    if(*d < 0x8000) return false; /* ROM */
//    if(*d < 0xE000) return false; /* RAM */

    return true;
}

bool Ctpc8300::Chk_Adr_R(UINT32 *d, UINT32 *data)
{
    Q_UNUSED(d)
    Q_UNUSED(data)

    return true;
}


UINT8 Ctpc8300::in(UINT8 Port,QString)
{
    switch (Port) {
    case 0x01 : return portB | (pCONNECTOR->Get_pin(12) ? 0x80 : 0x00); break;
    case 0x02 : return (getKey() & 0x3F); break;
    }

    return 0;
}



UINT8 Ctpc8300::out(UINT8 Port, UINT8 x, QString sender)
{
    switch (Port) {
//    case 0x01 : portB = x; break;
    case 0x00: if (x&0x40) initcmd = true; break;
    }

    return 0;
}

UINT16 Ctpc8300::out16(UINT16 address, UINT16 value, QString sender)
{
    if (address == UPD7907_PORTE) {
        kstrobe = value;
    }

    return 0;
}

bool Ctpc8300::Set_Connector(Cbus *_bus)
{
    if (sendToPrinter>0) {
        pCONNECTOR->Set_values(sendToPrinter);
        AddLog(LOG_PRINTER,QString("Send Char:%1").arg(sendToPrinter,2,16,QChar('0')));
    }
    else
        pCONNECTOR->Set_values(0);

    // TAPE
    pCONNECTOR->Set_pin(10,true);       // RMT
    pCONNECTOR->Set_pin(11,upd7907->upd7907stat.imem[0x00] & 0x10 ? 0xff : 0x00);    // Out

    return true;
}

bool Ctpc8300::Get_Connector(Cbus *_bus)
{

    if (pCONNECTOR->Get_pin(9)) {
        sendToPrinter = 0;
    }

    return true;
}


void Ctpc8300::TurnOFF(void) {
    mainwindow->saveAll = YES;
    CpcXXXX::TurnOFF();
    mainwindow->saveAll = ASK;
    AddLog(LOG_TEMP,"TURN OFF");

}

void Ctpc8300::TurnON(void){
    CpcXXXX::TurnON();
    upd7907->upd7907stat.pc.w.l=0;

}


void Ctpc8300::Reset()
{
    CpcXXXX::Reset();
    pLCDC->init();
    for (int i=0;i<4;i++) upd16434[i]->Reset();
    sendToPrinter=0;
    initcmd = true;

}

bool Ctpc8300::LoadConfig(QXmlStreamReader *xmlIn)
{
    for (int i=0;i<4;i++) upd16434[i]->Load_Internal(xmlIn);
    return true;
}

bool Ctpc8300::SaveConfig(QXmlStreamWriter *xmlOut)
{
    for (int i=0;i<4;i++) upd16434[i]->save_internal(xmlOut);
    return true;
}



#define KEY(c)	( pKEYB->keyPressedList.contains(TOUPPER(c)) || pKEYB->keyPressedList.contains(c) || pKEYB->keyPressedList.contains(TOLOWER(c)))

UINT16 Ctpc8300::getKey()
{

    UINT16 ks = kstrobe^0xFFFF;
    UINT16 data=0;

    if ((pKEYB->LastKey) && ks )
    {
//        if (fp_log) fprintf(fp_log,"KSTROBE=%04X\n",ks);
//        qWarning()<<QString("ks:%1").arg(ks,4,16,QChar('0'));

        if (ks&0x01) {
            if (KEY('Q'))			data|=0x01;
            if (KEY('A'))			data|=0x02;
            if (KEY('Z'))			data|=0x04;
            if (KEY('+'))			data|=0x08;
            if (KEY('1'))			data|=0x10;
        }

        if (ks&0x02) {
            if (KEY('W'))			data|=0x01;
            if (KEY('S'))			data|=0x02; // A
            if (KEY('X'))			data|=0x04; // Q
            if (KEY('*'))			data|=0x08;
            if (KEY('2'))			data|=0x10; // 1
        }
        if (ks&0x04) {
            if (KEY('E'))			data|=0x01; // X
            if (KEY('D'))			data|=0x02; // S
            if (KEY('C'))			data|=0x04; // W
            if (KEY('-'))			data|=0x08;
            if (KEY('3'))			data|=0x10; // 2
        }

        if (ks&0x08) {
            if (KEY('R'))			data|=0x01; // C
            if (KEY('F'))			data|=0x02; // D
            if (KEY('V'))			data|=0x04; // E
            if (KEY('/'))			data|=0x08;
            if (KEY('4'))			data|=0x10; // 3
        }

        if (ks&0x10) {
            if (KEY('T'))			data|=0x01; // V
            if (KEY('G'))			data|=0x02; // F
            if (KEY('B'))			data|=0x04; // R
            if (KEY(K_F1))			data|=0x08;
            if (KEY('5'))			data|=0x10; // 4
        }

        if (ks&0x20) {
            if (KEY('Y'))			data|=0x01; // B
            if (KEY('H'))			data|=0x02; // G
            if (KEY('N'))			data|=0x04; // T
            if (KEY(K_F2))			data|=0x08;
            if (KEY('6'))			data|=0x10;
        }
        if (ks&0x40) {
            if (KEY('U'))			data|=0x01; // N
            if (KEY('J'))			data|=0x02; // H
            if (KEY('M'))			data|=0x04; // Y
            if (KEY(K_F3))			data|=0x08;
            if (KEY('7'))			data|=0x10;
        }
        if (ks&0x80) {
            if (KEY('I'))			data|=0x01; // M
            if (KEY('K'))			data|=0x02; // J
            if (KEY(K_RA))			data|=0x04; // U
            if (KEY(K_F4))			data|=0x08;
            if (KEY('8'))			data|=0x10;
        }
        if (ks&0x100) {
            if (KEY('O'))			data|=0x01;
            if (KEY('L'))			data|=0x02; // K
            if (KEY(K_LA))			data|=0x04; // I
            if (KEY(K_F5))			data|=0x08;
            if (KEY('9'))			data|=0x10;
        }
        if (ks&0x200) {
            if (KEY('P'))			data|=0x01; // *
            if (KEY(K_INS))			data|=0x02;
            if (KEY(K_UA))			data|=0x04;
            if (KEY(' '))			data|=0x08;
            if (KEY('0'))			data|=0x10;
        }
        if (ks&0x400) {
            if (KEY('='))			data|=0x01;     // numpad -
            if (KEY(K_BS))			data|=0x02;
            if (KEY(K_DA))			data|=0x04;
            if (KEY(','))			data|=0x08;
            if (KEY(':'))			data|=0x10;
            if (KEY('.'))			data|=0x20; // :
        }
        if (ks&0x800) {
            if (KEY(K_RET))			data|=0x01;

//            if (pKEYB->isShift) data|=0x02;

//            if (KEY(K_F2))			data|=0x02;
//            if (KEY(K_F3))			data|=0x04;
//            if (KEY(K_F4))		data|=0x08;
//            if (KEY(K_F5))			data|=0x10;
//            if (KEY(K_F6))			data|=0x20;
        }
//        if (ks&0x1000) {
//            if (KEY(K_F1))			data|=0x01;
//            if (KEY(K_F2))			data|=0x02;
//            if (pKEYB->isShift)     data|=0x04;
//            if (KEY(K_F4))		data|=0x08;
//            if (KEY(K_F5))			data|=0x10;
//            if (KEY(K_F6))			data|=0x20;
//        }
        if (ks&0x2000) {
////            if (KEY(K_F5))			data|=0x01;
//            if (KEY(K_INS))			data|=0x02;
//            if (KEY(K_DEL))			data|=0x04;
//            if (KEY(K_DA))			data|=0x08;
//            if (KEY(K_LA))			data|=0x10;
//            if (KEY(K_RA))			data|=0x20;
        }
        if (ks&0x4000) {
//            if (KEY(K_F7))			data|=0x01;
//            if (KEY(K_RET))			data|=0x02;
//            if (KEY(K_F6))			data|=0x04;
//            if (KEY(K_SML))			data|=0x08;  // KANA ???
//            if (KEY(K_CLR))			data|=0x10; // CLR ???
//            if (KEY(K_F5))			data|=0x20;
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

/*


  SAMPLE BASIC CODE

10 INPUTX
20 G=1
30 IFX>5THEN70
40 G=G*X
50 X=X+1
60 GOTO30
70 PRINTX*LOG(X)-X+LOG(6.28319/X)/2+((1/99/X/X-1/30)/X/X+1)/12-LOG(G)









 **/


