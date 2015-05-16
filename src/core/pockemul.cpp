//TODO: replace all sprintf calls by snprintf. It's safer.

#include <QApplication>
#include <QtPlugin>
#include <QDebug>
#include <QDir>
#include <QSplashScreen>
#include <QScreen>
#include <QWidget>


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

//QTM_USE_NAMESPACE

#include "version.h"


#ifdef Q_OS_ANDROID
#include <QAndroidJniObject>

#endif

MainWindowPockemul* mainwindow;
DownloadManager* downloadManager;
CrenderView* view;


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



QString appDir;
QString workDir;

void test();

void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
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

int main(int argc, char *argv[])
{

    qInstallMessageHandler(myMessageOutput);

    QApplication *app = new QApplication(argc, argv);
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




#ifdef Q_OS_MAC
    QDir dir(QApplication::applicationDirPath());
    dir.cdUp();
    dir.cd("plugins");
    QApplication::setLibraryPaths(QStringList(dir.absolutePath()));
    qWarning()<<QApplication::libraryPaths();
#endif

    QDir home = QDir::home();
    if (!home.exists("pockemul")) {
        home.mkpath("pockemul/documents");
    }

    workDir = QDir::homePath()+"/pockemul/";

#ifdef Q_OS_ANDROID
//    QFont f = app.font();
//    f.setItalic(true); //bold also works
//    app.setFont(f);

    // Change currentPath to /sdcard/pockemul
    QDir d("/");
    d.mkpath("/sdcard/pockemul/documents");
    QDir::setCurrent("/sdcard/pockemul");
    workDir = "/sdcard/pockemul/";

//    QProxyStyle *s = new QProxyStyle();//QAndroidStyle();

//    QApplication::setStyle(s);//new QAndroidStyle());

#endif

    QSplashScreen splash;
    splash.setPixmap(QPixmap(P_RES(":/pockemul/splash.png")));//.scaled(mainwindow->geometry().size()));
    splash.show();
    splash.showMessage("Loading modules...",Qt::AlignLeft,Qt::white);
    app->processEvents();

    mainwindow = new MainWindowPockemul();

    appDir = app->applicationDirPath();
    qWarning()<<appDir;

//#ifdef Q_OS_ANDROID



//    mainwindow->menuBar()->setVisible(false);//->menuAction()->setVisible( false );
    // search for ShowMyModalDialog method

//#endif

#if 1
    QWidget *cw= new QWidget();
    mainwindow->setCentralWidget(cw);
    delete mainwindow->centralwidget;
    mainwindow->centralwidget = cw;
#else
    view = new CrenderView();
    mainwindow->setCentralWidget(view);
    delete mainwindow->centralwidget;
    mainwindow->centralwidget = view;
#endif



    mainwindow->setWindowIcon ( QIcon(":/core/pockemul.bmp") );
    mainwindow->resize(680,520);

#if 1 //def Q_OS_ANDROID
    mainwindow->centralwidget->setStyleSheet("background-color:black;color: white;selection-background-color: grey;");
#endif

#ifdef EMSCRIPTEN
    mainwindow->setWindowTitle("PockEmul Online");
#endif

#ifndef EMSCRIPTEN
    downloadManager = new DownloadManager();
    downloadManager->targetDir = QDir::homePath()+"/pockemul/documents";
#   ifdef Q_OS_ANDROID
        downloadManager->targetDir = "/sdcard/pockemul/documents";
#   endif
#endif

        soundEnabled =  (Cloud::getValueFor("soundEnabled","on")=="on") ? true : false;
        hiRes =  (Cloud::getValueFor("hiRes","on")=="on") ? true : false;

    float ratio = MAX(1,QGuiApplication::primaryScreen()->logicalDotsPerInch()/150);
    int iconSize = 48*ratio;
    int v_inter = 50*ratio;
    int v_pos = 12;
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
#if 1 //def Q_OS_ANDROID
                                                     ":/core/cloud-white.png");
#else
                                                       ":/core/cloud.png");
#endif
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
    mainwindow->connect(exitButton,SIGNAL(clicked()),mainwindow, SLOT(quitPockEmul()));//closeAllWindows()));

    exitButton->setGeometry(0,v_pos,iconSize,iconSize);
    exitButton->hide();

    v_pos += v_inter;
    exitButton->setToolTip("Exit PockEmul.");


#ifdef EMSCRIPTEN
    mainwindow->zoomSlider = new QSlider(mainwindow->centralwidget);
    mainwindow->zoomSlider->setMinimum(10);
    mainwindow->zoomSlider->setMaximum(300);
    mainwindow->zoomSlider->setTickInterval(10);
    mainwindow->zoomSlider->setValue(100);
#endif

    splash.close();

#ifdef Q_OS_ANDROID
    mainwindow->showFullScreen();
#else
    mainwindow->show();
#endif

//#ifndef Q_OS_ANDROID
    mainwindow->initCommandLine();
//#endif

    view = 0;
    if (mainwindow->openGlFlag) {
        qWarning()<<"opengl";
        QVBoxLayout *windowLayout = new QVBoxLayout(mainwindow->centralwidget);
        view = new CrenderView(mainwindow->centralwidget);
        windowLayout->addWidget(view);
        windowLayout->setMargin(0);
    }

    if (!mainwindow->loadPML.isEmpty()) {
        mainwindow->opensession(mainwindow->loadPML);
    }
    if (!mainwindow->runPocket.isEmpty()) {
        CPObject * pPC =mainwindow->LoadPocket(mainwindow->runPocket);

  #ifdef Q_OS_ANDROID
  //      if (pPC->getDX()> pPC->getDY())
  //          pPC->maximizeWidth();
  //      else
  //          pPC->maximizeHeight();
  #else
        Q_UNUSED(pPC)
  #endif
    }

#ifdef EMSCRIPTEN
    app->exec();
    return 0;
#endif


    return app->exec();

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

int ask(QWidget *parent, QString msg, int nbButton) {
    qWarning() << "Ask";
#ifdef Q_OS_ANDROID
    jint res = QAndroidJniObject::callStaticMethod<jint>("org/qtproject/pockemul/PockemulActivity",
                                        "ShowMyModalDialog",
                                        "(Ljava/lang/String;I)I",
                                        QAndroidJniObject::fromString(msg).object<jstring>(),
                                        nbButton);

        qWarning()<<res;

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

void Vibrate() {
#ifdef Q_OS_ANDROID
    qWarning() << "Vibrate";
    QAndroidJniObject::callStaticMethod<void>("org/qtproject/pockemul/PockemulActivity",
                                        "Vibrate",
                                        "()V");

#endif
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



