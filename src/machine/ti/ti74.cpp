#include <QtGui>
#include <QString>

#include "common.h"
#include "fluidlauncher.h"

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

    memsize		= 0x20000;
    InitMemValue	= 0x00;

    SlotList.clear();
    SlotList.append(CSlot(60  , 0x0000 ,	""                  , ""	, CSlot::RAM , "RAM"));
    SlotList.append(CSlot(32  , 0x4000 ,	""                  , ""	, CSlot::RAM , "CARTRIDGE"));
    SlotList.append(CSlot(4  , 0xF000 ,	P_RES(":/ti74/tms70c46.bin")  , ""	, CSlot::ROM , "ROM cpu"));
    SlotList.append(CSlot(32 , 0x10000,	P_RES(":/ti74/ti74.bin")        , ""	, CSlot::ROM , "ROM"));

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

    Lcd_Symb_X	= 50;
    Lcd_Symb_Y	= 50;
    Lcd_Symb_DX	= 210;
    Lcd_Symb_DY	= 23;
    Lcd_Symb_ratio_X	= 2;
    Lcd_Symb_ratio_Y	= 2;

    pLCDC		= new Clcdc_ti74(this);
    pCPU		= new Ctms70c46(this);
    pTIMER		= new Ctimer(this);
    pKEYB		= new Ckeyb(this,"ti74.map");
    pHD44780    = new CHD44780(P_RES(":/cc40/hd44780_a00.bin"),this);

    ioFreq = 0;             // Mandatory for Centronics synchronization
    ptms70c46cpu = (Ctms70c46*)pCPU;

}

Cti74::~Cti74() {
}

Cti95::Cti95(CPObject *parent)	: Cti74(parent)
{								//[constructor]
    setfrequency( (int) 4000000);
    setcfgfname(QString("ti95"));

    SessionHeader	= "TI95PKM";
    Initial_Session_Fname ="ti95.pkm";

    BackGroundFname	= P_RES(":/ti74/ti95.png");
    LcdFname		= P_RES(":/ti74/ti95lcd.png");
    SymbFname		= P_RES("");;

    memsize		= 0x20000;
    InitMemValue	= 0x00;

    SlotList.clear();
    SlotList.append(CSlot(60  , 0x0000 ,	""                  , ""	, CSlot::RAM , "RAM"));
    SlotList.append(CSlot(32  , 0x4000 ,	""                  , ""	, CSlot::RAM , "CARTRIDGE"));
    SlotList.append(CSlot(4  , 0xF000 ,	P_RES(":/ti74/c70011.bin")  , ""	, CSlot::ROM , "ROM cpu"));
    SlotList.append(CSlot(32 , 0x10000,	P_RES(":/ti74/hn61256pc95.bin")        , ""	, CSlot::ROM , "ROM"));


    Lcd_X		= 142;
    Lcd_Y		= 40;
    Lcd_DX		= 130;
    Lcd_DY		= 30;
    Lcd_ratio_X	= 340.0/Lcd_DX;
    Lcd_ratio_Y	= 75.0/Lcd_DY;

    Lcd_Symb_X	= 50;
    Lcd_Symb_Y	= 50;
    Lcd_Symb_DX	= 210;
    Lcd_Symb_DY	= 23;
    Lcd_Symb_ratio_X	= 2;
    Lcd_Symb_ratio_Y	= 2;

    delete pLCDC;
    pLCDC		= new Clcdc_ti95(this);
    pKEYB->fn_KeyMap = "ti95.map";

}

Cti95::~Cti95() {
}

void Cti74::power_w(UINT8 data)
{
    qWarning()<<"power_w:"<<data;
    // d0: power-on hold latch
    m_power = data & 1;

    // stop running
    if (!m_power)
        ptms70c46cpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
}


