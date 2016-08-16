//TODO: replace all sprintf calls by snprintf. It's safer.


#include <QApplication>
#include <QtPlugin>
#include <QDebug>
#include <QDir>
#include <QSplashScreen>
#include <QScreen>
#include <QWidget>
#include <QUuid>

#ifdef Q_OS_WIN
#include <windowsx.h>
#include <QtPlatformHeaders/QWindowsWindowFunctions>
#endif

#include "launchbuttonwidget.h"
#include "mainwindowpockemul.h"


#include "pobject.h"
#include "init.h"
#include "cloud/cloudwindow.h"
#include "renderView.h"

#ifdef P_AVOID
#include "libavoid.h"
#endif

#include "downloadmanager.h"
#include "Keyb.h"

//QTM_USE_NAMESPACE

#include "version.h"
#include "QZXing.h"

#include "vibrator.h"
Vibrator vibrate;
int vibDelay;

#ifdef Q_OS_ANDROID
#include <QAndroidJniObject>

#endif

#include "ganalytics.h"
GAnalytics *tracker;


MainWindowPockemul* mainwindow;
DownloadManager* downloadManager;
CrenderView* view;
QSettings* settings;
QUuid uniqueId;

#include "watchpoint.h"
CWatchPoint WatchPoint;

LaunchButtonWidget* launch1;
LaunchButtonWidget* launch2;
LaunchButtonWidget* dev;
LaunchButtonWidget* save;
LaunchButtonWidget* load;
LaunchButtonWidget* cloudButton;
LaunchButtonWidget* bookcase;
LaunchButtonWidget* exitButton;

bool soundEnabled=true;
bool hiRes=true;
bool syncEnabled=true;
bool flipOnEdge=true;
bool trackerEnabled=false;



QString appDir;
QString workDir;

class CPObject;
extern QList<CPObject *> listpPObject;

void test();
void buildMenu();
int ask(QWidget *parent, QString msg, int nbButton);

void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    Q_UNUSED(type)
    Q_UNUSED(context)

    if(msg.startsWith("Invalid parameter")) {
        qWarning()<<msg;
    }
//    QByteArray localMsg = msg.toLocal8Bit();
//    switch (type) {
//    case QtDebugMsg:
//        fprintf(stderr, "Debug: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
//        break;
//    case QtInfoMsg:
//        fprintf(stderr, "Info: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
//        break;
//    case QtWarningMsg:
//        fprintf(stderr, "Warning: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
//        break;
//    case QtCriticalMsg:
//        fprintf(stderr, "Critical: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
//        break;
//    case QtFatalMsg:
//        fprintf(stderr, "Fatal: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
//        abort();
//    }
}


#ifdef Q_OS_WIN

#define WM_TABLET_DEFBASE                    0x02C0
#define WM_TABLET_QUERYSYSTEMGESTURESTATUS   (WM_TABLET_DEFBASE + 12)

#define DISABLE_PRESSANDHOLD        0x00000001
#define DISABLE_PENTAPFEEDBACK      0x00000008
#define DISABLE_PENBARRELFEEDBACK   0x00000010
#define DISABLE_TOUCHUIFORCEON      0x00000100
#define DISABLE_TOUCHUIFORCEOFF     0x00000200
#define DISABLE_TOUCHSWITCH         0x00008000
#define DISABLE_FLICKS              0x00010000
#define ENABLE_FLICKSONCONTEXT      0x00020000
#define ENABLE_FLICKLEARNINGMODE    0x00040000
#define DISABLE_SMOOTHSCROLLING     0x00080000
#define DISABLE_FLICKFALLBACKKEYS   0x00100000
#define ENABLE_MULTITOUCHDATA       0x01000000

