#include <qglobal.h>
#if QT_VERSION >= 0x050000
#   include <QtWidgets>
#   include <QSensor>
#   include <QSensorReading>
#   include <QOrientationSensor>
#else
#   include <QtCore>
#   include <QtGui>
#   include <QStringRef>
#endif
#include <QString>
#include <QPainter>
#include <iostream>
#include <QtNetwork>


/** \mainpage
PockEmul is a Sharp Pocket Computer Emulator.

\version Version 0.9.3
\image html hexedit.png
*/




#include "qcommandline.h"

#include "mainwindowpockemul.h"

#ifdef P_AVOID
#include "libavoid.h"
#endif

#include "launchbuttonwidget.h"
#include "dialoganalog.h"
#include "dialogabout.h"
#include "dialogstartup.h"
#include "dialoglog.h"

#ifdef P_IDE
#include "ui/windowide.h"
#endif

#include "hexviewer.h"
#include "autoupdater.h"

#include "init.h"
#include "common.h"
#include "Log.h"
#include "pcxxxx.h"
#include "Connect.h"
#include "Inter.h"
#include "Lcdc.h"
#include "clink.h"
#include "downloadmanager.h"
#include "servertcp.h"
#include "cloud/cloudwindow.h"

#include "allobjects.h"

extern MainWindowPockemul* mainwindow;
extern DownloadManager *downloadManager;
extern int ask(QWidget *parent,QString msg,int nbButton);
extern QString m_getArgs();

#define NBFRAMEPERSEC		20
#define FRAMERATE			(1000/NBFRAMEPERSEC)
#define TIMER_RES			1

QTime t,tf;
QElapsedTimer et;
QTimer *timer;
QList<CPObject *> listpPObject;


MainWindowPockemul::MainWindowPockemul(QWidget * parent, Qt::WindowFlags f) : QMainWindow(parent, f)
{
//    setAttribute(Qt::WA_DeleteOnClose, true);
    rawclk = 0;

    setupUi(this);

    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    setStatusBar(0);
    dialoglog = 0;
    dialoganalogic = 0;
    cloud = 0;
#ifdef P_IDE
    windowide = 0;
#endif
    scaleFactor = 1;
    zoom = 100;
    saveAll = ASK;
    startKeyDrag = false;
    startPosDrag = false;

//    connect (this,SIGNAL()
    connect(actionAbout_PockEmul,	SIGNAL(triggered()),            this, SLOT(about()));
    connect(actionNew,				SIGNAL(triggered()),            this, SLOT(newsession()));
    connect(actionOpen,				SIGNAL(triggered()),            this, SLOT(opensession()));
    connect(actionSave_As,          SIGNAL(triggered()),            this, SLOT(saveassession()));
    connect(actionLog_Messages,		SIGNAL(triggered()),            this, SLOT(Log()));
    connect(actionAnalogic_Port,	SIGNAL(triggered()),            this, SLOT(Analogic()));
    connect(actionCheck_for_Updates,SIGNAL(triggered()),            this, SLOT(CheckUpdates()));
    connect(actionMinimize_All,     SIGNAL(triggered()),            this, SLOT(Minimize_All()));
    connect(actionReset_Zoom,       SIGNAL(triggered()),            this, SLOT(resetZoom()));
    connect(actionClose_All,        SIGNAL(triggered()),            this, SLOT(Close_All()));
    connect(menuPockets,            SIGNAL(triggered(QAction *)),   this, SLOT(SelectPocket( QAction *)));
    connect(actionEditor,           SIGNAL(triggered()),            this, SLOT(IDE()));


    pdirectLink = new CDirectLink;
    // Create a timer for Drawing screen FRAMERATE times per seconds
    FrameTimer = new QTimer(mainwindow);
    connect(FrameTimer, SIGNAL(timeout()), this, SLOT(updateFrameTimer()));
    FrameTimer->start(FRAMERATE);
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(updateTimer()));
    timer->start(TIMER_RES);

    // Create the Pocket Thread
    PcThread = new CPocketThread(this);
    PcThread->connect(PcThread,SIGNAL(Resize(QSize,CPObject * )),this,SLOT(resizeSlot(QSize,CPObject * )));
    PcThread->connect(PcThread,SIGNAL(Destroy(CPObject * )),this,SLOT(DestroySlot(CPObject * )));
#ifndef EMSCRIPTEN
    PcThread->start();
#endif

    grabGesture(Qt::PanGesture);
    grabGesture(Qt::PinchGesture);

    initObjectTable();
#ifdef AVOID
    router = new Avoid::Router(Avoid::OrthogonalRouting);
    router->setRoutingPenalty((Avoid::PenaltyType)0, 50);
    router->setOrthogonalNudgeDistance(15);
#endif

#ifndef EMSCRIPTEN
server = new ServeurTcp(this);
#endif

#if QT_VERSION >= 0x050000
    sensor = new QSensor("QRotationSensor");//QGyroscope");
    sensor->start();
#endif

}

MainWindowPockemul::~MainWindowPockemul() {
    delete PcThread;
    delete dialoglog;
    delete dialoganalogic;
#ifdef P_IDE
    delete windowide;
#endif
    delete FrameTimer;
    delete pdirectLink;

}