bool Cti74::Chk_Adr(UINT32 *d, UINT32 data)
{
    Q_UNUSED(data)


    if ( (*d>=0x0000) && (*d<=0x007F) )	{ return true;	}  // CPU RAM

    // CPU RAM

    if   (*d==0x010C) {  ks = data; }
    if ( (*d>=0x0100) && (*d<=0x010F) )	{ ptms70c46cpu->pf_write(*d-0x100,data); return false;	}

    if (*d==0x0118) { ptms70c46cpu->control_w(data); return true; }

    if ( (*d>=0x0000) && (*d<=0x0FFF) )	{ return true;	}  // CPU RAM
    if   (*d==0x1000) { pHD44780->control_write(data); pLCDC->redraw = true; return false; }
    if   (*d==0x1001) { pHD44780->data_write(data); pLCDC->redraw = true; return false; }
    if ( (*d>=0x1000) && (*d<=0x1FFF) )	{ return true;	}  // CPU RAM
    if ( (*d>=0x2000) && (*d<=0x3FFF) )	{ return true;	}  // CPU RAM
    if ( (*d>=0x4000) && (*d<=0xBFFF) )	{ return false; } // system ROM
    if ( (*d>=0xC000) && (*d<=0xDFFF) )	{ *d += 0x4000 + ( RomBank * 0x2000 );	return false; } // system ROM
    if ( (*d>=0xF000) && (*d<=0xFFFF) )	{ return false;	}                                       // CPU ROM
    return true;
}

bool Cti74::Chk_Adr_R(UINT32 *d, UINT32 *data)
{
    if ( (*d>=0x0000) && (*d<=0x007F) )	{ return true;	}  // CPU RAM
    if ( (*d>=0x0100) && (*d<=0x010F) )	{ *data = ptms70c46cpu->pf_read(*d-0x100); return false;	}  // CPU RAM
    if   (*d==0x0118) { *data = ptms70c46cpu->control_r(); return false; }
    if   (*d==0x1000) { *data = pHD44780->control_read(); return false; }
    if   (*d==0x1001) { *data = pHD44780->data_read(); return false; }
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
    case TMS7000_PORTD:  return 0x00; break;
    default :break;
    }

     return (Value);
}

UINT8 Cti74::out(UINT8 Port, UINT8 Value)
{
    switch (Port) {
    case TMS7000_PORTA: break;
    case TMS7000_PORTB: RomBank = Value & 0x03;
        // d2: power-on latch
        if (~Value & 4 && m_power)
        {
            m_power = 0;
            ptms70c46cpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE); // stop running
        }
        break;
    case TMS7000_PORTC: break;
    case TMS7000_PORTD: break;
    case TMS7000_PORTE: break;
    default :break;
    }

    return 0;
}

UINT8 Cti74::clock_r()
{
    return m_clock_control;
}



