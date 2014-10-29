#include <QtGui>
#include <QTime>
#include <QFileDialog>
#include <QThread>
#include <QDebug>

#include "common.h"
#include "fluidlauncher.h"

#include "rlext.h"
#include "pcxxxx.h"
#include "Log.h"
#include "Keyb.h"
#include "Connect.h"
#include "buspanasonic.h"

#define DOWN    0
#define UP              1

CrlExt::CrlExt(CPObject *parent):CPObject(this)
{ //[constructor]
    Q_UNUSED(parent)

    setfrequency( 0);

    BackGroundFname     = P_RES(":/rlh1000/rlp1004a.png");
    setcfgfname("rlp1004a");

    pTIMER              = new Ctimer(this);

    setDXmm(113);
    setDYmm(95);
    setDZmm(51);

    setDX(440);
    setDY(340);

    rotate = false;
    INTrequest = false;

    memsize             = 0;
    InitMemValue        = 0x7f;
    SlotList.clear();

}

CrlExt::~CrlExt() {
    delete pCONNECTOR;
}


bool CrlExt::run(void)
{

    CbusPanasonic bus;

    bus.fromUInt64(pCONNECTOR->Get_values());

    if (bus.getFunc()==BUS_SLEEP) return true;

    if (bus.getDest()!=0) return true;
    bus.setDest(0);


    if ( (bus.getFunc()==BUS_LINE0) && !bus.isWrite() ) {
        if (readLine0()) {
            pCONNECTOR->Set_values(bus.toUInt64());
            return true;
        }
    }
    if ( (bus.getFunc()==BUS_LINE0) && bus.isWrite() ) {
        if (writeLine0()) {
            pCONNECTOR->Set_values(bus.toUInt64());
            return true;
        }
    }
    if ( (bus.getFunc()==BUS_LINE1) && !bus.isWrite() ) {
        if (readLine1()) {
            pCONNECTOR->Set_values(bus.toUInt64());
            return true;
        }
    }
    if ( (bus.getFunc()==BUS_LINE1) && bus.isWrite() ) {
        if (writeLine1()) {
            pCONNECTOR->Set_values(bus.toUInt64());
            return true;
        }
    }
    if ( (bus.getFunc()==BUS_LINE2) && !bus.isWrite() ) {
        if (readLine2()) {
            pCONNECTOR->Set_values(bus.toUInt64());
            return true;
        }
    }
    if ( (bus.getFunc()==BUS_LINE2) && bus.isWrite() ) {
        if (writeLine2()) {
            pCONNECTOR->Set_values(bus.toUInt64());
            return true;
        }
    }

    if ( (bus.getFunc()==BUS_LINE3) && bus.isWrite() ) {
        if (writeLine3()) {
            pCONNECTOR->Set_values(bus.toUInt64());
            return true;
        }
    }

    if ( (bus.getFunc()==BUS_LINE3) && !bus.isWrite() ) {
        if (readLine3()) {
            pCONNECTOR->Set_values(bus.toUInt64());
            return true;
        }
    }


    if (!Power) return true;

    quint32 adr = bus.getAddr();
    quint8 data = bus.getData();

    switch (bus.getFunc()) {
    case BUS_SLEEP: break;
    case BUS_WRITEDATA:
        writeData(adr,data);
        break;
    case BUS_READDATA:
        readData(adr);
        break;
    default: break;
    }

    pCONNECTOR->Set_values(bus.toUInt64());
    return true;

}

void CrlExt::readData(quint32 adr) {
    Q_UNUSED(adr)
}

void CrlExt::writeData(quint32 adr, quint8 val)
{
    Q_UNUSED(adr)
    Q_UNUSED(val)
}

bool CrlExt::writeLine0() {

    return false;
}

bool CrlExt::readLine0() {
    return false;
}

bool CrlExt::writeLine1() {

    return false;
}
bool CrlExt::readLine1() {

    return false;
}
bool CrlExt::writeLine2() {

    return false;
}
bool CrlExt::readLine2() {

    return false;
}
bool CrlExt::writeLine3() {

    return false;
}
bool CrlExt::readLine3() {

    return false;
}


bool CrlExt::init(void)
{
    CPObject::init();

    setfrequency( 0);

    pCONNECTOR  = new Cconnector(this,
                                 44,
                                 0,
                                 Cconnector::Panasonic_44,
                                 "Main connector",
                                 true,
                                 QPoint(406,72),
                                 Cconnector::EAST);     publish(pCONNECTOR);
    WatchPoint.add(&pCONNECTOR_value,64,44,this,"Main connector");

    if(pKEYB)   pKEYB->init();
    if(pTIMER)  pTIMER->init();

    return true;
}



/*****************************************************/
/* Exit PRINTER                                                                          */
/*****************************************************/
bool CrlExt::exit(void)
{
    AddLog(LOG_PRINTER,"PRT Closing...");
    AddLog(LOG_PRINTER,"done.");
    CPObject::exit();
    return true;
}


/*****************************************************/
/* CE-126P PRINTER emulation                                             */
/*****************************************************/



#define         WAIT ( pPC->frequency / 10000*6)

#define RECEIVE_MODE    1
#define SEND_MODE               2
#define TEST_MODE               3



bool CrlExt::Get_Connector(Cbus *_bus) {
    return true;
}

bool CrlExt::Set_Connector(Cbus *_bus) {
    return true;
}





void CrlExt::paintEvent(QPaintEvent *event)
{
    CPObject::paintEvent(event);
}

void CrlExt::contextMenuEvent ( QContextMenuEvent * event )
{
    QMenu *menu= new QMenu(this);

    BuildContextMenu(menu);


    menu->addAction(tr("Dump Memory"),this,SLOT(Dump()));
    menu->addSeparator();

    menu->addAction(tr("Rotate 180"),this,SLOT(Rotate()));
//    menu.addAction(tr("Hide console"),this,SLOT(HideConsole()));

    menu->popup(event->globalPos () );
    event->accept();
}

void CrlExt::Rotate()
{
    rotate = ! rotate;

    delete BackgroundImageBackup;
    BackgroundImageBackup = CreateImage(QSize(getDX(), getDY()),BackGroundFname,false,false,rotate?180:0);
    delete BackgroundImage;
    BackgroundImage = new QImage(*BackgroundImageBackup);
    delete FinalImage;
    FinalImage = new QImage(*BackgroundImageBackup);

    pCONNECTOR->setSnap(rotate?QPoint(34,72):QPoint(406,72));

    pCONNECTOR->setDir(rotate?Cconnector::WEST:Cconnector::EAST);
    mask = QPixmap::fromImage(*BackgroundImageBackup).scaled(getDX()*mainwindow->zoom/100,getDY()*mainwindow->zoom/100);
    setMask(mask.mask());

    update();

        // adapt SNAP connector
}


extern int ask(QWidget *parent,QString msg,int nbButton);
#define KEY(c)	((pKEYB->keyPressedList.contains(TOUPPER(c)) || \
                  pKEYB->keyPressedList.contains(c) || \
                  pKEYB->keyPressedList.contains(TOLOWER(c)))?1:0)
void CrlExt::ComputeKey(KEYEVENT ke,int scancode)
{
    Q_UNUSED(ke)
    Q_UNUSED(scancode)



}