void MainWindowPockemul::initObjectTable() {
    objtable["PC-1211"]=PC1211;
    objtable["PC-1245"]=PC1245;
    objtable["MC-2200"]=MC2200;
    objtable["PC-1250"]=PC1250;
    objtable["PC-1251"]=PC1251;
    objtable["PC-1251H"]=PC1251H;
    objtable["PC-1255"]=PC1255;
    objtable["Tandy PC-3"]=TandyPC3;
    objtable["Tandy PC-3 (4Ko)"]=TandyPC3EXT;

    objtable["PC-1260"]=PC1260;
    objtable["PC-1261"]=PC1261;
    objtable["PC-1262"]=PC1262;
    objtable["PC-1280"]=PC1280;

    objtable["PC-1350"]=PC1350;
    objtable["PC-1360"]=PC1360;
    objtable["PC-1401"]=PC1401;
    objtable["PC-1402"]=PC1402;
    objtable["PC-1403"]=PC1403;
    objtable["PC-1403H"]=PC1403H;
    objtable["PC-1421"]=PC1421;
    objtable["PC-1450"]=PC1450;
    objtable["PC-1475"]=PC1475;
    objtable["PC-1425"]=PC1425;

    objtable["PC-1500"]=PC1500;
    objtable["PC-1500A"]=PC1500A;
    objtable["Tandy PC-2"]=TandyPC2;

    objtable["PC-1600"]=PC1600;

    objtable["PC-2500"]=PC2500;

    objtable["CE-122"]=CE122;
    objtable["CE-125"]=CE125;
    objtable["MP-220"]=MP220;
    objtable["CE-120P"]=CE120P;
    objtable["CE-126P"]=CE126P;
    objtable["CE-123P"]=CE123P;
    objtable["CE-129P"]=CE129P;
    objtable["CE-140P"]=CE140P;
    objtable["CE-140F"]=CE140F;
    objtable["CE-150"]= CE150;
    objtable["CE-153"]= CE153;
    objtable["CE-162E"]= CE162E;
    objtable["CE-152"]= CE152;
    objtable["CE-127R"]= CE127R;
    objtable["26-3591"]=TANDY263591;

    objtable["Serial Console"]=SerialConsole;
    objtable["11Pins Cable"]=CABLE11Pins;
    objtable["Potar"]=POTAR;
    objtable["Simulator"]=Simulator;

    objtable["CE-1600P"]=CE1600P;
    objtable["Canon X-07"]=X07;
    objtable["Canon X-710"]=X710;
    objtable["PC-E500"]=E500;
    objtable["PC-E500S"]=E500S;
    objtable["PC-E550"]=E550;
    objtable["PC-G850V"]=G850V;
    objtable["Casio PB-1000"]=PB1000;
    objtable["Casio PB-2000C"]=PB2000;
    objtable["Casio MD-100"]=MD100;
    objtable["Casio FP-100"]=FP100;
    objtable["Casio FP-40"]=FP40;
    objtable["Casio FP-200"]=FP200;
    objtable["Casio FX-890P"]=FX890P;
    objtable["Casio Z-1"]=Z1;
    objtable["Casio Z-1GR"]=Z1GR;
    objtable["Nec PC-2001"]=PC2001;
    objtable["Nec PC-2021"]=PC2021;
    objtable["Nec PC-2081"]=PC2081;
    objtable["General LBC-1100"]=LBC1100;
    objtable["General CL-1000"]=CL1000;
    objtable["SANCO TPC-8300"]=TPC8300;
    objtable["SANCO TP-83"]=TP83;
    objtable["Panasonic HHC RL-H1000"]=RLH1000;
    objtable["Panasonic RL-P3001 RS-232C"]=RLP3001;
    objtable["Panasonic RL-P4002 Modem"]=RLP4002;
    objtable["Panasonic RL-P6001"]=RLP6001;
    objtable["Panasonic RL-P9001 4Ko"]=RLP9001;
    objtable["Panasonic RL-P9002 8Ko"]=RLP9002;
    objtable["Panasonic RL-P9003 16Ko"]=RLP9003;
    objtable["Panasonic RL-P9003R 16Ko ROM Simulator"]=RLP9003R;
    objtable["Panasonic RL-P9004 32Ko"]=RLP9004;
    objtable["Panasonic RL-P9005 128Ko"]=RLP9005;
    objtable["Panasonic RL-P9006 EPROM Expander"]=RLP9006;
    objtable["Panasonic RL-P1002"]=RLP1002;
    objtable["Panasonic RL-P2001"]=RLP2001;
    objtable["Panasonic RL-P1004A"]=RLP1004A;
    objtable["Post-it"]=POSTIT;
    objtable["TI-57"]=TI57;
    objtable["HP-41"]=HP41;
    objtable["HP82143A"]=HP82143A;
    objtable["CE-1560"]= CE1560;
}


