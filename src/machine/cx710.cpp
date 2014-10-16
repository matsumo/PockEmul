
#include <QPainter>

#include "cx710.h"
#include "Connect.h"
#include "Keyb.h"
#include "dialogconsole.h"
#include "init.h"
#include "Log.h"

#define DOWN	0
#define UP		1

/*
TransMap KeyMapce140p[]={
    {1,	"FEED  ",	K_PFEED,34,234,	9},
    {2, "PRINTER ON",K_PRINT_ON,34,200,9},
    {3, "SIO ON",K_PRINT_OFF,35,150,9}
};
 int KeyMapce140pLenght = 3;
*/

Cx710::Cx710(CPObject *parent):Cce515p(this) {
    //setfrequency( 0);
    setcfgfname(QString("x710"));
    BackGroundFname	= P_RES(":/x07/x710.png");

    setDXmm(200);
    setDYmm(120);
    setDZmm(36);

    setDX(715);
    setDY(665);

    setPaperPos(QRect(155,46,400,300));

    delete(pKEYB);
    pKEYB		= new Ckeyb(this,"x710.map",0);

}

Cx710::~Cx710() {
    delete pCONNECTOR;
    delete pSavedCONNECTOR;
}

bool Cx710::init(void) {

    Cce515p::init();

    pCONNECTOR = new Cconnector(this,15,0,Cconnector::Canon_15,"Parrallel Connector",true,QPoint(631,468)); publish(pCONNECTOR);
    pSavedCONNECTOR = new Cconnector(this,15,0,Cconnector::Canon_15,"Saved Parrallel Connector",true,QPoint(631,468));

    WatchPoint.add(&pCONNECTOR_value,64,15,this,"// 15pins connector");

//    connect(pSIO,SIGNAL(newData(qint8)),this,SLOT(CommandSlot(qint8)));
    margin = 20;
    Paper_DX = 480+2*margin;

    return true;
}

bool Cx710::run(void) {

    if (pKEYB->LastKey == K_PFEED) { PaperFeed();}

    // Si strobe change de status et passe bas vers haut, alors prelever les data
    // mettre le busy
    //
    if (GoUp(1)) {
        UINT8 car = (pCONNECTOR->Get_values() >> 1) & 0xFF;
        AddLog(LOG_PRINTER,tr("RECIEVED CHAR : %1").arg(car,2,16,QChar('0')));
        if (car != 0xff && car !=0x0a) Command( car );
    }

    pSavedCONNECTOR->Set_values(pCONNECTOR->Get_values());
    pCONNECTOR_value = pCONNECTOR->Get_values();

    Draw();

    return true;
}

bool Cx710::exit(void) {

    Cce515p::exit();

    return true;
}

bool Cx710::GoDown(int pin) {

    return (( pCONNECTOR->Get_pin(pin) == DOWN ) && (pSavedCONNECTOR->Get_pin(pin) == UP)) ? true:false;
}
bool Cx710::GoUp(int pin) {

    return (( pCONNECTOR->Get_pin(pin) == UP ) && (pSavedCONNECTOR->Get_pin(pin) == DOWN)) ? true:false;
}
bool Cx710::Change(int pin) {
    return (pCONNECTOR->Get_pin(pin) != pSavedCONNECTOR->Get_pin(pin) ) ? true:false;
}

void Cx710::contextMenuEvent ( QContextMenuEvent * event )
{
    QMenu *menu= new QMenu(this);

    BuildContextMenu(menu);

    menu->popup(event->globalPos () );
    event->accept();
}

void Cx710::ComputeKey(void)
{

//    if (pKEYB->LastKey == K_PRINT_ON) {
//        printerSwitch = true;
//    }
//    if (pKEYB->LastKey == K_PRINT_OFF) {
//        printerSwitch = false;
//    }
}

//bool Cx710::UpdateFinalImage(void) {
//    Cce515p::UpdateFinalImage();

////    // Draw switch by 180° rotation
////    QPainter painter;

////    // PRINTER SWITCH
////    painter.begin(FinalImage);
////    painter.drawImage(800,430,FinalImage->copy(800,430,22,14).mirrored(!printerSwitch,false));

////    painter.end();

////    Refresh_Display = true;

//    return true;
//}

//void Cx710::paintEvent(QPaintEvent *event)
//{
//    Cce515p::paintEvent(event);
//}

void Cx710::CommandSlot(qint8 data) {
    Command(data);
}