bool Cti74::init()
{
//    pCPU->logsw = true;
#ifndef QT_NO_DEBUG
    pCPU->logsw = false;
#endif
    CpcXXXX::init();
    pHD44780->init();

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

bool Cti95::run()
{
    if (pKEYB->LastKey == K_POW_ON) {
        TurnON();
        pKEYB->LastKey = 0;
    }

    if (ptms70c46cpu->info.m_idle_state)
    {
        ptms70c46cpu->info.m_icount -= 17;
        ptms70c46cpu->info.m_pc++;
        ptms70c46cpu->info.m_idle_state = false;
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
            if (KEY('M'))			data|=0x01;
            if (KEY('K'))			data|=0x02;
            if (KEY('I'))			data|=0x04;
            if (KEY(K_LA))			data|=0x08;
//            if (KEY(''))			data|=0x10;
            if (KEY('U'))			data|=0x20;
            if (KEY('J'))			data|=0x40;
            if (KEY('N'))			data|=0x80;
        }

        if (ks & 0x02) {
            if (KEY(','))			data|=0x01;
            if (KEY('L'))			data|=0x02;
            if (KEY('O'))			data|=0x04;
            if (KEY(K_RA))			data|=0x08;
//            if (KEY(''))			data|=0x10;
            if (KEY('Y'))			data|=0x20;
            if (KEY('H'))			data|=0x40;
            if (KEY('B'))			data|=0x80;
        }

        if (ks & 0x04) {
            if (KEY(' '))			data|=0x01;
            if (KEY(';'))			data|=0x02;
            if (KEY('P'))			data|=0x04;
            if (KEY(K_UA))			data|=0x08;
//            if (KEY(''))			data|=0x10;
            if (KEY('T'))			data|=0x20;
            if (KEY('G'))			data|=0x40;
            if (KEY('V'))			data|=0x80;
        }
        if (ks & 0x08) {
            if (KEY(K_RET))			data|=0x01;
//            if (KEY(''))			data|=0x02;
            if (KEY(K_CLR))			data|=0x04;
            if (KEY(K_DA))			data|=0x08;
            if (KEY(K_RUN))			data|=0x10;
            if (KEY('R'))			data|=0x20;
            if (KEY('F'))			data|=0x40;
            if (KEY('C'))			data|=0x80;
        }
        if (ks & 0x10) {
            if (KEY('?'))			data|=0x01;
            if (KEY('1'))			data|=0x02;
            if (KEY('4'))			data|=0x04;
            if (KEY('7'))			data|=0x08;
            if (KEY(K_BRK))			data|=0x10;
            if (KEY('E'))			data|=0x20;
            if (KEY('D'))			data|=0x40;
            if (KEY('X'))			data|=0x80;
        }
        if (ks & 0x20) {
            if (KEY('0'))			data|=0x01;
            if (KEY('2'))			data|=0x02;
            if (KEY('5'))			data|=0x04;
            if (KEY('8'))			data|=0x08;
            if (KEY(K_MOD))			data|=0x10;
            if (KEY('W'))			data|=0x20;
            if (KEY('S'))			data|=0x40;
            if (KEY('Z'))			data|=0x80;
        }
        if (ks & 0x40) {
            if (KEY('.'))			data|=0x01;
            if (KEY('3'))			data|=0x02;
            if (KEY('6'))			data|=0x04;
            if (KEY('9'))			data|=0x08;
            if (KEY(K_POW_OFF))		data|=0x10;
            if (KEY('Q'))			data|=0x20;
            if (KEY('A'))			data|=0x40;
//            if (KEY(''))			data|=0x80;
        }

        if (ks & 0x80) {
            if (KEY('+'))		data|=0x01;
            if (KEY('-'))			data|=0x02;
            if (KEY('*'))			data|=0x04;
            if (KEY('/'))			data|=0x08;
//            if (KEY(''))			data|=0x10;
            if (KEY(K_FN))			{
#if 0
                pCPU->logsw = true;
                pCPU->Check_Log();
#else
                data|=0x20;
#endif
            }
            if (KEY(K_CTRL))			data|=0x40;
            if (KEY(K_SHT))			data|=0x80;
        }

//        if (fp_log) fprintf(fp_log,"Read key [%02x]: strobe=%02x result=%02x\n",pKEYB->LastKey,ks,data^0xff);

    }

    if (data>0) {

        AddLog(LOG_KEYBOARD,tr("KEY PRESSED=%1").arg(data,2,16,QChar('0')));
    }
    return data;//^0xff;

}


quint8 Cti95::getKey()
{
    quint8 data=0;

    if ((pKEYB->LastKey>0))
    {
        if (ks & 0x01) {
            if (KEY(K_POW_OFF))		data|=0x01;
            if (KEY('Q'))			data|=0x02;
            if (KEY('A'))			data|=0x04;
            if (KEY('Z'))			data|=0x08;
//            if (KEY(''))			data|=0x10;
            if (KEY(')'))			data|=0x20;
            if (KEY('/'))			data|=0x40;
            if (KEY('='))			data|=0x80;
        }

        if (ks & 0x02) {
            if (KEY('{'))			data|=0x01;
            if (KEY('W'))			data|=0x02;
            if (KEY('S'))			data|=0x04;
            if (KEY('X'))			data|=0x08;
//            if (KEY(''))			data|=0x10;
            if (KEY('3'))			data|=0x20;
            if (KEY('+'))			data|=0x40;
            if (KEY('2'))			data|=0x80;
        }

        if (ks & 0x04) {
            if (KEY(K_F1))			data|=0x01;
            if (KEY('E'))			data|=0x02;
            if (KEY('D'))			data|=0x04;
            if (KEY('C'))			data|=0x08;
//            if (KEY(''))			data|=0x10;
            if (KEY('6'))			data|=0x20;
            if (KEY('-'))			data|=0x40;
            if (KEY('5'))			data|=0x80;
        }
        if (ks & 0x08) {
            if (KEY(K_F2))			data|=0x01;
            if (KEY('R'))			data|=0x02;
            if (KEY('F'))			data|=0x04;
            if (KEY('V'))			data|=0x08;
//            if (KEY(K_RUN))			data|=0x10;
            if (KEY('9'))			data|=0x20;
            if (KEY('*'))			data|=0x40;
            if (KEY('8'))			data|=0x80;
        }
        if (ks & 0x10) {
            if (KEY(K_F3))			data|=0x01;
            if (KEY('T'))			data|=0x02;
            if (KEY('G'))			data|=0x04;
            if (KEY('B'))			data|=0x08;
            if (KEY(' '))			data|=0x10;
            if (KEY('0'))			data|=0x20;
            if (KEY('.'))			data|=0x40;
            if (KEY(K_SIGN))			data|=0x80;
        }
        if (ks & 0x20) {
            if (KEY(K_F4))			data|=0x01;
            if (KEY('Y'))			data|=0x02;
            if (KEY('H'))			data|=0x04;
            if (KEY('N'))			data|=0x08;
            if (KEY(K_LA))			data|=0x10;
            if (KEY('O'))			data|=0x20;
            if (KEY('P'))			data|=0x40;
            if (KEY('7'))			data|=0x80;
        }
        if (ks & 0x40) {
            if (KEY(K_F5))			data|=0x01;
            if (KEY('U'))			data|=0x02;
            if (KEY('J'))			data|=0x04;
            if (KEY('M'))			data|=0x08;
            if (KEY(K_RA))		data|=0x10;
            if (KEY('L'))			data|=0x20;
            if (KEY(K_SHT))			data|=0x40;
            if (KEY('4'))			data|=0x80;
        }

        if (ks & 0x80) {
            if (KEY('('))		data|=0x01;
            if (KEY('I'))			data|=0x02;
            if (KEY('K'))			data|=0x04;
//            if (KEY('/'))			data|=0x08;
//            if (KEY(''))			data|=0x10;
//            if (KEY(K_FN))			{
//#if 0
//                pCPU->logsw = true;
//                pCPU->Check_Log();
//#else
//                data|=0x20;
//#endif
//            }
            if (KEY(K_CLR))			data|=0x40;
            if (KEY('1'))			data|=0x80;
        }

//        if (fp_log) fprintf(fp_log,"Read key [%02x]: strobe=%02x result=%02x\n",pKEYB->LastKey,ks,data^0xff);

    }

    if (data>0) {

        AddLog(LOG_KEYBOARD,tr("KEY PRESSED=%1").arg(data,2,16,QChar('0')));
    }
    return data;//^0xff;

}


