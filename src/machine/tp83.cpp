//TODO: Key management

#include <QPainter>
#include <QDebug>

#include "tp83.h"
#include "Connect.h"
#include "Keyb.h"
#include "dialogconsole.h"
#include "watchpoint.h"
#include "Inter.h"
#include "Log.h"
#include "paperwidget.h"

#define DOWN	0
#define UP		1

Ctp83::Ctp83(CPObject *parent):Cce515p(this) {
    setfrequency( 0);
    ioFreq = 0;
    setcfgfname(QString("tp83"));
    BackGroundFname	= P_RES(":/ext/tp-83.png");

    delete pKEYB; pKEYB		= new Ckeyb(this,"tp83.map");

    setDXmm(329);
    setDYmm(117);
    setDZmm(51);

    setDX(1175);
    setDY(418);

    printerACK = false;
    printerBUSY = false;

    margin = 20;
    Paper_DX = 240+2*margin;
    setPaperPos(QRect(90,0,214,245));
}

Ctp83::~Ctp83() {
    delete pCONNECTOR;
    delete pSavedCONNECTOR;

}

bool Ctp83::init(void) {

    Cce515p::init();

    pCONNECTOR = new Cconnector(this,20,0,Cconnector::General_20,
                                "20 pins Connector",true,QPoint(466,207)); publish(pCONNECTOR);
    pSavedCONNECTOR = new Cconnector(this,20,1,Cconnector::General_20,"Saved Connector",false,QPoint(631,468));

    QHash<int,QString> lbl;
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
    WatchPoint.add(&pCONNECTOR_value,64,36,this,"// 36pins connector",lbl);

    paperWidget->hide();

    PaperFeed();PaperFeed();

    charSize = 2;

    return true;
}

bool Ctp83::run(void) {

    if (pKEYB->LastKey == K_PFEED) { PaperFeed();}

    // Si strobe change de status et passe bas vers haut, alors prelever les data
    // mettre le busy
    //
    quint8 c = pCONNECTOR->Get_values();

    if ( c>0) {
        SET_PIN(9,1);
        if (c != 0xff && c !=0x0a) {
            AddLog(LOG_PRINTER,QString("Recieve:%1 = (%2)").arg(c,2,16,QChar('0')).arg(QChar(c)));
            Command(c);
        }
    }

    pCONNECTOR_value = pCONNECTOR->Get_values();

    pSavedCONNECTOR->Set_values(pCONNECTOR->Get_values());
    pCONNECTOR_value = pCONNECTOR->Get_values();

    Draw();

    return true;
}

bool Ctp83::exit(void) {

    Cce515p::exit();

    return true;
}

bool Ctp83::GoDown(int pin) {

    return (( pCONNECTOR->Get_pin(pin) == DOWN ) && (pSavedCONNECTOR->Get_pin(pin) == UP)) ? true:false;
}
bool Ctp83::GoUp(int pin) {

    return (( pCONNECTOR->Get_pin(pin) == UP ) && (pSavedCONNECTOR->Get_pin(pin) == DOWN)) ? true:false;
}
bool Ctp83::Change(int pin) {
    return (pCONNECTOR->Get_pin(pin) != pSavedCONNECTOR->Get_pin(pin) ) ? true:false;
}

void Ctp83::CommandSlot(qint8 data) {
    Command(data);
}




