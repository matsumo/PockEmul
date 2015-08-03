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

    displayString = Display();
    if (ti59cpu->r->flags & FLG_DISP)
        pLCDC->updated = true;

    getKey();


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
  if (!pCPU) return "";
  s="";
#if 0
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


  if (!ti59cpu->r->digit) {
      static char disp_filter = 0;
      if (ti59cpu->r->flags & FLG_IDLE) {
          qWarning()<<"ok";
          // display enabled
          static unsigned char dA[16], dB[16];
          int i;
          if (ti59cpu->r->flags & FLG_DISP) {
              // check difference between current and saved registers
              for (i = 13; i >= 2; i--) {
                  if (dA[i] != ti59cpu->r->A[i] || dB[i] != ti59cpu->r->B[i]) {
                      ti59cpu->r->flags &= ~FLG_DISP;
                      break;
                  }
              }
              if ((ti59cpu->r->flags ^ ti59cpu->r->fA) & FLG_DISP_C)
                  ti59cpu->r->flags &= ~FLG_DISP;
          }
          if (!(ti59cpu->r->flags & FLG_DISP)) {
              int zero = 1;
              ti59cpu->r->flags |= FLG_DISP;
              //        putchar ('\r');
              if (ti59cpu->r->fA & 0x4000) {
                  //          putchar ('C');
                  ti59cpu->r->flags |= FLG_DISP_C;
              } else {
                  ti59cpu->r->flags &= ~FLG_DISP_C;
              }
#if 1
              for (i = 13; i >= 2; i--) {
                  dA[i] = ti59cpu->r->A[i];
                  dB[i] = ti59cpu->r->B[i];
                  if (i == 3 || ti59cpu->r->R5 == i || ti59cpu->r->B[i] >= 8)
                      zero = 0;
                  if (i == 2)
                      zero = 1;
                  if (ti59cpu->r->B[i] == 7 || ti59cpu->r->B[i] == 3 || (ti59cpu->r->B[i] <= 4 && zero && !ti59cpu->r->A[i]))
                      s.append(' ');
                  else
                      if (ti59cpu->r->B[i] == 6 || (ti59cpu->r->B[i] == 5 && !ti59cpu->r->A[i]))
                          s.append ('-');
                      else
                          if (ti59cpu->r->B[i] == 5)
                              putchar ('o');
                          else
                              if (ti59cpu->r->B[i] == 4)
                                  putchar ('\'');
                              else
                                  if (ti59cpu->r->B[3] == 2)
                                      putchar ('"');
                                  else {
                                      s.append ('0' + ti59cpu->r->A[i]);
                                      if (ti59cpu->r->A[i])
                                          zero = 0;
                                  }
                  if (ti59cpu->r->R5 == i)
                      s.append ('.');
              }
#else
              for (i = 13; i >= 2; i--) {
                  putchar ('0'+ti59cpu->r->A[i]);
                  if (ti59cpu->r->R5 == i)
                      putchar ('.');
              }
              putchar (' ');
              for (i = 13; i >= 2; i--)
                  putchar ('0'+ti59cpu->r->B[i]);
#endif
              putchar ('|'); putchar (' ');
                qWarning()<<"DISPLAY:"<<s;
          }
          disp_filter = 0;
      } else
      if (disp_filter < 3)
          disp_filter++;
      else {
          // display disabled
          if ((ti59cpu->r->flags & FLG_DISP) /*|| (!ti59cpu->r->fA && (ti59cpu->r->flags & FLG_DISP_C))*/ || (ti59cpu->r->fA && !(ti59cpu->r->flags & FLG_DISP_C))) {
              ti59cpu->r->flags &= ~FLG_DISP;
              if (ti59cpu->r->fA) {
                  printf ("\rC            |");
                  ti59cpu->r->flags |= FLG_DISP_C;
              } else {
                  printf ("\r             |");
                  ti59cpu->r->flags &= ~FLG_DISP_C;
              }
          }
      }
  }

  return s;
}

#define KEY(c)	( pKEYB->keyPressedList.contains(TOUPPER(c)) || pKEYB->keyPressedList.contains(c) || pKEYB->keyPressedList.contains(TOLOWER(c)))
#define KPORT(CODE)    ti59cpu->r->key[(CODE) & 0x0F] |= 1 << (((CODE) >> 4) & 0x07)

UINT8 Cti59::getKey()
{
    UINT8 code = 0;
    if (pKEYB->LastKey)
    {

        if (KEY(K_RS))			KPORT(0x19);
        if (KEY('0'))			KPORT(0x29);
        if (KEY('.'))			KPORT(0x39);
        if (KEY(K_SIGN))		KPORT(0x59);
        if (KEY('='))			KPORT(0x69);

        if (KEY(K_RST))			KPORT(0x18);
        if (KEY('1'))			KPORT(0x28);
        if (KEY('2'))			KPORT(0x38);
        if (KEY('3'))			KPORT(0x58);
        if (KEY('+'))			KPORT(0x68);

        if (KEY(K_SBR))			KPORT(0x17);
        if (KEY('4'))			KPORT(0x27);
        if (KEY('5'))			KPORT(0x37);
        if (KEY('6'))			KPORT(0x57);
        if (KEY('-'))			KPORT(0x67);

        if (KEY(K_GTO))			KPORT(0x16);
        if (KEY('7'))			KPORT(0x26);
        if (KEY('8'))			KPORT(0x36);
        if (KEY('9'))			KPORT(0x56);
        if (KEY('*'))			KPORT(0x66);

        if (KEY(K_BST))			KPORT(0x15);
        if (KEY(K_EE))			KPORT(0x25);
        if (KEY('('))			KPORT(0x35);
        if (KEY(')'))			KPORT(0x55);
        if (KEY('/'))			KPORT(0x65);

        if (KEY(K_SST))			KPORT(0x14);
        if (KEY(K_STO))			KPORT(0x24);
        if (KEY(K_RCL))			KPORT(0x34);
        if (KEY(K_SUM))			KPORT(0x54);
        if (KEY(K_POT))			KPORT(0x64);  // X^Y

        if (KEY(K_LRN))			KPORT(0x13);
        if (KEY(K_XT))			KPORT(0x23);
        if (KEY(K_SQR))			KPORT(0x33);
        if (KEY(K_ROOT))		KPORT(0x53);
        if (KEY(K_1X))			KPORT(0x63);

        if (KEY(K_SHT))			KPORT(0x12);  // 2nd
        if (KEY(K_DEF))			KPORT(0x22);  // INV
        if (KEY(K_LN))			KPORT(0x32);
        if (KEY(K_CE))			KPORT(0x52);
        if (KEY(K_CCE))			KPORT(0x62);  // CLR

        if (KEY('A'))			KPORT(0x11);  // 2nd
        if (KEY('B'))			KPORT(0x21);  // INV
        if (KEY('C'))			KPORT(0x31);
        if (KEY('D'))			KPORT(0x51);
        if (KEY('E'))			KPORT(0x61);  // CLR

    }

    return code;

}