bool Cti74::Get_Connector(void) {
    return true;
}

bool Cti74::Set_Connector(void) {
    return true;
}

extern int ask(QWidget *parent,QString msg,int nbButton);
void Cti74::ComputeKey()
{
    if (ptms70c46cpu->info.m_idle_state)
    {
        ptms70c46cpu->info.m_icount -= 17;
        ptms70c46cpu->info.m_pc++;
        ptms70c46cpu->info.m_idle_state = false;
    }
    else
        ptms70c46cpu->info.m_icount -= 19;

    int _slot = -1;
    if (KEY(0x240)) _slot = 1;

//    qWarning()<<"ComputKey:"<<_slot;
    pKEYB->keyPressedList.removeAll(0x240);



    if (_slot == -1) return;
    int _response = 0;
    BYTE* capsule = &mem[0x4000];
    if (!SlotList[_slot].isEmpty() || (capsule[0]!=0x00)) {
        _response=ask(this,
                      "The "+SlotList[_slot].getLabel()+ " capsule is already plugged in this slot.\nDo you want to unplug it ?",
                      2);
    }

    if (_response == 1) {
        SlotList[_slot].setEmpty(true);

        memset((void *)capsule ,0x7f,0x4000);
        SlotList[_slot].setLabel(QString("ROM bank %1").arg(_slot+1));

    }
    if (_response==2) return;
    currentSlot = _slot;
    FluidLauncher *launcher = new FluidLauncher(mainwindow,
                                                QStringList()<<P_RES(":/pockemul/configExt.xml"),
                                                FluidLauncher::PictureFlowType,
                                                "TI-74_Cartridge");
    connect(launcher,SIGNAL(Launched(QString,CPObject *)),this,SLOT(addModule(QString,CPObject *)));
    launcher->show();

}

void Cti74::addModule(QString item,CPObject *pPC)
{
    Q_UNUSED(pPC)

    qWarning()<<"Add Module:"<< item;
    if ( currentSlot!=1) return;

    int _res = 0;
    QString moduleName;
    if (item=="PASCAL74") moduleName = P_RES(":/ti74/ti74_pascal.bin");
    if (item=="PANACAPSFILE") {
        moduleName = QFileDialog::getOpenFileName(
                    mainwindow,
                    tr("Choose a Capsule file"),
                    ".",
                    tr("Module File (*.bin)"));
//        customModule = CSlot::CUSTOM_ROM;
    }

    if (moduleName.isEmpty()) return;

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

