
#include <assert.h>

#include <QDebug>
#include <QPainter>

#include "hx20.h"
#include "mc6800/mc6800.h"
#include "Log.h"
#include "Keyb.h"
#include "Inter.h"
#include "lcdc_hx20.h"
#include "Connect.h"
#include "watchpoint.h"
#include "upd16434.h"
#include "debug.h"
#include "m160.h"
#include "paperwidget.h"

/*
    memory:
        0002	in	---	port1 (cpu)
        0003	in/out	---	port2 (cpu)
        0006		---	port3 (cpu)
        0007		---	port4 (cpu)
        0020	out	bit0-7	key scan line
        0022	in	bit0-7	key scan result (lo)
        0026	out	bit0-2	selection of lcd driver (0,1-6)
                bit3	output selection for lcd driver (0=data 1=command)
                bit4	key input interrupt mask (0=Mask)
                bit5	pout (serial control line)
                bit6	shift/load select for rom cartridge (0=load 1=shift)
                bit7	clock for rom cartridge
        0028	in	bit0-1	key scan result (hi)
                bit6	power switch interrupt flag (0=active)
                bit7	busy signal of lcd controller (0=busy)
        002a	out	bit0-7	output data to lcd controller
            in	---	serial clock to lcd controller
        002b	in	---	serial clock to lcd controller
        002c	in/out	---	used for interrupt mask setting in sleep mode
        0030	in/out	---	select expansion unit rom (bank1)
        0032	in/out	---	select internal rom (bank0)
        0033	in/out	---	select internal rom (bank0)
        003c	in	---	XXX: unknown

    port1:
        p10	in	dsr (RS-232C)
        p11	in	cts (RS-232C)
        p12	in	error status of slave mcu (P34)
        p13	in	external interrupt flag(0=active)
        p14	in	battery voltage interrupt flag (0=active)
        p15	in	key input inerrupt flag (0=active)
        p16	in	pin (serial control line)
        p17	in	counter status of microcassete / rom data / plug-in option

    port 2:
        p20	in	barcode input signal (1=mark 0=space)
        p21	out	txd (RS-232C)
        p22	out	selection for CPU serial communication (0=slave 1=serial)
*/


#define INT_KEYBOARD	1
#define INT_CLOCK	2
#define INT_POWER	4

Chx20::Chx20(CPObject *parent)	: CpcXXXX(parent)
{								//[constructor]
    setfrequency( (int) 2457600 / 4);
    setcfgfname(QString("hx20"));

    SessionHeader	= "HX20PKM";
    Initial_Session_Fname ="hx20.pkm";

    BackGroundFname	= P_RES(":/hx20/hx20.png");

//    RightFname = P_RES(":/hx20/hx20Right.png");
//    LeftFname = P_RES(":/hx20/hx20Left.png");
//    TopFname = P_RES(":/hx20/hx20Top.png");
//    BackFname = P_RES(":/hx20/hx20Back.png");

    memsize		= 0xFFFF;
    InitMemValue	= 0x00;

    SlotList.clear();
    SlotList.append(CSlot(16, 0x0000 ,	"", ""	, CSlot::RAM , "RAM"));
    SlotList.append(CSlot(8, 0x8000 ,	P_RES(":/hx20/hx20_v11.12e"), ""	, CSlot::ROM , "ROM"));
    SlotList.append(CSlot(8, 0xA000 ,	P_RES(":/hx20/hx20_v11.13e"), ""	, CSlot::ROM , "ROM"));
    SlotList.append(CSlot(8, 0xC000 ,	P_RES(":/hx20/hx20_v11.14e"), ""	, CSlot::ROM , "ROM"));
    SlotList.append(CSlot(8, 0xE000 ,	P_RES(":/hx20/hx20_v11.15e"), ""	, CSlot::ROM , "ROM"));

    PowerSwitch	= 0;

    setDXmm(289);
    setDYmm(216);
    setDZmm(44);

    setDX(1006);
    setDY(752);

    PowerSwitch = 0;

    pLCDC		= new Clcdc_hx20(this,
                                   QRect(420,90,290,90),
                                   QRect());

    pMasterCPU	= new Cmc6800(this);
    pMasterCPU->setObjectName("Master");
    pSlaveCPU	= new Cmc6800(this,7,P_RES(":/hx20/hd6301v1.6d"));
    pSlaveCPU->setObjectName("Slave");
    pCPU = (CCPU*)pMasterCPU;

    for (int i=0;i<6;i++) {
        upd16434[i]  = new CUPD16434(this,i);
    }
    pTIMER		= new Ctimer(this);
    pKEYB		= new Ckeyb(this,"hx20.map");

    pM160 = new Cm160(this);

    ioFreq = 0;

    int_status = 0;
    int_mask = 0;
    key_intmask = 0;
    targetSlave = true;

    printerSW = false;

    lcdBit = true;
}

