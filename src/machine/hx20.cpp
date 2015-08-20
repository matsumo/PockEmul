


#include <QDebug>

#include "hx20.h"
#include "mc6800/mc6800.h"
#include "Log.h"
#include "Keyb.h"
#include "Inter.h"
#include "lcdc_hx20.h"
#include "Connect.h"
#include "watchpoint.h"
#include "upd16434.h"

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
    setfrequency( (int) 614400);
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

    pLCDC		= new Clcdc_hx20(this,
                                   QRect(420,90,290,90),//192*2,64*2),
                                   QRect());
    pCPU		= new Cmc6800(this);  pmc6301 = (Cmc6800*)pCPU;
    for (int i=0;i<6;i++) {
        upd16434[i]  = new CUPD16434(this,i); //,CUPD16434::UPD07728);
    }
    pTIMER		= new Ctimer(this);
    pKEYB		= new Ckeyb(this,"hx20.map");

    ioFreq = 0;

    int_status = 0;
    int_mask = 0;
    key_intmask = 0;
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
    return true;
}

bool Chx20::run() {

    CpcXXXX::run();

    BYTE _soundData = 0;
//    if((((Cmc6800*)pCPU)->regs.port[0].wreg & 0x08)) {
//        _soundData = (((Cmc6800*)pCPU)->regs.port[0].wreg & 0x10) ? 0xff : 0x00;
////        qWarning()<<_soundData;
//        fillSoundBuffer(_soundData);
//    }



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
        *data = in(*d);
        return false;
    }

    return true;
}