CPObject * MainWindowPockemul::InitApp(int idPC )
{
CPObject *pPC=0;

    switch (idPC)
    {

        case EMPTY	: return 0;
        case PC1211	: pPC = new Cpc1211;	pPC->setName("PC-1211");break;
        case PC1245	: pPC = new Cpc1245;	pPC->setName("PC-1245");break;
        case MC2200	: pPC = new Cmc2200;	pPC->setName("MC-2200");break;
        case PC1250	: pPC = new Cpc1250;	pPC->setName("PC-1250");break;
        case PC1251	: pPC = new Cpc1251;	pPC->setName("PC-1251");break;
        case PC1251H: pPC = new Cpc1251H;	pPC->setName("PC-1251H");break;
        case PC1255	: pPC = new Cpc1255;	pPC->setName("PC-1255");break;
        case TandyPC3:pPC = new Ctrspc3;	pPC->setName("Tandy PC-3");break;
        case TandyPC3EXT:pPC = new Ctrspc3Ext;	pPC->setName("Tandy PC-3 (4Ko)");break;

        case PC1260	: pPC = new Cpc1260;	pPC->setName("PC-1260");break;
        case PC1261	: pPC = new Cpc1261;	pPC->setName("PC-1261");break;
        case PC1262	: pPC = new Cpc1262;	pPC->setName("PC-1262");break;
        case PC1280	: pPC = new Cpc1280;	pPC->setName("PC-1280");break;

        case PC1350	: pPC = new Cpc1350;	pPC->setName("PC-1350");break;
        case PC1360	: pPC = new Cpc1360;	pPC->setName("PC-1360");break;
        case PC1401	: pPC = new Cpc1401;	pPC->setName("PC-1401");break;
        case PC1402	: pPC = new Cpc1402;	pPC->setName("PC-1402");break;
        case PC1403	: pPC = new Cpc1403;	pPC->setName("PC-1403");break;
        case PC1403H: pPC = new Cpc1403H;	pPC->setName("PC-1403H");break;
        case PC1421	: pPC = new Cpc1421;	pPC->setName("PC-1421");break;
        case PC1450	: pPC = new Cpc1450;	pPC->setName("PC-1450");break;
        case PC1475	: pPC = new Cpc1475;	pPC->setName("PC-1475");break;
        case PC1425	: pPC = new Cpc1425;	pPC->setName("PC-1425");break;

        case PC1500	: pPC = new Cpc1500;	pPC->setName("PC-1500");break;
        case PC1500A: pPC = new Cpc1500A;	pPC->setName("PC-1500A");break;
        case TandyPC2:pPC = new Ctrspc2;	pPC->setName("Tandy PC-2");break;

        case PC1600 : pPC = new Cpc1600;	pPC->setName("PC-1600");break;

        case PC2500 : pPC = new Cpc2500;	pPC->setName("PC-2500");break;

        case CE122  : pPC = new Cce122;		pPC->setName("CE-122");break;
        case CE125  : pPC = new Cce125;		pPC->setName("CE-125");break;
        case MP220  : pPC = new Cmp220;     pPC->setName("MP-220");break;
        case CE120P : pPC = new Cce120p;	pPC->setName("CE-120P");break;
        case CE126P : pPC = new Cce126;		pPC->setName("CE-126P");break;
        case CE123P : pPC = new Cce123;		pPC->setName("CE-123P");break;
        case CE129P : pPC = new Cce129;		pPC->setName("CE-129P");break;
        case CE140P : pPC = new Cce140p;	pPC->setName("CE-140P");break;
        case CE140F : pPC = new Cce140f;	pPC->setName("CE-140F");break;
        case CE150  : pPC = new Cce150;		pPC->setName("CE-150");break;
        case CE153  : pPC = new Cce153;		pPC->setName("CE-153");break;
        case CE162E : pPC = new Cce162e;	pPC->setName("CE-162E");break;
        case CE152  : pPC = new Cce152;		pPC->setName("CE-152");break;
        case CE127R : pPC = new Cce127r;	pPC->setName("CE-127R");break;
        case TANDY263591: pPC = new C263591;pPC->setName("26-3591");break;
        case CE1560 : pPC = new Cce1560;    pPC->setName("CE-1560");break;

        case SerialConsole: pPC = new Csio;	pPC->setName("Serial Console");break;
        case CABLE11Pins: pPC = new Ccable;	pPC->setName("11Pins Cable");break;
        case POTAR      : pPC = new Cpotar;	pPC->setName("Potar");break;
        case Simulator  : pPC = new Ccesimu;pPC->setName("Simulator");break;

        case CE1600P: pPC = new Cce1600p;	pPC->setName("CE-1600P");break;

        case X07    : pPC = new Cx07;       pPC->setName("Canon X-07"); break;
        case X710   : pPC = new Cx710;      pPC->setName("Canon X-710"); break;
        case E500   : pPC = new Ce500;      pPC->setName("PC-E500"); break;
        case E500S  : pPC = new Ce500(0,E500S);      pPC->setName("PC-E500S"); break;
        case E550   : pPC = new Ce550;      pPC->setName("PC-E550"); break;
        case G850V  : pPC = new Cg850v;     pPC->setName("PC-G850V"); break;
        case PB1000 : pPC = new Cpb1000;    pPC->setName("Casio PB-1000"); break;
        case PB2000 : pPC = new Cpb2000;    pPC->setName("Casio PB-2000C"); break;
        case FP200  : pPC = new Cfp200;     pPC->setName("Casio FP-200"); break;
        case MD100  : pPC = new Cmd100;     pPC->setName("Casio MD-100"); break;
        case FP100  : pPC = new Cfp100;     pPC->setName("Casio FP-100"); break;
        case FP40   : pPC = new Cfp40;      pPC->setName("Casio FP-40"); break;
        case FX890P : pPC = new Cz1(0,FX890P);  pPC->setName("Casio FX-890P"); break;
        case Z1     : pPC = new Cz1;            pPC->setName("Casio Z-1"); break;
        case Z1GR   : pPC = new Cz1(0,Z1GR);    pPC->setName("Casio Z-1GR"); break;

        case PC2001   : pPC = new Cpc2001;      pPC->setName("Nec PC-2001"); break;
        case PC2021   : pPC = new Cpc2021;      pPC->setName("Nec PC-2021"); break;
        case PC2081   : pPC = new Cpc2081;      pPC->setName("Nec PC-2081"); break;
        case LBC1100  : pPC = new Clbc1100;     pPC->setName("General LBC-1100"); break;

        case CL1000  : pPC = new Ccl1000;       pPC->setName("General CL-1000"); break;
        case TPC8300 : pPC = new Ctpc8300;      pPC->setName("SANCO TPC-8300"); break;
        case TP83    : pPC = new Ctp83;         pPC->setName("SANCO TP-83"); break;

        case RLH1000 : pPC = new Crlh1000;      pPC->setName("Panasonic HHC RL-H1000"); break;
        case RLP3001 : pPC = new Crlp3001;      pPC->setName("Panasonic RL-P3001 RS-232C"); break;
        case RLP4002 : pPC = new Crlp4002;      pPC->setName("Panasonic RL-P4002 Modem"); break;
        case RLP6001 : pPC = new Crlp6001;      pPC->setName("Panasonic RL-P6001"); break;
        case RLP9001 : pPC = new Crlp9001;      pPC->setName("Panasonic RL-P9001 4Ko"); break;
        case RLP9002 : pPC = new Crlp9001(0,RLP9002);      pPC->setName("Panasonic RL-P9002 8Ko"); break;
        case RLP9003 : pPC = new Crlp9001(0,RLP9003);      pPC->setName("Panasonic RL-P9003 16Ko"); break;
        case RLP9003R: pPC = new Crlp9001(0,RLP9003R);     pPC->setName("Panasonic RL-P9003R 16Ko ROM Simulator"); break;
        case RLP9004 : pPC = new Crlp9001(0,RLP9004);      pPC->setName("Panasonic RL-P9004 32Ko"); break;
        case RLP9005 : pPC = new Crlp9001(0,RLP9005);      pPC->setName("Panasonic RL-P9005 128Ko"); break;
        case RLP9006 : pPC = new Crlp9001(0,RLP9006);      pPC->setName("Panasonic RL-P9006 EPROM Expander"); break;
        case RLP1004A: pPC = new Crlp1004a;      pPC->setName("Panasonic RL-P1004A"); break;
        case RLP1002: pPC = new Crlp1002;      pPC->setName("Panasonic RL-P1002"); break;
        case RLP2001: pPC = new Crlp2001;      pPC->setName("Panasonic RL-P2001"); break;

        case POSTIT : pPC = new Cpostit;      pPC->setName("Post-it"); break;

        case TI57 : pPC = new Cti57;      pPC->setName("TI-57"); break;
        case HP41 : pPC = new Chp41;      pPC->setName("HP-41"); break;
    case HP82143A : pPC = new Chp82143A;      pPC->setName("HP82143A"); break;


        default			: return 0;
    }
    qWarning()<<"init";
    AddLog(LOG_MASTER,"OK1");
    int dx = pPC->getDX()*mainwindow->zoom/100;
    int dy = pPC->getDY()*mainwindow->zoom/100;
#ifdef AVOID
    Avoid::Rectangle rectangle(Avoid::Point(-10.0, -10.0), Avoid::Point(dx+20, dy+20));
    mainwindow->shapeRefList[pPC] = new Avoid::ShapeRef(mainwindow->router, rectangle);
    mainwindow->router->addShape(mainwindow->shapeRefList[pPC]);
#endif
    if (!pPC->init()) return 0;
    qWarning()<<"init ok";

//	int l = mainwindow->menuBar()->height();
//	mainwindow->resize(QSize(pPC->Pc_DX, pPC->Pc_DY+l));

    pPC->FinalImage = new QImage(QSize(pPC->getDX(), pPC->getDY()),QImage::Format_RGB32);


#define NBFRAMEPERSEC		20
#define FRAMERATE			(1000/NBFRAMEPERSEC)
//#define TIMER_RES 30

    pPC->InitDisplay();





    pPC->Move(QPoint(0,0));
    pPC->setGeometry(0,0,dx,dy);
    pPC->show();



    return pPC;
}


