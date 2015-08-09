/*
 * this code is based on TI-5x emulator
 * (c) 2014 Hynek Sladky
 * http://hsl.wz.cz/ti_59.htm
 */

#include <QDebug>
#include <QFileDialog>

#include "ti59.h"
#include "tmc0501.h"
#include "Lcdc_ti59.h"
#include "Log.h"
#include "Keyb.h"
#include "Inter.h"
#include "Connect.h"
#include "watchpoint.h"

#define	CARD_INSERTED	(ti59cpu->r.key[10] & (1 << KR_BIT))
#define	PRN_CONNECTED	(ti59cpu->r.key[0] & (1 << KP_BIT))
#define	PRN_TRACE	(ti59cpu->r.key[15] & (1 << KP_BIT))
#define	PRN_ADVANCE	(ti59cpu->r.key[12] & (1 << KN_BIT))
#define	PRN_PRINT	(ti59cpu->r.key[12] & (1 << KP_BIT))

#define KPORT(COND,CODE)    if(COND) \
                                ti59cpu->r.key[(CODE) & 0x0F] |= 1 << (((CODE) >> 4) & 0x07); \
                            else \
                                ti59cpu->r.key[(CODE) & 0x0F] &= ~(1 << (((CODE) >> 4) & 0x07));
#define KPORT_ON_OFF(COND,CODE)    if(COND) \
                                ti59cpu->r.key[(CODE) & 0x0F] ^= 1 << (((CODE) >> 4) & 0x07); \


Cti59::Cti59(CPObject *parent,Models mod):CpcXXXX(parent)
{								//[constructor]
    currentModel = mod;

    setfrequency( (int) 455000 / 2 / 16);
    setcfgfname(QString("ti59"));

    SessionHeader	= "TI59PKM";
    Initial_Session_Fname ="ti59.pkm";

    switch (mod) {
    case TI59:  BackGroundFname	= P_RES(":/ti59/ti59.png"); break;
    case TI59C: BackGroundFname	= P_RES(":/ti59/ti59c.png"); break;
    case TI58C: BackGroundFname	= P_RES(":/ti59/ti58c.png"); break;
    }


    LeftFname       = P_RES(":/ti59/ti59LEFT.png");
    RightFname      = P_RES(":/ti59/ti59RIGHT.png");
    BackFname       = P_RES(":/ti59/ti59BACK.png");

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
                                 QRect(40,68,210,40),
                                 QRect(),
                                 P_RES(":/ti59/ti59lcd.png"));
    pCPU		= new Ctmc0501(this,currentModel);    ti59cpu = (Ctmc0501*)pCPU;
    pKEYB		= new Ckeyb(this,"ti59.map");

    ioFreq = 0;

    changeCardAction = 0;
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

    pPRINTERCONNECTOR	= new Cconnector(this,9,1,Cconnector::TI_8,"Printer",false,
                                         QPoint(402,0),Cconnector::NORTH);
    publish(pPRINTERCONNECTOR);
    WatchPoint.add(&pPRINTERCONNECTOR_value,64,9,this,"Printer");

    disp_filter = 0;

    if (currentModel == TI59) Reset();

    return true;
}



bool Cti59::run() {

    getKey();

    CpcXXXX::run();

    Display();

    if (sendToPrinter) {
//        qWarning()<<"sendToPrinter="<<sendToPrinter;
    }

    pPRINTERCONNECTOR_value = pPRINTERCONNECTOR->Get_values();

    return true;
}

bool Cti59::Chk_Adr(UINT32 *d, UINT32 data) { return false; }
bool Cti59::Chk_Adr_R(UINT32 *d, UINT32 *data) { return true; }
UINT8 Cti59::in(UINT8 Port) { return 0;}
UINT8 Cti59::out(UINT8 Port, UINT8 x) { return 0; }

bool Cti59::Set_Connector(Cbus *_bus) {

    Q_UNUSED(_bus)

    pPRINTERCONNECTOR->Set_pin(10,0);

    if (sendToPrinter>0) {
        pPRINTERCONNECTOR->Set_values(sendToPrinter);
        AddLog(LOG_PRINTER,QString("Send Char:%1").arg(sendToPrinter,2,16,QChar('0')));
    }
    else
        pPRINTERCONNECTOR->Set_values(0);

    return true;
}

