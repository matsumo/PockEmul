#include <QtGui>
#include <QString>

#include "common.h"
#include "fp200.h"
#include "Lcdc_fp200.h"
#include "i8085.h"
#include "rp5c01.h"

#include "Inter.h"
#include "Keyb.h"
#include "cextension.h"
#include "uart.h"
#include "ctronics.h"

#include "Connect.h"
#include "watchpoint.h"

#include "Log.h"

// TODO: MultiTouch Events


#define STROBE_TIMER 5

Cfp200::Cfp200(CPObject *parent)	: CpcXXXX(parent)
{								//[constructor]
    setfrequency( (int) 6144000/2);
    setcfgfname(QString("fp200"));

    SessionHeader	= "FP200PKM";
    Initial_Session_Fname ="fp200.pkm";

    BackGroundFname	= P_RES(":/fp200/fp200.png");

    TopFname    = P_RES(":/fp200/fp200Top.png");

    memsize		= 0x10000;
    InitMemValue	= 0xFF;

    SlotList.clear();
    SlotList.append(CSlot(32 , 0x0000 ,	P_RES(":/fp200/fp200rom.bin")   , ""	, CSlot::ROM , "ROM"));
    SlotList.append(CSlot(8  , 0x8000 ,	""                  , ""	, CSlot::RAM , "RAM"));
    SlotList.append(CSlot(8  , 0xa000 ,	""                  , ""	, CSlot::RAM , "RAM"));
    SlotList.append(CSlot(8  , 0xc000 ,	""                  , ""	, CSlot::RAM , "RAM"));
    SlotList.append(CSlot(8  , 0xe000 ,	""                  , ""	, CSlot::RAM , "RAM"));

    setDXmm(320);
    setDYmm(220);
    setDZmm(55);

    setDX(1034);
    setDY(705);

    pLCDC		= new Clcdc_fp200(this,
                                  QRect(158,57,320,128),
                                  QRect());
    pCPU		= new Ci8085(this);
    pTIMER		= new Ctimer(this);
    pKEYB		= new Ckeyb(this,"fp200.map");
//    pUART        = new Cuart(this);
    pCENT       = new Cctronics(this);
    pRP5C01     = new CRP5C01(this);

    lastKeyBufSize = 0;
    newKey = false;

    ioFreq = 0;             // Mandatory for Centronics synchronization
    i85cpu = (Ci8085*)pCPU;

    initExtension();
}

Cfp200::~Cfp200() {
    delete pCENTCONNECTOR;
//    delete pSIOCONNECTOR;
//    delete pUART;
    delete pCENT;
    delete pRP5C01;
}



bool Cfp200::Chk_Adr(UINT32 *d, UINT32 data)
{
    Q_UNUSED(data)

    if (                 (*d<=0x7FFF) )	return(false);		// ROM area(0000-7FFF)
    if ( (*d>=0x8000) && (*d<=0x9FFF) )	{ return(true);	}
    if ( (*d>=0xA000) && (*d<=0xBFFF) )	{ return(ext_MemSlot1->ExtArray[ID_FP201]->IsChecked);	}
    if ( (*d>=0xC000) && (*d<=0xDFFF) )	{ return(ext_MemSlot2->ExtArray[ID_FP201]->IsChecked);	}
    if ( (*d>=0xE000) && (*d<=0xFFFF) )	{ return(ext_MemSlot3->ExtArray[ID_FP201]->IsChecked || ext_MemSlot3->ExtArray[ID_FP205]->IsChecked || ext_MemSlot3->ExtArray[ID_FP231CE]->IsChecked);	}

    return true;
}

bool Cfp200::Chk_Adr_R(UINT32 *d, UINT32 *data)
{
    Q_UNUSED(d)
    Q_UNUSED(data)

    return true;
}

#define SOD (i85cpu->i85stat.regs.sod_state)

UINT8 Cfp200::in(UINT8 Port)
{
    UINT8 Value=0;
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
     return (Value);
}

UINT8 Cfp200::out(UINT8 Port, UINT8 Value)
{
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
    return 0;
}