//
void MainWindowPockemul::SendSignal_AddLogItem(QString str) {
    emit AddLogItem(str);
}

void MainWindowPockemul::SendSignal_AddLogConsole(QString str) {
    emit AddLogConsole(str);
}

void MainWindowPockemul::slotUnLink(Cconnector * conn) {
    int i;
    for ( i = 0 ; i< mainwindow->pdirectLink->AConnList.size() ; i++)
    {
        if (mainwindow->pdirectLink->AConnList.at(i) == conn)
        {
            mainwindow->pdirectLink->removeLink(i);
            i--;
        }
    }
    for ( i = 0 ; i< mainwindow->pdirectLink->BConnList.size() ; i++)
    {
        if (mainwindow->pdirectLink->BConnList.at(i) == conn)
        {
            mainwindow->pdirectLink->removeLink(i);
            i--;
        }
    }

}

void MainWindowPockemul::slotUnlink(CPObject * obj) {
    int i;
    for (i = 0;i < obj->ConnList.size(); i++)
    {
        slotUnLink(obj->ConnList.at(i));
    }
}

void MainWindowPockemul::slotUnLink(QAction * action)
{
    if (action->data().toString().startsWith("A")) {
        CPObject * obj = (CPObject*) action->data().toString().mid(1).toULongLong();
        slotUnlink(obj);
    }
    else if (action->data().toString().startsWith("C"))
        {
            Cconnector * conn = (Cconnector*) action->data().toString().mid(1).toULongLong();
            slotUnLink(conn);
        }
}

void MainWindowPockemul::slotWebLink(QAction *action) {
    QString s = action->data().toString();
    QUrl url(s);
    QDesktopServices::openUrl(url);
}
#define POCKEMUL_DOCUMENTS_URL "http://pockemul.free.fr/Documents/userguide/"


extern void m_openURL(QUrl url);
void MainWindowPockemul::slotDocument(QAction *action) {

#ifdef EMSCRIPEN
    QString s = action->data().toString();
    QUrl url(s);
    QDesktopServices::openUrl(url);
#else
    // Check if document is already downloaded
    // if yes open it
    // if no propose to download it : display the size ?

    QString homeDir = QDir::homePath();
#ifdef Q_OS_ANDROID
    homeDir = "/sdcard";
#endif

    QString fn=homeDir+"/pockemul/documents/"+QFileInfo(action->data().toString()).fileName();
    QFile file(fn);
    if (!file.open(QIODevice::ReadOnly)) {

        if (ask(mainwindow,
                   tr("The document %1 is not available locally. Do you want do download it ?").
                   arg(action->data().toString()),2)!=1) return;

        // Download it
        QUrl url(action->data().toString());
        downloadManager->doDownload(url);
        return;
    }
    QUrl url = QUrl::fromLocalFile(fn);
#if 0
    QDesktopServices::openUrl(url);
#else
    m_openURL(url);
#endif
#endif
}

void MainWindowPockemul::slotNewLink(QAction * action)
{
    QString s = action->data().toString();
    QStringList list = s.split(":");

    qint64 l1 = list.at(0).toULongLong();
    qint64 l2 = list.at(1).toULongLong();

    Cconnector * p1 = (Cconnector*)l1;
    Cconnector * p2 = (Cconnector*)l2;

    mainwindow->pdirectLink->addLink(p1,p2,false);

    AddLog(LOG_MASTER,tr("DirectLink Iinsert (%1,%2)").arg((qint64)p1).arg((qint64)p2));
}

int MainWindowPockemul::newsession()
{
    DialogStartup dialogstartup(this);
    int result = dialogstartup.exec();
    LoadPocket(result);
    return 1;
}

CPObject * MainWindowPockemul::LoadPocket(QString Id) {

    int pocketId = objtable.value(Id);
    qWarning()<<"Load:"<<pocketId;
    if (pocketId == 0) {
        ask(this,tr("the '%1' model is not available (try to upgrade PockEmul)").arg(Id),1);
        return 0;
    }
    return LoadPocket(objtable.value(Id));
}

CPObject * MainWindowPockemul::LoadPocket(int result) {
    qWarning()<<"Load Pocket:"<<result;
    CPObject *newpPC;
    if (result)	{
                newpPC = InitApp(result);
                if (! newpPC) {
                    ask(this,"pPC is NULL in slotStart",1);
                }
                else
                {
                    AddLog(LOG_MASTER,tr("%1").arg((long)newpPC));
                                listpPObject.append(newpPC);

                    QAction * actionDistConn = menuPockets->addAction(newpPC->getName());
                    actionDistConn->setData(tr("%1").arg((long)newpPC));
                    QMenu *ctxMenu = new QMenu(newpPC);
                    newpPC->BuildContextMenu(ctxMenu);
                    actionDistConn->setMenu(ctxMenu);

                    emit NewPObjectsSignal(newpPC);

                    if (dialoganalogic) {
                        dialoganalogic->fill_twWatchPoint();
                        dialoganalogic->update();
                    }
#ifdef P_IDE
                    if (windowide) {
                        windowide->addtargetCB(newpPC);
                        windowide->update();
                    }
#endif
                    return newpPC;
                }

        }

        return 0;

}

