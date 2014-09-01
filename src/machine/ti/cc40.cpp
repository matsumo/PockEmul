#include <QtGui>
#include <QString>

#include "common.h"
#include "cc40.h"
#include "tms7000/tms7000.h"
#include "hd44780.h"
#include "Lcdc_cc40.h"
#include "Inter.h"
#include "Keyb.h"
#include "cextension.h"

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
    LcdFname		= P_RES(":/cc40/cc40lcd.png");
    SymbFname		= "";

//    TopFname    = P_RES(":/fp200/fp200Top.png");

    memsize		= 0x20000;
    InitMemValue	= 0x00;

    SlotList.clear();
    SlotList.append(CSlot(52  , 0x0000 ,	""                  , ""	, CSlot::RAM , "RAM"));
    SlotList.append(CSlot(2  , 0xF800 ,	P_RES(":/cc40/cc40_2krom.bin")  , ""	, CSlot::ROM , "ROM cpu"));
    SlotList.append(CSlot(32 , 0x10000,	P_RES(":/cc40/cc40.bin")        , ""	, CSlot::ROM , "ROM"));


    setDXmm(236);
    setDYmm(147);
    setDZmm(24);

    setDX(845);
    setDY(525);

    Lcd_X		= 50;
    Lcd_Y		= 160;
    Lcd_DX		= 510;
    Lcd_DY		= 60;
    Lcd_ratio_X	= 1;
    Lcd_ratio_Y	= 1;

    pLCDC		= new Clcdc_cc40(this);
    pCPU		= new Ctms70c20(this);
    pTIMER		= new Ctimer(this);
    pKEYB		= new Ckeyb(this,"cc40.map");
    pHD44780    = new CHD44780(P_RES(":/cc40/hd44780_a00.bin"));



    lastKeyBufSize = 0;
    newKey = false;

    ioFreq = 0;             // Mandatory for Centronics synchronization
    tms7000cpu = (Ctms70c20*)pCPU;
}

Ccc40::~Ccc40() {
}



bool Ccc40::Chk_Adr(UINT32 *d, UINT32 data)
{
    Q_UNUSED(data)


    if (*d==0x0115) { fillSoundBuffer((data & 1) ? 0x7f : 0); return false; }
    if (*d==0x0119) { RomBank = data & 0x0f; /*qWarning()<<"romBank:"<<RomBank;*/	return true; }
    if (*d==0x011E) {
        qWarning()<<"pHD44780->control_write:"<<data;
        pHD44780->control_write(data);
        pLCDC->redraw = true;
        return false;
    }
    if (*d==0x011F) {
        qWarning()<<"pHD44780->data_write:"<<data;
        pHD44780->data_write(data);
        pLCDC->redraw = true;
        return false;
    }
    if ( (*d>=0x0000) && (*d<=0xCFFF) )	{ return true;	}  // CPU RAM
    if ( (*d>=0xD000) && (*d<=0xEFFF) )	{ *d += 0x3000 + ( RomBank * 0x2000 );	return false; } // system ROM
    if ( (*d>=0xF800) && (*d<=0xFFFF) )	{ return false;	}                                       // CPU ROM




    if (                 (*d<=0x7FFF) )	return(false);		// ROM area(0000-7FFF)

    if ( (*d>=0xA000) && (*d<=0xBFFF) )	{ return(ext_MemSlot1->ExtArray[ID_FP201]->IsChecked);	}
    if ( (*d>=0xC000) && (*d<=0xDFFF) )	{ return(ext_MemSlot2->ExtArray[ID_FP201]->IsChecked);	}


    return true;
}

bool Ccc40::Chk_Adr_R(UINT32 *d, UINT32 *data)
{
    Q_UNUSED(d)
    Q_UNUSED(data)

    if (*d==0x011E) { *data = pHD44780->control_read(); return false; }
    if (*d==0x011F) { *data = pHD44780->data_read(); return false; }
    if ( (*d>=0xD000) && (*d<=0xEFFF) )	{ *d += 0x3000 + ( RomBank * 0x2000 );	return true; } // system ROM

    return true;
}



