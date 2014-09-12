#include <QDebug>

#include "fx8000g.h"
#include "upd1007.h"
#include "hd44352.h"
#include "Lcdc_fx8000g.h"
#include "Log.h"
#include "Keyb.h"
#include "Inter.h"
#include "Connect.h"



Cfx8000g::Cfx8000g(CPObject *parent)	: CpcXXXX(parent)
{								//[constructor]
    setfrequency( (int) 910000);  // 80µs per cycle
    setcfgfname(QString("fx8000g"));

    SessionHeader	= "FX8000GPKM";
    Initial_Session_Fname ="fx8000g.pkm";

    BackGroundFname	= P_RES(":/fx8000g/fx8000g.png");
    LcdFname		= P_RES(":/fx8000g/fx8000glcd.png");
    SymbFname		= "";

    memsize		= 0xFFFF;
    InitMemValue	= 0xFF;

    SlotList.clear();
    SlotList.append(CSlot(4 , 0x3000 ,	P_RES(":/fx8000g/rom1a.bin"), ""	, CSlot::ROM , "ROM"));
    SlotList.append(CSlot(16 ,0x4000 ,	"", ""	, CSlot::RAM , "RAM"));
    SlotList.append(CSlot(16 ,0x8000 ,	P_RES(":/fx8000g/rom1b.bin"), ""	, CSlot::ROM , "ROM"));
    SlotList.append(CSlot(16 , 0xC000 ,	"", ""	, CSlot::RAM , "RAM"));
    setDXmm(78);
    setDYmm(148);
    setDZmm(36);

    setDX(321);
    setDY(687);

    Lcd_X		= 90;
    Lcd_Y		= 130;
    Lcd_DX		= 192;//168;//144 ;
    Lcd_DY		= 32;
    Lcd_ratio_X	= 1;// * 1.18;
    Lcd_ratio_Y	= 1;// * 1.18;

    Lcd_Symb_X	= 55;
    Lcd_Symb_Y	= 41;
    Lcd_Symb_DX	= 339;
    Lcd_Symb_DY	= 5;
    Lcd_Symb_ratio_X	= 1;

    pTIMER		= new Ctimer(this);
    pLCDC		= new Clcdc_fx8000g(this);
    pCPU		= new CUPD1007(this,P_RES(":/fx8000g/rom0.bin"));
    fx8000gcpu = (CUPD1007*)pCPU;
    pKEYB		= new Ckeyb(this,"fx8000g.map");
    pHD44352    = new CHD44352(P_RES(":/fx8000g/charset.bin"));

    ioFreq = 0;
}

Cfx8000g::~Cfx8000g() {
    delete pHD44352;
}

bool Cfx8000g::init(void)				// initialize
{

//pCPU->logsw = true;
#ifndef QT_NO_DEBUG
//    pCPU->logsw = true;
//    if (!fp_log) fp_log=fopen("pc2001.log","wt");	// Open log file
#endif
    CpcXXXX::init();

    pHD44352->init();

    return true;
}



bool Cfx8000g::run() {


    CpcXXXX::run();

    return true;
}

bool Cfx8000g::Chk_Adr(UINT32 *d, UINT32 data) {

    Q_UNUSED(data)

    if ( (*d>=0x4000) && (*d<=0x7FFF) )	{ return(true);	}
    if ( (*d>=0xC000) && (*d<=0xFFFF) )	{ return(true);	}

    return false;
}

bool Cfx8000g::Chk_Adr_R(UINT32 *d, UINT32 *data) {

    return true;
}

UINT8 Cfx8000g::in(UINT8 Port) {

    switch (Port) {
    case 1: {
        BYTE ret = pHD44352->data_read();
        qWarning()<<"HD44352 data_read:"<<ret;
        return ret;
    }
    }

    return 0;
}

UINT8 Cfx8000g::out(UINT8 Port, UINT8 x) {
    switch (Port) {
    case 0:
        qWarning()<<"HD44352 Ctrl_write:"<<x;
        pHD44352->control_write(x); pLCDC->redraw = true; break;
    case 1:
        qWarning()<<"HD44352 Data_write:"<<x;pHD44352->data_write(x); pLCDC->redraw = true; break;
    }

    return 0;
}

bool Cfx8000g::Set_Connector() { return true; }
bool Cfx8000g::Get_Connector() { return true; }


void Cfx8000g::TurnOFF(void) {
    mainwindow->saveAll = YES;
    CpcXXXX::TurnOFF();
    mainwindow->saveAll = ASK;
    AddLog(LOG_TEMP,"TURN OFF");

}

void Cfx8000g::TurnON(void){
    CpcXXXX::TurnON();

//    ti57cpu->r->Run = true;
//    ti57cpu->r->Power = !ti57cpu->r->Power;
//    ti57cpu->halt = !ti57cpu->halt;
//    pLCDC->updated = true;


}


void Cfx8000g::Reset()
{
    CpcXXXX::Reset();

}

bool Cfx8000g::LoadConfig(QXmlStreamReader *xmlIn)
{
    return true;
}

bool Cfx8000g::SaveConfig(QXmlStreamWriter *xmlOut)
{
    return true;
}

#define KEY(c)	( pKEYB->keyPressedList.contains(TOUPPER(c)) || pKEYB->keyPressedList.contains(c) || pKEYB->keyPressedList.contains(TOLOWER(c)))

UINT8 Cfx8000g::getKey()
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


