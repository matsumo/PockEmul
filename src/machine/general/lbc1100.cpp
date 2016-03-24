#include <QDebug>

#include "lbc1100.h"
#include "upd7907/upd7907.h"
#include "upd16434.h"
#include "pd1990ac.h"
#include "Log.h"
#include "Keyb.h"
#include "Inter.h"
#include "Lcdc_lbc1100.h"
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


Clbc1100::Clbc1100(CPObject *parent)	: CpcXXXX(parent)
{								//[constructor]
    setfrequency( (int) 3865000/8);
    setcfgfname(QString("lbc1100"));

    SessionHeader	= "LBC1100PKM";
    Initial_Session_Fname ="lbc1100.pkm";

    BackGroundFname	= P_RES(":/lbc1100/lbc-1100.png");
//    LcdFname		= P_RES(":/lbc1100/lbc-1100lcd.png");
//    SymbFname		= "";

    memsize		= 0x10000;
    InitMemValue	= 0xFF;

    SlotList.clear();
    SlotList.append(CSlot(4 , 0x0000 ,	P_RES(":/lbc1100/rom-0000-0FFF.bin"), ""	, CSlot::ROM , "ROM"));
    SlotList.append(CSlot(12, 0x1000 ,	"", ""	,                             CSlot::RAM , "RAM"));
    SlotList.append(CSlot(4,  0x4000 ,	P_RES(":/lbc1100/lbc-4000-4FFF.bin"), ""	, CSlot::ROM , "ROM"));
    SlotList.append(CSlot(4,  0x5000 ,	P_RES(":/lbc1100/lbc-5000-5FFF.bin"), ""	, CSlot::ROM , "ROM"));
    SlotList.append(CSlot(4,  0x6000 ,	P_RES(":/lbc1100/lbc-6000-6FFF.bin"), ""	, CSlot::ROM , "ROM"));
    SlotList.append(CSlot(4,  0x7000 ,	P_RES(":/lbc1100/lbc-7000-7FFF.bin"), ""	, CSlot::ROM , "ROM"));
    SlotList.append(CSlot(32 ,0x8000 ,	""	, ""	,                         CSlot::RAM , "RAM"));

//    Pc_Offset_X = Pc_Offset_Y = 0;

    setDXmm(220);//Pc_DX_mm 206
    setDYmm(105);//Pc_DY_mm =83)
    setDZmm(25);//Pc_DZ_mm =30)

    setDX(786);//Pc_DX		= 483;//409;
    setDY(376);//Pc_DY		= 252;//213;

//    Lcd_X		= 70;
//    Lcd_Y		= 60;
//    Lcd_DX		= 240;//168;//144 ;
//    Lcd_DY		= 21;
//    Lcd_ratio_X	= 2.15;// * 1.18;
//    Lcd_ratio_Y	= 2.75;// * 1.18;

    PowerSwitch = 0;

    pLCDC		= new Clcdc_lbc1100(this,
                                    QRect(70,60,240*2.15,21*2.75),
                                    QRect(),
                                    P_RES(":/lbc1100/lbc-1100lcd.png"));
    pCPU		= new Cupd7907(this);    upd7907 = (Cupd7907*)pCPU;
    for (int i=0;i<4;i++) upd16434[i]  = new CUPD16434(this);
    pd1990ac    = new CPD1990AC(this);
    pTIMER		= new Ctimer(this);
    pKEYB		= new Ckeyb(this,"lbc1100.map");

    ioFreq = 0;
}

Clbc1100::~Clbc1100() {
    for (int i=0;i<4;i++) {
        delete(upd16434[i]);
    }
}