UINT8 Ccc40::in(UINT8 Port)
{
    UINT8 Value=0;
#if 0
    quint16 tmp=0;
    Clcdc_fp200 *pLcd = (Clcdc_fp200*)pLCDC;

    if (SOD)
    {      // SOD = 1
        switch (Port)
        {
        case 0x01 : /* Read 8bits data to LCD left-half */
            Value = pLcd->Read(1);
            AddLog(LOG_DISPLAY,tr("IN [01]=[%1]=%2\n").arg(Value,2,16,QChar('0')).arg(QChar(Value).toLatin1()!=0?QChar(Value):QChar(' ')));
            break;
        case 0x02 : /* Read 8bits data to LCD right-half */
            Value = pLcd->Read(2);
            AddLog(LOG_DISPLAY,tr("IN [02]=[%1]=%2\n").arg(Value,2,16,QChar('0')).arg(QChar(Value).toLatin1()!=0?QChar(Value):QChar(' ')));
            break;
        case 0x08 : /* Read 6 bits data : */
            Value = (pLcd->Status << 4) | ((pLcd->Y >> 4) & 0x03);
            AddLog(LOG_DISPLAY,tr("IN [08]=[%1]\n").arg(Value,2,16,QChar('0')));
            break;
        case 0x09: /* D0-D3 for X, D4-D7 for part of Y */
            Value = (pLcd->X & 0x0f) | ((pLcd->Y & 0x0f) <<4);
            AddLog(LOG_DISPLAY,tr("IN [09]=[%1]\n").arg(Value,2,16,QChar('0')));
            break;

        case 0x10: Value = pCPU->imem[Port] | 0x04;
//            pCPU->logsw=true;pCPU->Check_Log();
            AddLog(LOG_PRINTER,tr("IN [%1] = %2\n").arg(Port,2,16,QChar('0')).arg(Value,2,16,QChar('0')));
            break;

        case 0x20: tmp = getKey();
            Value= tmp & 0xff;
            sid = (tmp >> 8);
            if ((tmp>>8) > 0) {
//                AddLog(LOG_CONSOLE,tr("SID=[%1]\n").arg(tmp>>8,2,16,QChar('0')));
            }
            break;
        case 0x81: Value = pCENT->isAvailable()?0x00:0x80;
            AddLog(LOG_CONSOLE,tr("IN [%1]\n").arg(Port,2,16,QChar('0')));
//            pCPU->logsw=true;pCPU->Check_Log();
            break;//printerBUSY ? 0x80:0x00;
        default: AddLog(LOG_CONSOLE,tr("IN [%1]\n").arg(Port,2,16,QChar('0')));
            break;
        }
    }
    else {
        if ((Port >=0x10) && (Port <= 0x1F)) {
            Value = pRP5C01->read(Port - 0x10);
        }
    }

     pCPU->imem[Port] = Value;
#endif
     return (Value);
}