class MyMSGEventFilter : public QAbstractNativeEventFilter
{
public:
    virtual bool nativeEventFilter(const QByteArray &eventType, void *message, long *result) Q_DECL_OVERRIDE
    {
        if ( (eventType == "windows_generic_MSG") || (eventType == "windows_dispatcher_MSG") ) {
            MSG* msg = static_cast<MSG *>(message);
            if (msg->message==WM_TABLET_QUERYSYSTEMGESTURESTATUS) {
                // check if keyAt
                int  x = GET_X_LPARAM(msg->lParam);
                int  y = GET_Y_LPARAM(msg->lParam);
//                qWarning()<<x<<y;
                for (int i =0;i < listpPObject.count();i++) {
                    if (listpPObject[i]->pKEYB) {
                        if (listpPObject[i]->pKEYB->KeyClick(listpPObject[i]->mapFromGlobal(QPoint(x,y)))) {
                            *result = DISABLE_PRESSANDHOLD;
                        }
                    }
                }
                return true;
            }
        }
        return false;
    }
};
#endif

int main(int argc, char *argv[])
{

//    try {
//        qWarning()<<"throw";
//        throw 1;
//    }
//    catch (...) {
//        qWarning()<<"CATCHED:";
//    }

//    return 0;

    //    qInstallMessageHandler(myMessageOutput);


    launch1 = launch2 = dev = save = load = cloudButton = bookcase = exitButton = 0;

    // Force the software backend always. Qt 5.8
//    QQuickWindow::setSceneGraphBackend(QSGRendererInterface::Software);

    QApplication *app = new QApplication(argc, argv);

#ifdef Q_OS_WIN
    app->installNativeEventFilter(new MyMSGEventFilter());
#endif

    app->setAttribute(Qt::AA_DontCreateNativeWidgetSiblings);
#if QT_VERSION >= 0x050000
#ifdef Q_OS_ANDROID
     app->setAttribute(Qt::AA_SynthesizeMouseForUnhandledTouchEvents,true);
#else
     app->setAttribute(Qt::AA_SynthesizeMouseForUnhandledTouchEvents,true);
#endif
//     test();
#else

#endif

    QCoreApplication::setOrganizationDomain("pockemul.free.fr");
    QCoreApplication::setOrganizationName("Remy Corp.");
    QCoreApplication::setApplicationName("PockEmul");
    QCoreApplication::setApplicationVersion(POCKEMUL_VERSION);

    QZXing::registerQMLTypes();


#ifdef Q_OS_MAC
    QDir tmpdir(QApplication::applicationDirPath());
    tmpdir.cdUp();
    tmpdir.cd("plugins");
    QApplication::setLibraryPaths(QStringList(tmpdir.absolutePath()));
    qWarning()<<QApplication::libraryPaths();
#endif

    QDir home = QDir::home();
    if (!home.exists("pockemul")) {
        home.mkpath("pockemul/documents");
    }

    workDir = QDir::homePath()+"/pockemul/";

#ifdef Q_OS_ANDROID

    // Change currentPath t;o /sdcard/pockemul
    QDir d("/");
    d.mkpath("/sdcard/pockemul/documents");
    QDir::setCurrent("/sdcard/pockemul");
    workDir = "/sdcard/pockemul/";

#endif

    settings = new QSettings(workDir+"config.ini",QSettings::IniFormat);

    QString _Id = Cloud::getValueFor("uniqueId","0000");
    if (_Id=="0000") {
        uniqueId = QUuid::createUuid();
        Cloud::saveValueFor("uniqueId",uniqueId.toString());
    }
    else {
        uniqueId = QUuid(_Id);
    }

    qWarning()<<"uniqueId"<<uniqueId;

    tracker = new GAnalytics("UA-82656903-1");

    tracker->setUserID(uniqueId.toString().mid(1,36));
    tracker->setLogLevel(GAnalytics::Debug);

    tracker->startSession();
    tracker->sendAppView("main");

    QString _te = Cloud::getValueFor("trackerEnabled","not defined");
    if (_te == "not defined") {
           QString _msg;
           _msg += "Are you agree to enable activity tracking ?\n";
           _msg += "It is only to track functions usage\n";
           _msg += "Only anonymous information will be transmitted to PockEmul Server";

           int _res = ask(0,_msg,2);
           trackerEnabled = (_res == 1);
           Cloud::saveValueFor("trackerEnabled",trackerEnabled ? "on" : "off");
    }
    else {
        trackerEnabled = (_te == "on") ? true : false;
        qWarning()<<"trackerEnabled"<<trackerEnabled;
    }

    vibDelay = Cloud::getValueFor("vibDelay","50").toInt();

    QSplashScreen splash;
    splash.setPixmap(QPixmap(P_RES(":/pockemul/splash.png")));//.scaled(mainwindow->geometry().size()));
    splash.show();
    splash.showMessage("Loading modules...",Qt::AlignLeft,Qt::white);
    app->processEvents();

    mainwindow = new MainWindowPockemul();

    appDir = app->applicationDirPath();
    qWarning()<<appDir;

    QWidget *cw= new QWidget();
    mainwindow->setCentralWidget(cw);
    delete mainwindow->centralwidget;
    mainwindow->centralwidget = cw;


    mainwindow->setWindowIcon ( QIcon(":/core/pockemul.bmp") );
    mainwindow->resize(680,520);
    qWarning()<<Cloud::getValueFor("geometry").toLatin1();

    mainwindow->centralwidget->setStyleSheet("background-color:black;color: white;selection-background-color: grey;");

#ifdef EMSCRIPTEN
    mainwindow->setWindowTitle("PockEmul Online");
#endif

#ifndef EMSCRIPTEN
    downloadManager = new DownloadManager();
    downloadManager->targetDir = workDir+"documents";
#endif

    soundEnabled =  (Cloud::getValueFor("soundEnabled","on")=="on") ? true : false;
    hiRes =  (Cloud::getValueFor("hiRes","on")=="on") ? true : false;


#ifdef EMSCRIPTEN
    mainwindow->zoomSlider = new QSlider(mainwindow->centralwidget);
    mainwindow->zoomSlider->setMinimum(10);
    mainwindow->zoomSlider->setMaximum(300);
    mainwindow->zoomSlider->setTickInterval(10);
    mainwindow->zoomSlider->setValue(100);
#endif



    mainwindow->openGlFlag=true;
#ifdef Q_OS_ANDROID
    mainwindow->showFullScreen();
    mainwindow->menuBar()->hide();
#else
    mainwindow->show();
    QWindowsWindowFunctions::setHasBorderInFullScreen(mainwindow->windowHandle(), true);
    mainwindow->restoreGeometry(QByteArray::fromHex(Cloud::getValueFor("geometry").toLatin1()));
#endif


//    test();

#ifdef Q_OS_ANDROID
    qInstallMessageHandler(0);
#else
    qInstallMessageHandler(myMessageOutput);
#endif

    mainwindow->initCommandLine();

    float ratio = Cloud::getValueFor("hiResRatio","0").toFloat();
    if (0==ratio) {
        // max 8*50*ratio+2*v_pos = height
        float _minSize =  MIN(QGuiApplication::primaryScreen()->size().width(),QGuiApplication::primaryScreen()->size().height());
        float _maxRatio = (_minSize - 2*12)/400.0;

        ratio = MIN(_maxRatio,
                    MAX(1,QGuiApplication::primaryScreen()->physicalDotsPerInch()/150)
                    );

        Cloud::saveValueFor("hiResRatio",QString("%1").arg(ratio));
    }


//qWarning()<<"okl";
    view = 0;
    if (mainwindow->openGlFlag) {
        qWarning()<<"opengl";


        mainwindow->menuBar()->setVisible(false);

        QVBoxLayout *windowLayout = new QVBoxLayout(mainwindow->centralwidget);
        view = new CrenderView(mainwindow->centralwidget);
        windowLayout->addWidget(view);
        windowLayout->setMargin(0);
    }
    else {
        qWarning()<<"no opengl";

        buildMenu();
        launch1->show();
        launch2->show();
        dev->show();
        save->show();
        load->show();
        cloudButton->show();
        bookcase->show();
        exitButton->show();
        qWarning()<<"end fullopengl";
    }

    if (!mainwindow->loadPML.isEmpty()) {
        mainwindow->opensession(workDir+"sessions/"+mainwindow->loadPML);
    }
    if (!mainwindow->runPocket.isEmpty()) {
        CPObject * pPC =mainwindow->LoadPocket(mainwindow->runPocket);
//        pPC->slotDoubleClick(QPoint(0,0));
  #ifdef Q_OS_ANDROID
        pPC->maximize(pPC->RectWithLinked().center().toPoint());
//        if (pPC->getDX()> pPC->getDY())
//            pPC->maximizeWidth();
//        else
//            pPC->maximizeHeight();
  #else
        Q_UNUSED(pPC)
  #endif
    }

    splash.close();

#ifdef EMSCRIPTEN
    app->exec();
    return 0;
#endif

    int _res =  app->exec();

#ifdef Q_OS_ANDROID
    // On Android, the application is still running in the background after quit.
    // So I kill the process ... :-(
    QAndroidJniObject::callStaticMethod<void>("org/qtproject/pockemul/PockemulActivity",
                                        "KillProcess",
                                        "()V");
#endif

    return _res;

}