bool Cti59::Get_Connector(Cbus *_bus) {

    Q_UNUSED(_bus)

    if (pPRINTERCONNECTOR->Get_pin(9)) {
        sendToPrinter = 0;
    }

    if (pPRINTERCONNECTOR->Get_pin(10)) {
        // printer: D0-KP diode
        ti59cpu->r.key[0] |= (1 << KP_BIT);
    }

    KPORT(pPRINTERCONNECTOR->Get_pin(11),0x2C);
    KPORT_ON_OFF(pPRINTERCONNECTOR->Get_pin(12),0x2F);
    KPORT(pPRINTERCONNECTOR->Get_pin(13),0x0C);

    return true;
}


void Cti59::TurnOFF(void) {
    mainwindow->saveAll = YES;
    CpcXXXX::TurnOFF();
    mainwindow->saveAll = ASK;
    AddLog(LOG_TEMP,"TURN OFF");

}

void Cti59::TurnON(void){
    CpcXXXX::TurnON();

    pLCDC->updated = true;
}


void Cti59::Reset()
{
    CpcXXXX::Reset();

    // TI-58/59 mode
    if ((currentModel == TI59)||(currentModel == TI59C)) {
        // TI-59: D10-KR card switch normally closed
        ti59cpu->r.key[10] |= (1 << KR_BIT);
    }
    if (currentModel == TI58C) {
        // TI-58: D7-KR diode
        ti59cpu->r.key[7] |= (1 << KR_BIT);
    }

//    // printer mode
//    if (mode_flags & MODE_PRINTER)
//      // printer: D0-KP diode
//      ti59cpu->r.key[0] |= (1 << KP_BIT);


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

  if (!ti59cpu->r.digit) {

      if (ti59cpu->r.flags & FLG_IDLE) {
//          qWarning()<<"ok";
          // display enabled

          int i;
          if (ti59cpu->r.flags & FLG_DISP) {
              // check difference between current and saved registers
              for (i = 13; i >= 2; i--) {
                  if (dA[i] != ti59cpu->r.A[i] || dB[i] != ti59cpu->r.B[i]) {
                      ti59cpu->r.flags &= ~FLG_DISP;
                      break;
                  }
              }
              if ((ti59cpu->r.flags ^ ti59cpu->r.fA) & FLG_DISP_C)
                  ti59cpu->r.flags &= ~FLG_DISP;
          }
          if (!(ti59cpu->r.flags & FLG_DISP)) {
              int zero = 1;
              ti59cpu->r.flags |= FLG_DISP;
              pLCDC->updated = true;
              //        putchar ('\r');
              if (ti59cpu->r.fA & 0x4000) {
                  //          putchar ('C');
                  s.append('C');
                  ti59cpu->r.flags |= FLG_DISP_C;
              } else {
                  ti59cpu->r.flags &= ~FLG_DISP_C;
              }
#if 1
              for (i = 13; i >= 2; i--) {
                  dA[i] = ti59cpu->r.A[i];
                  dB[i] = ti59cpu->r.B[i];
                  if (i == 3 || ti59cpu->r.R5 == i || ti59cpu->r.B[i] >= 8)
                      zero = 0;
                  if (i == 2)
                      zero = 1;
                  if (ti59cpu->r.B[i] == 7 || ti59cpu->r.B[i] == 3 || (ti59cpu->r.B[i] <= 4 && zero && !ti59cpu->r.A[i]))
                      s.append(' ');
                  else
                  if (ti59cpu->r.B[i] == 6 || (ti59cpu->r.B[i] == 5 && !ti59cpu->r.A[i]))
                      s.append ('-');
                  else
                  if (ti59cpu->r.B[i] == 5)
                      putchar ('o');
                  else
                  if (ti59cpu->r.B[i] == 4)
                      putchar ('\'');
                  else
                  if (ti59cpu->r.B[3] == 2)
                      putchar ('"');
                  else {
                      s.append ('0' + ti59cpu->r.A[i]);
                      if (ti59cpu->r.A[i])
                          zero = 0;
                  }
                  if (ti59cpu->r.R5 == i)
                      s.append ('.');
              }
#else
              for (i = 13; i >= 2; i--) {
                  putchar ('0'+ti59cpu->r.A[i]);
                  if (ti59cpu->r.R5 == i)
                      putchar ('.');
              }
              putchar (' ');
              for (i = 13; i >= 2; i--)
                  putchar ('0'+ti59cpu->r.B[i]);
#endif
//              putchar ('|'); putchar (' ');
                qWarning()<<"DISPLAY:"<<s;
               displayString = s;
               pLCDC->updated = true;
          }
          disp_filter = 0;
      } else
      if (disp_filter < 3)
          disp_filter++;
      else {
          // display disabled

          if ((ti59cpu->r.flags & FLG_DISP) /*|| (!ti59cpu->r.fA && (ti59cpu->r.flags & FLG_DISP_C))*/ || (ti59cpu->r.fA && !(ti59cpu->r.flags & FLG_DISP_C)))
          {
              ti59cpu->r.flags &= ~FLG_DISP;
              if (ti59cpu->r.fA) {
//                  printf ("\rC            |");
                  ti59cpu->r.flags |= FLG_DISP_C;
                  displayString = "C";
              } else {
//                  printf ("\r             |");
                  ti59cpu->r.flags &= ~FLG_DISP_C;
                  displayString = "";
              }
          }
          pLCDC->updated = true;
      }
  }

  return s;
}