//TODO Minimize should be transform to fit all pocket into screen
void MainWindowPockemul::Minimize_All() {
    // Fetch listpPObject and minimize if not minimized
    for (int k = 0; k < listpPObject.size(); k++)
    {
        CPObject *pc = listpPObject.at(k);
        if (pc->isFront()) {
            QMouseEvent *m = new QMouseEvent(QEvent::MouseButtonDblClick, QPoint(0,0), Qt::LeftButton, Qt::LeftButton,Qt::NoModifier);
            pc->mouseDoubleClickEvent(m);
        }
    }

}

bool MainWindowPockemul::Close_All() {

    if (!listpPObject.isEmpty()) {
#ifdef EMSCRIPTEN
        saveAll = NO;
#else
        switch (ask(mainwindow,"Do you want to save all sessions ?",3)) {
        case 1: saveAll = YES;break;
        case 2: saveAll = NO;break;
        case 3: return false;
        default: return true;
        }
#endif

        for (int k = 0; k < listpPObject.size(); k++)
        {
            listpPObject.at(k)->slotExit();
        }
    }

    return true;
}

void MainWindowPockemul::resetZoom() {
    zoom = 100;
    update();
}

void MainWindowPockemul::SelectPocket(QAction * action) {
    if (action->data().isNull()) return;

    CPObject *pc = (CPObject*) action->data().toString().toULongLong();
    pc->raise();
    pc->setFocus();
    if (!pc->isFront()) {
        QMouseEvent *e=new QMouseEvent(QEvent::MouseButtonDblClick, QPoint(0,0), Qt::LeftButton, Qt::LeftButton,Qt::NoModifier);
        QApplication::sendEvent(pc, e);
        delete e;
    }
}

void MainWindowPockemul::about()
{
    DialogAbout *dialogabout = new DialogAbout(this);
    dialogabout->setModal(true);
    dialogabout->show();
}

void MainWindowPockemul::Log()
{
        dialoglog = new DialogLog(this);
        dialoglog->show();
}

void MainWindowPockemul::IDE()
{

#ifdef P_IDE
        if (windowide==0) windowide = new WindowIDE(this);
        windowide->show();
#endif
}

void MainWindowPockemul::CloudSlot()
{

    if (cloud==0) {
        cloud = new CloudWindow(this);
    }

    cloud->setGeometry(0,0,width(),height());

    cloud->show();
    cloud->raise();

}

void MainWindowPockemul::Analogic()
{
        if (dialoganalogic==0) dialoganalogic = new dialogAnalog(11,this);
        dialoganalogic->setWindowTitle("Logic Analyser");
        dialoganalogic->show();
}

void MainWindowPockemul::CheckUpdates()
{
    CAutoUpdater *dialogcheckupdate = new CAutoUpdater(this);
    dialogcheckupdate->show();
}

// FIXME: TURNON POCKET after full load
void MainWindowPockemul::opensession(QXmlStreamReader *xml) {
    QMap<int,CPObject*> map;
    CPObject * firstPC = 0;
    QList<CPObject *> toPowerOn;

    if (xml->readNextStartElement()) {
        if (xml->name() == "pml" && xml->attributes().value("version") == "1.0") {
            zoom = xml->attributes().value("zoom").toString().toFloat();
            if (zoom==0)zoom=100;
            while (!xml->atEnd()) {
                while (xml->readNextStartElement()) {
                    QString eltname = xml->name().toString();
                    CPObject * locPC;
                    if (eltname == "object") {
                        QString name = xml->attributes().value("name").toString();
                        locPC = LoadPocket(name);
                        qWarning()<<"LOCPC :"<<name<<":"<<locPC;
                        if (locPC == 0) continue;
                        if (firstPC == 0) firstPC = locPC;      // Store the first pocket to manage stack
                        int id = xml->attributes().value("id").toString().toInt();
                        map.insert(id,locPC);
                        locPC->Front = (xml->attributes().value("front")=="true") ?true:false;

                        locPC->Power = (xml->attributes().value("power")=="true") ?true:false;
                        if (locPC->Power) {
                            toPowerOn.append(locPC);
//                            locPC->TurnON();
                        }

                        while (xml->readNextStartElement()) {
                            QString eltname = xml->name().toString();
//                            AddLog(LOG_TEMP,eltname);
                            if (eltname == "position") {
                                int posX = xml->attributes().value("x").toLatin1().toInt();
                                int posY = xml->attributes().value("y").toLatin1().toInt();
                                int width = xml->attributes().value("width").toLatin1().toInt();
                                int height = xml->attributes().value("height").toLatin1().toInt();
                                locPC->setPosX(posX);
                                locPC->setPosY(posY);
                                if ((width>0) && (height>0)) {
                                    locPC->setDX(width);
                                    locPC->setDY(height);
                                }
                                if (locPC->Front) {
//                                    locPC->setGeometry(posX.toFloat(),posY.toFloat(),locPC->getDX()*zoom/100,locPC->getDY()*zoom/100);
//                                    locPC->setMask(locPC->mask.scaled(locPC->getDX()*zoom/100,locPC->getDY()*zoom/100).mask());
                                    locPC->changeGeometry(posX,posY,locPC->getDX()*zoom/100,locPC->getDY()*zoom/100);
                                }
                                else {
                                    locPC->setGeometry(posX,posY,locPC->getDX()/4,locPC->getDY()/4);

                                }
                                xml->skipCurrentElement();

                            }
                            else
                            if (eltname == "session") {
                                locPC->LoadSession_File(xml);
                                xml->skipCurrentElement();
                            }
                            else
                                xml->skipCurrentElement();
                        }
                     }
                    else if (eltname == "link") {
                        int idpc1 = xml->attributes().value("idpcFrom").toString().toInt();
                        int idco1 = xml->attributes().value("idcoFrom").toString().toInt();
                        int idpc2 = xml->attributes().value("idpcTo").toString().toInt();
                        int idco2 = xml->attributes().value("idcoTo").toString().toInt();
                        bool close= (xml->attributes().value("close")=="false") ?false:true;
                        CPObject * locpc1 = map.value(idpc1);
                        CPObject * locpc2 = map.value(idpc2);
                        Cconnector * locco1 = locpc1->ConnList.value(idco1);
                        Cconnector * locco2 = locpc2->ConnList.value(idco2);
                        mainwindow->pdirectLink->addLink(locco1,locco2,close);

                    }
                    else
                        xml->skipCurrentElement();


                }
            }
            // Turn on all pockets
            for (int i=0;i<toPowerOn.count();i++) toPowerOn.at(i)->TurnON();
        }
        else
            xml->raiseError(QObject::tr("The file is not a PML version 1.0 file."));
    }
    for (int i=0;i <listpPObject.size();i++) {
        QMouseEvent *e=new QMouseEvent(QEvent::MouseButtonPress, QPoint(0,0), Qt::LeftButton, Qt::LeftButton,Qt::NoModifier);
        QApplication::sendEvent(listpPObject.at(i), e);
        QMouseEvent *e2=new QMouseEvent(QEvent::MouseButtonRelease, QPoint(0,0), Qt::LeftButton, Qt::LeftButton,Qt::NoModifier);
        QApplication::sendEvent(listpPObject.at(i), e2);
        delete e;
        delete e2;
    }

}

