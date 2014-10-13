//TODO: Key management

#include <QPainter>
#include <QDebug>

#include "cl1000.h"
#include "Connect.h"
#include "Keyb.h"
#include "dialogconsole.h"
#include "init.h"
#include "Inter.h"
#include "Log.h"
#include "paperwidget.h"

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

Ccl1000::Ccl1000(CPObject *parent):Cce515p(this) {
    setfrequency( 0);
    ioFreq = 0;
    setcfgfname(QString("cl1000"));
    BackGroundFname	= P_RES(":/ext/cl-1000.png");

    delete pKEYB; pKEYB		= new Ckeyb(this,"cl1000.map");

    setDXmm(260);//Pc_DX_mm = 256;
    setDYmm(225);//Pc_DY_mm = 185;
    setDZmm(42);//Pc_DZ_mm = 42;

    setDX(928);//Pc_DX	= 895;
    setDY(804);//Pc_DY	= 615;

    printerACK = false;
    printerBUSY = false;

    margin = 20;
    Paper_DX = 240+2*margin;
    setPaperPos(QRect(90,0,214,245));
}

Ccl1000::~Ccl1000() {
    delete pCONNECTOR;
    delete pSavedCONNECTOR;

}

bool Ccl1000::init(void) {

    Cce515p::init();

    pCONNECTOR = new Cconnector(this,20,0,Cconnector::General_20,
                                "20 pins Connector",true,QPoint(135,498)); publish(pCONNECTOR);
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

    charSize = 1;

    return true;
}

bool Ccl1000::run(void) {

    if (pKEYB->LastKey == K_PFEED) { PaperFeed();}

    // Si strobe change de status et passe bas vers haut, alors prelever les data
    // mettre le busy
    //
    quint8 c = pCONNECTOR->Get_values();

    if ( c>0 && c != 0xff && c !=0x0a) {
        AddLog(LOG_PRINTER,QString("Recieve:%1 = (%2)").arg(c,2,16,QChar('0')).arg(QChar(c)));
        SET_PIN(9,1);
        Command(c);
    }

    pCONNECTOR_value = pCONNECTOR->Get_values();

    pSavedCONNECTOR->Set_values(pCONNECTOR->Get_values());
    pCONNECTOR_value = pCONNECTOR->Get_values();

    Draw();

    return true;
}

bool Ccl1000::exit(void) {

    Cce515p::exit();

    return true;
}

bool Ccl1000::GoDown(int pin) {

    return (( pCONNECTOR->Get_pin(pin) == DOWN ) && (pSavedCONNECTOR->Get_pin(pin) == UP)) ? true:false;
}
bool Ccl1000::GoUp(int pin) {

    return (( pCONNECTOR->Get_pin(pin) == UP ) && (pSavedCONNECTOR->Get_pin(pin) == DOWN)) ? true:false;
}
bool Ccl1000::Change(int pin) {
    return (pCONNECTOR->Get_pin(pin) != pSavedCONNECTOR->Get_pin(pin) ) ? true:false;
}

void Ccl1000::contextMenuEvent ( QContextMenuEvent * event )
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

void Ccl1000::ComputeKey(void)
{

//    if (pKEYB->LastKey == K_PRINT_ON) {
//        printerSwitch = true;
//    }
//    if (pKEYB->LastKey == K_PRINT_OFF) {
//        printerSwitch = false;
//    }
}

bool Ccl1000::UpdateFinalImage(void) {


    Cce515p::UpdateFinalImage();

    QPainter painter;
    painter.begin(FinalImage);

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

//    painter.setOpacity(0.5);
//    painter.fillRect(PaperPos(),Qt::black);
//    painter.setOpacity(1);

//    painter.drawImage(112,145,*capot);

//    int offset = (lastX ) * ratio /( mainwindow->zoom/100);
//    painter.drawImage(152+offset,178,*head);       // Draw head
//    painter.drawImage(793 - offset,214,*cable);    // Draw cable
//    qWarning()<<"update";

    painter.end();
    emit updatedPObject(this);
    return true;
}

void Ccl1000::paintEvent(QPaintEvent *event)
{
    Cce515p::paintEvent(event);
}

void Ccl1000::CommandSlot(qint8 data) {
    Command(data);
}