#define KEY(c)	( pKEYB->keyPressedList.contains(TOUPPER(c)) || pKEYB->keyPressedList.contains(c) || pKEYB->keyPressedList.contains(TOLOWER(c)))

UINT8 Cti59::getKey()
{
    UINT8 code = 0;
//    memset(ti59cpu->r.key,0,sizeof(ti59cpu->r.key));
//    if (pKEYB->LastKey)
    {

        KPORT(KEY(K_RS),0x19);
        KPORT(KEY('0'),0x29);
        KPORT(KEY('.'),0x39);
        KPORT(KEY(K_SIGN),0x59);
        KPORT(KEY('='),0x69);

        KPORT(KEY(K_RST),0x18);
        KPORT(KEY('1'),0x28);
        KPORT(KEY('2'),0x38);
        KPORT(KEY('3'),0x58);
        KPORT(KEY('+'),0x68);

        KPORT(KEY(K_SBR),0x17);
        KPORT(KEY('4'),0x27);
        KPORT(KEY('5'),0x37);
        KPORT(KEY('6'),0x57);
        KPORT(KEY('-'),0x67);

        KPORT(KEY(K_GTO),0x16);
        KPORT(KEY('7'),0x26);
        KPORT(KEY('8'),0x36);
        KPORT(KEY('9'),0x56);
        KPORT(KEY('*'),0x66);

        KPORT(KEY(K_BST),0x15);
        KPORT(KEY(K_EE),0x25);
        KPORT(KEY('('),0x35);
        KPORT(KEY(')'),0x55);
        KPORT(KEY('/'),0x65);

        KPORT(KEY(K_SST),0x14);
        KPORT(KEY(K_STO),0x24);
        KPORT(KEY(K_RCL),0x34);
        KPORT(KEY(K_SUM),0x54);
        KPORT(KEY(K_POT),0x64);  // X^Y

        KPORT(KEY(K_LRN),0x13);
        KPORT(KEY(K_XT),0x23);
        KPORT(KEY(K_SQR),0x33);
        KPORT(KEY(K_ROOT),0x53);
        KPORT(KEY(K_1X),0x63);

        KPORT(KEY(K_SHT),0x12);  // 2nd
        KPORT(KEY(K_DEF),0x22);  // INV
        KPORT(KEY(K_LN),0x32);
        KPORT(KEY(K_CE),0x52);
        KPORT(KEY(K_CCE),0x62);  // CLR

        KPORT(KEY('A'),0x11);  // 2nd
        KPORT(KEY('B'),0x21);  // INV
        KPORT(KEY('C'),0x31);
        KPORT(KEY('D'),0x51);
        KPORT(KEY('E'),0x61);  // CLR

        KPORT(!KEY('R'),0x4A);
        KPORT(KEY('P'),0x0C);

    }

    return code;

}

void Cti59::contextMenuEvent ( QContextMenuEvent * event )
{

    QMenu *menu = new QMenu(this);

    BuildContextMenu(menu);

    if (currentModel == TI59) {
        menu->addSeparator();

        QString _fn = QFileInfo(ti59cpu->card_output).fileName();
        changeCardAction = menu->addAction(tr("Card Name : ")+_fn,this,SLOT(changeCard()));

    }

    menu->popup(event->globalPos () );
    event->accept();
}

void Cti59::changeCard()
{
    QString CardFileName = QFileDialog::getSaveFileName(
                    this,
                    "Choose a filename to save under",
                    ".",
                    "Card Files (*.bin)",
                    0,
                    QFileDialog::DontConfirmOverwrite);

    if (!CardFileName.isEmpty()) {
        strcpy(ti59cpu->card_output,qstrdup(CardFileName.toLocal8Bit()));
    }
}