void MainWindowPockemul::quitPockEmul()
{
    if (ask(this,"Do you really want to quit ?",2)==1) {
        Close_All();
        QApplication::quit();
    }
}



void MainWindowPockemul::opensession(QString sessionFN)
{

    if (sessionFN=="") {
        sessionFN = QFileDialog::getOpenFileName(
                mainwindow,
                tr("Choose a file"),
                ".",
                tr("PockEmul sessions (*.pml)"));
    }

    QFile file(sessionFN);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(mainwindow,tr("PockEmul"),
                             tr("Cannot read file %1:\n%2.")
                             .arg(file.fileName())
                             .arg(file.errorString()));
            return ;
    }

    QXmlStreamReader *xml = new QXmlStreamReader(&file);

    opensession(xml);
}





void MainWindowPockemul::saveassession(QXmlStreamWriter *xml)
{
    QMap<CPObject*,int> map;
    QByteArray ba;
    QBuffer buffer(&ba);
    buffer.open(QIODevice::WriteOnly);
#if QT_VERSION >= 0x050000
    grab().toImage().scaled(QSize(600,600),Qt::KeepAspectRatio,Qt::SmoothTransformation).save(&buffer, "JPG");
#else
    QPixmap::grabWidget(this).toImage().scaled(QSize(600,600),Qt::KeepAspectRatio,Qt::SmoothTransformation).save(&buffer, "PNG");
#endif


    qWarning()<<"screenshot done";
    xml->setAutoFormatting(true);
    xml->writeStartElement("pml");
    xml->writeAttribute("version", "1.0");
    xml->writeAttribute("zoom",QString("%1").arg(zoom));
    xml->writeStartElement("snapshot");
    xml->writeAttribute("format", "JPG");
    xml->writeCharacters(ba.toBase64());
    xml->writeEndElement();

    // Fetch all objects
    for (int i=0;i<listpPObject.size();i++)
    {
        qWarning()<<"object:"<<i;
        CPObject *po = listpPObject.at(i);
        map.insert(po,i);
        po->serialize(xml,i);
    }

    // Connectors
    //xml->writeStartElement("links");

    // fetch AConnList
    for (int j = 0;j < mainwindow->pdirectLink->AConnList.size(); j++)
    {
        int idpc1 = map.value(mainwindow->pdirectLink->AConnList.at(j)->Parent);
        int idco1 = mainwindow->pdirectLink->AConnList.at(j)->Parent->ConnList.indexOf(mainwindow->pdirectLink->AConnList.at(j));
                //mainwindow->pdirectLink->AConnList.at(j)->Id;
        int idpc2 = map.value(mainwindow->pdirectLink->BConnList.at(j)->Parent);
        //int idco2 = mainwindow->pdirectLink->BConnList.at(j)->Id;
        int idco2 = mainwindow->pdirectLink->BConnList.at(j)->Parent->ConnList.indexOf(mainwindow->pdirectLink->BConnList.at(j));
        bool close = mainwindow->pdirectLink->closeList.at(j);
//        qWarning()<<"linkks";
        xml->writeStartElement("link");
        xml->writeAttribute("idpcFrom",QString("%1").arg(idpc1));
        xml->writeAttribute("idcoFrom",QString("%1").arg(idco1));
        xml->writeAttribute("idpcTo",QString("%1").arg(idpc2));
        xml->writeAttribute("idcoTo",QString("%1").arg(idco2));
        xml->writeAttribute("close",QString("%1").arg(close?"true":"false"));
        xml->writeEndElement();
    }



    //xml->writeEndElement();  // links

    xml->writeEndElement();  // pml
    qWarning()<<"END";
}

QString MainWindowPockemul::saveassessionString() {
    saveAll = YES;
    QString s;
    QXmlStreamWriter *xml = new QXmlStreamWriter(&s);
    saveassession(xml);
    qWarning()<<"save:"<<s;
    return s;
}

QString MainWindowPockemul::saveassession()
{


    // Take a snapshot

        saveAll = YES;
    QString s;
    QXmlStreamWriter *xml = new QXmlStreamWriter(&s);
    saveassession(xml);
    //MSG_ERROR(s)

    QString fn = QFileDialog::getSaveFileName(
                mainwindow,
                tr("Choose a filename to save session"),
                ".",
                tr("Session File (*.pml)"));


    if (!fn.isEmpty()) {

        QFileInfo fi( fn );
        if (fi.suffix().isEmpty())
        {
            // no suffix, adding .pml  - BUG For Android
            fn.append(".pml");
        }
        QFile f(fn);
        if (f.open(QFile::WriteOnly | QFile::Truncate)) {
            QTextStream out(&f);
            out << s;
        }
    }
    saveAll = ASK;

    delete xml;

    return fn;

}




void MainWindowPockemul::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event)
}

void MainWindowPockemul::updateTimer()
{
    static int deltaTime = -1;

    if (deltaTime == -1) {	t.start();}
    deltaTime = t.restart();

    rawclk += deltaTime;


#ifdef EMSCRIPTEN
    PcThread->run();
#endif

}

