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
    SymbFname	= "";

    LeftFname   = P_RES(":/fx8000g/fx8000gleft.png");
    BackFname   = P_RES(":/fx8000g/fx8000gback.png");

    memsize		= 0xFFFF;
    InitMemValue	= 0x00;

    SlotList.clear();
    SlotList.append(CSlot(4 , 0x3000 ,	P_RES(":/fx8000g/rom1a.bin"), ""	, CSlot::ROM , "ROM"));
    SlotList.append(CSlot(8 ,0x4000 ,	"", ""	, CSlot::RAM , "RAM"));
    SlotList.append(CSlot(16 ,0x8000 ,	P_RES(":/fx8000g/rom1b.bin"), ""	, CSlot::ROM , "ROM"));
//    SlotList.append(CSlot(16 , 0xC000 ,	"", ""	, CSlot::RAM , "RAM"));
    setDXmm(82);
    setDYmm(165);
    setDZmm(15);

    setDX(321);
    setDY(687);

    Lcd_X		= 64;
    Lcd_Y		= 54;
    Lcd_DX		= 96;// 194x134
    Lcd_DY		= 64;
    Lcd_ratio_X	= 2;
    Lcd_ratio_Y	= 2;

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
    pHD44352->OP_bit = 0x20;
    pHD44352->byteLenght = 4;

    ioFreq = 0;
}

Cfx8000g::~Cfx8000g() {
    delete pHD44352;
}

#define LCD_TIMER 3

bool Cfx8000g::init(void)				// initialize
{

//pCPU->logsw = true;
#ifndef QT_NO_DEBUG
//    pCPU->logsw = true;
//    if (!fp_log) fp_log=fopen("fx8000g.log","wt");	// Open log file
#endif
    CpcXXXX::init();

    pHD44352->init();

    pTIMER->resetTimer(LCD_TIMER);
    return true;
}



bool Cfx8000g::run() {

    timerRate = pHD44352->on_timer_rate;

#if 1
    if ((( fx8000gcpu->reginfo.lcdctrl & 0x80) == 0) |
        (( fx8000gcpu->reginfo.iereg & 0x84) == 0x84)) {
        fx8000gcpu->reginfo.ifreg = fx8000gcpu->reginfo.ifreg & ~fx8000gcpu->INT_input[1];
        pTIMER->resetTimer(LCD_TIMER);
    }
    else {
        // lcd test fire int1 each 20ms
        if (pTIMER->stElapsedId(LCD_TIMER) >= 2*timerRate) {
            fx8000gcpu->reginfo.ifreg ^= fx8000gcpu->INT_input[1];
            if ((fx8000gcpu->reginfo.ifreg & fx8000gcpu->INT_input[1]) !=0) {
                fx8000gcpu->IntReq(&fx8000gcpu->reginfo,1);
                //            qWarning()<<"INT1";
            }
            pTIMER->resetTimer(LCD_TIMER);
        }
    }
#else

#endif

    getKey();

    CpcXXXX::run();

    return true;
}

bool Cfx8000g::Chk_Adr(UINT32 *d, UINT32 data) {

    Q_UNUSED(data)

#if 0

    if ( (*d>=0x4000) && (*d<=0x7FFF) )	{ *d = (*d & 0x1FFF) + 0x4000; return(true);	}
    if ( (*d>=0xC000) && (*d<=0xFFFF) )	{ *d = (*d & 0x1FFF) + 0x4000; return(true);	}
#else
    if (*d & 0x4000) {    *d = (*d & 0x1FFF) + 0x4000; return(true);	}
#endif
    return false;
}

