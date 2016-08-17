#include <QDebug>

#include "ti57.h"
#include "ti57cpu.h"
#include "Lcdc_ti57.h"
#include "Log.h"
#include "Keyb.h"
#include "Inter.h"
#include "Connect.h"



Cti57::Cti57(CPObject *parent)	: CpcXXXX(parent)
{								//[constructor]
    setfrequency( (int) 12500);  // 80µs per cycle
    setcfgfname(QString("ti57"));

    SessionHeader	= "TI57PKM";
    Initial_Session_Fname ="ti57.pkm";

    BackGroundFname	= P_RES(":/ti57/ti57.png");
//    LcdFname		= P_RES(":/ti57/ti57lcd.png");
//    SymbFname		= "";

    memsize		= 0xFFFF;
    InitMemValue	= 0xFF;

    SlotList.clear();
    SlotList.append(CSlot(4 , 0x0000 ,	P_RES(":/ti57/ti57.bin"), ""	, CSlot::ROM , "ROM"));

    setDXmm(78);
    setDYmm(148);
    setDZmm(36);

    setDX(279);
    setDY(529);

//    Lcd_X		= 30;
//    Lcd_Y		= 43;
//    Lcd_DX		= 220;
//    Lcd_DY		= 40;
//    Lcd_ratio_X	= 1;
//    Lcd_ratio_Y	= 1;

    pTIMER		= new Ctimer(this);
    pLCDC		= new Clcdc_ti57(this,
                                 QRect(30,43,220,40),
                                 QRect(),
                                 P_RES(":/ti57/ti57lcd.png"));
    pCPU		= new Cti57cpu(this);    ti57cpu = (Cti57cpu*)pCPU;
    pKEYB->setMap("ti57.map");

    ioFreq = 0;
}

Cti57::~Cti57() {
}

bool Cti57::init(void)				// initialize
{

//pCPU->logsw = true;
#ifndef QT_NO_DEBUG
//    pCPU->logsw = true;
//    if (!fp_log) fp_log=fopen("pc2001.log","wt");	// Open log file
#endif
    CpcXXXX::init();


    return true;
}



bool Cti57::run() {

    TI57regs * _regs = ti57cpu->r;

    _regs->KEYR = getKey();

    if (
            !_regs->Power &&
         (_regs->KEYR !=0 )) {
        _regs->COND = 1;
        _regs->R5 = _regs->KEYR;
        _regs->Timer=0;
        _regs->Power = true;
        ti57cpu->halt = false;
        qWarning()<<"Wakeup";

    }


//    if (ti57cpu->r->KEYR != 0) {
//        qWarning() << "KEY:"<<ti57cpu->r->KEYR;
//    }

    CpcXXXX::run();

    pTIMER->state++;
    return true;
}

bool Cti57::Chk_Adr(UINT32 *d, UINT32 data) { return false; }
bool Cti57::Chk_Adr_R(UINT32 *d, UINT32 *data) { return true; }
UINT8 Cti57::in(UINT8 Port,QString) { return 0;}
UINT8 Cti57::out(UINT8 Port, UINT8 x, QString sender) { return 0; }
bool Cti57::Set_Connector(Cbus *_bus) { return true; }
bool Cti57::Get_Connector(Cbus *_bus) { return true; }


void Cti57::TurnOFF(void) {
    ASKYN _tmp = mainwindow->saveAll;
    mainwindow->saveAll = YES;
    CpcXXXX::TurnOFF();
    mainwindow->saveAll = _tmp;
    AddLog(LOG_TEMP,"TURN OFF");

}

void Cti57::TurnON(void){
    CpcXXXX::TurnON();

    ti57cpu->r->Run = true;
    ti57cpu->r->Power = !ti57cpu->r->Power;
    ti57cpu->halt = !ti57cpu->halt;
    pLCDC->updated = true;
//    Reset();
//    ti57cpu->r->PC = 0x0052;

}


void Cti57::Reset()
{
    CpcXXXX::Reset();

}

bool Cti57::LoadConfig(QXmlStreamReader *xmlIn)
{
    return true;
}

bool Cti57::SaveConfig(QXmlStreamWriter *xmlOut)
{
    return true;
}

QString Cti57::Display() {
  QChar c;
  QString s;

  if (!pCPU) return "";
  s="";
  if (getPower())
    for (int i = 11;i >=0; i--) {
        if (ti57cpu->r->RB[i] & 8) c=' ';
        else if (ti57cpu->r->RB[i] & 1) c='-';
        else
            switch(ti57cpu->r->RA[i]) {
            case 0:
            case 1:
            case 2:
            case 3:
            case 4:
            case 5:
            case 6:
            case 7:
            case 8:
            case 9: c=QChar(0x30 + ti57cpu->r->RA[i]); break;
            default: qWarning()<<"default:"<<i;c=QChar(0x41-10+ti57cpu->r->RA[i]); break;
            }
        s.append(c);
        if (ti57cpu->r->RB[i] & 2) s.append('.');
    }
  // Check for error
  if (ti57cpu->r->RA[14]==0x0e) {
      // error, blink screen
      s.prepend('E');
  }

//  qWarning()<<"DISPLAY:"<<s;
  return s;
}


UINT8 Cti57::getKey()
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