void MainWindowPockemul::doZoom(QPoint point,float delta,int step) {

    if (((zoom >= 20) && (delta<0)) ||
        ((zoom <300) && (delta >0))){
        int d = (delta>0) ? step : -step;
        delta = ((zoom+d)/zoom - 1)*100;
        zoom += d;

    }
    else delta = 0;


    this->setWindowTitle(QString("  Zoom=%2%").arg(zoom));

    for (int i=0;i<listpPObject.size();i++) {
        CPObject * locpc = listpPObject.at(i);


        // calculate the new origine

        float newposx = locpc->posx() + (locpc->posx()-point.x())*(delta)/100.0;
        float newposy = locpc->posy() + (locpc->posy()-point.y())*(delta)/100.0;

        locpc->changeGeometry(newposx,
                              newposy,
                              locpc->currentViewRect().width()*zoom/100/(locpc->Front?1:4),
                              locpc->currentViewRect().height()*zoom/100/(locpc->Front?1:4));


    }
}

void MainWindowPockemul::wheelEvent(QWheelEvent *event) {
//    qWarning()<<"MainWindowPockemul::wheelEvent";
    QPoint point = event->pos();

    float delta = event->delta()/12;

    doZoom(point,delta);

}

bool MainWindowPockemul::event(QEvent *event)
 {
//    qWarning()<<"MainWindowPockemul::event "<<event->type();
     if (event->type() == QEvent::Gesture)
         return gestureEvent(static_cast<QGestureEvent*>(event));
     return QWidget::event(event);
 }

bool MainWindowPockemul::gestureEvent(QGestureEvent *event)
{
//    qWarning()<<"MainWindowPockemul::gestureEvent";

//    if (QGesture *pan = event->gesture(Qt::PanGesture))
//        panTriggered(static_cast<QPanGesture *>(pan));
    if (QGesture *pinch = event->gesture(Qt::PinchGesture)) {
        startPosDrag = false;
        pinchTriggered(static_cast<QPinchGesture *>(pinch));
        event->accept();
    }
    return true;
}

//void MainWindowPockemul::panTriggered(QPanGesture *gesture)
// {
// #ifndef QT_NO_CURSOR
//     switch (gesture->state()) {
//         case Qt::GestureStarted:
//         case Qt::GestureUpdated:
//             setCursor(Qt::SizeAllCursor);
//             break;
//         default:
//             setCursor(Qt::ArrowCursor);
//     }
// #endif
//     QPointF delta = gesture->delta();
//     horizontalOffset += delta.x();
//     verticalOffset += delta.y();
//     update();
// }



 void MainWindowPockemul::pinchTriggered(QPinchGesture *gesture)
 {
     QPinchGesture::ChangeFlags changeFlags = gesture->changeFlags();
//     if (changeFlags & QPinchGesture::RotationAngleChanged) {
//         qreal value = gesture->property("rotationAngle").toReal();
//         qreal lastValue = gesture->property("lastRotationAngle").toReal();
//         rotationAngle += value - lastValue;
//     }
     if (changeFlags & QPinchGesture::ScaleFactorChanged) {
         qreal value = gesture->scaleFactor();//property("scaleFactor").toReal();
         scaleFactor *= value;
         if ((scaleFactor >=1.1) || (scaleFactor <=.9)) {
             doZoom(gesture->centerPoint().toPoint(),(scaleFactor*100.0)-100);
             scaleFactor = 1;
             update();
         }
     }
     if (gesture->state() == Qt::GestureFinished) {
         scaleFactor = 1;

         update();
     }

 }

void MainWindowPockemul::updateTimeTimer()
{
}

void MainWindowPockemul::updateFrameTimer()
{

    int statepersec;
    int rate=0;
    static int OneSecTimer=0;
    static int nbframe = 0;
    quint64 Current_State;

// Calculate emulation speed
// Normally each frame equal pPC->frequency state / NBFRAMEPERSEC


    if ( listpPObject.isEmpty()) return;

    static int deltaTime = -1;

    if (deltaTime == -1) {	tf.start(); }

    deltaTime = tf.elapsed();
    if (deltaTime >= 1000) tf.restart();

    for (int i = 0;i < listpPObject.size(); i++)
    {
            CPObject* CurrentpPC = listpPObject.at(i);

            if (CurrentpPC && CurrentpPC->pTIMER) {
                Current_State = CurrentpPC->pTIMER->state;

                CurrentpPC->pTIMER->nb_state += (Current_State - CurrentpPC->pTIMER->last_state);
                CurrentpPC->pTIMER->last_state = Current_State;

                // Update ToolTip only one time per second
                if ( deltaTime >= 1000)
                {
                    // later

                    QString str;
                    if (CurrentpPC->getfrequency()) {
                        //	AddLog(LOG_TIME,tr("Time Frame elapsed : %1 ms  nb=%2 cur=%3 last=%4").arg(deltaTime).arg(CurrentpPC->pTIMER->nb_state).arg(Current_State).arg(CurrentpPC->pTIMER->last_state));
                        statepersec = (int) ( CurrentpPC->getfrequency());
                        rate = (int) ((100L*CurrentpPC->pTIMER->nb_state)/((statepersec/1000L)*deltaTime));
                        CurrentpPC->pTIMER->nb_state=0;
#ifndef Q_OS_ANDROID
                        CurrentpPC->rate = nbframe;//rate;
#else
                        CurrentpPC->rate = nbframe;
#endif
                        nbframe = 0;
                        str.setNum((int)rate);
                        str = ": "+str+tr("% original speed");
                    }
#ifndef Q_OS_ANDROID
                    CurrentpPC->setToolTip(CurrentpPC->getName()+str);
#endif

                }

//                bool disp_on = true;
#if 1
                if (CurrentpPC->pLCDC)
                {
                    if (dynamic_cast<CpcXXXX *>(CurrentpPC) )
                    {
//                        CpcXXXX *tmpPC = (CpcXXXX*)CurrentpPC;
                        //if (tmpPC->getDisp_on())
                        {
                            nbframe++;
                            CurrentpPC->pLCDC->disp();
                            if (CurrentpPC->pLCDC->Refresh) CurrentpPC->Refresh_Display = true;
                        }
                    }
                }
#endif
                if ( CurrentpPC->Refresh_Display) {
//                   AddLog(LOG_DISPLAY,tr("Refresh Display"));
                    CurrentpPC->update();
                    CurrentpPC->Refresh_Display= false;
//                    qWarning()<<"Refresh Display";
                }
            }
        }

    if (OneSecTimer >= 1000) OneSecTimer=0;
}