Chx20::~Chx20() {

}

bool Chx20::init(void)				// initialize
{

//pCPU->logsw = true;
//    pSlaveCPU->logsw = true;
#ifndef QT_NO_DEBUG
//    pCPU->logsw = true;
//    if (!fp_log) fp_log=fopen("hx20.log","wt");	// Open log file
#endif
    CpcXXXX::init();
    pSlaveCPU->init();

    pTIMER->resetTimer(1);

    pTAPECONNECTOR	= new Cconnector(this,3,0,Cconnector::Jack,"Line in / Rec / Rmt",false,
                                     QPoint(804,0),Cconnector::NORTH);
    publish(pTAPECONNECTOR);
    pPRINTERCONNECTOR	= new Cconnector(this,64,1,Cconnector::Custom,"Printer",false);

    pCN8	= new Cconnector(this,14,2,Cconnector::Epson_CN8,"Micro K7 / ROM Cartrifge Connector",true,
                                     QPoint(773,113),Cconnector::EAST);
    publish(pCN8);

    WatchPoint.add(&pTAPECONNECTOR_value,64,3,this,"Line In / Rec");
    WatchPoint.add(&pCN8_value,64,14,this,"Micro K7 / ROM Cartrifge Connector");
//    WatchPoint.add(&pPRINTERCONNECTOR_value,64,9,this,"Printer");

    for (int i=0;i<6;i++) {
        upd16434[i]->init();
    }

   if (pM160) {
       pM160->init();
       pM160->hide();
   }

    Reset();
    return true;
}

void Chx20::run(CCPU *_cpu) {
    CCPU *_keepCPU = pCPU;
    pCPU = _cpu;
    CpcXXXX::run();
    pCPU = _keepCPU;
}

bool Chx20::run() {
    static quint64 masterCount=0;
    static quint64 lastSlaveCount=0;

    BYTE _soundData = (pSlaveCPU->regs.port[0].wreg & 0x20) ? 0xff : 0x00;
    fillSoundBuffer(_soundData);



    quint64 _tmpCount = pTIMER->state;
    run(pMasterCPU);
    quint64 _tmpCount2 = pTIMER->state;
    masterCount += _tmpCount2 - _tmpCount;

    pTIMER->state = lastSlaveCount;
    while (lastSlaveCount < _tmpCount2) {
        // Synchronize slave CPU speed with master CPU speed
        run(pSlaveCPU);
        lastSlaveCount = pTIMER->state;

        Set_PrinterConnector(pM160->pCONNECTOR);
        pM160->run();
        Get_PrinterConnector(pM160->pCONNECTOR);
    }
    // Slave P34 -> Master P12
    pMasterCPU->write_signal(SIG_MC6801_PORT_1,
                          (pSlaveCPU->regs.port[2].wreg & 0x10)?0x04:0x00, 0x04);  // P34


    pTIMER->state = _tmpCount2;
//    qWarning()<<"master:"<<masterCount<<"  slave:"<<slaveCount;



    pTAPECONNECTOR_value   = pTAPECONNECTOR->Get_values();
    pPRINTERCONNECTOR_value = pPRINTERCONNECTOR->Get_values();
    return true;
}


bool Chx20::Chk_Adr(UINT32 *d, UINT32 data)
{
    Q_UNUSED(data)

    if(*d < 0x40) {
        out(*d,data,"Master");
        return true;
    }
    else if(*d < 0x80) {
//		d_rtc->write_io8(1, addr & 0x3f);
//		d_rtc->write_io8(0, data);
        return true;
    }

    if ((*d>=0x0000) && (*d<=0x3FFF)) {
        return true; /* RAM */
    }

    return false;
}

bool Chx20::Chk_Adr_R(UINT32 *d, UINT32 *data)
{
    Q_UNUSED(d)
    Q_UNUSED(data)

    if(*d < 0x40) {
        *data = in(*d,"Master");
        return false;
    }

    return true;
}