bool Clbc1100::init(void)				// initialize
{

//pCPU->logsw = true;
#ifndef QT_NO_DEBUG
//    pCPU->logsw = true;
//    if (!fp_log) fp_log=fopen("lbc1100.log","wt");	// Open log file
#endif
    CpcXXXX::init();

    pTIMER->resetTimer(1);
    pCONNECTOR	= new Cconnector(this,20,0,Cconnector::General_20,"20 pins connector",false,
                                     QPoint(0,72));
    publish(pCONNECTOR);

    WatchPoint.add(&pCONNECTOR_value,64,20,this,"20 pins connector");

    portB = 0;

initcmd = true;

    return true;
}

#define LCD_PORT    (upd7907->upd7907stat.imem[0x00])

bool Clbc1100::run() {


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
        switch(LCD_PORT) {
        case 0x53: // Send to Printer
            sendToPrinter = data;
            upd7907->upd7907stat.imem[0x08] = 0;
            break;
        case 0x52: // Select LCD chip
            if (initcmd) {
                currentLCDctrl = data & 0x03;
                if (pCPU->fp_log) fprintf(pCPU->fp_log,"Select LCD Controler:%02x\n",currentLCDctrl);
                initcmd = false;
            }
            else {
                upd16434[currentLCDctrl]->instruction(data);
            }
            break;
        case 0x50: // Send data to LCD
            if (initcmd) {
                currentLCDctrl = data & 0x03;
                if (pCPU->fp_log) fprintf(pCPU->fp_log,"Select LCD Controler:%02x\n",currentLCDctrl);
                initcmd = false;
            }
            else {
                upd16434[currentLCDctrl]->data(data);
            }
            break;
        case 0x51: initcmd = true;
            break;
        default:
            break;
        }
#if 0
        switch (LCD_PORT>>6) {
        case 0x01:   // LCD transmission
        {
            // flip flop PB1 0-2-0
            //        SET_BIT(portB,1);
            pTIMER->resetTimer(2);
            quint8 currentLCDctrl = LCD_PORT & 0x03;
            quint8 cmddata = (LCD_PORT >> 4) & 0x01;
            switch(cmddata) {
            case 0x01: upd16434[currentLCDctrl]->instruction(data);
                break;
            case 0x00: upd16434[currentLCDctrl]->data(data);
                break;
            }
            upd7907->upd7907stat.imem[0x08] = 0;
        }
            break;
        case 0x00:  // PRINTER PORT
            sendToPrinter = data;
            upd7907->upd7907stat.imem[0x08] = 0;
            break;
        }
#endif

    }

     fillSoundBuffer(upd7907->upd7907stat.to ? 0xff : 0x00);

     pCONNECTOR_value = pCONNECTOR->Get_values();
//     pTAPECONNECTOR_value   = pTAPECONNECTOR->Get_values();
//     pPRINTERCONNECTOR_value = pPRINTERCONNECTOR->Get_values();
    return true;
}

bool Clbc1100::Chk_Adr(UINT32 *d, UINT32 data)
{
    Q_UNUSED(data)

    if(*d < 0x1000) return false; /* ROM */
    if( (*d >= 0x1000) && (*d < 0x4000) ) return true; /* RAM */
    if( (*d >= 0x4000) && (*d < 0x8000) ) return false; /* ROM */
    if( (*d >= 0x8000) && (*d < 0x10000) ) return true; /* ROM */

    return true;
}

bool Clbc1100::Chk_Adr_R(UINT32 *d, UINT32 *data)
{
    Q_UNUSED(d)
    Q_UNUSED(data)

    return true;
}


UINT8 Clbc1100::in(UINT8 Port,QString)
{
    switch (Port) {
    case 0x01 : return portB ;// | (pTAPECONNECTOR->Get_pin(1) ? 0x80 : 0x00); break;
    case 0x02 : return (getKey() & 0x3F); break;
    }

    return 0;
}



UINT8 Clbc1100::out(UINT8 Port, UINT8 x, QString sender)
{
    switch (Port) {
//    case 0x01 : portB = x; break;
    case 0x00: if (x&0x01) initcmd = true; break;
    }

    return 0;
}

UINT16 Clbc1100::out16(UINT16 address, UINT16 value, QString sender)
{
    if (address == UPD7907_PORTE) {
        kstrobe = value;
    }

    return 0;
}

