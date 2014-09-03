#include <QtGui>
#include <QString>

#include "common.h"
#include "ti74.h"
#include "tms7000/tms7000.h"
#include "hd44780.h"
#include "Lcdc_ti74.h"
#include "Inter.h"
#include "Keyb.h"
#include "cextension.h"
#include "Lcdc_symb.h"

#include "Connect.h"
#include "dialoganalog.h"

#include "Log.h"



#define STROBE_TIMER 5

Cti74::Cti74(CPObject *parent)	: CpcXXXX(parent)
{								//[constructor]
    setfrequency( (int) 4000000);
    setcfgfname(QString("ti74"));

    SessionHeader	= "TI74PKM";
    Initial_Session_Fname ="ti74.pkm";

    BackGroundFname	= P_RES(":/ti74/ti74.png");
    LcdFname		= P_RES(":/ti74/ti74lcd.png");
    SymbFname		= P_RES(":/ti74/ti74lcd.png");;


    memsize		= 0x40000;
    InitMemValue	= 0x00;

    SlotList.clear();
    SlotList.append(CSlot(52  , 0x0000 ,	""                  , ""	, CSlot::RAM , "RAM"));
    SlotList.append(CSlot(4  , 0xF000 ,	P_RES(":/ti74/tms70c46.bin")  , ""	, CSlot::ROM , "ROM cpu"));
    SlotList.append(CSlot(32 , 0x10000,	P_RES(":/ti74/ti74.bin")        , ""	, CSlot::ROM , "ROM"));
    SlotList.append(CSlot(128 ,0x20000 ,	""                  , ""	, CSlot::RAM , "RAM"));


    setDXmm(204);
    setDYmm(95);
    setDZmm(25);

    setDX(730);
    setDY(340);

    Lcd_X		= 50;
    Lcd_Y		= 60;
    Lcd_DX		= 186;
    Lcd_DY		= 10;
    Lcd_ratio_X	= 2.25;
    Lcd_ratio_Y	= 2.25;

    Lcd_Symb_X	= 50;//(int) (45 * 1.18);
    Lcd_Symb_Y	= 50;//(int) (35 * 1.18);
    Lcd_Symb_DX	= 210;
    Lcd_Symb_DY	= 20;
    Lcd_Symb_ratio_X	= 2;//1.18;
    Lcd_Symb_ratio_Y	= 2;//1.18;

    pLCDC		= new Clcdc_ti74(this);
    pCPU		= new Ctms70c46(this);
    pTIMER		= new Ctimer(this);
    pKEYB		= new Ckeyb(this,"ti74.map");
    pHD44780    = new CHD44780(P_RES(":/cc40/hd44780_a00.bin"),this);


    ioFreq = 0;             // Mandatory for Centronics synchronization
    ptms7000cpu = (Ctms70c20*)pCPU;

    m_sysram[0] = NULL;
    m_sysram[1] = NULL;
}

Cti74::~Cti74() {
}



void Cti74::power_w(UINT8 data)
{
    qWarning()<<"power_w:"<<data;
    // d0: power-on hold latch
    m_power = data & 1;

    // stop running
    if (!m_power)
        ptms7000cpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
}

UINT8 Cti74::sysram_r(UINT16 offset)
{
    // read system ram, based on addressing configured in bus_control_w
    if (offset < m_sysram_end[0] && m_sysram_size[0] != 0)
        return m_sysram[0][offset & (m_sysram_size[0] - 1)];
    else if (offset < m_sysram_end[1] && m_sysram_size[1] != 0)
        return m_sysram[1][(offset - m_sysram_end[0]) & (m_sysram_size[1] - 1)];
    else
        return 0xff;
}

void Cti74::sysram_w(UINT16 offset,UINT8 data)
{
    // write system ram, based on addressing configured in bus_control_w
    if (offset < m_sysram_end[0] && m_sysram_size[0] != 0)
        m_sysram[0][offset & (m_sysram_size[0] - 1)] = data;
    else if (offset < m_sysram_end[1] && m_sysram_size[1] != 0)
        m_sysram[1][(offset - m_sysram_end[0]) & (m_sysram_size[1] - 1)] = data;
}

UINT8 Cti74::bus_control_r()
{
    return m_bus_control;
}

