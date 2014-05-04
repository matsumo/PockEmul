#include <QPainter>
#include <QDebug>

#include "common.h"
#include "Log.h"
#include "pcxxxx.h"
#include "Connect.h"

#include "rlp6001.h"
#include "clink.h"
#include "Connect.h"
#include "buspanasonic.h"
#include "Inter.h"
#include "cpu.h"

#define NB_SLOTS 6

Crlp6001::Crlp6001(CPObject *parent )   : CPObject(this)
{                                                       //[constructor]
    Q_UNUSED(parent)

    setfrequency( 0);
    BackGroundFname     = P_RES(":/rlh1000/rlp6001.png");

    setDXmm(85);
    setDYmm(318);
    setDZmm(51);

    setDX(252);
    setDY(1075);
}

Crlp6001::~Crlp6001(){
    delete(pMAINCONNECTOR);

    for (int i=0;i<NB_SLOTS;i++) delete(pEXTCONNECTOR[i]);
}

FILE *_log;

bool Crlp6001::run(void)
{
//    if (!_log) _log=fopen("bus.log","wt");	// Open log file

    CbusPanasonic bus;
    bus.fromUInt64(pMAINCONNECTOR->Get_values());
    if (bus.getFunc()==BUS_SLEEP) return true;

    quint8 dest = bus.getDest()-1;

    if (_log) fprintf(_log,bus.toLog().toLatin1().data());

    if (bus.getDest()==30) {
        // BUS_QUERY on Dest 30 : Ask all ext for F7 return (ROM Expander)
        if ( (bus.getFunc()==BUS_LINE0) && !bus.isWrite() ) {
//            qWarning()<<"BUS_QUERY on Dest 30";
            for (int i=0; i<5;i++) {
                CbusPanasonic _b(0,BUS_LINE0,0);
                pEXTCONNECTOR[i]->Set_values(_b.toUInt64());
                mainwindow->pdirectLink->outConnector(pEXTCONNECTOR[i]);
                _b.fromUInt64(pEXTCONNECTOR[i]->Get_values());
                if ( (_b.getFunc()==BUS_READDATA) &&
                     ((~(_b.getData())&0x08)==0x08)) {
                    bus.setFunc(BUS_READDATA);
                    bus.setData(0xf7);
                    pMAINCONNECTOR->Set_values(bus.toUInt64());
                    return true;
                }
            }
        }
        if ( (bus.getFunc()==BUS_LINE2) && bus.isWrite() ) {
//            qWarning()<<"BUS_SELECT on Dest 30";
            for (int i=0; i<5;i++) {
                pEXTCONNECTOR[i]->Set_values(bus.toUInt64());
                mainwindow->pdirectLink->outConnector(pEXTCONNECTOR[i]);
                CbusPanasonic _b;
                _b.fromUInt64(pEXTCONNECTOR[i]->Get_values());
                if (_b.getFunc()==BUS_READDATA) {
//                    qWarning()<<"EXT ROM POWER ON";
                    bus.setFunc(BUS_READDATA);
                    pMAINCONNECTOR->Set_values(bus.toUInt64());
                    return true;
                }
            }
        }
        if (bus.getFunc()==BUS_READDATA) {
//            qWarning()<<"BUS_READDATA on Dest 30";
            for (int i=0; i<5;i++) {
//                bus.setDest(0);
                bus.setData(0xff);
                pEXTCONNECTOR[i]->Set_values(bus.toUInt64());
                mainwindow->pdirectLink->outConnector(pEXTCONNECTOR[i]);
                CbusPanasonic _b;
                _b.fromUInt64(pEXTCONNECTOR[i]->Get_values());
                if(pTIMER->pPC->pCPU->fp_log) fprintf(pTIMER->pPC->pCPU->fp_log,"EXT ROM DATA READ:(%i) = %02x",i,_b.getData());

                if (_b.getFunc()==BUS_ACK) {
//                    qWarning()<<"EXT ROM DATA READ";
                    if(pTIMER->pPC->pCPU->fp_log) fprintf(pTIMER->pPC->pCPU->fp_log,"EXT ROM DATA READ:%02x",_b.getData());
                    bus.setFunc(BUS_READDATA);
                    bus.setData(_b.getData());
                    pMAINCONNECTOR->Set_values(bus.toUInt64());
                    return true;
                }
            }
        }
    }

    if (dest >= NB_SLOTS) {
        if (_log) fprintf(_log,"\n");
        return true;
    }

    bus.setDest(0);
    // copy MainConnector to Ext Connectors
    pEXTCONNECTOR[dest]->Set_values(bus.toUInt64());

    // execute Ext
    mainwindow->pdirectLink->outConnector(pEXTCONNECTOR[dest]);

    bus.fromUInt64(pEXTCONNECTOR[dest]->Get_values());
    if ( (bus.getFunc()==BUS_READDATA) || (bus.getFunc()==BUS_ACK) )
        pMAINCONNECTOR->Set_values(bus.toUInt64());

    if (_log) fprintf(_log,"    --->  ");
    if (_log) fprintf(_log,bus.toLog().toLatin1().data());
    if (_log) fprintf(_log,"\n");
    return true;
}