void buildMenu() {

    qWarning()<<QGuiApplication::primaryScreen()->physicalDotsPerInch();

    int v_pos = 12;
    float ratio = Cloud::getValueFor("hiResRatio","0").toFloat();
    if (0==ratio) {
        // max 8*50*ratio+2*v_pos = height
        float _minSize =  MIN(QGuiApplication::primaryScreen()->size().width(),QGuiApplication::primaryScreen()->size().height());
        float _maxRatio = (_minSize - 2*v_pos)/400.0;

        ratio = MIN(_maxRatio,
                    MAX(1,QGuiApplication::primaryScreen()->physicalDotsPerInch()/150)
                    );

        Cloud::saveValueFor("hiResRatio",QString("%1").arg(ratio));
    }
    int iconSize = 48*ratio;
    int v_inter = 50*ratio;


    qWarning()<<"ratio:"<<ratio<<"  iconSize:"<<iconSize<<"  inter:"<<v_inter;

    launch1 = new LaunchButtonWidget(mainwindow->centralwidget,
                                                LaunchButtonWidget::PictureFlow,
                                                QStringList()<<P_RES(":/pockemul/config.xml"),
                                                ":/core/pocket.png","BRAND");
    launch1->setGeometry(0,v_pos,iconSize,iconSize);
    launch1->hide();
    v_pos += v_inter;
    launch1->setToolTip("Start a new Pocket Emulation.");
    qWarning()<<"launch1="<<launch1;

#ifndef EMSCRIPTEN
    launch2 = new LaunchButtonWidget(mainwindow->centralwidget,
                                                 LaunchButtonWidget::PictureFlow,
                                                 QStringList()<<P_RES(":/pockemul/configExt.xml"),
                                                 ":/core/ext.png");
    launch2->setGeometry(0,v_pos,iconSize,iconSize);
    launch2->hide();
    v_pos += v_inter;
    launch2->setToolTip("Start a new Extension Emulation.");
#endif

#ifdef P_IDE
    dev = new LaunchButtonWidget(mainwindow->centralwidget,
                                                     LaunchButtonWidget::Action,
                                                     QStringList(),
                                                     ":/core/dev.png");
    mainwindow->connect(dev,SIGNAL(clicked()),mainwindow,SLOT(IDE()));
    dev->setGeometry(0,v_pos,iconSize,iconSize);
    dev->hide();
    v_pos += v_inter;
    dev->setToolTip("Start the Integrated development Environment.");
#endif

    save = new LaunchButtonWidget(mainwindow->centralwidget,
                                                      LaunchButtonWidget::Action,
                                                      QStringList(),
                                                      ":/core/save.png");
    mainwindow->connect(save,SIGNAL(clicked()),mainwindow,SLOT(saveassession()));
    save->setGeometry(0,v_pos,iconSize,iconSize);
    save->hide();
    v_pos += v_inter;
    save->setToolTip("Save the current session.");

    QDir dir;
    dir.mkpath(workDir+"/sessions/");
    dir.setPath(workDir+"/sessions/");
    load = new LaunchButtonWidget(mainwindow->centralwidget,
                                                      LaunchButtonWidget::FileBrowser,
                                                      QStringList()<<dir.path()<<"*.pml",
                                                      ":/core/load.png");
//    mainwindow->connect(load,SIGNAL(clicked()),mainwindow,SLOT(opensession()));
    load->setGeometry(0,v_pos,iconSize,iconSize);
    load->hide();
    v_pos += v_inter;
    load->setToolTip("Load an existing session.");

#ifdef P_CLOUD
    cloudButton = new LaunchButtonWidget(mainwindow->centralwidget,
                                                     LaunchButtonWidget::Action,
                                                     QStringList(),
                                                     ":/core/cloud-white.png");

    mainwindow->connect(cloudButton,SIGNAL(clicked()),mainwindow,SLOT(CloudSlot()));
    cloudButton->setGeometry(0,v_pos,iconSize,iconSize);
    cloudButton->hide();
    v_pos += v_inter;
    cloudButton->setToolTip("Go to the Cloud.");
#endif

    bookcase = new LaunchButtonWidget(mainwindow->centralwidget,
                                                      LaunchButtonWidget::FileBrowser,
                                                          QStringList()<< (downloadManager->targetDir)<<"*.pdf",
                                                      ":/core/bookcase.png");
//    mainwindow->connect(load,SIGNAL(clicked()),mainwindow,SLOT(opensession()));
    bookcase->setGeometry(0,v_pos,iconSize,iconSize);
    bookcase->hide();

    v_pos += v_inter;
    bookcase->setToolTip("Browse the bookcase.");

    exitButton = new LaunchButtonWidget(mainwindow->centralwidget,
                                                      LaunchButtonWidget::Action,
                                                      QStringList(),
                                                      ":/core/exit.png");
    mainwindow->connect(exitButton,SIGNAL(clicked()),mainwindow, SLOT(close()));//closeAllWindows()));

    exitButton->setGeometry(0,v_pos,iconSize,iconSize);
    exitButton->hide();

    v_pos += v_inter;
    exitButton->setToolTip("Exit PockEmul.");

}