UINT8 Chx20::in(UINT8 addr)
{
    Q_UNUSED(addr)


    switch(addr) {
    case 0x20:
        return pKEYB->Get_KS();
    case 0x22:
        return getKey() & 0xff;
//    case 0x26:
//        // interrupt mask reset in sleep mode
//        if(int_mask) {
//            int_mask = 0;
//            //				update_intr();
//        }
//        break;
    case 0x28:
        // bit6: power switch interrupt flag (0=active)
        // bit7: busy signal of lcd controller (0=busy)
        return 0x50 | 0xa8;
//        return ((key_data >> 8) & 3) | ((int_status & INT_POWER) ? 0 : 0x40) | 0xa8;
    case 0x2a:
    case 0x2b:
//        qWarning()<<"READ "<<addr;
//        if(lcd_clock > 0 && --lcd_clock <= 0) {
//            int c = lcd_select & 7;
//            if(c >= 1 && c <= 6) {
//                lcd_t *block = &lcd[c - 1];
//                if(lcd_select & 8) {
//                    block->bank = lcd_data & 0x40 ? 40 : 0;
//                    block->addr = lcd_data & 0x3f;
//                } else if(block->addr < 40) {
//                    block->buffer[block->bank + block->addr] = lcd_data;
//                    block->addr++;
//                }
//            }
//        }
        break;
//    case 0x2c:
//        // used for interrupt mask setting in sleep mode
//        if(!int_mask) {
//            int_mask = 1;
//            //				update_intr();
//        }
//        break;
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
    //		return ram[addr];
    return 0;

}

UINT8 Chx20::out(UINT8 addr, UINT8 data)
{
    Q_UNUSED(addr)
    Q_UNUSED(data)
    switch(addr) {
    case 0x01: // send to slave
        send_to_slave(data);
        break;
    case 0x20:
        if(pKEYB->Get_KS() != data) {
            pKEYB->Set_KS(data);
            if((int_status & INT_KEYBOARD) && (int_status &= ~INT_KEYBOARD) == 0) {
                pmc6301->write_signal(SIG_CPU_IRQ, int_status ? 1 : 0, 1);
            }
            pmc6301->write_signal(SIG_MC6801_PORT_1, 0x20, 0x20);
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
    mainwindow->saveAll = NO;
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

    ((Cmc6800*)pCPU)->write_signal(SIG_MC6801_PORT_1, 0x78, 0xff);
    ((Cmc6800*)pCPU)->write_signal(SIG_MC6801_PORT_2, 0x9e, 0xff);

    pLCDC->init();
    for (int i=0;i<6;i++) upd16434[i]->Reset();

}

bool Chx20::LoadConfig(QXmlStreamReader *xmlIn)
{
//    for (int i=0;i<6;i++) upd16434[i]->Load_Internal(xmlIn);
    return true;
}

bool Chx20::SaveConfig(QXmlStreamWriter *xmlOut)
{
//    for (int i=0;i<6;i++) upd16434[i]->save_internal(xmlOut);
    return true;
}



#define KEY(c)	( pKEYB->keyPressedList.contains(TOUPPER(c)) || pKEYB->keyPressedList.contains(c) || pKEYB->keyPressedList.contains(TOLOWER(c)))
void Chx20::ComputeKey(KEYEVENT ke, int scancode, QMouseEvent *event)
{
    Q_UNUSED(ke)
    Q_UNUSED(scancode)

    if (ke==KEY_PRESSED) {
        // raise key interrupt
        if(!(int_status & INT_KEYBOARD)) {
            int_status |= INT_KEYBOARD;
            pmc6301->write_signal(SIG_CPU_IRQ, int_status ? 1 : 0, 1);
        }
        pmc6301->write_signal(SIG_MC6801_PORT_1, 0, 0x20);
    }
}

UINT16 Chx20::getKey()
{

    UINT16 ks = kstrobe^0xFFFF;
    UINT16 data=0;

//    if ((pKEYB->LastKey) && ks )
    {
//        if (fp_log) fprintf(fp_log,"KSTROBE=%04X\n",ks);

        if (ks&0x01) {
            if (KEY('0'))			data|=0x01; // OK
            if (KEY('1'))			data|=0x02; // OK
            if (KEY('2'))			data|=0x04; // OK
            if (KEY('3'))			data|=0x08;
            if (KEY('4'))			data|=0x10;
            if (KEY('5'))			data|=0x20;
            if (KEY('6'))			data|=0x40;
            if (KEY('7'))			data|=0x80;
//             n ^= keyboard_map[(uint8_t)'0']  | (keyboard_map[(uint8_t)'1'] << 1) | (keyboard_map[(uint8_t)'2'] << 2)  | (keyboard_map[(uint8_t)'3'] << 3) | (keyboard_map[(uint8_t)'4'] << 4) | (keyboard_map[(uint8_t)'5'] << 5)  | (keyboard_map[(uint8_t)'6'] << 6) | (keyboard_map[(uint8_t)'7'] << 7);

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

//            if (l & 0x02) n ^= keyboard_map[(uint8_t)'8']  | (keyboard_map[(uint8_t)'9'] << 1) |
//                    (keyboard_map[(uint8_t)':'] << 2)  | (keyboard_map[(uint8_t)';'] << 3) |
//                    (keyboard_map[(uint8_t)','] << 4) | (keyboard_map[(uint8_t)'-'] << 5)  |
//                    (keyboard_map[(uint8_t)'.'] << 6) | (keyboard_map[(uint8_t)'/'] << 7);

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

//            if (l & 0x04) n ^= keyboard_map[(uint8_t)'@']  | (keyboard_map[(uint8_t)'a'] << 1) |
//                    (keyboard_map[(uint8_t)'b'] << 2)  | (keyboard_map[(uint8_t)'c'] << 3) |
//                    (keyboard_map[(uint8_t)'d'] << 4) | (keyboard_map[(uint8_t)'e'] << 5)  |
//                    (keyboard_map[(uint8_t)'f'] << 6) | (keyboard_map[(uint8_t)'g'] << 7);

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

//            if (l & 0x08) n ^= keyboard_map[(uint8_t)'h']  | (keyboard_map[(uint8_t)'i'] << 1) |
//                    (keyboard_map[(uint8_t)'j'] << 2)  | (keyboard_map[(uint8_t)'k'] << 3) |
//                    (keyboard_map[(uint8_t)'l'] << 4) | (keyboard_map[(uint8_t)'m'] << 5)  |
//                    (keyboard_map[(uint8_t)'n'] << 6) | (keyboard_map[(uint8_t)'o'] << 7);

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

//            if (l & 0x10) n ^= keyboard_map[(uint8_t)'p']  | (keyboard_map[(uint8_t)'q'] << 1) |
//                    (keyboard_map[(uint8_t)'r'] << 2)  | (keyboard_map[(uint8_t)'s'] << 3) |
//                    (keyboard_map[(uint8_t)'t'] << 4) | (keyboard_map[(uint8_t)'u'] << 5)  |
//                    (keyboard_map[(uint8_t)'v'] << 6) | (keyboard_map[(uint8_t)'w'] << 7);

        }
        if (ks&0x20) {
            if (KEY('X'))			data|=0x01; // OK
            if (KEY('Y'))			data|=0x02; // OK
            if (KEY('Z'))			data|=0x04; // OK
            if (KEY('['))			data|=0x08;
            if (KEY(']'))			data|=0x10;
            if (KEY('\\'))			data|=0x20;
//            if (KEY(''))			data|=0x40;
//            if (KEY(''))			data|=0x80;

//            if (l & 0x20) n ^= keyboard_map[(uint8_t)'x']  | (keyboard_map[(uint8_t)'y'] << 1) |
//                    (keyboard_map[(uint8_t)'z'] << 2)  | (keyboard_map[(uint8_t)'['] << 3) |
//                    (keyboard_map[(uint8_t)']'] << 4) | (keyboard_map[(uint8_t)'\\'] << 5) |
//                    (keyboard_map[0] << 6)            | (keyboard_map[0] << 7);


        }
        if (ks&0x40) {
            if (KEY(K_RET))			data|=0x01; // OK
            if (KEY(' '))			data|=0x02; // OK
            if (KEY(K_TAB))			data|=0x04; // OK
//            if (KEY(''))			data|=0x08;
//            if (KEY(''))			data|=0x10;
//            if (KEY(''))			data|=0x20;
//            if (KEY(''))			data|=0x40;
//            if (KEY(''))			data|=0x80;

//            if (l & 0x40) n ^= keyboard_map[(uint8_t)'\n'] | (keyboard_map[(uint8_t)' '] << 1) |
//                    (keyboard_map[(uint8_t)'\t'] << 2) | (keyboard_map[0] << 3)            |
//                    (keyboard_map[0] << 4)            | (keyboard_map[0] << 5)             |
//                    (keyboard_map[0] << 6)            | (keyboard_map[0] << 7);


        }


//        if (fp_log) fprintf(fp_log,"Read key [%02x]: strobe=%02x result=%02x\n",pKEYB->LastKey,ks,data^0xff);

    }
    return (data^0xff);

}

void Chx20::send_to_main(quint8 val)
{
    ((Cmc6800*)pCPU)->recv_buffer.append(val);
}

void Chx20::send_to_slave(quint8 val)
{
    cmd_buf.append(val);
    quint8 cmd = cmd_buf.at(0);

//	emu->out_debug_log("Command = %2x", cmd);
//	for(int i = 1; i < cmd_buf->count(); i++) {
//		emu->out_debug_log(" %2x", cmd_buf->read_not_remove(i));
//	}
//	emu->out_debug_log("\n");

    switch(cmd) {
    case 0x00: // slave mcpu ready check
    case 0x01: // sets the constants required by slave mcu
    case 0x02: // initialization
        cmd_buf.remove(0,1);
        send_to_main(0x01);
        break;
//	case 0x03: // opens masks for special commands
//		if(cmd_buf->count() == 2) {
//			cmd_buf->read();
//			special_cmd_masked = (cmd_buf->read() != 0xaa);
//		}
//		send_to_main(0x01);
//		break;
//	case 0x04: // closes masks for special commands
//		special_cmd_masked = true;
//		cmd_buf->read();
//		send_to_main(0x01);
//		break;
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
//	case 0x09: // bar-code reader power on
//	case 0x0a: // bar-code reader power off
//		cmd_buf->read();
//		send_to_main(0x01);
//		break;
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
//	case 0x0c: // terminate process
//		cmd_buf->read();
//		send_to_main(0x02);
//		// stop sound
//		d_beep->write_signal(SIG_BEEP_ON, 0, 0);
//		sound_ptr = sound_count;
//		break;
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
//	case 0x10: // prints out 6-dot data (bit0-5) to the built-in printer
//	case 0x11: // feeds the specified number of dot lines to the built-in printer
//		if(cmd_buf->count() == 2) {
//			cmd_buf->clear();
//		}
//		send_to_main(0x01);
//		break;
//	case 0x12: // paper feed operation (1.2sec)
//		cmd_buf->read();
//		send_to_main(0x01);
//		break;
//	case 0x20: // executes external cassette ready check
//		send_to_main(0x21);
//		cmd_buf->read();
//		break;
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
//	case 0x22: // turns the external cassette rem terminal on
//	case 0x23: // turns the external cassette rem terminal off
//		cmd_buf->read();
//		send_to_main(0x01);
//		break;
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
//	case 0x50: // identifies the plug-in option
//		cmd_buf->read();
//		send_to_main(0x02);
//		break;
//	case 0x51: // turns power of plug-in rom cartridge on
//	case 0x52: // turns power of plug-in rom cartridge off
//		cmd_buf->read();
//		send_to_main(0x01);
//		break;
//	case 0x60: // executes micro cassette ready check (no respose)
//		cmd_buf->read();
//		break;
    default:
        // unknown command
//        emu->out_debug_log("Unknown Command = %2x\n", cmd);
        send_to_main(0x0f);
        break;
    }
}

