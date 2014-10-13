//TODO: Key management

#include <QPainter>

#include "fp100.h"
#include "Connect.h"
#include "Keyb.h"
#include "dialogconsole.h"
#include "init.h"
#include "Inter.h"
#include "Log.h"
#include "paperwidget.h"

#define DOWN	0
#define UP		1


Cfp100::Cfp100(CPObject *parent):Cce515p(this) {
    Q_UNUSED(parent)

    setfrequency( 4000);
    ioFreq = 0;
    setcfgfname(QString("fp100"));
    BackGroundFname	= P_RES(":/ext/fp100.png");



    delete pKEYB; pKEYB		= new Ckeyb(this,"x710.map");

    setDXmm(302);
    setDYmm(120);
    setDZmm(36);

    setDX(1078);
    setDY(817);



    printerACK = false;
    printerBUSY = false;


    capot = CreateImage(QSize(849,274),P_RES(":/ext/fp100-capot.png"));
    head = CreateImage(QSize(79,161),P_RES(":/ext/fp100head.png"));
    cable = CreateImage(QSize(75,10),P_RES(":/ext/fp100cable.png"));

    margin = 40;
    Paper_DX = 960+2*margin;
    setPaperPos(QRect(154,26,731,300));
}

Cfp100::~Cfp100() {
    delete pCONNECTOR;
    delete pSavedCONNECTOR;
    delete capot;
    delete head;
    delete cable;
}

bool Cfp100::init(void) {

    Cce515p::init();

    pCONNECTOR = new Cconnector(this,36,0,Cconnector::Centronics_36,"Parrallel Connector",false,QPoint(631,468)); publish(pCONNECTOR);
    pSavedCONNECTOR = new Cconnector(this,36,1,Cconnector::Canon_15,"Saved Parrallel Connector",true,QPoint(631,468));

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
    return true;
}

bool Cfp100::run(void) {

    if (pKEYB->LastKey == K_PFEED) { PaperFeed();}

    // Si strobe change de status et passe bas vers haut, alors prelever les data
    // mettre le busy
    //
    if (GoUp(1)) {
        UINT8 car = (pCONNECTOR->Get_values() >> 1) & 0xFF;
        AddLog(LOG_PRINTER,tr("RECIEVED CHAR : %1").arg(car,2,16,QChar('0')));
        if (car != 0xff && car !=0x0a) Command( car );
        printerACK = true;
        pTIMER->resetTimer(6);
    }


    if (printerACK && (pTIMER->nsElapsedId(6) > 500)) {
        AddLog(LOG_PRINTER,tr("PRINTER printerStatusPort ACK OFF, BUSY OFF"));
        printerACK = false;
    }

    printerBUSY = (moveBuffer.size()>100) ? true:false;
    pCONNECTOR->Set_pin(10,printerACK);
    pCONNECTOR->Set_pin(11,printerBUSY);
    pCONNECTOR->Set_pin(32,true);

    pSavedCONNECTOR->Set_values(pCONNECTOR->Get_values());
    pCONNECTOR_value = pCONNECTOR->Get_values();

    Draw();

    return true;
}

bool Cfp100::exit(void) {

    Cce515p::exit();

    return true;
}

bool Cfp100::GoDown(int pin) {

    return (( pCONNECTOR->Get_pin(pin) == DOWN ) && (pSavedCONNECTOR->Get_pin(pin) == UP)) ? true:false;
}
bool Cfp100::GoUp(int pin) {

    return (( pCONNECTOR->Get_pin(pin) == UP ) && (pSavedCONNECTOR->Get_pin(pin) == DOWN)) ? true:false;
}
bool Cfp100::Change(int pin) {
    return (pCONNECTOR->Get_pin(pin) != pSavedCONNECTOR->Get_pin(pin) ) ? true:false;
}

void Cfp100::contextMenuEvent ( QContextMenuEvent * event )
{
    QMenu *menu= new QMenu(this);

    BuildContextMenu(menu);

    QMenu * menuPaper = menu->addMenu(tr("Paper"));
    menuPaper->addAction(tr("Copy"),paperWidget,SLOT(paperCopy()));
    menuPaper->addAction(tr("Cut"),paperWidget,SLOT(paperCut()));
    menuPaper->addAction(tr("Save Image ..."),paperWidget,SLOT(paperSaveImage()));
    menuPaper->addAction(tr("Save Text ..."),paperWidget,SLOT(paperSaveText()));

    menu->popup(event->globalPos () );
    event->accept();
}

void Cfp100::ComputeKey(void)
{

//    if (pKEYB->LastKey == K_PRINT_ON) {
//        printerSwitch = true;
//    }
//    if (pKEYB->LastKey == K_PRINT_OFF) {
//        printerSwitch = false;
//    }
}

bool Cfp100::UpdateFinalImage(void) {

//    Cce515p::UpdateFinalImage();

    QPainter painter;
    painter.begin(FinalImage);


    painter.drawImage(QPoint(0,0),*BackgroundImage);

    float ratio = ( (float) paperWidget->width() ) / ( paperWidget->bufferImage->width() - paperWidget->getOffset().x() );

    QRect source = QRect( QPoint(paperWidget->getOffset().x() ,
                                 paperWidget->getOffset().y()  - paperWidget->height() / ratio ) ,
                          QPoint(paperWidget->bufferImage->width(),
                                 paperWidget->getOffset().y() +10)
                          );
//    MSG_ERROR(QString("%1 - %2").arg(source.width()).arg(PaperPos().width()));
    painter.drawImage(PaperPos(),
                      paperWidget->bufferImage->copy(source).scaled(PaperPos().size(),Qt::IgnoreAspectRatio, Qt::SmoothTransformation )
                      );
#if 1
    painter.setOpacity(0.5);
    painter.fillRect(PaperPos(),Qt::black);
    painter.setOpacity(1);

    painter.drawImage(112,145,*capot);

    int offset = (lastX ) * ratio /( mainwindow->zoom/100);
    painter.drawImage(152+offset,178,*head);       // Draw head
    painter.drawImage(793 - offset,214,*cable);    // Draw cable
#endif
    painter.end();

    emit updatedPObject(this);

    return true;
}

void Cfp100::paintEvent(QPaintEvent *event)
{
    Cce515p::paintEvent(event);
}

void Cfp100::CommandSlot(qint8 data) {
    Command(data);
}