void Cti74::bus_control_w(UINT8 data)
{
    // d0,d1: auto enable clock divider on cartridge memory access (d0: area 1, d1: area 2)

    // d2,d3: system ram addressing
    // 00: 8K, 8K @ $1000-$2fff, $3000-$4fff
    // 01: 8K, 2K @ $1000-$2fff, $3000-$37ff
    // 10: 2K, 8K @ $1000-$17ff, $1800-$37ff
    // 11: 2K, 2K @ $1000-$17ff, $1800-$1fff
    int d2 = (data & 4) ? 0x0800 : 0x2000;
    int d3 = (data & 8) ? 0x0800 : 0x2000;
    m_sysram_end[0] = d3;
    m_sysram_mask[0] = d3 - 1;
    m_sysram_end[1] = d3 + d2;
    m_sysram_mask[1] = d2 - 1;

    // d4,d5: cartridge memory addressing
    // 00: 2K @ $5000-$57ff & $5800-$5fff
    // 01: 8K @ $5000-$6fff & $7000-$8fff
    // 10:16K @ $5000-$8fff & $9000-$cfff
    // 11: 8K @ $1000-$2fff & $3000-$4fff - system ram is disabled

    // d6: auto enable clock divider on system rom access

    // d7: unused?
    m_bus_control = data;
}

bool Cti74::Chk_Adr(UINT32 *d, UINT32 data)
{
    Q_UNUSED(data)


    if ( (*d>=0x0000) && (*d<=0x007F) )	{ return true;	}  // CPU RAM

    // CPU RAM
    if (*d==0x0110) { bus_control_w(data); return false; }
    if (*d==0x0111) { power_w(data); return false; }
    if (*d==0x0115) { fillSoundBuffer((data & 1) ? 0x7f : 0); return false; }
    if (*d==0x0119) {
        RomBank = data & 0x03; /*qWarning()<<"romBank:"<<RomBank;*/
        RamBank = (data >> 2 & 3);
        m_banks = data & 0x0f;
        return true;
    }
    if (*d==0x011A) { clock_w(data); return false; }
    if (*d==0x011E) {
//        qWarning()<<"pHD44780->control_write:"<<data;
        pHD44780->control_write(data);
        pLCDC->redraw = true;
        return false;
    }
    if (*d==0x011F) {
//        qWarning()<<"pHD44780->data_write:"<<data;
        pHD44780->data_write(data);
        pLCDC->redraw = true;
        return false;
    }
    if ( (*d>=0x0100) && (*d<=0x010B) )	{ ptms7000cpu->pf_write(*d-0x100,data); return false;	}

    if ( (*d>=0x0000) && (*d<=0x0FFF) )	{ return true;	}  // CPU RAM
    if ( (*d>=0x1000) && (*d<=0x3FFF) )	{ sysram_w(*d-0x1000,data); return false;	}  // CPU RAM
    if ( (*d>=0x4000) && (*d<=0xBFFF) )	{ *d += 0x1C000 + ( RamBank * 0x8000 );	return false; } // system ROM
    if ( (*d>=0xC000) && (*d<=0xDFFF) )	{ *d += 0x4000 + ( RomBank * 0x2000 );	return false; } // system ROM
    if ( (*d>=0xF000) && (*d<=0xFFFF) )	{ return false;	}                                       // CPU ROM


    return true;
}

bool Cti74::Chk_Adr_R(UINT32 *d, UINT32 *data)
{
    Q_UNUSED(d)
    Q_UNUSED(data)

    if ( (*d>=0x0000) && (*d<=0x007F) )	{ return true;	}  // CPU RAM

    if (*d==0x0110) { *data = bus_control_r(); return false; }
    if (*d==0x0114) { *data = 1; return false; }
    if (*d==0x0116) { *data = 1; return false; }
    if (*d==0x0119) { *data = m_banks; return false; }
    if (*d==0x011A) { *data = clock_r(); return false; }
    if (*d==0x011E) { *data = pHD44780->control_read();
//        qWarning()<<"pHD44780->control_read:"<<*data;
        return false; }
    if (*d==0x011F) { *data = pHD44780->data_read();
//        qWarning()<<"pHD44780->data_read:"<<*data;
        return false; }

    if ( (*d>=0x0100) && (*d<=0x010B) )	{ *data = ptms7000cpu->pf_read(*d-0x100); return false;	}  // CPU RAM

    if ( (*d>=0x1000) && (*d<=0x3FFF) )	{ *data = sysram_r(*d-0x1000); return false;	}  // CPU RAM

    if ( (*d>=0x4000) && (*d<=0xBFFF) )	{ *d += 0x1C000 + ( RamBank * 0x8000 );	return true; } // Cartridge
    if ( (*d>=0xC000) && (*d<=0xDFFF) )	{ *d += 0x4000 + ( RomBank * 0x2000 );	return true; } // system ROM

    return true;
}



UINT8 Cti74::in(UINT8 Port)
{
    UINT8 Value=0;

    switch (Port) {
    case TMS7000_PORTA: // keyboard read
        return getKey();
        break;
    case TMS7000_PORTB: break;
    case TMS7000_PORTC: break;
    case TMS7000_PORTD: break;
    default :break;
    }

     return (Value);
}

