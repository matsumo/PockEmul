

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
#include "watchpoint.h"
#include "Log.h"
#include "bus.h"
#include "hd61710.h"


Cfa80::Cfa80(CPObject *parent):CPObject(parent)
{								//[constructor]
    setfrequency( 1000000);

    BackGroundFname	= P_RES(":/fx8000g/fa80.png");
    setcfgfname("fa80");

    TopFname = P_RES(":/fx8000g/fa80top.png");
    LeftFname = P_RES(":/fx8000g/fa80left.png");

    pTIMER		= new Ctimer(this);

    pHD61710 = new CHD61710(this);


    setDX(714);
    setDY(693);

    setDXmm(200);
    setDYmm(194);
    setDZmm(37);

    port = 0x08;
    bus = new Cbus();
    adrBus=0;

}

Cfa80::~Cfa80() {
    delete pCONNECTOR;
    delete pCENTCONNECTOR;
    delete pHD61710;
}

bool Cfa80::UpdateFinalImage(void) {
    CPObject::UpdateFinalImage();

    paintingImage.lock();

    QPainter painter;
    painter.begin(FinalImage);

    // POWER LED
//    if (READ_BIT(port,3)==0) {
//        painter.fillRect(698,550,18,9,QColor(Qt::green));
//    }

    painter.end();
    paintingImage.unlock();
    Refresh_Display = true;
    return true;

}

bool Cfa80::SaveSession_File(QXmlStreamWriter *xmlOut)
{
    pHD61710->save_internal(xmlOut);
    return true;
}

bool Cfa80::LoadSession_File(QXmlStreamReader *xmlIn)
{
    pHD61710->Load_Internal(xmlIn);
    return true;
}



bool Cfa80::init(void)
{
    CPObject::init();

    setfrequency( 0);

    pCONNECTOR	   = new Cconnector(this,30,0,Cconnector::Casio_30,"Connector 30 pins",true,QPoint(414,375));	publish(pCONNECTOR);
    pCENTCONNECTOR = new Cconnector(this,36,1,Cconnector::Centronics_36,"Centronic 36 pins",false,QPoint(417,13)); publish(pCENTCONNECTOR);
    pTAPECONNECTOR	= new Cconnector(this,3,1,Cconnector::Jack,"Line in / Rec / Rmt",false);	publish(pTAPECONNECTOR);

    pHD61710->linkConnectors(pCENTCONNECTOR,pTAPECONNECTOR,NULL);

    pHD61710->init();

    WatchPoint.add(&pCONNECTOR_value,64,30,this,"Standard 30pins connector");
    WatchPoint.add(&pCENTCONNECTOR_value,64,36,this,"Centronic 36pins connector");
    WatchPoint.add(&pTAPECONNECTOR_value,64,2,this,"Line In / Rec");

    AddLog(LOG_PRINTER,tr("FA-80 initializing..."));

    if(pTIMER)	pTIMER->init();



    data = 0;

    return true;
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

bool Cfa80::Get_Connector(Cbus *_bus) {
    Get_MainConnector();
    pHD61710->Get_CentConnector();
    pHD61710->Get_TAPEConnector();

    return true;
}
bool Cfa80::Set_Connector(Cbus *_bus) {
    pHD61710->Set_CentConnector();
    pHD61710->Set_TAPEConnector();
    Set_MainConnector();

    return true;
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

    // Manage sio
    pHD61710->step();


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
    pTAPECONNECTOR_value = pTAPECONNECTOR->Get_values();

    adrBus = bus->getAddr();
    data = bus->getData();

    if (bus->isWrite()) {
        pHD61710->regWrite(adrBus,data);
    }
    else {
        bus->setData(pHD61710->regRead(adrBus));
    }

    Set_Connector();
    pCONNECTOR_value = pCONNECTOR->Get_values();
    pCENTCONNECTOR_value = pCENTCONNECTOR->Get_values();
    pTAPECONNECTOR_value = pTAPECONNECTOR->Get_values();

    return true;
}



