/*
 * this code is based on TI-5x emulator
 * (c) 2014 Hynek Sladky
 * http://hsl.wz.cz/ti_59.htm
 */

#include <QDebug>

#include "ti59.h"
#include "tmc0501.h"
#include "Lcdc_ti59.h"
#include "Log.h"
#include "Keyb.h"
#include "Inter.h"
#include "Connect.h"

#define	CARD_INSERTED	(ti59cpu->r->key[10] & (1 << KR_BIT))
#define	PRN_CONNECTED	(ti59cpu->r->key[0] & (1 << KP_BIT))
#define	PRN_TRACE	(ti59cpu->r->key[15] & (1 << KP_BIT))
#define	PRN_ADVANCE	(ti59cpu->r->key[12] & (1 << KN_BIT))
#define	PRN_PRINT	(ti59cpu->r->key[12] & (1 << KP_BIT))

// SCOM registers
unsigned char SCOM[16][16];

// RAM registers
unsigned char RAM[120][16];


Cti59::Cti59(CPObject *parent,Models mod):CpcXXXX(parent)
{								//[constructor]
    currentModel = mod;

    setfrequency( (int) 455000 / 2 / 16);
    setcfgfname(QString("ti59"));

    SessionHeader	= "TI59PKM";
    Initial_Session_Fname ="ti59.pkm";

    BackGroundFname	= P_RES(":/ti59/ti59.png");
//    LcdFname		= P_RES(":/ti59/ti59lcd.png");
//    SymbFname		= "";

    memsize		= 0xFFFF;
    InitMemValue	= 0xFF;

    SlotList.clear();
    SlotList.append(CSlot(12 , 0x0000 ,	P_RES(":/ti59/ti59.bin"), ""	, CSlot::ROM , "ROM"));

    setDXmm(81);
    setDYmm(162);
    setDZmm(36);


    setDX(279);
    setDY(564);

//    Lcd_X		= 30;
//    Lcd_Y		= 43;
//    Lcd_DX		= 220;
//    Lcd_DY		= 40;
//    Lcd_ratio_X	= 1;
//    Lcd_ratio_Y	= 1;

    pTIMER		= new Ctimer(this);
    pLCDC		= new Clcdc_ti59(this,
                                 QRect(30,43,220,40),
                                 QRect(),
                                 P_RES(":/ti59/ti59lcd.png"));
    pCPU		= new Ctmc0501(this,currentModel);    ti59cpu = (Ctmc0501*)pCPU;
    pKEYB		= new Ckeyb(this,"ti59.map");

    ioFreq = 0;
}

Cti59::~Cti59() {
}

bool Cti59::init(void)				// initialize
{

//pCPU->logsw = true;
#ifndef QT_NO_DEBUG
//    pCPU->logsw = true;
//    if (!fp_log) fp_log=fopen("pc2001.log","wt");	// Open log file
#endif
    CpcXXXX::init();


    return true;
}



bool Cti59::run() {

    TMC0501regs * _regs = ti59cpu->r;

//    _regs->KEYR = getKey();

//    if (
//            !_regs->Power &&
//         (_regs->KEYR !=0 )) {
//        _regs->COND = 1;
//        _regs->R5 = _regs->KEYR;
//        _regs->Timer=0;
//        _regs->Power = true;
//        ti59cpu->halt = false;
//        qWarning()<<"Wakeup";

//    }


//    if (ti59cpu->r->KEYR != 0) {
//        qWarning() << "KEY:"<<ti59cpu->r->KEYR;
//    }

    CpcXXXX::run();

    pTIMER->state++;
    return true;
}

bool Cti59::Chk_Adr(UINT32 *d, UINT32 data) { return false; }
bool Cti59::Chk_Adr_R(UINT32 *d, UINT32 *data) { return true; }
UINT8 Cti59::in(UINT8 Port) { return 0;}
UINT8 Cti59::out(UINT8 Port, UINT8 x) { return 0; }
bool Cti59::Set_Connector(Cbus *_bus) { return true; }
bool Cti59::Get_Connector(Cbus *_bus) { return true; }


void Cti59::TurnOFF(void) {
    mainwindow->saveAll = YES;
    CpcXXXX::TurnOFF();
    mainwindow->saveAll = ASK;
    AddLog(LOG_TEMP,"TURN OFF");

}

