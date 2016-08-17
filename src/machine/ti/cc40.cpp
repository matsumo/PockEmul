#include <QtGui>
#include <QString>

#include "common.h"
#include "fluidlauncher.h"

#include "cc40.h"
#include "tms7000/tms7000.h"
#include "hd44780.h"
#include "Lcdc_cc40.h"
#include "Inter.h"
#include "Keyb.h"
#include "cextension.h"
#include "Lcdc_symb.h"

#include "Connect.h"
#include "dialoganalog.h"

#include "Log.h"

/*
CC-40 MEMORY MAP1:


    DECIMAL    HEX     DESCRIPTION         SIZE
   -------------------------------------------------------------------------
       0      0000
                      REGISTER FILE       128 BYTES PROCESSOR RAM
     127      007F
   -------------------------------------------------------------------------
     128      0080
                      UNUSED              128 BYTES
     255      00FF
   -------------------------------------------------------------------------
     256      0100
                      PERIPHERAL FILE     256 BYTES MEMORY MAP I/O
     511      01FF
   -------------------------------------------------------------------------
     512      0200
                      UNUSED              1.5K
    2047      07FF
   -------------------------------------------------------------------------
    2048      0800
                      SYSTEM RAM          2K INSTALLED
    4095      0FFF
   -------------------------------------------------------------------------
    4096      1000
                      SYSTEM RAM          2K INSTALLED
    6143      17FF
   -------------------------------------------------------------------------
    6144      1800
                      SYSTEM RAM          6K NOT INSTALLED
   12287      2FFF
   -------------------------------------------------------------------------
   12288      3000
                      SYSTEM RAM          2K INSTALLED
   14335      37FF
   -------------------------------------------------------------------------
   14336      3800
                      SYSTEM RAM          6K NOT INSTALLED
   20479      4FFF
   -------------------------------------------------------------------------
   20480      5000
                      CARTRIDE PORT       32K
   53247      CFFF
   -------------------------------------------------------------------------
   53248      D000
                      SYSTEM ROM          8K
   61439      EFFF
   -------------------------------------------------------------------------
   61440      F000
                      UNUSED              2K
   63487      F7FF
   -------------------------------------------------------------------------
   63488      F800
                      PROCESSOR ROM       2K
   65536      FFFF
   -------------------------------------------------------------------------

============================================================================

   CC-40 MEMORY MAP: AFTER 12K RAM ADDED:

    DECIMAL    HEX     DESCRIPTION         SIZE
   -------------------------------------------------------------------------
       0      0000
                      REGISTER FILE       128 BYTES PROCESSOR RAM
     127      007F
   -------------------------------------------------------------------------
     128      0080
                      UNUSED              128 BYTES
     255      00FF
   -------------------------------------------------------------------------
     256      0100
                      PERIPHERAL FILE     256 BYTES MEMORY MAP I/O
     511      01FF
   -------------------------------------------------------------------------
     512      0200
                      UNUSED              1.5K
    2047      07FF
   -------------------------------------------------------------------------
    2048      0800
                      SYSTEM RAM          2K INSTALLED
    4095      0FFF
   -------------------------------------------------------------------------
    4096      1000
                      SYSTEM RAM          8K INSTALLED
   12287      2FFF
   -------------------------------------------------------------------------
   12288      3000
                      SYSTEM RAM          8K INSTALLED
   20479      4FFF
   -------------------------------------------------------------------------
   20480      5000
                      CARTRIDE PORT       32K
   53247      CFFF
   -------------------------------------------------------------------------
   53248      D000
                      SYSTEM ROM          8K
   61439      EFFF
   -------------------------------------------------------------------------
   61440      F000
                      UNUSED              2K
   63487      F7FF
   -------------------------------------------------------------------------
   63488      F800
                      PROCESSOR ROM       2K
   65536      FFFF
   -------------------------------------------------------------------------

===========================================================================
    */

#define STROBE_TIMER 5

