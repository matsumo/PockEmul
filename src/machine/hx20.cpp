


#include <QDebug>

#include "hx20.h"
#include "mc6800/mc6800.h"
#include "Log.h"
#include "Keyb.h"
#include "Inter.h"
//#include "lcdc_hx20.h"
#include "Connect.h"
#include "watchpoint.h"
#include "upd16434.h"


Chx20::Chx20(CPObject *parent)	: CpcXXXX(parent)
{								//[constructor]
    setfrequency( (int) 614000);
    setcfgfname(QString("hx20"));

    SessionHeader	= "HX20PKM";
    Initial_Session_Fname ="hx20.pkm";

    BackGroundFname	= P_RES(":/hx20/hx20.png");

//    RightFname = P_RES(":/hx20/hx20Right.png");
//    LeftFname = P_RES(":/hx20/hx20Left.png");
//    TopFname = P_RES(":/hx20/hx20Top.png");
//    BackFname = P_RES(":/hx20/hx20Back.png");

    memsize		= 0xFFFF;
    InitMemValue	= 0xFF;


    //ROM_REGION( 0x8000, HD6301V1_MAIN_TAG, ROMREGION_ERASEFF )
    //ROMX_LOAD( "hx20_v11e.12e", 0x0000, 0x2000, CRC(4de0b4b6) SHA1(f15c537824b7effde9d9b9a21e92a081fb089371), ROM_BIOS(3) )
    //ROMX_LOAD( "hx20_v11e.13e", 0x2000, 0x2000, CRC(10d6ae76) SHA1(3163954ed9981f70f590ee98bcc8e19e4be6527a), ROM_BIOS(3) )
    //ROMX_LOAD( "hx20_v11e.14e", 0x4000, 0x2000, CRC(26c203a1) SHA1(b282d7233b2689820fcf718dbe1e93d623b67e4f), ROM_BIOS(3) )
    //ROMX_LOAD( "hx20_v11e.15e", 0x6000, 0x2000, CRC(fd339aa5) SHA1(860c3579c45e96c5e6a877f4fbe77abacb0d674e), ROM_BIOS(3) )

    SlotList.clear();
    SlotList.append(CSlot(16, 0x0000 ,	"", ""	, CSlot::RAM , "RAM"));
    SlotList.append(CSlot(8, 0x8000 ,	P_RES(":/hx20/hx20_v11e.12e"), ""	, CSlot::ROM , "ROM"));
    SlotList.append(CSlot(8, 0xA000 ,	P_RES(":/hx20/hx20_v11e.13e"), ""	, CSlot::ROM , "ROM"));
    SlotList.append(CSlot(8, 0xC000 ,	P_RES(":/hx20/hx20_v11e.14e"), ""	, CSlot::ROM , "ROM"));
    SlotList.append(CSlot(8, 0xE000 ,	P_RES(":/hx20/hx20_v11e.15e"), ""	, CSlot::ROM , "ROM"));

    PowerSwitch	= 0;

    setDXmm(289);
    setDYmm(216);
    setDZmm(44);

    setDX(1006);
    setDY(752);

    PowerSwitch = 0;

//    pLCDC		= new Clcdc_hx20(this,
//                                   QRect(98,94,340,115),//192*2,64*2),
//                                   QRect(86,94,364,115));
    pCPU		= new Cmc6800(this);
    for (int i=0;i<6;i++) {
        upd16434[i]  = new CUPD16434(this);
    }
    pTIMER		= new Ctimer(this);
    pKEYB		= new Ckeyb(this,"hx20.map");

    ioFreq = 0;
}

Chx20::~Chx20() {

}

bool Chx20::init(void)				// initialize
{

//pCPU->logsw = true;
#ifndef QT_NO_DEBUG
//    pCPU->logsw = true;
//    if (!fp_log) fp_log=fopen("hx20.log","wt");	// Open log file
#endif
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

    for (int i=0;i<6;i++) {
        upd16434[i]->init();
    }
    lcd_clk_counter = 0;
    return true;
}

bool Chx20::run() {

    CpcXXXX::run();

    BYTE _soundData = 0;
    if((((Cmc6800*)pCPU)->regs.port[0].wreg & 0x08)) {
        _soundData = (((Cmc6800*)pCPU)->regs.port[0].wreg & 0x10) ? 0xff : 0x00;
//        qWarning()<<_soundData;
        fillSoundBuffer(_soundData);
    }



    pTAPECONNECTOR_value   = pTAPECONNECTOR->Get_values();
    pPRINTERCONNECTOR_value = pPRINTERCONNECTOR->Get_values();
    return true;
}