QString m_getArgs() {

#ifdef Q_OS_ANDROID

    QAndroidJniObject stringArgs = QAndroidJniObject::callStaticObjectMethod("org/qtproject/pockemul/PockemulActivity",
                                        "getArgs",
                                        "()Ljava/lang/String;");
    qWarning()<<"return:"<<stringArgs.toString();


    return stringArgs.toString();

#endif
    return QString("");
}


void Vibrate() {
#ifdef Q_OS_ANDROID
    qWarning() << "Vibrate";
    QAndroidJniObject::callStaticMethod<void>("org/qtproject/pockemul/PockemulActivity",
                                        "Vibrate",
                                        "(I)V",vibDelay);

//    vibrate.vibrate(vibDelay);
#endif
}

int ask(QWidget *parent, QString msg, int nbButton) {
    qWarning() << "Ask";
#ifdef Q_OS_ANDROID
//    mainwindow->showMaximized();

    jint res = QAndroidJniObject::callStaticMethod<jint>("org/qtproject/pockemul/PockemulActivity",
                                        "ShowMyModalDialog",
                                        "(Ljava/lang/String;I)I",
                                        QAndroidJniObject::fromString(msg).object<jstring>(),
                                        nbButton);

        qWarning()<<res;

        Vibrate();


        return res;
#else
    if (nbButton==1) {
        QMessageBox::warning(parent, "PockEmul",msg);
        return 1;
    }
    if (nbButton==2) {
        switch (QMessageBox::question(parent, "PockEmul",msg,QMessageBox::Yes|QMessageBox::No)) {
        case QMessageBox::Yes: return 1;
        case QMessageBox::No: return 2;
        default: return 0;
        }
    }
    if (nbButton==3) {
        switch (QMessageBox::question(parent, "PockEmul",msg,QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel)) {
        case QMessageBox::Yes: return 1;
        case QMessageBox::No: return 2;
        case QMessageBox::Cancel: return 3;
        default: return 0;
        }
    }

#endif
    return 0;
}