UINT8 Ccc40::out(UINT8 Port, UINT8 Value)
{
#if 0
    Clcdc_fp200 *pLcd = (Clcdc_fp200*)pLCDC;

    if (SOD)
    {      // SOD = 1
        switch (Port)
        {
        case 0x01 : /* Write 8bits data to LCD left-half */
            pLcd->Write(1,Value);
            AddLog(LOG_DISPLAY,tr("OUT[01]=[%1]=%2\n").arg(Value,2,16,QChar('0')).arg(QChar(Value).toLatin1()!=0?QChar(Value):QChar(' ')));
            break;
        case 0x02 : /* Write 8bits data to LCD right-half */
            pLcd->Write(2,Value);
            AddLog(LOG_DISPLAY,tr("OUT[02]=[%1]=%2\n").arg(Value,2,16,QChar('0')).arg(QChar(Value).toLatin1()!=0?QChar(Value):QChar(' ')));
            break;
        case 0x08 : /* write 6 bits data : */

            pLcd->Status = (Value >>4) & 0x0f;
            if (pLcd->Status==0x0b) pLcd->Y = (pLcd->Y & 0x0f) | ((Value & 0x03) << 4);
            AddLog(LOG_DISPLAY,tr("OUT[08]=[%1] Status=%2 Y=%3\n").
                   arg(Value,2,16,QChar('0')).
                   arg(pLcd->Status,2,16,QChar('0')).
                   arg(pLcd->Y,2,16,QChar('0')));

            break;
        case 0x09: /* D0-D3 for X, D4-D7 for part of Y */
            pLcd->X = Value & 0x0f;
            pLcd->Y = (pLcd->Y & 0x30) | (Value >> 4);
            AddLog(LOG_DISPLAY,tr("OUT[09]=[%1] X=%2 Y=%3\n").arg(Value,2,16,QChar('0')).
                   arg(pLcd->X,2,16,QChar('0')).
                   arg(pLcd->Y,2,16,QChar('0')));
            break;
            //    case 0x20: i85cpu->i8085_set_irq_line(I8085_RST75_LINE,0);
            //        break;

        case 0x20:break;
        case 0x21: ks = Value & 0x0f;
            break;

        case 0x80: pCENT->newOutChar( Value );
//            pCPU->logsw=true;pCPU->Check_Log();
            AddLog(LOG_PRINTER,tr("OUT[%1]=[%2]=%3\n").arg(Port,2,16,QChar('0')).arg(Value,2,16,QChar('0')).arg(QChar(Value).toLatin1()!=0?QChar(Value):QChar(' ')));
            break;
        default: AddLog(LOG_CONSOLE,tr("OUT[%1]=[%2]=%3\n").arg(Port,2,16,QChar('0')).arg(Value,2,16,QChar('0')).arg(QChar(Value).toLatin1()!=0?QChar(Value):QChar(' ')));
            pCPU->imem[Port] = Value;
            break;
        }
    }
    else {
        if ((Port >=0x10) && (Port <= 0x1F)) {
            pRP5C01->write(Port - 0x10,Value);
        }
    }
#endif
    return 0;
}


bool Ccc40::init()
{
    pCPU->logsw = false;
#ifndef QT_NO_DEBUG
    pCPU->logsw = false;
#endif
    CpcXXXX::init();
    pHD44780->init();
    initExtension();
    Reset();
    Cetl = false;
    sid = 0;



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


    CpcXXXX::run();


    return true;
}

void Ccc40::Reset()
{
    CpcXXXX::Reset();
    pHD44780->Reset();
}

void Ccc40::TurnON()
{
    CpcXXXX::TurnON();
//    pCPU->Reset();
}

void Ccc40::TurnOFF()
{
    mainwindow->saveAll = YES;
    CpcXXXX::TurnOFF();
    mainwindow->saveAll = ASK;
}

bool Ccc40::SaveConfig(QXmlStreamWriter *xmlOut)
{
    Q_UNUSED(xmlOut)

    return true;
}

bool Ccc40::LoadConfig(QXmlStreamReader *xmlIn)
{
    Q_UNUSED(xmlIn)

    return true;
}


#define KEY(c)	( pKEYB->keyPressedList.contains(TOUPPER(c)) || pKEYB->keyPressedList.contains(c) || pKEYB->keyPressedList.contains(TOLOWER(c)))