void Cti59::TurnON(void){
    CpcXXXX::TurnON();

//    ti59cpu->r->Run = true;
//    ti59cpu->r->Power = !ti59cpu->r->Power;
//    ti59cpu->halt = !ti59cpu->halt;
    pLCDC->updated = true;
}


void Cti59::Reset()
{
    CpcXXXX::Reset();

    // TI-58/59 mode
    if (currentModel == TI59) {
        // TI-59: D10-KR card switch normally closed
        ti59cpu->r->key[10] |= (1 << KR_BIT);
    }
    if (currentModel == TI58) {
        // TI-58: D7-KR diode
        ti59cpu->r->key[7] |= (1 << KR_BIT);
    }

//    // printer mode
//    if (mode_flags & MODE_PRINTER)
//      // printer: D0-KP diode
//      ti59cpu->r->key[0] |= (1 << KP_BIT);


}

bool Cti59::LoadConfig(QXmlStreamReader *xmlIn)
{
    return true;
}

bool Cti59::SaveConfig(QXmlStreamWriter *xmlOut)
{
    return true;
}

QString Cti59::Display() {
  QChar c;
  QString s;
#if 0
  if (!pCPU) return "";
  s="";
  if (Power)
    for (int i = 11;i >=0; i--) {
        if (ti59cpu->r->RB[i] & 8) c=' ';
        else if (ti59cpu->r->RB[i] & 1) c='-';
        else
            switch(ti59cpu->r->RA[i]) {
            case 0:
            case 1:
            case 2:
            case 3:
            case 4:
            case 5:
            case 6:
            case 7:
            case 8:
            case 9: c=QChar(0x30 + ti59cpu->r->RA[i]); break;
            default: qWarning()<<"default:"<<i;c=QChar(0x41-10+ti59cpu->r->RA[i]); break;
            }
        s.append(c);
        if (ti59cpu->r->RB[i] & 2) s.append('.');
    }
  // Check for error
  if (ti59cpu->r->RA[14]==0x0e) {
      // error, blink screen
      s.prepend('E');
  }
#endif
//  qWarning()<<"DISPLAY:"<<s;
  return s;
}

#define KEY(c)	( pKEYB->keyPressedList.contains(TOUPPER(c)) || pKEYB->keyPressedList.contains(c) || pKEYB->keyPressedList.contains(TOLOWER(c)))

UINT8 Cti59::getKey()
{
    UINT8 code = 0;
    if (pKEYB->LastKey)
    {
        if (KEY('0'))			code = 0x27;
        if (KEY('1'))			code = 0x26;
        if (KEY('2'))			code = 0x36;
        if (KEY('3'))			code = 0x46;
        if (KEY('4'))			code = 0x25;
        if (KEY('5'))			code = 0x35;
        if (KEY('6'))			code = 0x45;
        if (KEY('7'))			code = 0x24;
        if (KEY('8'))			code = 0x34;
        if (KEY('9'))			code = 0x44;

        if (KEY('.'))			code = 0x37;
        if (KEY('+'))			code = 0x56;
        if (KEY('('))			code = 51;
        if (KEY(')'))			code = 67;
        if (KEY('-'))			code = 0x55;
        if (KEY('*'))			code = 0x54;
        if (KEY('/'))			code = 0x53;
        if (KEY('='))			code = 0x57;

        if (KEY(K_CCE))			code = 80;  // CLR
        if (KEY(K_LN))			code = 48;
        if (KEY(K_CE))			code = 64;
        if (KEY(K_GTO))			code = 20;
        if (KEY(K_SBR))			code = 21;
        if (KEY(K_RST))			code = 22;
        if (KEY(K_SHT))			code = 0x10;  // 2nd
        if (KEY(K_DEF))			code = 0x20;  // INV
        if (KEY(K_RS))			code = 23;
        if (KEY(K_SIGN))		code = 71;
        if (KEY(K_LRN))			code = 17;
        if (KEY(K_XT))			code = 33;

        if (KEY(K_SQR))			code = 49;
        if (KEY(K_ROOT))		code = 65;
        if (KEY(K_1X))			code = 81;
        if (KEY(K_SST))			code = 18;
        if (KEY(K_STO))			code = 34;
        if (KEY(K_RCL))			code = 50;
        if (KEY(K_SUM))			code = 66;
        if (KEY(K_POT))			code = 82;  // X^Y
        if (KEY(K_BST))			code = 19;
        if (KEY(K_EE))			code = 35;

    }

    return code;

}