/*****************************************************************************/
/* Initialize                                                                                                                    */
/*****************************************************************************/


bool Crlp6001::init(void)
{

    int snap[17][2] = {
        {0,367+367+72},
        {0,367+72},
        {0,72},
        {252,72},
        {252,367+72},
        {0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},
        {252,367+367+72}
    };
    Cconnector::ConnectorDir dir[] = {
        Cconnector::WEST,
        Cconnector::WEST,
        Cconnector::WEST,
        Cconnector::EAST,
        Cconnector::EAST,
        Cconnector::EAST,
        Cconnector::EAST,
        Cconnector::EAST,
        Cconnector::EAST,
        Cconnector::EAST,
        Cconnector::EAST,
        Cconnector::EAST,
        Cconnector::EAST,
        Cconnector::EAST,
        Cconnector::EAST,
        Cconnector::EAST
    };

    AddLog(LOG_MASTER,"RL-P6001 initializing...");
    qWarning()<<"RL-P6001 initializing...";
    CPObject::init();
qWarning()<<"RL-P6001 initializing...step1";
    pMAINCONNECTOR = new Cconnector(this,44,0,
                                    Cconnector::Panasonic_44,
                                    "44 pins main connector",
                                    true,
                                    QPoint(snap[16][0],snap[16][1]),
                                    Cconnector::EAST);
    publish(pMAINCONNECTOR);
qWarning()<<"RL-P6001 initializing...step2";

    for (int i=0;i<NB_SLOTS;i++) {
        pEXTCONNECTOR[i] = new Cconnector(this,44,i+1,
                                          Cconnector::Panasonic_44,
                                          QString("Slot %1 - 44 pins Ext. connector").arg(i+1),
                                          false,
                                          QPoint(snap[i][0],snap[i][1]),
                                          dir[i]);
        publish(pEXTCONNECTOR[i]);
        qWarning()<<"RL-P6001 initializing...step2 - "<<i;
    }

    AddLog(LOG_MASTER,"done.\n");
    qWarning()<<"RL-P6001 initializing...DONE";
    return true;
}

/*****************************************************************************/
/* Exit                                                                                                                                          */
/*****************************************************************************/
bool Crlp6001::exit(void)
{
    return true;
}


void Crlp6001::paintEvent(QPaintEvent *event)
{
    CPObject::paintEvent(event);
}

void Crlp6001::contextMenuEvent ( QContextMenuEvent * event )
{
    QMenu *menu= new QMenu(this);

    BuildContextMenu(menu);

    menu->popup(event->globalPos () );
    event->accept();
}