UINT8 Chx20::in(UINT8 addr,QString sender)
{
    Q_UNUSED(addr)

    if (sender == "Master") {
        switch(addr) {
        case 0x20:
            return pKEYB->Get_KS();
        case 0x22:
            return key_data & 0xff;
        case 0x26:
            // interrupt mask reset in sleep mode
            if(int_mask) {
                int_mask = 0;
                //				update_intr();
            }
            break;
        case 0x28:
            // bit6: power switch interrupt flag (0=active)
            // bit7: busy signal of lcd controller (0=busy)
            //        return 0x43 | 0xa8;
        {
            bool _bit7 = true;
            if (lcd_select & 0x07) {
                quint8 n = (lcd_select & 0x07);

                if (n > 0) {
                    if (lcd_select & 0x08) {
                    }
                    else {
                        _bit7 = lcdBit ;
                    }
                }
            }
            UINT8 _loc =((key_data >> 8) & 3) |
                        ((int_status & INT_POWER) ? 0 : 0x40) |
                        (_bit7 ? 0x80 : 0x00);
            return _loc;
        }
        case 0x2a:
        case 0x2b:
            lcdBit = true;
            if (lcd_select & 0x07) {
                quint8 n = (lcd_select & 0x07);

                if (n > 0) {
                    if (lcd_select & 0x08) {
//                        qWarning()<<"Read bit instr:"<<_bit7;
                    }
                    else {
                        lcdBit = upd16434[n-1]->getBit();
//                        qWarning()<<"Read bit data:"<<lcdBit;
                    }
                }
            }
            break;
        case 0x2c:
            // used for interrupt mask setting in sleep mode
            if(!int_mask) {
                int_mask = 1;
                //				update_intr();
            }
            break;
            //    case 0x30:
            //        //			SET_BANK(0x4000, 0x7fff, ram + 0x4000, ram + 0x4000);
            //        SET_BANK(0x8000, 0xbfff, ext, rom);
            //        break;
            //    case 0x32:
            //    case 0x33:
            //        //			SET_BANK(0x4000, 0x7fff, wdmy, rdmy);
            //        SET_BANK(0x8000, 0xbfff, wdmy, rom);
            //        break;
        }
    }
    //		return ram[addr];
    return 0;

}