void m_openURL(QUrl url) {
#ifdef Q_OS_ANDROID
    if (url.isLocalFile()) {
        qWarning()<<url;
        QString fn = url.toLocalFile();
        QAndroidJniObject::callStaticMethod<void>("org/qtproject/pockemul/PockemulActivity",
                                            "openURL",
                                            "(Ljava/lang/String;)V",
                                            QAndroidJniObject::fromString(fn).object<jstring>()     );


            return;
    }

#endif
    qWarning()<<"open url:"<<url;
    QDesktopServices::openUrl(url);
}

void m_addShortcut(QString name, QString param) {
#ifdef Q_OS_ANDROID

    qWarning()<<"assShortcut";
    QAndroidJniObject::callStaticMethod<void>("org/qtproject/pockemul/PockemulActivity",
                                              "addShortcut",
                                              "(Ljava/lang/String;Ljava/lang/String;)V",
                                              QAndroidJniObject::fromString(name).object<jstring>(),
                                              QAndroidJniObject::fromString(param).object<jstring>());

    qWarning()<<"End addShortcut";
#else
    Q_UNUSED(name)
    Q_UNUSED(param)
#endif
}

QString P_RES(QString _name) {

    QString _locName = _name;
    _locName.replace(":",workDir+"/res");
    QFileInfo info_locName(_locName);

#ifdef LOCRES
    return _locName;
#else
    if (hiRes) {
        // if file exists, rename it
        QString hiResFname = info_locName.path()+"/"+info_locName.baseName()+"@2X."+info_locName.suffix();
//        qWarning()<<hiResFname;
        if (QFile::exists(hiResFname)) {
            return hiResFname;
        }
    }
    if (QFile::exists(_locName)) {
        return _locName;
    }
    return _name;

#endif
}