UINT8 Cfp200::in8(UINT16 Port)
{
    Q_UNUSED(Port)

    return 0;
}



UINT8 Cfp200::out8(UINT16 Port, UINT8 x)
{
    Q_UNUSED(Port)
    Q_UNUSED(x)

    return 0;
}

UINT16 Cfp200::in16(UINT16 Port)
{
    Q_UNUSED(Port)

    return 0;
}

UINT16 Cfp200::out16(UINT16 Port, UINT16 x)
{
    Q_UNUSED(Port)
    Q_UNUSED(x)

    return 0;
}

bool Cfp200::init()
{
//    pCPU->logsw = true;
#ifndef QT_NO_DEBUG
    pCPU->logsw = false;
#endif
    CpcXXXX::init();
    Reset();
    Cetl = false;
    sid = 0;

    pCENTCONNECTOR = new Cconnector(this,36,1,Cconnector::Centronics_36,"Parrallel Connector",false,QPoint(715,50));
    publish(pCENTCONNECTOR);
    pSIOCONNECTOR = new Cconnector(this,9,2,Cconnector::DIN_8,"Serial Connector",false,QPoint(0,50));
    publish(pSIOCONNECTOR);

//    pUART->init();
//    pUART->pTIMER = pTIMER;
    pCENT->pTIMER = pTIMER;
    pCENT->init();
    pCENT->setBufferSize(10);


    pRP5C01->init();

    QHash<int,QString> lbl;
    lbl.clear();
    lbl[1] = "STROBE";
    lbl[2] = "D1";
    lbl[3] = "D2";
    lbl[4] = "D3";
    lbl[5] = "D4";
    lbl[6] = "D5";
    lbl[7] = "D6";
    lbl[8] = "D7";
    lbl[9] = "D8";
    lbl[10]= "ACK";
    lbl[11]= "BUSY";
    lbl[31]= "INIT";
    lbl[32]= "ERROR";
    WatchPoint.add(&pCENTCONNECTOR_value,64,36,this,"Centronic 36pins connector",lbl);

    return true;
}

void	Cfp200::initExtension(void)
{
    AddLog(LOG_MASTER,"INIT EXT FP-200");
    // initialise ext_MemSlot1
    ext_MemSlot1 = new CExtensionArray("RAM Slot 1","Add RAM Module");
    ext_MemSlot2 = new CExtensionArray("RAM Slot 2","Add RAM Module");
    ext_MemSlot3 = new CExtensionArray("RAM/ROM Slot 3","Add RAM or ROM Module");
    ext_MemSlot1->setAvailable(ID_FP201,true); ext_MemSlot1->setChecked(ID_FP201,false);
    ext_MemSlot2->setAvailable(ID_FP201,true); ext_MemSlot2->setChecked(ID_FP201,false);
    ext_MemSlot3->setAvailable(ID_FP201,true); ext_MemSlot3->setChecked(ID_FP201,false);
    ext_MemSlot3->setAvailable(ID_FP205,true);
    ext_MemSlot3->setAvailable(ID_FP231CE,true);

    addExtMenu(ext_MemSlot1);
    addExtMenu(ext_MemSlot2);
    addExtMenu(ext_MemSlot3);
    extensionArray[0] = ext_MemSlot1;
    extensionArray[1] = ext_MemSlot2;
    extensionArray[2] = ext_MemSlot3;
}

bool Cfp200::run()
{

    pCENTCONNECTOR_value = pCENTCONNECTOR->Get_values();
    pSIOCONNECTOR_value = pSIOCONNECTOR->Get_values();

    if (ks==5) i85cpu->i8085_set_SID(Cetl?0:1);
    if (ks==6) i85cpu->i8085_set_SID(pKEYB->isShift?0:1);
    if (ks==7) i85cpu->i8085_set_SID(pKEYB->LastKey == 0x03 ? 0:1);        // BREAK
//    if (ks==8) i85cpu->i8085_set_SID(1);        // GRAPH
    if (ks==9) i85cpu->i8085_set_SID(pKEYB->isCtrl?0:1);

    if (pKEYB->LastKey>0) {

        i85cpu->i8085_set_irq_line(I8085_RST75_LINE,1);
    }
    else
        i85cpu->i8085_set_irq_line(I8085_RST75_LINE,0);

    CpcXXXX::run();

    pCENT->run();
    pRP5C01->step();


    pCENTCONNECTOR_value = pCENTCONNECTOR->Get_values();
    pSIOCONNECTOR_value = pSIOCONNECTOR->Get_values();

    return true;
}