UINT8 Cti74::out(UINT8 Port, UINT8 Value)
{
    switch (Port) {
    case TMS7000_PORTA: break;
    case TMS7000_PORTB: // Keyboard strobe
//        qWarning()<<"strobe:"<<Value;
        ks = Value;
        break;
    case TMS7000_PORTC: break;
    case TMS7000_PORTD: break;
    default :break;
    }

    return 0;
}

UINT8 Cti74::clock_r()
{
    return m_clock_control;
}

void Cti74::clock_w(UINT8 data)
{
    // d3: enable clock divider
    if (data & 8)
    {
        if (m_clock_control != (data & 0x0f))
        {
            // d0-d2: clock divider (2.5MHz /3 to /17 in steps of 2)
            double div = (~data & 7) * 2 + 1;
            qWarning()<<"frequency change to "<<2500000/div;
            setfrequency(2500000/div);
//            m_maincpu->set_clock_scale(1 / div);
        }
    }
    else if (m_clock_control & 8)
    {
        // high to low
        setfrequency(2500000);
        qWarning()<<"frequency change to 2500000";
//        m_maincpu->set_clock_scale(1);
    }

    m_clock_control = data & 0x0f;
}

void Cti74::init_sysram(int chip, UINT16 size)
{
    if (m_sysram[chip] == NULL)
    {
        // init to largest possible
        m_sysram[chip] = (UINT8 *)malloc(0x2000*sizeof(UINT8));
    }

//	m_nvram[chip]->set_base(m_sysram[chip], size);
    m_sysram_size[chip] = size;
}

bool Cti74::init()
{
//    pCPU->logsw = true;
#ifndef QT_NO_DEBUG
    pCPU->logsw = false;
#endif
    CpcXXXX::init();
    pHD44780->init();

    init_sysram(0, 0x800); // default to 6KB
    init_sysram(1, 0x800); // "

    initExtension();
    Reset();

    return true;
}

void	Cti74::initExtension(void)
{
    AddLog(LOG_MASTER,"INIT EXT CC-40");
    // initialise ext_MemSlot1
//    ext_MemSlot1 = new CExtensionArray("RAM Slot 1","Add RAM Module");
//    ext_MemSlot2 = new CExtensionArray("RAM Slot 2","Add RAM Module");
//    ext_MemSlot3 = new CExtensionArray("RAM/ROM Slot 3","Add RAM or ROM Module");
//    ext_MemSlot1->setAvailable(ID_FP201,true); ext_MemSlot1->setChecked(ID_FP201,false);
//    ext_MemSlot2->setAvailable(ID_FP201,true); ext_MemSlot2->setChecked(ID_FP201,false);
//    ext_MemSlot3->setAvailable(ID_FP201,true); ext_MemSlot3->setChecked(ID_FP201,false);
//    ext_MemSlot3->setAvailable(ID_FP205,true);
//    ext_MemSlot3->setAvailable(ID_FP231CE,true);

//    addExtMenu(ext_MemSlot1);
//    addExtMenu(ext_MemSlot2);
//    addExtMenu(ext_MemSlot3);
//    extensionArray[0] = ext_MemSlot1;
//    extensionArray[1] = ext_MemSlot2;
//    extensionArray[2] = ext_MemSlot3;
}

bool Cti74::run()
{
    if (pKEYB->LastKey == K_POW_ON) {
        TurnON();
        pKEYB->LastKey = 0;
    }

    CpcXXXX::run();

    return true;
}

void Cti74::Reset()
{
    CpcXXXX::Reset();

    m_clock_control = 0;
    m_power = 1;
    ks = 0;
    m_banks = 0;
    RomBank = RamBank = 0;

    pHD44780->Reset();
}

void Cti74::TurnON()
{
    CpcXXXX::TurnON();
//    pCPU->Reset();
}

void Cti74::TurnOFF()
{
    mainwindow->saveAll = YES;
    CpcXXXX::TurnOFF();
    mainwindow->saveAll = ASK;
}

bool Cti74::SaveConfig(QXmlStreamWriter *xmlOut)
{
    Q_UNUSED(xmlOut)

    pHD44780->save_internal(xmlOut);

    return true;
}

bool Cti74::LoadConfig(QXmlStreamReader *xmlIn)
{
    Q_UNUSED(xmlIn)

    pHD44780->Load_Internal(xmlIn);

    return true;
}


#define KEY(c)	( pKEYB->keyPressedList.contains(TOUPPER(c)) || pKEYB->keyPressedList.contains(c) || pKEYB->keyPressedList.contains(TOLOWER(c)))