UINT8 Chx20::out(UINT8 addr, UINT8 data, QString sender)
{
    Q_UNUSED(addr)
    Q_UNUSED(data)

    if (sender == "Slave") {
        switch(addr) {
        case 0x01: // send to master
            pMasterCPU->recv_buffer.append(data);
            if(pSlaveCPU->logsw) fprintf(pSlaveCPU->fp_log,"\nSEND TO MASTER\n");
            break;
        }
    }


    if (sender == "Master") {
        switch(addr) {
        case 0x01: // send to slave
            if (targetSlave) {
//                send_to_slave(data);
                // send to real slave CPU too
                pSlaveCPU->recv_buffer.append(data);
                if(pSlaveCPU->logsw) fprintf(pSlaveCPU->fp_log,"\nSEND TO SLAVE\n");
            }
            break;
        case 0x02: // check Serial target (1=serial, 0=Slave)
            targetSlave = !(data & 0x04);
            break;
        case 0x20:
            if(pKEYB->Get_KS() != data) {
                pKEYB->Set_KS(data);
                if((int_status & INT_KEYBOARD) && (int_status &= ~INT_KEYBOARD) == 0) {
                    pMasterCPU->write_signal(SIG_CPU_IRQ, int_status ? 1 : 0, 1);
                }
                pMasterCPU->write_signal(SIG_MC6801_PORT_1, 0x20, 0x20);
                //            qWarning()<<"strobe:"<<data;
                key_data = getKey();
            }
            break;
        case 0x26:
            lcd_select = data & 0x0f;
            key_intmask = data & 0x10;
            // interrupt mask reset in sleep mode
            if(int_mask) {
                int_mask = 0;
                //				update_intr();
            }
            break;
        case 0x2a:
            if (lcd_select & 0x07) {
                quint8 n = (lcd_select & 0x07);

                if (n > 0) {
                    if (lcd_select & 0x08) {
                        upd16434[n-1]->instruction(data);
                    }
                    else {
                        upd16434[n-1]->data(data);
                    }
                }
            }
            break;
        case 0x2c:
            // used for interrupt mask setting in sleep mode
            if(!int_mask) {
                int_mask = 1;
                //				update_intr();
            }
            break;
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

    pTAPECONNECTOR->Set_pin(3,!(pSlaveCPU->regs.port[2].wreg & 0x01));     // P30 RMT
    pTAPECONNECTOR->Set_pin(2,pSlaveCPU->regs.port[2].wreg & 0x08 ); // P33 TAPE OUT

    Set_CN8(pCN8);




//    if (sendToPrinter>0) {
//        pPRINTERCONNECTOR->Set_values(sendToPrinter);
//        AddLog(LOG_PRINTER,QString("Send Char:%1").arg(sendToPrinter,2,16,QChar('0')));
//    }
//    else
//        pPRINTERCONNECTOR->Set_values(0);

    return true;
}

bool Chx20::Set_CN8(Cconnector *_conn) {
    _conn->Set_pin(9,pSlaveCPU->regs.port[3].wreg & 0x10);  // P44
    _conn->Set_pin(8,pSlaveCPU->regs.port[3].wreg & 0x08);  // P43
    _conn->Set_pin(7,pSlaveCPU->regs.port[3].wreg & 0x04);  // P42

    _conn->Set_pin(4,mem[0x26] & 0x80);  // P267
    _conn->Set_pin(3,mem[0x26] & 0x40);  // P266

    return true;
}
bool Chx20::Get_CN8(Cconnector *_conn)
{
    pSlaveCPU->write_signal(SIG_MC6801_PORT_4, _conn->Get_pin(14)?0x40:0x00, 0x40);  // P46 DV0
    pSlaveCPU->write_signal(SIG_MC6801_PORT_2, _conn->Get_pin(12)?0x01:0x00, 0x01);  // P20 DV1
    pMasterCPU->write_signal(SIG_MC6801_PORT_1, _conn->Get_pin(5)?0x80:0x00, 0x80);  // P17

    return true;
}

bool Chx20::Set_PrinterConnector(Cconnector *_conn) {
    _conn->Set_pin(1,pSlaveCPU->regs.port[0].wreg & 0x01);  // P10 H1
    _conn->Set_pin(2,pSlaveCPU->regs.port[0].wreg & 0x02);  // P11 H2
    _conn->Set_pin(3,pSlaveCPU->regs.port[0].wreg & 0x04);  // P12 H3
    _conn->Set_pin(4,pSlaveCPU->regs.port[0].wreg & 0x08);  // P13 H4

    _conn->Set_pin(5,pSlaveCPU->regs.port[0].wreg & 0x10);  // P14 M+
    _conn->Set_pin(6,pSlaveCPU->regs.port[3].wreg & 0x02);  // P41  17 Motor Break

    return true;
}

bool Chx20::Get_Connector(Cbus *_bus)
{
    Q_UNUSED(_bus)

    pSlaveCPU->write_signal(SIG_MC6801_PORT_3, pTAPECONNECTOR->Get_pin(1)?0x04:0x00, 0x04);  // P32 MT IN

    Get_CN8(pCN8);

    return true;
}

bool Chx20::Get_PrinterConnector(Cconnector *_conn)
{

    pSlaveCPU->write_signal(SIG_MC6801_PORT_1, _conn->Get_pin(7)?0x80:0x00, 0x80);  // P17 TS
    pSlaveCPU->write_signal(SIG_MC6801_PORT_1, _conn->Get_pin(8)?0x40:0x00, 0x40);  // P16 RS

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

    pLCDC->init();
    for (int i=0;i<6;i++) upd16434[i]->Reset();

    if (pM160) pM160->Reset();

    pMasterCPU->write_signal(SIG_MC6801_PORT_1, 0x78, 0xff);
    pMasterCPU->write_signal(SIG_MC6801_PORT_2, 0x9e, 0xff);

    pSlaveCPU->Reset();
}

bool Chx20::LoadConfig(QXmlStreamReader *xmlIn)
{
    Q_UNUSED(xmlIn)
//    for (int i=0;i<6;i++) upd16434[i]->Load_Internal(xmlIn);
    return true;
}

bool Chx20::SaveConfig(QXmlStreamWriter *xmlOut)
{
    Q_UNUSED(xmlOut)
//    for (int i=0;i<6;i++) upd16434[i]->save_internal(xmlOut);
    return true;
}



#define KEY(c)	( pKEYB->keyPressedList.contains(TOUPPER(c)) || pKEYB->keyPressedList.contains(c) || pKEYB->keyPressedList.contains(TOLOWER(c)))
void Chx20::ComputeKey(KEYEVENT ke, int scancode, QMouseEvent *event)
{
    Q_UNUSED(ke)
    Q_UNUSED(scancode)
    Q_UNUSED(event)

    if ( (getKey() & 0x1ff) != 0x1ff) {
        // raise key interrupt
        if(!(int_status & INT_KEYBOARD)) {
            int_status |= INT_KEYBOARD;
            pMasterCPU->write_signal(SIG_CPU_IRQ, int_status ? 1 : 0, 1);
        }
        pMasterCPU->write_signal(SIG_MC6801_PORT_1, 0, 0x20);
    }
}

UINT16 Chx20::getKey()
{

    UINT8 ks = pKEYB->Get_KS()^0xff;
    UINT16 data=0;

    UINT8 DipSwitch = 0x07;

    if (ks&0x01) {
        if (KEY('0'))			data|=0x01; // OK
        if (KEY('1'))			data|=0x02; // OK
        if (KEY('2'))			data|=0x04; // OK
        if (KEY('3'))			data|=0x08;
        if (KEY('4'))			data|=0x10;
        if (KEY('5'))			data|=0x20;
        if (KEY('6'))			data|=0x40;
        if (KEY('7'))			data|=0x80;
        if (KEY(K_F1))			data|=0x100;
        if (DipSwitch & 0x01)	data|=0x200; // Deep Switch 1
    }

    if (ks&0x02) {
        if (KEY('8'))			data|=0x01; // OK
        if (KEY('9'))			data|=0x02; // OK
        if (KEY(':'))			data|=0x04; // OK
        if (KEY(';'))			data|=0x08;
        if (KEY(','))			data|=0x10;
        if (KEY('-'))			data|=0x20;
        if (KEY('.'))			data|=0x40;
        if (KEY('/'))			data|=0x80;
        if (KEY(K_F2))			data|=0x100;
        if (DipSwitch & 0x02)	data|=0x200; // Deep Switch 2
    }

    if (ks&0x04) {
        if (KEY('@'))			data|=0x01; // OK
        if (KEY('A'))			data|=0x02; // OK
        if (KEY('B'))			data|=0x04; // OK
        if (KEY('C'))			data|=0x08;
        if (KEY('D'))			data|=0x10;
        if (KEY('E'))			data|=0x20;
        if (KEY('F'))			data|=0x40;
        if (KEY('G'))			data|=0x80;
        if (KEY(K_F3))			{
//            pCPU->logsw = true;
//            pCPU->Check_Log();
            data|=0x100;
        }
        if (DipSwitch & 0x04)	data|=0x200; // Deep Switch 3
    }

    if (ks&0x08) {
        if (KEY('H'))			data|=0x01; // OK
        if (KEY('I'))			data|=0x02; // OK
        if (KEY('J'))			data|=0x04; // OK
        if (KEY('K'))			data|=0x08;
        if (KEY('L'))			data|=0x10;
        if (KEY('M'))			data|=0x20;
        if (KEY('N'))			data|=0x40;
        if (KEY('O'))			data|=0x80;
        if (KEY(K_F4))			data|=0x100;
        if (DipSwitch & 0x08)			data|=0x200; // Deep Switch 4
    }

    if (ks&0x10) {
        if (KEY('P'))			data|=0x01; // OK
        if (KEY('Q'))			data|=0x02; // OK
        if (KEY('R'))			data|=0x04; // OK
        if (KEY('S'))			data|=0x08;
        if (KEY('T'))			data|=0x10;
        if (KEY('U'))			data|=0x20;
        if (KEY('V'))			data|=0x40;
        if (KEY('W'))			data|=0x80;
        if (KEY(K_F5))			data|=0x100;
    }

    if (ks&0x20) {
        if (KEY('X'))			data|=0x01; // OK
        if (KEY('Y'))			data|=0x02; // OK
        if (KEY('Z'))			data|=0x04; // OK
        if (KEY('['))			data|=0x08;
        if (KEY(']'))			data|=0x10;
        if (KEY('\\'))			data|=0x20;
        if (KEY(K_RA))			data|=0x40;
        if (KEY(K_LA))			data|=0x80;
        if (KEY(K_PFEED))		data|=0x100;
        if (KEY(K_SHT))			data|=0x200;
    }

    if (ks&0x40) {
        if (KEY(K_RET))			data|=0x01; // OK
        if (KEY(' '))			data|=0x02; // OK
        if (KEY(K_TAB))			data|=0x04; // OK
        //            if (KEY(''))			data|=0x08;
        //            if (KEY(''))			data|=0x10;
        if (KEY(K_NUM))			data|=0x20; // NUM
        if (KEY(0x15))			data|=0x40; // GRAPH
        if (KEY(K_SML))			data|=0x80;
        //            if (KEY(''))			data|=0x100;
        if (KEY(K_CTRL)) {
            data|=0x200;
            pKEYB->isCtrl = true;
        }
    }

    if (ks&0x80) {
        if (KEY(K_HOME))			data|=0x01; // OK
        if (KEY(K_SCREEN))			data|=0x02; // HOME SCR
        if (KEY(K_BRK))			data|=0x04; // OK
        if (KEY(K_PAUSE))			data|=0x08;
        if (KEY(K_INS))			data|=0x10;
        if (KEY(K_MENU))		data|=0x20; // MENU

        if (KEY(K_PRINT_ON)) 	{ printerSW = true; qWarning()<<"PrtSw:"<<printerSW;}
        if (KEY(K_PRINT_OFF)) 	{ printerSW = false;qWarning()<<"PrtSw:"<<printerSW;}
        if (printerSW) {
            data|=0x200;
        }
    }



//        if (fp_log) fprintf(fp_log,"Read key [%02x]: strobe=%02x result=%02x\n",pKEYB->LastKey,ks,data^0xff);

//        qWarning()<<"getkey:"<<tr("%1 , %2").arg(ks,2,16,QChar('0')).arg(data^0xffff,4,16,QChar('0'));

    return (data ^ 0x03ff);

}

void Chx20::send_to_main(quint8 val)
{
    pMasterCPU->recv_buffer.append(val);
}

void Chx20::send_to_slave(quint8 val)
{
    cmd_buf.append(val);
    quint8 cmd = cmd_buf.first();

    QString _out = QString("Command = %1 : ").arg(cmd,2,16,QChar('0'));
    for(int i = 1; i < cmd_buf.size(); i++) {
        _out += QString("%1 ").arg(cmd_buf.at(i),2,16,QChar('0'));
    }
    qWarning()<<_out;

    switch(cmd) {
    case 0x00: // slave mcpu ready check
    case 0x01: // sets the constants required by slave mcu
    case 0x02: // initialization
        cmd_buf.removeFirst();
        send_to_main(0x01);
        break;
    case 0x03: // opens masks for special commands
        if(cmd_buf.size() == 2) {
            cmd_buf.removeFirst();
            special_cmd_masked = (cmd_buf.takeFirst() != 0xaa);
        }
        send_to_main(0x01);
        break;
    case 0x04: // closes masks for special commands
        special_cmd_masked = true;
        cmd_buf.takeFirst(),
        send_to_main(0x01);
        break;
//	case 0x05: // reads slave mcu memory
//		if(special_cmd_masked) {
//			cmd_buf->read();
//			send_to_main(0x0f);
//			break;
//		}
//		if(cmd_buf->count() == 3) {
//			cmd_buf->read();
//			int ofs = cmd_buf->read() << 8;
//			ofs |= cmd_buf->read();
//			send_to_main(slave_mem[ofs]);
//			break;
//		}
//		send_to_main(0x01);
//		break;
//	case 0x06: // stores slave mcu memory
//	case 0x07: // logical or operation
//	case 0x08: // logical and operation
//		if(special_cmd_masked) {
//			cmd_buf->read();
//			send_to_main(0x0f);
//			break;
//		}
//		if(cmd_buf->count() == 4) {
//			cmd_buf->read();
//			int ofs = cmd_buf->read() << 8;
//			ofs |= cmd_buf->read();
//			if(cmd == 6) {
//				slave_mem[ofs] = cmd_buf->read();
//			} else if(cmd == 7) {
//				slave_mem[ofs] |= cmd_buf->read();
//			} else if(cmd == 8) {
//				slave_mem[ofs] &= cmd_buf->read();
//			}
//		}
//		send_to_main(0x01);
//		break;
    case 0x09: // bar-code reader power on
    case 0x0a: // bar-code reader power off
        cmd_buf.takeFirst();
        send_to_main(0x01);
        break;
//	case 0x0b: // sets the program counter to a specified value
//		if(special_cmd_masked) {
//			cmd_buf->read();
//			send_to_main(0x0f);
//			break;
//		}
//		if(cmd_buf->count() == 3) {
//			cmd_buf->read();
//			int ofs = cmd_buf->read() << 8;
//			ofs |= cmd_buf->read();
//			// todo: implements known routines
//		}
//		send_to_main(0x01);
//		break;
    case 0x0c: // terminate process
        cmd_buf.removeFirst();
        send_to_main(0x02);
        // stop sound
//		d_beep->write_signal(SIG_BEEP_ON, 0, 0);
//		sound_ptr = sound_count;
        break;
//	case 0x0d: // cuts off power supply
//		if(cmd_buf->count() == 2) {
//			cmd_buf->read();
//			if(cmd_buf->read() == 0xaa) {
//				emu->power_off();
//				break;
//			}
//		}
//		send_to_main(0x01);
//		break;
    case 0x10: // prints out 6-dot data (bit0-5) to the built-in printer
    case 0x11: // feeds the specified number of dot lines to the built-in printer
        if(cmd_buf.size() == 2) {
            cmd_buf.clear();
        }
        send_to_main(0x01);
        break;
    case 0x12: // paper feed operation (1.2sec)
        cmd_buf.removeFirst();
        send_to_main(0x01);
        break;
    case 0x20: // executes external cassette ready check
        send_to_main(0x21);
        cmd_buf.takeFirst();
        break;
//	case 0x21: // sets constants for the external cassette
//		if(cmd_buf->count() == 1) {
//			send_to_main(0x01);
//			break;
//		}
//		if(cmd_buf->count() == 9) {
//			cmd_buf->clear();
//		}
//		send_to_main(0x21);
//		break;
    case 0x22: // turns the external cassette rem terminal on
    case 0x23: // turns the external cassette rem terminal off
        cmd_buf.takeFirst();
        send_to_main(0x01);
        break;
//	case 0x24: // writes 1 block of data in EPSON format
//		if(cmd_buf->count() == 1) {
//			send_to_main(0x01);
//			break;
//		}
//		if(cmd_buf->count() >= 5 && cmd_buf->count() == cmd_buf->read_not_remove(3) * 256 + cmd_buf->read_not_remove(4) + 5) {
//			if(cmt_rec) {
//				for(int i = 0; i < 5; i++) {
//					cmd_buf->read();
//				}
//				while(!cmd_buf->empty()) {
//					cmt_buffer[cmt_count++] = cmd_buf->read();
//					if(cmt_count >= CMT_BUFFER_SIZE) {
//						cmt_fio->Fwrite(cmt_buffer, cmt_count, 1);
//						cmt_count = 0;
//					}
//				}
//			} else {
//				cmd_buf->clear();
//			}
//		}
//		send_to_main(0x21);
//		break;
//	case 0x25: // outputs number of ff patterns
//		if(cmd_buf->count() == 1) {
//			send_to_main(0x01);
//			break;
//		}
//		if(cmd_buf->count() == 3) {
//			cmd_buf->clear();
//		}
//		send_to_main(0x21);
//		break;
//	case 0x26: // inputs files from the external cassette
//	case 0x27: // inputs files from the external cassette
//	case 0x28: // inputs files from the external cassette
//		if(cmd_buf->count() == 1) {
//			send_to_main(0x01);
//			break;
//		}
//		if(cmd_buf->count() == 5) {
//			int len = cmd_buf->read_not_remove(3) * 256 + cmd_buf->read_not_remove(4);
//			cmd_buf->clear();
//			send_to_main(0x21);
//			for(int i = 0; i < len; i++) {
//				send_to_main(cmt_buffer[cmt_count++]);
//			}
//			// ???
//			send_to_main(0x01);
//			break;
//		}
//		send_to_main(0x21);
//		break;
//	case 0x2b: // specifies the input signal for the external cassette
//		if(cmd_buf->count() == 1) {
//			send_to_main(0x01);
//			break;
//		}
//		if(cmd_buf->count() == 2) {
//			cmd_buf->clear();
//		}
//		send_to_main(0x21);
//		break;
//	case 0x30: // specifies the tone and duration and sounds the piezo speaker
//		if(cmd_buf->count() == 1) {
//			send_to_main(0x01);
//			break;
//		}
//		if(cmd_buf->count() == 3) {
//			cmd_buf->read();
//			int tone = cmd_buf->read();
//			int period = cmd_buf->read();
//			if(tone >= 0 && tone <= 56 && period != 0) {
//				sound[0].freq = tone_table[tone];
//				sound[0].period = CPU_CLOCKS * period / 256 / 10;
//				sound[0].remain = sound[0].period;
//				sound_ptr = 0;
//				sound_count = 1;
//				sound_reply = 0x31;
//				break;
//			}
//		}
//		send_to_main(0x31);
//		break;
//	case 0x31: // specifies the frequency and duration and sounds the piezo speaker
//		if(cmd_buf->count() == 1) {
//			send_to_main(0x01);
//			break;
//		}
//		if(cmd_buf->count() == 5) {
//			cmd_buf->read();
//			int freq = cmd_buf->read() << 8;
//			freq |= cmd_buf->read();
//			int period = cmd_buf->read() << 8;
//			period |= cmd_buf->read();
//			if(freq != 0 && period != 0) {
//				sound[0].freq = CPU_CLOCKS / freq / 2.0;
//				sound[0].period = period;
//				sound[0].remain = sound[0].period;
//				sound_ptr = 0;
//				sound_count = 1;
//				sound_reply = 0x31;
//				break;
//			}
//		}
//		send_to_main(0x31);
//		break;
//	case 0x32: // sounds the speaker for 0.03 sec at tone 6
//	case 0x33: // sounds the speaker for 1 sec at tone 20
//		cmd_buf->read();
//		if(cmd == 0x32) {
//			sound[0].freq = tone_table[6];
//			sound[0].period = CPU_CLOCKS * 3 / 256 / 100;
//		} else {
//			sound[0].freq = tone_table[20];
//			sound[0].period = CPU_CLOCKS / 256;
//		}
//		sound[0].remain = sound[0].period;
//		sound_ptr = 0;
//		sound_count = 1;
//		sound_reply = 0x01;
//		break;
//	case 0x34: // sets melody data in the slave mcu
//		if(cmd_buf->count() == 1) {
//			send_to_main(0x01);
//			break;
//		}
//		if(val == 0xff) {
//			cmd_buf->read();
//			sound_count = 0;
//			while(!cmd_buf->empty()) {
//				int tone = cmd_buf->read();
//				int period = cmd_buf->read();
//				if(tone >= 0 && tone <= 56 && period != 0) {
//					sound[sound_count].freq = tone_table[tone];
//					sound[sound_count].period = CPU_CLOCKS * period / 256 / 10;
//					sound_count++;
//				}
//			}
//			sound_ptr = sound_count;
//		}
//		send_to_main(0x31);
//		break;
//	case 0x35: // sounds the melody data specified in command 34
//		if(sound_count) {
//			sound[0].remain = sound[0].period;
//			sound_ptr = 0;
//			sound_reply = 0x01;
//			break;
//		}
//		send_to_main(0x01);
//		break;
//	case 0x40: // turns the serial driver on
//	case 0x41: // turns the serial driver off
//		cmd_buf->read();
//		send_to_main(0x01);
//		break;
//	case 0x48: // sets the polynomial expression used for CRC check
//		if(cmd_buf->count() == 1) {
//			send_to_main(0x01);
//			break;
//		}
//		if(cmd_buf->count() == 3) {
//			cmd_buf->clear();
//		}
//		send_to_main(0x41);
//		break;
    case 0x50: // identifies the plug-in option
        cmd_buf.removeFirst();
        send_to_main(0x02);
        break;
//	case 0x51: // turns power of plug-in rom cartridge on
//	case 0x52: // turns power of plug-in rom cartridge off
//		cmd_buf->read();
//		send_to_main(0x01);
//		break;
    case 0x60: // executes micro cassette ready check (no respose)
        cmd_buf.removeFirst();
        break;
    default:
        // unknown command
        qWarning()<<tr("Unknown Slave CPU Command = %1").arg(cmd,2,16,QChar('0'));
        send_to_main(0x0f);
        break;
    }
}

bool Chx20::UpdateFinalImage(void) {

    assert(FinalImage!=0);
    assert(pM160 != 0);
    assert(pM160->paperWidget != 0);

    CpcXXXX::UpdateFinalImage();

    QPainter painter;

    painter.begin(FinalImage);

    float ratio = ( (float) pM160->paperWidget->width() ) / ( pM160->paperWidget->bufferImage->width() - pM160->paperWidget->getOffset().x() );

    QRect source = QRect( QPoint(pM160->paperWidget->getOffset().x() ,
                                 pM160->paperWidget->getOffset().y()  - pM160->paperWidget->height() / ratio ) ,
                          QPoint(pM160->paperWidget->bufferImage->width(),
                                 pM160->paperWidget->getOffset().y() /*+10*/)
                          );
//    MSG_ERROR(QString("%1 - %2").arg(source.width()).arg(PaperPos().width()));
    int _x = pM160->PaperPos().x() * internalImageRatio;
    int _y = pM160->PaperPos().y() * internalImageRatio;
    painter.drawImage(_x , _y,
                      pM160->paperWidget->bufferImage->copy(source).scaled(pM160->PaperPos().size()*internalImageRatio,Qt::IgnoreAspectRatio, Qt::SmoothTransformation )
                      );

    painter.end();

    return true;
}
void Chx20::contextMenuEvent(QContextMenuEvent *e)
{
    if (pM160->PaperPos().contains(e->pos())) {
        pM160->contextMenuEvent(e);
    }
    else
        CpcXXXX::contextMenuEvent(e);

    e->accept();
}