Ccc40::Ccc40(CPObject *parent)	: CpcXXXX(parent)
{								//[constructor]
    setfrequency( (int) 2500000);
    setcfgfname(QString("cc40"));

    SessionHeader	= "CC40PKM";
    Initial_Session_Fname ="cc40.pkm";

    BackGroundFname	= P_RES(":/cc40/cc40.png");
//    LcdFname		= P_RES(":/cc40/cc40lcd.png");
//    SymbFname		= P_RES(":/cc40/cc40lcd.png");



    memsize		= 0x40000;
    InitMemValue	= 0x00;

    SlotList.clear();
    SlotList.append(CSlot(52  , 0x0000 ,	""                  , ""	, CSlot::RAM , "RAM"));
    SlotList.append(CSlot(2  , 0xF800 ,	P_RES(":/cc40/cc40_2krom.bin")  , ""	, CSlot::ROM , "ROM cpu"));
    SlotList.append(CSlot(32 , 0x10000,	P_RES(":/cc40/cc40.bin")        , ""	, CSlot::ROM , "ROM"));
    SlotList.append(CSlot(128 , 0x20000 ,	""                  , ""	, CSlot::RAM , "RAM"));


    setDXmm(236);
    setDYmm(147);
    setDZmm(24);

    setDX(845);
    setDY(525);

    pLCDC		= new Clcdc_cc40(this,
                                 QRect(50,175,186*2.7,10*2.7),
                                 QRect(50,160,250,30));
    pLCDC->Lcd_Symb_ratio_X = 2;
    pLCDC->Lcd_Symb_ratio_Y = 2;

    pCPU		= new Ctms70c20(this);
    pTIMER		= new Ctimer(this);
    pKEYB->setMap("cc40.map");
    pHD44780    = new CHD44780(P_RES(":/cc40/hd44780_a00.bin"),this);


    ioFreq = 0;             // Mandatory for Centronics synchronization
    ptms7000cpu = (Ctms70c20*)pCPU;

    m_sysram[0] = NULL;
    m_sysram[1] = NULL;
}

Ccc40::~Ccc40() {
}



void Ccc40::power_w(UINT8 data)
{
    qWarning()<<"power_w:"<<data;
    // d0: power-on hold latch
    m_power = data & 1;

    // stop running
    if (!m_power) {
        TurnOFF();
        ptms7000cpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
    }
}

UINT8 Ccc40::sysram_r(UINT16 offset)
{
    // read system ram, based on addressing configured in bus_control_w
    if (offset < m_sysram_end[0] && m_sysram_size[0] != 0)
        return m_sysram[0][offset & (m_sysram_size[0] - 1)];
    else if (offset < m_sysram_end[1] && m_sysram_size[1] != 0)
        return m_sysram[1][(offset - m_sysram_end[0]) & (m_sysram_size[1] - 1)];
    else
        return 0xff;
}

void Ccc40::sysram_w(UINT16 offset,UINT8 data)
{
    // write system ram, based on addressing configured in bus_control_w
    if (offset < m_sysram_end[0] && m_sysram_size[0] != 0)
        m_sysram[0][offset & (m_sysram_size[0] - 1)] = data;
    else if (offset < m_sysram_end[1] && m_sysram_size[1] != 0)
        m_sysram[1][(offset - m_sysram_end[0]) & (m_sysram_size[1] - 1)] = data;
}

UINT8 Ccc40::bus_control_r()
{
    return m_bus_control;
}

void Ccc40::bus_control_w(UINT8 data)
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

bool Ccc40::Chk_Adr(UINT32 *d, UINT32 data)
{
    Q_UNUSED(data)


    if (*d<=0x007F)	{ return true;	}  // CPU RAM

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
        pLCDC->updated = true;
        return false;
    }
    if (*d==0x011F) {
//        qWarning()<<"pHD44780->data_write:"<<data;
        pHD44780->data_write(data);
        pLCDC->updated = true;
        return false;
    }
    if ( (*d>=0x0100) && (*d<=0x010B) )	{ ptms7000cpu->pf_write(*d-0x100,data); return false;	}

    if (                 (*d<=0x0FFF) )	{ return true;	}  // CPU RAM
    if ( (*d>=0x1000) && (*d<=0x4FFF) )	{ sysram_w(*d-0x1000,data); return false;	}  // CPU RAM
    if ( (*d>=0x5000) && (*d<=0xCFFF) )	{ *d += 0x1B000 + ( RamBank * 0x8000 );
        return (SlotList[3].getType()==CSlot::RAM ? true: false); } // system ROM
    if ( (*d>=0xD000) && (*d<=0xEFFF) )	{ *d += 0x3000 + ( RomBank * 0x2000 );	return false; } // system ROM
    if ( (*d>=0xF800) && (*d<=0xFFFF) )	{ return false;	}                                       // CPU ROM


    return true;
}