//#define KEY(c)	( TOUPPER(pKEYB->LastKey) == TOUPPER(c) )
quint16 Ccc40::getKey()
{

    quint16 data=0;
    UINT16 strobe = (0x01 << (ks));

//    i85cpu->i8085_set_SID(0);
//    if (ks == 0x0B) strobe = 0xffff;

    if ((pKEYB->LastKey>0))
    {
        if (strobe & 0x01) {
            if (KEY('7'))			data|=0x10;
            if (KEY('U'))			data|=0x20;
            if (KEY('J'))			data|=0x40;
            if (KEY('M'))			data|=0x80;
        }
        if (strobe & 0x02) {
            if (KEY('8'))			data|=0x10;
            if (KEY('I'))			data|=0x20;
            if (KEY('K'))			data|=0x40;
            if (KEY('/'))			data|=0x80;
        }
        if (strobe & 0x04) {
            if (KEY('9'))			data|=0x10;
            if (KEY('O'))			data|=0x20;
            if (KEY('L'))			data|=0x40;
            if (KEY('.'))			data|=0x80;
        }
        if (strobe & 0x08) {
            if (KEY('0'))			data|=0x10;
            if (KEY('P'))			data|=0x20;
            if (KEY(','))			data|=0x40;
            if (KEY(';'))			data|=0x80;
        }
        if (strobe & 0x10) {

            if (KEY(K_DEF))			data|=0x01;
            if (KEY(K_INS))			data|=0x04;
            if (KEY(K_CLR))			data|=0x04;
            if (KEY(K_F0))			data|=0x08;
            if (KEY('1'))			data|=0x10;
            if (KEY('Q'))			data|=0x20;
            if (KEY('A'))			data|=0x40;
            if (KEY('Z'))			data|=0x80;
            // K SWITCH
//            data|=0x100;
//                    i85cpu->i8085_set_SID(1);
        }
        if (strobe & 0x20) {
            if (KEY(K_RA))			data|=0x04;
            if (KEY(K_F1))			data|=0x08;
            if (KEY('2'))			data|=0x10;
            if (KEY('W'))			data|=0x20;
            if (KEY('S'))			data|=0x40;
            if (KEY('X'))			data|=0x80;
            // CETL BASIC SWITCH
            if (KEY(K_BASIC))       {
                Cetl = false;
                data|=0x100;
            }
            if (KEY(K_CETL))       {
                Cetl = true;
            }

        }
        if (strobe & 0x40) {
            if (KEY(' '))			data|=0x01;
            if (KEY('^'))			data|=0x02;
            if (KEY(K_LA))			data|=0x04;
            if (KEY(K_F2))			data|=0x08;
            if (KEY('3'))			data|=0x10;
            if (KEY('E'))			data|=0x20;
            if (KEY('D'))			data|=0x40;
            if (KEY('C'))			data|=0x80;

            if (pKEYB->isShift) data|=0x100;
//            i85cpu->i8085_set_SID(pKEYB->isShift?1:0);
        }
        if (strobe & 0x80) {
            if (KEY('@'))			data|=0x01;
            if (KEY('-'))			data|=0x02;
            if (KEY(K_DA))			data|=0x04;
            if (KEY(K_F3))			data|=0x08;
            if (KEY('4'))			data|=0x10;
            if (KEY('R'))			data|=0x20;
            if (KEY('F'))			data|=0x40;
            if (KEY('V'))			data|=0x80;
        }
        if (strobe & 0x100)  {
            if (KEY('['))			data|=0x01;
            if (KEY(':'))			data|=0x02;
            if (KEY(K_UA))			data|=0x04;
            if (KEY(K_F4))			data|=0x08;
            if (KEY('5'))			data|=0x10;
            if (KEY('T'))           data|=0x20;
            if (KEY('G'))			data|=0x40;
            if (KEY('B'))			data|=0x80;
        }
        if (strobe & 0x200) {
            if (KEY(K_RET))			data|=0x01;
            if (KEY(K_SML))			data|=0x04;
            if (KEY('_'))			data|=0x08;
            if (KEY('6'))			data|=0x10;
            if (KEY('Y'))           data|=0x20;
            if (KEY('H'))			data|=0x40;
            if (KEY('N'))			data|=0x80;

            if (pKEYB->isCtrl) data|=0x100;
//            i85cpu->i8085_set_SID(pKEYB->isCtrl?1:0);
        }


//        if (fp_log) fprintf(fp_log,"Read key [%02x]: strobe=%02x result=%02x\n",pKEYB->LastKey,ks,data^0xff);

    }
//                data|=0x20;
    if (data>0) {
//        i85cpu->i8085_set_RST75(1);
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

bool Ccc40::Get_Connector(void) {


    return true;
}
bool Ccc40::Set_Connector(void) {


    return true;
}