void MainWindowPockemul::mousePressEvent	( QMouseEvent *event){
//    qWarning()<<"MainWindowPockemul::mousePressEvent";
    if (event->button() != Qt::LeftButton) {
        event->ignore();
//        qWarning()<<"ignore event";
        return;
    }
    setCursor(Qt::ClosedHandCursor);	// Change mouse pointer
    startPosDrag = true;
    PosDrag = event->globalPos();
    event->accept();
}

void MainWindowPockemul::MoveAll(QPoint p) {
    // Fetch all_object and move them
    for (int i=0;i<listpPObject.size();i++)
    {
        listpPObject.at(i)->Move(p);
    }
}

void MainWindowPockemul::mouseMoveEvent		( QMouseEvent * event ){
    if (startPosDrag)
    {
        QPoint delta(event->globalPos() - PosDrag);

        // Fetch all_object and move them
        MoveAll(delta);

        PosDrag = event->globalPos();
        update();
        return;
    }
}

void MainWindowPockemul::mouseReleaseEvent	( QMouseEvent *event){
    Q_UNUSED(event)

    startPosDrag = false;
    setCursor(Qt::ArrowCursor);

}

void MainWindowPockemul::keyReleaseEvent	( QKeyEvent * event ){
    Q_UNUSED(event)
}

void MainWindowPockemul::keyPressEvent		( QKeyEvent * event ){
//    qWarning()<<"MainWindowPockemul::keyPressEvent";
    event->ignore();
}

void MainWindowPockemul::resizeEvent		( QResizeEvent * event ){
    Q_UNUSED(event)

#ifndef EMSCRIPTEN
    downloadManager->resize();
#endif
#ifdef EMSCRIPTEN
    zoomSlider->setGeometry(mainwindow->width()-30,20,20,mainwindow->height()-40);
#endif
    if (cloud) cloud->resize(this->size());
//    qWarning()<<"Mainwindow resize";

    emit resizeSignal();
}

void MainWindowPockemul::resizeSlot( QSize size , CPObject *pObject)
{
    delete(pObject->FinalImage);

    int l = menuBar()->height();
    pObject->FinalImage = new QImage(size-QSize(0,l),QImage::Format_RGB32);
}

void MainWindowPockemul::DestroySlot( CPObject *pObject)
{
//qWarning()<< QApplication::topLevelWidgets();
    QList< QAction *> actionList = menuPockets->actions();
    for (int i=0; i< actionList.size();i++) {
        QAction* action = actionList.at(i);
        if (! action->data().isNull()) {
            CPObject *pc = (CPObject*) action->data().toString().toULongLong();
            if (pc == pObject) {
                 menuPockets->removeAction(action);
            }
        }
    }

    emit DestroySignal(pObject);
    WatchPoint.remove(pObject);

    pObject->exit();
    pObject->close();
#ifdef EMSCRIPTEN
    //delete pObject;
#else
    pObject->deleteLater();
#endif
}

void MainWindowPockemul::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event)
    if (Close_All()){
        QMainWindow::closeEvent(event);
        event->accept();
    }
    else
        event->ignore();
}

void MainWindowPockemul::slotMsgError(QString msg) {
    MSG_ERROR(msg);
}



void MainWindowPockemul::initCommandLine(void) {
//    static const struct QCommandLineConfigEntry conf[] =
//        {
//          { QCommandLine::Option, "v", "verbose", "Verbose level (0-3)", QCommandLine::Mandatory },
//          { QCommandLine::Switch, "l", "list", "Show a list", QCommandLine::Optional },
//          { QCommandLine::Param, NULL, "target", "The target", QCommandLine::Mandatory },
//          { QCommandLine::Param, NULL, "source", "The sources", QCommandLine::MandatoryMultiple },
//          { QCommandLine::None, NULL, NULL, NULL, QCommandLine::Default }
//        };
#ifdef Q_OS_ANDROID
    QString args = m_getArgs();
    qWarning()<<"android args:"<<args;
    if ( ! args.isEmpty()) {
        cmdline = new QCommandLine(args.prepend("pockemul ").
                                   split( QRegExp(" (?=[^\"]*(\"[^\"]*\"[^\"]*)*$)") ).
                                   replaceInStrings("\"",""));
    }
    else return;

#else

        cmdline = new QCommandLine(this);
#endif
//       cmdline->addOption(QChar('v'), "verbose", "verbose level (0-3)");
//       cmdline->addSwitch(QChar('l'), "list", "show a list");
//       cmdline->addParam("source", "the sources", QCommandLine::MandatoryMultiple);
//       cmdline->addParam("target", "the target", QCommandLine::Mandatory);

       cmdline->addOption('l',"load","Load a .pml session file");
       cmdline->addOption('r',"run","Run a pocket");
       cmdline->addSwitch('v', "version", "show current version");



//       cmdline->setConfig(conf);
       cmdline->enableVersion(true); // enable -v // --version
       cmdline->enableHelp(true); // enable -h / --help

      connect(cmdline, SIGNAL(switchFound(const QString &)),
              this, SLOT(switchFound(const QString &)));
      connect(cmdline, SIGNAL(optionFound(const QString &, const QVariant &)),
              this, SLOT(optionFound(const QString &, const QVariant &)));
      connect(cmdline, SIGNAL(paramFound(const QString &, const QVariant &)),
              this, SLOT(paramFound(const QString &, const QVariant &)));
      connect(cmdline, SIGNAL(parseError(const QString &)),
              this, SLOT(parseError(const QString &)));

      cmdline->parse();
}

void MainWindowPockemul::switchFound(const QString & name)
{
  qDebug() << "Switch:" << name;
}

void MainWindowPockemul::optionFound(const QString & name, const QVariant & value)
{
  qDebug() << "Option:" << name << value;
  if (name == "load") { opensession(value.toString()); }
  if (name == "run") {
      CPObject * pPC =LoadPocket(value.toString());
#ifdef Q_OS_ANDROID
//      if (pPC->getDX()> pPC->getDY())
//          pPC->maximizeWidth();
//      else
//          pPC->maximizeHeight();
#else
      Q_UNUSED(pPC)
#endif
  }
}

void MainWindowPockemul::paramFound(const QString & name, const QVariant & value)
{
  qDebug() << "Param:" << name << value;
}

void MainWindowPockemul::parseError(const QString & error)
{
  qDebug() << qPrintable(error);
  cmdline->showHelp(true, -1);
  QCoreApplication::quit();
}