bool Clbc1100::Set_Connector(Cbus *_bus)
{
    Q_UNUSED(_bus)

//    pTAPECONNECTOR->Set_pin(3,true);       // RMT
//    pTAPECONNECTOR->Set_pin(2,upd7907->upd7907stat.imem[0x00] & 0x10 ? 0xff : 0x00);    // Out

    if (sendToPrinter>0) {
        pCONNECTOR->Set_values(sendToPrinter);
        AddLog(LOG_PRINTER,QString("Send Char:%1").arg(sendToPrinter,2,16,QChar('0')));
    }
    else
        pCONNECTOR->Set_values(0);

    return true;
}

bool Clbc1100::Get_Connector(Cbus *_bus)
{
    Q_UNUSED(_bus)

    if (pCONNECTOR->Get_pin(9)) {
        sendToPrinter = 0;
    }

    return true;
}


void Clbc1100::TurnOFF(void) {
    mainwindow->saveAll = YES;
    CpcXXXX::TurnOFF();
    mainwindow->saveAll = ASK;
    AddLog(LOG_TEMP,"TURN OFF");

}

void Clbc1100::TurnON(void){
    CpcXXXX::TurnON();
    upd7907->upd7907stat.pc.w.l=0;

}


void Clbc1100::Reset()
{
    CpcXXXX::Reset();
//    pLCDC->init();
    for (int i=0;i<4;i++) upd16434[i]->Reset();
    sendToPrinter=0;
    initcmd = true;

}

bool Clbc1100::LoadConfig(QXmlStreamReader *xmlIn)
{
    for (int i=0;i<4;i++) upd16434[i]->Load_Internal(xmlIn);
    return true;
}

bool Clbc1100::SaveConfig(QXmlStreamWriter *xmlOut)
{
    for (int i=0;i<4;i++) upd16434[i]->save_internal(xmlOut);
    return true;
}



#define KEY(c)	( pKEYB->keyPressedList.contains(TOUPPER(c)) || pKEYB->keyPressedList.contains(c) || pKEYB->keyPressedList.contains(TOLOWER(c)))