bool Cfx8000g::Chk_Adr_R(UINT32 *d, UINT32 *data) {

    if ( (*d>=0x0000) && (*d<=0x2FFF) )	{ *data = 0xff; return false; }
    if ( (*d>=0x4000) && (*d<=0x7FFF) )	{ *d = (*d & 0x1FFF) + 0x4000; return(true);	}
    if ( (*d>=0xC000) && (*d<=0xFFFF) )	{ *d = (*d & 0x1FFF) + 0x4000; return(true);	}

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
//        qWarning()<<"HD44352 Ctrl_write:"<<(x);
        pHD44352->control_write(x); pLCDC->redraw = true; break;
    case 1:
//        qWarning()<<"HD44352 Data_write:"<<QString("%1").arg(x,2,16,QChar('0'))<<QChar(x);
//        if (pCPU->fp_log) fprintf(pCPU->fp_log,"\nLcdTransfert:%02X\n",x);
        pHD44352->data_write4(x & 0x0F);
        pLCDC->redraw = true;
        break;
    case 2:
//        qWarning()<<"HD44352 Ctrl_write:"<<(x);
        pHD44352->sync(); break;
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

//    pLCDC->updated = true;

    Reset();

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
    UINT8 data=0;

    UINT32 ks = fx8000gcpu->reginfo.koreg;

    if (ks<0x40) return 0;

    if ((pKEYB->LastKey) )
    {
        AddLog(LOG_KEYBOARD,tr("GetKEY : %1").arg(ks,4,16,QChar('0')));
        if ((ks & 0x41)==0x41) {
            if (KEY(K_CE))     data|=0x01;
        }

        if ((ks & 0x42)==0x42) {
            if (KEY(K_SHT))     data|=0x01;
            if (KEY(K_ALPHA))	{
#if 0
                pCPU->logsw = true;
                pCPU->Check_Log();
#else
                data|=0x02;
#endif
            }
            if (KEY(K_MOD))		data|=0x04;
            if (KEY(K_RET))		data|=0x08;
//            if (KEY(''))		data|=0x10;
//            if (KEY(''))		data|=0x20;
            if (KEY(K_POW_OFF))	data|=0x40;
            if (KEY(K_INIT))	data|=0x80;			// UP ARROW
        }
        if ((ks & 0x44)==0x44) {
            if (KEY(K_DISP))	data|=0x01;
            if (KEY(K_GRAPH))	data|=0x02;
            if (KEY(K_RANGE))	data|=0x04;			// OFF
            if (KEY(K_LA))		data|=0x08;
            if (KEY(K_RA))		data|=0x10;
            if (KEY(K_UA))		data|=0x20;
            if (KEY(K_DA))		data|=0x40;
            if (KEY(K_GT))		data|=0x80;
        }
        if ((ks & 0x50)==0x50) {   // KO5
            if (KEY(':'))		data|=0x01;
            if (KEY(K_ENG))		data|=0x02;
            if (KEY(K_PROG))	data|=0x04;
            if (KEY(K_ROOT))	data|=0x08;
            if (KEY(K_SQR))		data|=0x10;
            if (KEY(K_LOG))		data|=0x20;
            if (KEY(K_LN))		data|=0x40;
            if (KEY('F'))		data|=0x80;
        }
        if ((ks & 0x60)==0x60) {
            if (KEY('G'))		data|=0x01;
            if (KEY('A'))		data|=0x02;
            if (KEY('B'))		data|=0x04;
            if (KEY('C'))		data|=0x08;
            if (KEY('D'))		data|=0x10;
            if (KEY('E'))		data|=0x20;
            if (KEY('K'))		data|=0x40;
            if (KEY('L'))       data|=0x80;
        }
        if ((ks & 0x81)==0x81) {
            if (KEY('7'))			data|=0x01;
            if (KEY('M'))			data|=0x01;
            if (KEY('8'))			data|=0x02;
            if (KEY('N'))			data|=0x02;
            if (KEY('9'))			data|=0x04;
            if (KEY('O'))			data|=0x04;
            if (KEY('4'))			data|=0x08;
            if (KEY('P'))			data|=0x08;
            if (KEY('5'))			data|=0x10;
            if (KEY('Q'))			data|=0x10;
            if (KEY('6'))			data|=0x20;
            if (KEY('R'))			data|=0x20;
            if (KEY('*'))			data|=0x40;
            if (KEY('S'))			data|=0x40;
            if (KEY('/'))			data|=0x80;
            if (KEY('T'))			data|=0x80;
        }
        if ((ks & 0x82)==0x82) {
            if (KEY('1'))			data|=0x01;
            if (KEY('U'))			data|=0x01;
            if (KEY('2'))			data|=0x02;
            if (KEY('V'))			data|=0x02;
            if (KEY('3'))			data|=0x04;
            if (KEY('W'))			data|=0x04;
            if (KEY('0'))			data|=0x08;
            if (KEY('.'))			data|=0x10;
            if (KEY(K_EXP))			data|=0x20;
            if (KEY('+'))			data|=0x40;
            if (KEY('X'))			data|=0x40;
            if (KEY('-'))			data|=0x80;
            if (KEY('Y'))			data|=0x80;
        }
        if ((ks & 0x84)==0x84) {
//            if (KEY(''))			data|=0x01;			// R ARROW
//            if (KEY(''))			data|=0x02;			// MODE
//            if (KEY(''))			data|=0x04;			// CLS
            if (KEY('H'))			data|=0x08;
            if (KEY('I'))			data|=0x10;
            if (KEY('J'))			data|=0x20;
            if (KEY(K_DEL))			data|=0x40;
            if (KEY(K_ANS))			data|=0x80;
        }
//    qWarning()<<"ko="<<QString("%1").arg(ks,2,16,QChar('0'))<< "   ki="<<QString("%1").arg(data,2,16,QChar('0'));

    }
    fx8000gcpu->reginfo.kireg = data;
    if (data) fx8000gcpu->reginfo.CpuSleep = fx8000gcpu->halt = false;

    return data;

}