//#define KEY(c)	( TOUPPER(pKEYB->LastKey) == TOUPPER(c) )
quint8 Cti74::getKey()
{

    quint8 data=0;

    if ((pKEYB->LastKey>0))
    {


        if (ks & 0x01) {
            if (KEY('1'))			data|=0x01;
            if (KEY('2'))			data|=0x02;
            if (KEY('3'))			data|=0x04;
            if (KEY('4'))			data|=0x08;
            if (KEY('5'))			data|=0x10;
            if (KEY('6'))			data|=0x20;
            if (KEY('7'))			data|=0x40;
            if (KEY('8'))			data|=0x80;
        }
        if (ks & 0x02) {
            if (KEY('Q'))			data|=0x01;
            if (KEY('W'))			data|=0x02;
            if (KEY('E'))			data|=0x04;
            if (KEY('R'))			data|=0x08;
            if (KEY('T'))			data|=0x10;
            if (KEY('Y'))			data|=0x20;
            if (KEY('U'))			data|=0x40;
            if (KEY('I'))			data|=0x80;
        }
        if (ks & 0x04) {
            if (KEY('A'))			data|=0x01;
            if (KEY('S'))			data|=0x02;
            if (KEY('D'))			data|=0x04;
            if (KEY('F'))			data|=0x08;
            if (KEY('G'))			data|=0x10;
            if (KEY('H'))			data|=0x20;
            if (KEY('J'))			data|=0x40;
            if (KEY('K'))			data|=0x80;
        }
        if (ks & 0x08) {
            if (KEY('Z'))			data|=0x01;
            if (KEY('X'))			data|=0x02;
            if (KEY('C'))			data|=0x04;
            if (KEY('V'))			data|=0x08;
            if (KEY('B'))			data|=0x10;
            if (KEY('N'))			data|=0x20;
            if (KEY('M'))			data|=0x40;
            if (KEY(','))			data|=0x80;
        }
        if (ks & 0x10) {
//            if (KEY(''))			data|=0x01;
            if (KEY(' '))			data|=0x02;
//            if (KEY(''))			data|=0x04;
            if (KEY(';'))			data|=0x08;
            if (KEY('L'))			data|=0x10;
            if (KEY('='))			data|=0x20;
            if (KEY('P'))			data|=0x40;
            if (KEY('O'))			data|=0x80;
        }
        if (ks & 0x20) {
            if (KEY('0'))			data|=0x01;
            if (KEY(K_CLR))			data|=0x02;
            if (KEY(K_LA))			data|=0x04;
            if (KEY(K_RA))			data|=0x08;
            if (KEY(K_UA))			data|=0x10;
            if (KEY('/'))			data|=0x20;
            if (KEY(K_DA))			data|=0x40;
            if (KEY('9'))			data|=0x80;
        }
        if (ks & 0x40) {
//            if (KEY(''))			data|=0x01;
            if (KEY('.'))			data|=0x02;
            if (KEY('+'))			data|=0x04;
            if (KEY(K_RET))			data|=0x08;
            if (KEY('-'))			data|=0x10;
//            if (KEY(''))			data|=0x20;
//            if (KEY(''))			data|=0x40;
            if (KEY('*'))			data|=0x80;
        }

        if (ks & 0x80) {
            if (KEY(K_CTRL))		data|=0x01;
            if (KEY(K_SHT))			data|=0x02;
            if (KEY(K_BRK))			data|=0x04;
            if (KEY(K_RUN))			data|=0x08;
//            if (KEY(''))			data|=0x10;
            if (KEY(K_FN))			{
#if 0
                pCPU->logsw = true;
                pCPU->Check_Log();
#else
                data|=0x20;
#endif
            }
            if (KEY(K_POW_OFF))			data|=0x40;
//            if (KEY(''))			data|=0x80;
        }

//        if (fp_log) fprintf(fp_log,"Read key [%02x]: strobe=%02x result=%02x\n",pKEYB->LastKey,ks,data^0xff);

    }

    if (data>0) {

        AddLog(LOG_KEYBOARD,tr("KEY PRESSED=%1").arg(data,2,16,QChar('0')));
    }
    return data;//^0xff;

}

//void Cti74::keyReleaseEvent(QKeyEvent *event)
//{
////if (event->isAutoRepeat()) return;

//    if (pCPU->fp_log) fprintf(pCPU->fp_log,"\nKEY RELEASED= %c\n",event->key());
//    CpcXXXX::keyReleaseEvent(event);
//}


//void Cti74::keyPressEvent(QKeyEvent *event)
//{
////    if (event->isAutoRepeat()) return;

//    if (pCPU->fp_log) fprintf(pCPU->fp_log,"\nKEY PRESSED= %c\n",event->key());
//    CpcXXXX::keyPressEvent(event);
//}

bool Cti74::Get_Connector(void) {


    return true;
}
bool Cti74::Set_Connector(void) {


    return true;
}