#if QT_VERSION >= 0x050000
QString Orientation(Qt::ScreenOrientation orientation)
{
    switch (orientation) {
        case Qt::PrimaryOrientation           : return "Primary";
        case Qt::LandscapeOrientation         : return "Landscape";
        case Qt::PortraitOrientation          : return "Portrait";
        case Qt::InvertedLandscapeOrientation : return "Inverted landscape";
        case Qt::InvertedPortraitOrientation  : return "Inverted portrait";
        default                               : return "Unknown";
    }
}

void test() {
    foreach (QScreen *screen, QGuiApplication::screens()) {
            qWarning() << "Information for screen:" << screen->name();
            qWarning() << "  Available geometry:" << screen->availableGeometry().x() << screen->availableGeometry().y() << screen->availableGeometry().width() << "x" << screen->availableGeometry().height();
            qWarning() << "  Available size:" << screen->availableSize().width() << "x" << screen->availableSize().height();
            qWarning() << "  Available virtual geometry:" << screen->availableVirtualGeometry().x() << screen->availableVirtualGeometry().y() << screen->availableVirtualGeometry().width() << "x" << screen->availableVirtualGeometry().height();
            qWarning() << "  Available virtual size:" << screen->availableVirtualSize().width() << "x" << screen->availableVirtualSize().height();
            qWarning() << "  Depth:" << screen->depth() << "bits";
            qWarning() << "  Geometry:" << screen->geometry().x() << screen->geometry().y() << screen->geometry().width() << "x" << screen->geometry().height();
            qWarning() << "  Logical DPI:" << screen->logicalDotsPerInch();
            qWarning() << "  Logical DPI X:" << screen->logicalDotsPerInchX();
            qWarning() << "  Logical DPI Y:" << screen->logicalDotsPerInchY();
            qWarning() << "  Orientation:" << Orientation(screen->orientation());
            qWarning() << "  Physical DPI:" << screen->physicalDotsPerInch();
            qWarning() << "  Physical DPI X:" << screen->physicalDotsPerInchX();
            qWarning() << "  Physical DPI Y:" << screen->physicalDotsPerInchY();
            qWarning() << "  Physical size:" << screen->physicalSize().width() << "x" << screen->physicalSize().height() << "mm";
            qWarning() << "  Primary orientation:" << Orientation(screen->primaryOrientation());
            qWarning() << "  Refresh rate:" << screen->refreshRate() << "Hz";
            qWarning() << "  Size:" << screen->size().width() << "x" << screen->size().height();
            qWarning() << "  Virtual geometry:" << screen->virtualGeometry().x() << screen->virtualGeometry().y() << screen->virtualGeometry().width() << "x" << screen->virtualGeometry().height();
            qWarning() << "  Virtual size:" << screen->virtualSize().width() << "x" << screen->virtualSize().height();
        }
}
#endif