void Cfp200::Reset()
{
    CpcXXXX::Reset();

}

void Cfp200::TurnON()
{
    CpcXXXX::TurnON();
//    pCPU->Reset();
}

void Cfp200::TurnOFF()
{
    mainwindow->saveAll = YES;
    CpcXXXX::TurnOFF();
    mainwindow->saveAll = ASK;
}

bool Cfp200::SaveConfig(QXmlStreamWriter *xmlOut)
{
    Q_UNUSED(xmlOut)

    return true;
}

bool Cfp200::LoadConfig(QXmlStreamReader *xmlIn)
{
    Q_UNUSED(xmlIn)

    return true;
}

void Cfp200::ExtChanged()
{
    CpcXXXX::ExtChanged();
    Reset();
}

#define KEY(c)	( pKEYB->keyPressedList.contains(TOUPPER(c)) || pKEYB->keyPressedList.contains(c) || pKEYB->keyPressedList.contains(TOLOWER(c)))

//#define KEY(c)	( TOUPPER(pKEYB->LastKey) == TOUPPER(c) )
quint16 Cfp200::getKey()
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

//void Cfp200::keyReleaseEvent(QKeyEvent *event)
//{
////if (event->isAutoRepeat()) return;

//    if (pCPU->fp_log) fprintf(pCPU->fp_log,"\nKEY RELEASED= %c\n",event->key());
//    CpcXXXX::keyReleaseEvent(event);
//}


//void Cfp200::keyPressEvent(QKeyEvent *event)
//{
////    if (event->isAutoRepeat()) return;

//    if (pCPU->fp_log) fprintf(pCPU->fp_log,"\nKEY PRESSED= %c\n",event->key());
//    CpcXXXX::keyPressEvent(event);
//}

bool Cfp200::Get_Connector(Cbus *_bus) {
    Get_MainConnector();
    Get_CentConnector();
    Get_SIOConnector();

    return true;
}
bool Cfp200::Set_Connector(Cbus *_bus) {
    Set_SIOConnector();
    Set_CentConnecor();
    Set_MainConnector();

    return true;
}

void Cfp200::Get_CentConnector(void) {

    pCENT->Set_ACK( pCENTCONNECTOR->Get_pin(10));
    pCENT->Set_BUSY( pCENTCONNECTOR->Get_pin(11));
    pCENT->Set_ERROR( pCENTCONNECTOR->Get_pin(32));
}

void Cfp200::Set_CentConnecor(void) {

    pCENTCONNECTOR->Set_pin((1) ,pCENT->Get_STROBE());

    quint8 d = pCENT->Get_DATA();
    pCENTCONNECTOR->Set_pin(2	,READ_BIT(d,0));
    pCENTCONNECTOR->Set_pin(3	,READ_BIT(d,1));
    pCENTCONNECTOR->Set_pin(4	,READ_BIT(d,2));
    pCENTCONNECTOR->Set_pin(5	,READ_BIT(d,3));
    pCENTCONNECTOR->Set_pin(6	,READ_BIT(d,4));
    pCENTCONNECTOR->Set_pin(7	,READ_BIT(d,5));
    pCENTCONNECTOR->Set_pin(8	,READ_BIT(d,6));
    pCENTCONNECTOR->Set_pin(9	,READ_BIT(d,7));

    pCENTCONNECTOR->Set_pin(31	,pCENT->Get_INIT());

}

void Cfp200::Get_SIOConnector(void) {

}

void Cfp200::Set_SIOConnector(void) {

}

#define PIN(x)    (pCONNECTOR->Get_pin(x))
bool Cfp200::Get_MainConnector(void) {



    return true;
}

#define PORT(x)  (port << (x))
bool Cfp200::Set_MainConnector(void) {



    return true;
}