UINT16 Clbc1100::getKey()
{

    UINT16 ks = kstrobe^0xFFFF;
    UINT16 data=0;

    if ((pKEYB->LastKey) && ks )
    {
//        if (fp_log) fprintf(fp_log,"KSTROBE=%04X\n",ks);
        qWarning()<<"key"<<pKEYB->LastKey;

        if (ks&0x01) {
//            if (KEY(K_F1))			data|=0x01;
//            if (KEY(K_F3))			data|=0x02;
//            if (KEY(K_SHT))			data|=0x04;
            if (pKEYB->isShift) data|=0x04;
//            if (KEY(K_F5))			data|=0x08;
//            if (KEY(K_F6))			data|=0x10;
//            if (KEY(K_F7))			data|=0x20;
        }

        if (ks&0x02) {
//            if (KEY(K_F1))			data|=0x01;
            if (KEY('A'))			data|=0x02;
            if (KEY('Q'))			data|=0x04;
            if (KEY(K_0))			data|=0x08;
            if (KEY('1'))			data|=0x10;
            if (KEY('Z'))			data|=0x20;
        }
        if (ks&0x04) {
            if (KEY('X'))			data|=0x01;
            if (KEY('S'))			data|=0x02;
            if (KEY('W'))			data|=0x04;
            if (KEY(K_1))			data|=0x08;
            if (KEY('2'))			data|=0x10;
            if (KEY(K_F1))			data|=0x20; // F1
        }

        if (ks&0x08) {
            if (KEY('C'))			data|=0x01;
            if (KEY('D'))			data|=0x02;
            if (KEY('E'))			data|=0x04;
            if (KEY(K_2))			data|=0x08;
            if (KEY('3'))			data|=0x10;
            if (KEY(K_F2))			data|=0x20;     //F2
//            if (KEY(K_F1))			data|=0x40;     //F2

        }

        if (ks&0x10) {
            if (KEY('V'))			data|=0x01;
            if (KEY('F'))			data|=0x02;
            if (KEY('R'))			data|=0x04;
            if (KEY(K_3))			data|=0x08;
            if (KEY('4'))			data|=0x10;
            if (KEY(K_F3))			data|=0x20;     //F3
        }
        if (ks&0x20) {
            if (KEY('B'))			data|=0x01;
            if (KEY('G'))			data|=0x02;
            if (KEY('T'))			data|=0x04;
            if (KEY(K_4))			data|=0x08;
            if (KEY('5'))			data|=0x10;
            if (KEY(K_F4))			data|=0x20;     //F4
        }
        if (ks&0x40) {
            if (KEY('N'))			data|=0x01;
            if (KEY('H'))			data|=0x02;
            if (KEY('Y'))			data|=0x04;
            if (KEY(K_5))			data|=0x08;
            if (KEY('6'))			data|=0x10;
            if (KEY(','))			data|=0x20;
        }
        if (ks&0x80) {
            if (KEY('M'))			data|=0x01;
            if (KEY('J'))			data|=0x02;
            if (KEY('U'))			data|=0x04;
            if (KEY(K_6))			data|=0x08;
            if (KEY('7'))			data|=0x10;
            if (KEY('.'))			data|=0x20;
        }
        if (ks&0x100) {
            if (KEY(K_SLH))			data|=0x01;
            if (KEY('K'))			data|=0x02;
            if (KEY('I'))			data|=0x04;
            if (KEY(K_7))			data|=0x08;
            if (KEY('8'))			data|=0x10;
            if (KEY('/'))			data|=0x20;
        }
        if (ks&0x200) {
            if (KEY('*'))			data|=0x01;
            if (KEY('L'))			data|=0x02;
            if (KEY('O'))			data|=0x04;
            if (KEY(K_8))			data|=0x08;
            if (KEY('9'))			data|=0x10;
            if (KEY(';'))			data|=0x20;
        }
        if (ks&0x400) {
            if (KEY(K_MIN))			data|=0x01;     // numpad -
            if (KEY('^'))			data|=0x02;
            if (KEY('P'))			data|=0x04;
            if (KEY(K_9))			data|=0x08;
            if (KEY('0'))			data|=0x10;
            if (KEY(':'))			data|=0x20;
        }
        if (ks&0x800) {
            if (KEY('+'))			data|=0x01;
//            if (KEY(K_F5))			data|=0x02;
            if (KEY('@'))			data|=0x04;
            if (KEY(K_COMMA))		data|=0x08;
            if (KEY('-'))			data|=0x10;
            if (KEY(']'))			data|=0x20;
        }
        if (ks&0x1000) {
            if (KEY(K_PT))			data|=0x01;
            if (KEY(' '))			data|=0x02; //???
            if (KEY('\\'))			data|=0x04; // hidden key \|
            if (KEY(K_UA))			data|=0x08;
            if (KEY('['))			data|=0x10;
            if (KEY(K_BLANK))		data|=0x20; // Blank key ( _ shifted)
        }
        if (ks&0x2000) {
//            if (KEY(K_F5))			data|=0x01;
            if (KEY(K_INS))			data|=0x02;
            if (KEY(K_DEL))			data|=0x04;
            if (KEY(K_DA))			data|=0x08;
            if (KEY(K_LA))			data|=0x10;
            if (KEY(K_RA))			data|=0x20;
        }
        if (ks&0x4000) {
//            if (KEY(K_F7))			data|=0x01;
            if (KEY(K_RET))			data|=0x02;
//            if (KEY(K_F6))			data|=0x04;
            if (KEY(K_SML))			data|=0x08;  // KANA ???
            if (KEY(K_CLR))			data|=0x10; // CLR ???
            if (KEY(K_F5))			data|=0x20;
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

