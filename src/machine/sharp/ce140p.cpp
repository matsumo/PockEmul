//TODO:  Paper positionning
//FIXME: Gérer correctement le commutateur SIO/PRN. Si sur SIO, l'imprimante intercepte les commande PRN
//FIXME: Lorsque les PRQ et PAK sont gérés, plus rien ne marche


#include <QPainter>

#include "ce140p.h"
#include "Connect.h"
#include "Keyb.h"
#include "dialogconsole.h"
#include "watchpoint.h"
#include "sio.h"


TransMap KeyMapce140p[]={
    {1,	"FEED  ",	K_PFEED,34,234,	9},
    {2, "PRINTER ON",K_PRINT_ON,34,200,9},
    {3, "SIO ON",K_PRINT_OFF,35,150,9}
};
int KeyMapce140pLenght = 3;


Cce140p::Cce140p(CPObject *parent):Cce515p(this) {
    //setfrequency( 0);
    setcfgfname(QString("ce140p"));
    BackGroundFname	= P_RES(":/ext/ce-140p.png");
    printerSwitch =true;

    //pTIMER		= new Ctimer(this);
    KeyMap      = KeyMapce140p;
    KeyMapLenght= KeyMapce140pLenght;
    delete pKEYB; pKEYB		= new Ckeyb(this,"ce140p.map");

    setDXmm(256);
    setDYmm(185);
    setDZmm(42);

    setDX(895);
    setDY(615);

    setPaperPos(QRect(200,46,380,170));

}

Cce140p::~Cce140p() {
    delete pSIO;
    delete pSIOCONNECTOR;
    delete pSIOCONNECTOR_OUT;
}

bool Cce140p::init(void) {

    Cce515p::init();

    pSIOCONNECTOR = new Cconnector(this,15,0,Cconnector::Sharp_15,"Connector 15 pins (Input)",true,QPoint(631,468)); publish(pSIOCONNECTOR);
    pSIOCONNECTOR_OUT = new Cconnector(this,15,1,Cconnector::Sharp_15,"Connector 15 pins (Output)",false,QPoint(885,468)); publish(pSIOCONNECTOR_OUT);

    WatchPoint.add(&pSIOCONNECTOR_value,64,15,this,"Serial 15pins connector");
    pSIO = new Csio(this);

    if (pSIO) pSIO->init();
    if (pSIO) pSIO->dialogconsole->hide();
    pSIO->Set_BaudRate(1200);


    connect(pSIO,SIGNAL(newByteRecv(qint8)),this,SLOT(CommandSlot(qint8)));

    return true;
}

#define SIO_GNDP	1
#define SIO_SD 		2
#define SIO_RD		3
#define SIO_RS		4
#define SIO_CS		5

#define SIO_GND		7
#define SIO_CD		8
#define SIO_VC1		10
#define SIO_RR		11
#define SIO_PAK		12
#define SIO_VC2		13
#define SIO_ER		14
#define SIO_PRQ		15

bool Cce140p::run(void) {

    if (pKEYB->LastKey == K_PFEED) { PaperFeed();}

#if 0
    if (!pSIOCONNECTOR->Get_pin(SIO_PAK)) {
        if (pSIOCONNECTOR->Get_pin(SIO_PRQ)) {
            pTIMER->resetTimer(0);
            pSIOCONNECTOR->Set_pin(SIO_PAK,true);
        }
    }
    else {
         if (pTIMER->msElapsedId(0)>40) pSIOCONNECTOR->Set_pin(SIO_PAK,false);
    }
#endif
#if 0
    if (pSIOCONNECTOR->Get_pin(SIO_PRQ)) {
        pSIOCONNECTOR->Set_pin(SIO_PAK,true);
    }
    else {
        pSIOCONNECTOR->Set_pin(SIO_PAK,false);
    }
#endif

    if (printerSwitch) {
        pSIOCONNECTOR_value = pSIOCONNECTOR->Get_values();
        pSIO->pSIOCONNECTOR->Set_values(pSIOCONNECTOR_value);
        // Be sure pSIO TIMER IS Connected to CE140p TIMER (can change if connect/unconnect action)
        pSIO->pTIMER = pTIMER;

        pSIO->run();
        if (pSIO->Refresh_Display) {
            pSIO->Refresh_Display = false;
            Refresh_Display=true;
        }

        Draw();


        pSIOCONNECTOR->Set_values(pSIO->pSIOCONNECTOR->Get_values());
    }
    else {
        // NOT FINISHED
        pSIOCONNECTOR_OUT->Set_values(pSIOCONNECTOR->Get_values());
    }


    return true;
}

bool Cce140p::exit(void) {

    Cce515p::exit();

    return true;
}

void Cce140p::BuildContextMenu(QMenu *menu)
{
    Cce515p::BuildContextMenu(menu);

    menu->addSeparator();

    menu->addAction(tr("Show serial console"),pSIO,SLOT(ShowConsole()));
    menu->addAction(tr("Hide serial console"),pSIO,SLOT(HideConsole()));
}

void Cce140p::ComputeKey(KEYEVENT ke,int scancode)
{
    Q_UNUSED(ke)
    Q_UNUSED(scancode)


    if (pKEYB->LastKey == K_PRINT_ON) {
        printerSwitch = true;
    }
    if (pKEYB->LastKey == K_PRINT_OFF) {
        printerSwitch = false;
    }
}

bool Cce140p::UpdateFinalImage(void) {
    Cce515p::UpdateFinalImage();

    // Draw switch by 180° rotation
    QPainter painter;

    // PRINTER SWITCH
    painter.begin(FinalImage);

    painter.drawImage(800*internalImageRatio,430*internalImageRatio,
                      BackgroundImageBackup->copy(800*internalImageRatio,430*internalImageRatio,
                                                  22*internalImageRatio,14*internalImageRatio).mirrored(!printerSwitch,false));

    painter.end();

    Refresh_Display = false;
    emit updatedPObject(this);
    return true;
}

void Cce140p::paintEvent(QPaintEvent *event)
{
    Cce515p::paintEvent(event);
    pSIO->dialogconsole->refresh();

}

void Cce140p::CommandSlot(qint8 data) {
    Command(data);
}
