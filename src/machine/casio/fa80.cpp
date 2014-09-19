

#include <QPainter>
#include <QTime>
#include <QFileDialog>
#include <QThread>
#include <QPixmap>
#include <QBitmap>

#include "common.h"

#include "fa80.h"
#include "Connect.h"
#include "Inter.h"
#include "init.h"
#include "Log.h"
#include "dialoganalog.h"
#include "bus.h"


Cfa80::Cfa80(CPObject *parent):CPObject(parent)
{								//[constructor]
    setfrequency( 0);

    BackGroundFname	= P_RES(":/fx8000g/fa80.png");
    setcfgfname("fa80");

    TopFname = P_RES(":/fx8000g/fa80top.png");
    LeftFname = P_RES(":/fx8000g/fa80left.png");

    pTIMER		= new Ctimer(this);

    setDX(714);
    setDY(693);

    setDXmm(200);
    setDYmm(194);
    setDZmm(37);

    port = 0x08;
    bus = new Cbus();
    adrBus=prev_adrBus=0;

}

Cfa80::~Cfa80() {
    delete pCONNECTOR;
    delete pCENTCONNECTOR;
//    delete pSIOCONNECTOR;
}

bool Cfa80::UpdateFinalImage(void) {
    CPObject::UpdateFinalImage();


    QPainter painter;
    painter.begin(FinalImage);

    // POWER LED
    if (READ_BIT(port,3)==0) {
        painter.fillRect(698,550,18,9,QColor(Qt::green));
    }

    painter.end();
    return true;

}



bool Cfa80::init(void)
{
    CPObject::init();

    setfrequency( 0);

    pCONNECTOR	   = new Cconnector(this,30,0,Cconnector::Casio_30,"Connector 30 pins",true,QPoint(414,375));	publish(pCONNECTOR);
    pCENTCONNECTOR = new Cconnector(this,36,1,Cconnector::Centronics_36,"Centronic 36 pins",false,QPoint(417,13)); publish(pCENTCONNECTOR);

    WatchPoint.add(&pCONNECTOR_value,64,30,this,"Standard 30pins connector");

    WatchPoint.add(&pCENTCONNECTOR_value,64,36,this,"Centronic 36pins connector");

    AddLog(LOG_PRINTER,tr("FA-80 initializing..."));

    if(pTIMER)	pTIMER->init();


    data = 0;
    sendData = false;

    return true;
}



#define TIMER_ACK 9
#define TIMER_BUSY 8
void Cfa80::printerControlPort(BYTE value)
{
//    bit 0 - state of the STROBE output
//    bit 1 - state of the INIT output
//    bit 2 - logical 1 resets the ACK latch
    AddLog(LOG_PRINTER,tr("PRINTER controlPort= %1").arg(value,2,16,QChar('0')));
    if (mainwindow->dialoganalogic) mainwindow->dialoganalogic->setMarker(8);

    printerSTROBE = (value & 0x01)?true:false;
    printerINIT = (value & 0x02)?true:false;


    if (value & 0x04) {
        printerACK = false;
//        pTIMER->resetTimer(9);
        AddLog(LOG_PRINTER,tr("PRINTER controlPort RESET ACK"));
    }

    prev_printerSTROBE = printerSTROBE;
    prev_printerINIT = printerINIT;
}

BYTE Cfa80::printerStatusPort()
{
//    bit 0 - state of the BUSY input
//    bit 1 - state of the FAULT input
//    bit 2 - set by a pulse on the ACK input, reset by writing logical 1 to the bit 2 of register 110


    if (mainwindow->dialoganalogic) mainwindow->dialoganalogic->setMarker(9);
    BYTE ret = 0;

    if (printerBUSY) ret |= 0x01;
//    if (printerERROR)
        ret |= 0x02;        // FAULT to 1
    if (!printerACK) ret |= 0x04;

    if (ret != prev_printerStatusPort) {
        AddLog(LOG_PRINTER,tr("PRINTER return status PORT : %1").arg(ret,2,16,QChar('0')));
        prev_printerStatusPort = ret;
    }

    return (ret & 0x07);
}

void Cfa80::printerDataPort(BYTE value)
{
    if (value != 0xff) {
        AddLog(LOG_PRINTER,tr("PRINTER data : %1").arg(value,2,16,QChar('0')));
        if (mainwindow->dialoganalogic) mainwindow->dialoganalogic->setMarker(7);
        printerDATA = value;
    }
}