bool Ccc40::Chk_Adr_R(UINT32 *d, UINT32 *data)
{
    Q_UNUSED(d)
    Q_UNUSED(data)

    if (*d<=0x007F)	{ return true;	}  // CPU RAM

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

    if ( (*d>=0x1000) && (*d<=0x4FFF) )	{ *data = sysram_r(*d-0x1000); return false;	}  // CPU RAM

    if ( (*d>=0x5000) && (*d<=0xCFFF) )	{ *d += 0x1B000 + ( RamBank * 0x8000 );	return true; } // Cartridge
    if ( (*d>=0xD000) && (*d<=0xEFFF) )	{ *d += 0x3000 + ( RomBank * 0x2000 );	return true; } // system ROM

    return true;
}



UINT8 Ccc40::in(UINT8 Port,QString)
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

UINT8 Ccc40::out(UINT8 Port, UINT8 Value, QString sender)
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

UINT8 Ccc40::clock_r()
{
    return m_clock_control;
}

void Ccc40::clock_w(UINT8 data)
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

void Ccc40::init_sysram(int chip, UINT16 size)
{
    if (m_sysram[chip] == NULL)
    {
        // init to largest possible
        m_sysram[chip] = (UINT8 *)malloc(0x2000*sizeof(UINT8));
    }

//	m_nvram[chip]->set_base(m_sysram[chip], size);
    m_sysram_size[chip] = size;
}

bool Ccc40::init()
{
//    pCPU->logsw = true;
#ifndef QT_NO_DEBUG
    pCPU->logsw = false;
#endif
    initExtension();
    pHD44780->init();

    init_sysram(0, 0x2000); // default to 6KB
    init_sysram(1, 0x2000); // "

    CpcXXXX::init();

    Reset();

    return true;
}

void	Ccc40::initExtension(void)
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

bool Ccc40::run()
{
    if (pKEYB->LastKey == K_POW_ON) {
        TurnON();
        pKEYB->LastKey = 0;
    }

    CpcXXXX::run();

    return true;
}

void Ccc40::Reset()
{
    m_clock_control = 0;
    m_power = 1;
    setPower(true);
    ks = 0;
    m_banks = 0;
    RomBank = RamBank = 0;

    pHD44780->Reset();
    CpcXXXX::Reset();
}

void Ccc40::TurnON()
{
    CpcXXXX::TurnON();
    m_power = 1;
    pCPU->Reset();
}

void Ccc40::TurnOFF()
{
    ASKYN _tmp = mainwindow->saveAll;
    mainwindow->saveAll = YES;
    CpcXXXX::TurnOFF();
    mainwindow->saveAll = _tmp;
}

bool Ccc40::SaveConfig(QXmlStreamWriter *xmlOut)
{
    Q_UNUSED(xmlOut)

    pHD44780->save_internal(xmlOut);

    return true;
}

bool Ccc40::LoadConfig(QXmlStreamReader *xmlIn)
{
    Q_UNUSED(xmlIn)

    pHD44780->Load_Internal(xmlIn);

    return true;
}



quint8 Ccc40::getKey()
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
#if 1
                pCPU->logsw = true;
                pCPU->Check_Log();
#else
                data|=0x20;
#endif
            }
            if (KEY(K_OFF))			data|=0x40;
//            if (KEY(''))			data|=0x80;
        }

//        if (fp_log) fprintf(fp_log,"Read key [%02x]: strobe=%02x result=%02x\n",pKEYB->LastKey,ks,data^0xff);

    }

    if (data>0) {

        AddLog(LOG_KEYBOARD,tr("KEY PRESSED=%1").arg(data,2,16,QChar('0')));
    }
    return data;//^0xff;

}

//void Ccc40::keyReleaseEvent(QKeyEvent *event)
//{
////if (event->isAutoRepeat()) return;

//    if (pCPU->fp_log) fprintf(pCPU->fp_log,"\nKEY RELEASED= %c\n",event->key());
//    CpcXXXX::keyReleaseEvent(event);
//}


//void Ccc40::keyPressEvent(QKeyEvent *event)
//{
////    if (event->isAutoRepeat()) return;

//    if (pCPU->fp_log) fprintf(pCPU->fp_log,"\nKEY PRESSED= %c\n",event->key());
//    CpcXXXX::keyPressEvent(event);
//}