bool Chx20::Chk_Adr(UINT32 *d, UINT32 data)
{
    Q_UNUSED(data)

    if(*d < 0x40) {
        out(*d,data);
    }
//    else if(addr < 0x80) {
//		d_rtc->write_io8(1, addr & 0x3f);
//		d_rtc->write_io8(0, data);
//	}

    if ((*d>=0x2000) && (*d<=0x7FFF)) {
        return true; /* RAM */
    }

    return false;
}

bool Chx20::Chk_Adr_R(UINT32 *d, UINT32 *data)
{
    Q_UNUSED(d)
    Q_UNUSED(data)

    if(*d < 0x40) {
        *data = in(*d);
        return false;
    }

    return true;
}

UINT8 Chx20::in(UINT8 addr)
{
    Q_UNUSED(addr)



    return 0;
}

UINT8 Chx20::out(UINT8 addr, UINT8 data)
{
    Q_UNUSED(addr)
    Q_UNUSED(data)
    switch(addr) {
//    case 0x20:
//        if(key_strobe != data) {
//            key_strobe = data;
//            update_keyboard();
//        }
//        break;
    case 0x26:
        lcd_select = data & 0x0f;
//        key_intmask = data & 0x10;
//        // interrupt mask reset in sleep mode
//        if(int_mask) {
//            int_mask = 0;
////				update_intr();
//        }
        break;
    case 0x2a:
        if (++lcd_clk_counter == 8) {
            lcd_clk_counter = 0;

            // TODO: n==6 is ignored due to array out-of-bounds. Find out how the real hardware handles this case.
            if (lcd_select & 0x07) {
                quint8 n = (lcd_select & 0x07) - 1;

                if (n < 6) {
                    if (lcd_select & 0x08) {
                        upd16434[n]->instruction(data);
                    }
                    else {
                        upd16434[n]->data(data);
                    }
                }
            }
        }
        break;
//    case 0x2c:
//        // used for interrupt mask setting in sleep mode
//        if(!int_mask) {
//            int_mask = 1;
////				update_intr();
//        }
//        break;
//    case 0x30:
////			SET_BANK(0x4000, 0x7fff, ram + 0x4000, ram + 0x4000);
//        SET_BANK(0x8000, 0xbfff, wdmy, ext);
//        break;
//    case 0x32:
//    case 0x33:
////			SET_BANK(0x4000, 0x7fff, wdmy, rdmy);
//        SET_BANK(0x8000, 0xbfff, wdmy, rom);
//        break;
    }

//    ram[addr] = data;

    return data;
}

UINT16 Chx20::out16(UINT16 address, UINT16 value)
{
    Q_UNUSED(address)
    Q_UNUSED(value)

//    if (address == UPD7907_PORTE) {
//        kstrobe = value;
//    }

    return 0;
}

bool Chx20::Set_Connector(Cbus *_bus)
{
    Q_UNUSED(_bus)

    pTAPECONNECTOR->Set_pin(3,true);       // RMT
    pTAPECONNECTOR->Set_pin(2,(((Cmc6800*)pCPU)->regs.port[0].wreg & 0x01) ? 0x00 : 0xff); // TAPE OUT

//    if (sendToPrinter>0) {
//        pPRINTERCONNECTOR->Set_values(sendToPrinter);
//        AddLog(LOG_PRINTER,QString("Send Char:%1").arg(sendToPrinter,2,16,QChar('0')));
//    }
//    else
//        pPRINTERCONNECTOR->Set_values(0);

    return true;
}

bool Chx20::Get_Connector(Cbus *_bus)
{
    Q_UNUSED(_bus)

//    if (pPRINTERCONNECTOR->Get_pin(9)) {
//        sendToPrinter = 0;
//    }

    return true;
}

void Chx20::TurnOFF(void) {
    mainwindow->saveAll = YES;
    CpcXXXX::TurnOFF();
    mainwindow->saveAll = ASK;
    AddLog(LOG_TEMP,"TURN OFF");

}

void Chx20::TurnON(void){
    CpcXXXX::TurnON();
//    upd7907->Reset();
//    upd7907->upd7907stat.pc.w.l=0;
//qWarning()<<"LCD ON:"<<pLCDC->On;
}

void Chx20::Reset()
{
    CpcXXXX::Reset();

//    pLCDC->init();
//    for (int i=0;i<8;i++) upd16434[i]->Reset();

}

bool Chx20::LoadConfig(QXmlStreamReader *xmlIn)
{
//    for (int i=0;i<8;i++) upd16434[i]->Load_Internal(xmlIn);
    return true;
}

bool Chx20::SaveConfig(QXmlStreamWriter *xmlOut)
{
//    for (int i=0;i<8;i++) upd16434[i]->save_internal(xmlOut);
    return true;
}



#define KEY(c)	( pKEYB->keyPressedList.contains(TOUPPER(c)) || pKEYB->keyPressedList.contains(c) || pKEYB->keyPressedList.contains(TOLOWER(c)))

UINT16 Chx20::getKey()
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