/*****************************************************/
/* Exit PRINTER										 */
/*****************************************************/
bool Cfa80::exit(void)
{
    AddLog(LOG_PRINTER,"FA-80 Closing...");
    AddLog(LOG_PRINTER,"done.");
    CPObject::exit();
    return true;
}

bool Cfa80::Get_Connector(void) {
    Get_MainConnector();
    Get_CentConnector();

    return true;
}
bool Cfa80::Set_Connector(void) {
    Set_CentConnecor();
    Set_MainConnector();

    return true;
}

void Cfa80::Get_CentConnector(void) {

    printerACK = pCENTCONNECTOR->Get_pin(10);
    printerBUSY = pCENTCONNECTOR->Get_pin(11);
    printerERROR=pCENTCONNECTOR->Get_pin(32);
}

void Cfa80::Set_CentConnecor(void) {

    pCENTCONNECTOR->Set_pin((1) ,printerSTROBE);

    pCENTCONNECTOR->Set_pin(2	,READ_BIT(printerDATA,0));
    pCENTCONNECTOR->Set_pin(3	,READ_BIT(printerDATA,1));
    pCENTCONNECTOR->Set_pin(4	,READ_BIT(printerDATA,2));
    pCENTCONNECTOR->Set_pin(5	,READ_BIT(printerDATA,3));
    pCENTCONNECTOR->Set_pin(6	,READ_BIT(printerDATA,4));
    pCENTCONNECTOR->Set_pin(7	,READ_BIT(printerDATA,5));
    pCENTCONNECTOR->Set_pin(8	,READ_BIT(printerDATA,6));
    pCENTCONNECTOR->Set_pin(9	,READ_BIT(printerDATA,7));

    pCENTCONNECTOR->Set_pin(31	,printerINIT);

}

#define PIN(x)    (pCONNECTOR->Get_pin(x))
bool Cfa80::Get_MainConnector(void) {

    bus->fromUInt64(pCONNECTOR->Get_values());

    return true;
}

#define PORT(x)  (port << (x))
bool Cfa80::Set_MainConnector(void) {

    pCONNECTOR->Set_values(bus->toUInt64());

    return true;
}

#define DOWN	0
#define UP		1

#define LOG { \
    qWarning()<<(bus->isWrite()?"WRITE ":"READ :")<<QString("%1").arg(adrBus,4,16,QChar('0')) \
              << "data:"<<QString("%1").arg(data,4,16,QChar('0'))<<QChar(data); \
        }

bool Cfa80::run(void)
{
    Get_Connector();

    if (!bus->isEnable()) {
//        if (keyEvent) {
//            pLH5810->step();
//            bus->setINT(pLH5810->INT);
//            pCONNECTOR->Set_values(bus->toUInt64());
//        }
        return true;
    }
    pCONNECTOR_value = pCONNECTOR->Get_values();
    pCENTCONNECTOR_value = pCENTCONNECTOR->Get_values();

    adrBus = bus->getAddr();
    data = bus->getData();

    if (bus->isWrite()) {
        switch (adrBus) {
        case 0x00: break;
        case 0x01: break;
        case 0x02: break;
        case 0x03: break;
        case 0x04: break;
        case 0x05: LOG;
            printerDataPort(data);
            break;
        case 0x06:
            printerControlPort(data);
            break;
        case 0x07: break;
        }
    }
    else {
        switch (adrBus) {
        case 0x00: break;
        case 0x01: break;
        case 0x02: break;
        case 0x03: bus->setData(0xF4); break;
        case 0x04: bus->setData(printerStatusPort()); break;   // 0x02
        case 0x05: break;
        case 0x06: break;
        case 0x07: break;
        }
    }
//        else if (adrBus==0x05) {
//            printerDataPort(data);
//        }
//        else if ((adrBus==0x06))//&&(P2==UP))
//        {
//            printerControlPort(data);
//        }
//        else if (adrBus==0x04) {
//            // Send the printerStatusPort
//            sendData = true;
//            out_adrBus = 0x04;
//            data = printerStatusPort();
//        }
//        else if (adrBus == 0x00) {


    Set_Connector();
    pCONNECTOR_value = pCONNECTOR->Get_values();
    pCENTCONNECTOR_value = pCENTCONNECTOR->Get_values();
//    pSIOCONNECTOR_value = pSIOCONNECTOR->Get_values();

    return true;
}