bool Ccc40::Get_Connector(Cbus *_bus) {


    return true;
}
bool Ccc40::Set_Connector(Cbus *_bus) {


    return true;
}

extern int ask(QWidget *parent,QString msg,int nbButton);
void Ccc40::ComputeKey(KEYEVENT ke, int scancode, QMouseEvent *event)
{
    Q_UNUSED(ke)
    Q_UNUSED(scancode)


    int _slot = -1;
    if (KEY(0x240)) {
        _slot = 3;
        pKEYB->keyPressedList.remove(0x240);
        if (m_power == 1) {
            ask(this, "Please turn off the pocket before unplugging the cartridge.",1);
            return;
        }
    }

    if (_slot == -1) return;
    int _response = 0;
    BYTE* capsule = &mem[0x20000];
    if (!SlotList[_slot].isEmpty() || (capsule[0]!=0x00)) {
        _response=ask(this,
                      "The "+SlotList[_slot].getLabel()+ " capsule is already plugged in this slot.\nDo you want to unplug it ?",
                      2);
    }

    if (_response == 1) {
        SlotList[_slot].setEmpty(true);

        memset((void *)capsule ,0x00,0x20000);
        SlotList[_slot].setLabel(QString("ROM bank %1").arg(_slot+1));

    }
    if (_response==2) return;
    currentSlot = _slot;
    FluidLauncher *launcher = new FluidLauncher(mainwindow,
                                                QStringList()<<P_RES(":/pockemul/configExt.xml"),
                                                FluidLauncher::PictureFlowType,QString(),
                                                "CC-40_Cartridge");
    connect(launcher,SIGNAL(Launched(QString,CPObject *)),this,SLOT(addModule(QString,CPObject *)));
    launcher->show();

}

void Ccc40::addModule(QString item,CPObject *pPC)
{
    Q_UNUSED(pPC)

    qWarning()<<"Add Module:"<< item;
    bool load = false;
    if ( currentSlot!=3) return;

    int _res = 0;
    QString moduleName;
//    if (item=="PASCALCC40") moduleName = P_RES(":/cc40/SnapBasic.bin");
    if (item=="MEMOCC40")   { load = true; moduleName = P_RES(":/cc40/cc40_memoprocessor.bin"); }
    if (item=="FINANCECC40") { load = true; moduleName = P_RES(":/cc40/cc40_finance.bin"); }
    if (item=="STATCC40")   { load = true; moduleName = P_RES(":/cc40/cc40_statistics.bin"); }
    if (item=="MATHCC40")   { load = true; moduleName = P_RES(":/cc40/cc40_mathematics.bin"); }
    if (item=="GAMESCC40")  { load = true; moduleName = P_RES(":/cc40/cc40_games1.bin"); }
    if (item=="RAM32KCC40") {
        load = false;
        SlotList[currentSlot].setEmpty(false);
        SlotList[currentSlot].setResID("32Ko RAM module");
        SlotList[currentSlot].setType(CSlot::RAM);
        slotChanged = true;
    }
    if (item=="CC40FILE") {
        moduleName = QFileDialog::getOpenFileName(
                    mainwindow,
                    tr("Choose a Capsule file"),
                    ".",
                    tr("Module File (*.bin)"));
        load = true;
//        customModule = CSlot::CUSTOM_ROM;
    }

    if (!load) return;

    bool result = true; // check this is a true capsule

    qWarning()<<"loaded:"<<_res;
    if (result) {
        SlotList[currentSlot].setEmpty(false);
        SlotList[currentSlot].setResID(moduleName);
        Mem_Load(currentSlot);
        // Analyse capsule
        // 0x01 = 'C'
        // 0x01 - 0x28 : Copyright
        // 0x2C : title lenght
        // 0x2D - .. : title

        BYTE* capsule = &mem[currentSlot*0x4000];
        if (capsule[1]=='C') {
            QString copyright = QString::fromLocal8Bit(QByteArray((const char*)&capsule[1],0x26));
            QString title  = QString::fromLocal8Bit(QByteArray((const char*)&capsule[0x2d],capsule[0x2c]));
            qWarning()<<"title:"<<title;
            SlotList[currentSlot].setLabel(title);
        }

        slotChanged = true;
    }

    currentSlot = -1;

}
