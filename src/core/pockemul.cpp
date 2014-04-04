
#include <QApplication>
#include <QtPlugin>
#include <QDebug>
#include <QDir>
#include <QSplashScreen>
#include <QScreen>

#include "launchbuttonwidget.h"
#include "mainwindowpockemul.h"


#include "pobject.h"
#include "dialogstartup.h"
#include "init.h"

#ifdef P_AVOID
#include "libavoid.h"
#endif

#include "downloadmanager.h"

//QTM_USE_NAMESPACE

#include "version.h"



#ifdef Q_OS_ANDROID
#include <jni.h>
static JavaVM* s_javaVM = 0;
static jclass s_PockemulObjectClassID = 0;
static jmethodID s_PockemulObjectConstructorMethodID=0;
static jmethodID s_PockemulObjectDialogMethodID=0;
static jmethodID s_PockemulObjectVibrateMethodID=0;
static jmethodID s_PockemulObjectopenURLMethodID=0;
static jmethodID s_PockemulObjectaddShortcutMethodID=0;
static jmethodID s_PockemulObjectgetArgsMethodID=0;
static jmethodID s_HapticObjectPlayMethodID=0;
static jmethodID s_HapticObjectPauseMethodID=0;
static jmethodID s_HapticObjectStopMethodID=0;
static jmethodID s_HapticObjectReleaseMethodID=0;
jobject m_PockemulObject;

#endif

MainWindowPockemul* mainwindow;
DownloadManager* downloadManager;



QString appDir;
QString workDir;

void test();
int main(int argc, char *argv[])
{


    QApplication *app = new QApplication(argc, argv);
     app->setAttribute(Qt::AA_DontCreateNativeWidgetSiblings);
#if QT_VERSION >= 0x050000
#ifdef Q_OS_ANDROID
     app->setAttribute(Qt::AA_SynthesizeMouseForUnhandledTouchEvents,false);
#else
     app->setAttribute(Qt::AA_SynthesizeMouseForUnhandledTouchEvents,true);
#endif
     test();
#else

#endif

    QCoreApplication::setOrganizationDomain("pockemul.free.fr");
    QCoreApplication::setOrganizationName("Remy Corp.");
    QCoreApplication::setApplicationName("PockEmul");
    QCoreApplication::setApplicationVersion(POCKEMUL_VERSION);


    int loadedFontID = QFontDatabase::addApplicationFont(":/hp41/41chrset.ttf");
  QStringList loadedFontFamilies = QFontDatabase::applicationFontFamilies(loadedFontID);
  qWarning()<<loadedFontFamilies;

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
mainwindow = new MainWindowPockemul();

    appDir = app->applicationDirPath();

#ifdef Q_OS_ANDROID
//    QSplashScreen splash;
//    splash.setPixmap(QPixmap(P_RES(":/pockemul/splash.png")).scaled(mainwindow->geometry().size()));
//    splash.showFullScreen();
//    splash.showMessage("Loading modules...",Qt::AlignLeft,Qt::white);
//    app->processEvents();
//    splash.finish(mainwindow);

//    mainwindow->menuBar()->setVisible(false);//->menuAction()->setVisible( false );
#endif

    QWidget *cw= new QWidget();
    mainwindow->setCentralWidget(cw);

    delete mainwindow->centralwidget;

    mainwindow->centralwidget = cw;
    mainwindow->setWindowIcon ( QIcon(":/core/pockemul.bmp") );
    mainwindow->resize(680,520);

#ifdef Q_OS_ANDROID
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

    float ratio = MAX(1,QGuiApplication::primaryScreen()->logicalDotsPerInch()/150);
    int iconSize = 48*ratio;
    int v_inter = 50*ratio;
    int v_pos = 12;
    LaunchButtonWidget* launch1 = new LaunchButtonWidget(mainwindow->centralwidget,
                                                LaunchButtonWidget::PictureFlow,
                                                QStringList()<<P_RES(":/pockemul/config.xml"),
                                                ":/core/pocket.png");
    launch1->setGeometry(0,v_pos,iconSize,iconSize);
    v_pos += v_inter;
    launch1->setToolTip("Start a new Pocket Emulation.");

#ifndef EMSCRIPTEN
    LaunchButtonWidget* launch2 = new LaunchButtonWidget(mainwindow->centralwidget,
                                                 LaunchButtonWidget::PictureFlow,
                                                 QStringList()<<P_RES(":/pockemul/configExt.xml"),
                                                 ":/core/ext.png");
    launch2->setGeometry(0,v_pos,iconSize,iconSize);
    v_pos += v_inter;
    launch2->setToolTip("Start a new Extension Emulation.");
#endif

#ifdef P_IDE
    LaunchButtonWidget* dev = new LaunchButtonWidget(mainwindow->centralwidget,
                                                     LaunchButtonWidget::Action,
                                                     QStringList(),
                                                     ":/core/dev.png");
    mainwindow->connect(dev,SIGNAL(clicked()),mainwindow,SLOT(IDE()));
    dev->setGeometry(0,v_pos,iconSize,iconSize);
    v_pos += v_inter;
    dev->setToolTip("Start the Integrated development Environment.");
#endif

    LaunchButtonWidget* save = new LaunchButtonWidget(mainwindow->centralwidget,
                                                      LaunchButtonWidget::Action,
                                                      QStringList(),
                                                      ":/core/save.png");
    mainwindow->connect(save,SIGNAL(clicked()),mainwindow,SLOT(saveassession()));
    save->setGeometry(0,v_pos,iconSize,iconSize);
    v_pos += v_inter;
    save->setToolTip("Save the current session.");

    LaunchButtonWidget* load = new LaunchButtonWidget(mainwindow->centralwidget,
                                                      LaunchButtonWidget::FileBrowser,
                                                      QStringList()<<"."<<"*.pml",
                                                      ":/core/load.png");
//    mainwindow->connect(load,SIGNAL(clicked()),mainwindow,SLOT(opensession()));
    load->setGeometry(0,v_pos,iconSize,iconSize);
    v_pos += v_inter;
    load->setToolTip("Load an existing session.");

#ifdef P_CLOUD
    LaunchButtonWidget* cloudButton = new LaunchButtonWidget(mainwindow->centralwidget,
                                                     LaunchButtonWidget::Action,
                                                     QStringList(),
#   ifdef Q_OS_ANDROID
                                                     ":/core/cloud-white.png");
#   else
                                                      ":/core/cloud.png");
#   endif
    mainwindow->connect(cloudButton,SIGNAL(clicked()),mainwindow,SLOT(CloudSlot()));
    cloudButton->setGeometry(0,v_pos,iconSize,iconSize);
    v_pos += v_inter;
    cloudButton->setToolTip("Go to the Cloud.");
#endif

    LaunchButtonWidget* bookcase = new LaunchButtonWidget(mainwindow->centralwidget,
                                                      LaunchButtonWidget::FileBrowser,
                                                          QStringList()<< (downloadManager->targetDir)<<"*.pdf",
                                                      ":/core/bookcase.png");
//    mainwindow->connect(load,SIGNAL(clicked()),mainwindow,SLOT(opensession()));
    bookcase->setGeometry(0,v_pos,iconSize,iconSize);
    v_pos += v_inter;
    bookcase->setToolTip("Browse the bookcase.");

    LaunchButtonWidget* exit = new LaunchButtonWidget(mainwindow->centralwidget,
                                                      LaunchButtonWidget::Action,
                                                      QStringList(),
                                                      ":/core/exit.png");
    mainwindow->connect(exit,SIGNAL(clicked()),mainwindow, SLOT(quitPockEmul()));//closeAllWindows()));

    exit->setGeometry(0,v_pos,iconSize,iconSize);
    v_pos += v_inter;

    exit->setToolTip("Exit PockEmul.");

//    CTinyBasic tb;
//    tb.test();

#ifdef EMSCRIPTEN
    mainwindow->zoomSlider = new QSlider(mainwindow->centralwidget);
    mainwindow->zoomSlider->setMinimum(10);
    mainwindow->zoomSlider->setMaximum(300);
    mainwindow->zoomSlider->setTickInterval(10);
    mainwindow->zoomSlider->setValue(100);
#endif

    mainwindow->show();

//#ifndef Q_OS_ANDROID
    mainwindow->initCommandLine();
//#endif

#ifdef EMSCRIPTEN
    app->exec();
    return 0;
#endif

    return app->exec();

}

#ifdef Q_OS_ANDROID

//Convert from QString to Java String
jstring fromQString	(	JNIEnv * 	env,QString * 	qstring	 )
{
      if (qstring == 0) {
            return 0;
      }

      return env->NewString((const jchar *) qstring->unicode(), (long) qstring->length());
}

// converting jstring to QString
QString toQString(JNIEnv * env,jstring 	str)
{
    if (str == 0L) {
        return "";
    }
    // converting jstring to QString
    const char *strResult = env->GetStringUTFChars( str, 0 );
    QString strres(strResult);
    env->ReleaseStringUTFChars( str, strResult);

    return strres;
}

      // this method is called immediately after the module is load
JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void* /*reserved*/)
{
    JNIEnv* env;
    if (vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK) {
        qCritical()<<"Can't get the enviroument";
        return -1;
    }

    s_javaVM = vm;
    // search for our class
    jclass clazz=env->FindClass("org/qtproject/pockemul/Pockemul");
    if (!clazz)
    {
        qCritical()<<"Can't find Pockemul class";
        return -1;
    }
    // keep a global reference to it
    s_PockemulObjectClassID = (jclass)env->NewGlobalRef(clazz);

    // search for its contructor
    s_PockemulObjectConstructorMethodID = env->GetMethodID(s_PockemulObjectClassID, "<init>", "()V");
    if (!s_PockemulObjectConstructorMethodID)
    {
        qCritical()<<"Can't find Pockemul class contructor";
        return -1;
    }

    // search for ShowMyModalDialog method
    s_PockemulObjectDialogMethodID = env->GetMethodID(s_PockemulObjectClassID, "ShowMyModalDialog", "(Ljava/lang/String;I)I");
    if (!s_PockemulObjectDialogMethodID)
    {
        qCritical()<<"Can't find ShowMyModalDialog method";
        return -1;
    }
    // search for Vibrate method
    s_PockemulObjectVibrateMethodID = env->GetMethodID(s_PockemulObjectClassID, "Vibrate", "()V");
    if (!s_PockemulObjectVibrateMethodID)
    {
        qCritical()<<"Can't find Vibrate method";
        return -1;
    }
    // search for openURL method
    s_PockemulObjectopenURLMethodID = env->GetMethodID(s_PockemulObjectClassID, "openURL", "(Ljava/lang/String;)V");
    if (!s_PockemulObjectopenURLMethodID)
    {
        qCritical()<<"Can't find Vibrate method";
        return -1;
    }

    // search for openURL method
    s_PockemulObjectaddShortcutMethodID = env->GetMethodID(s_PockemulObjectClassID, "addShortcut", "(Ljava/lang/String;Ljava/lang/String;)V");
    if (!s_PockemulObjectaddShortcutMethodID)
    {
        qCritical()<<"Can't find openURL method";
        return -1;
    }

    // search for getArgs method
    s_PockemulObjectgetArgsMethodID = env->GetMethodID(s_PockemulObjectClassID, "getArgs", "()Ljava/lang/String;");
    if (!s_PockemulObjectgetArgsMethodID)
    {
        qCritical()<<"Can't find openURL method";
        return -1;
    }
    qWarning()<<"Yahooo !";
    return JNI_VERSION_1_6;
}

#endif

QString m_getArgs() {
#ifdef Q_OS_ANDROID
    JNIEnv* env;
        // Qt is running in a different thread than Java UI, so you always Java VM *MUST* be attached to current thread
        if (s_javaVM->AttachCurrentThread(&env, NULL)<0)
        {
            qCritical()<<"AttachCurrentThread failed";
            return 0;
        }
        m_PockemulObject = env->NewGlobalRef(env->NewObject(s_PockemulObjectClassID, s_PockemulObjectConstructorMethodID));

        jstring res = (jstring) env->CallObjectMethod(m_PockemulObject, s_PockemulObjectgetArgsMethodID);

        qWarning()<<"return:"<<res;



            QString s= toQString(env,res);
            qWarning()<<"s="<<s;

            // Don't forget to detach from current thread
            s_javaVM->DetachCurrentThread();
            return s;

#endif
            return QString("");
}

int ask(QWidget *parent, QString msg, int nbButton) {
#ifdef Q_OS_ANDROID
    JNIEnv* env;
        // Qt is running in a different thread than Java UI, so you always Java VM *MUST* be attached to current thread
        if (s_javaVM->AttachCurrentThread(&env, NULL)<0)
        {
            qCritical()<<"AttachCurrentThread failed";
            return 0;
        }
        m_PockemulObject = env->NewGlobalRef(env->NewObject(s_PockemulObjectClassID, s_PockemulObjectConstructorMethodID));
        jstring parameter = fromQString(env,&msg);
        jint res = env->CallIntMethod(m_PockemulObject, s_PockemulObjectDialogMethodID,parameter,nbButton);

        qWarning()<<res;
        // Don't forget to detach from current thread
            s_javaVM->DetachCurrentThread();

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
    JNIEnv* env;
        // Qt is running in a different thread than Java UI, so you always Java VM *MUST* be attached to current thread
        if (s_javaVM->AttachCurrentThread(&env, NULL)<0)
        {
            qCritical()<<"AttachCurrentThread failed";

        }
        m_PockemulObject = env->NewGlobalRef(env->NewObject(s_PockemulObjectClassID, s_PockemulObjectConstructorMethodID));
        env->CallVoidMethod(m_PockemulObject, s_PockemulObjectVibrateMethodID);

        // Don't forget to detach from current thread
        s_javaVM->DetachCurrentThread();

#endif
}

void m_openURL(QUrl url) {
#ifdef Q_OS_ANDROID
    if (url.isLocalFile()) {
        qWarning()<<url;
        JNIEnv* env;
            // Qt is running in a different thread than Java UI, so you always Java VM *MUST* be attached to current thread
            if (s_javaVM->AttachCurrentThread(&env, NULL)<0)
            {
                qCritical()<<"AttachCurrentThread failed";

            }
            m_PockemulObject = env->NewGlobalRef(env->NewObject(s_PockemulObjectClassID, s_PockemulObjectConstructorMethodID));
            QString fn = url.toLocalFile();
            jstring parameter = fromQString(env,&fn);
            env->CallVoidMethod(m_PockemulObject, s_PockemulObjectopenURLMethodID,parameter);

            // Don't forget to detach from current thread
            s_javaVM->DetachCurrentThread();

            return;
    }

#endif
    QDesktopServices::openUrl(url);
}

void m_addShortcut(QString name, QString param) {
#ifdef Q_OS_ANDROID

    qWarning()<<"assShortcut";
        JNIEnv* env;
            // Qt is running in a different thread than Java UI, so you always Java VM *MUST* be attached to current thread
            if (s_javaVM->AttachCurrentThread(&env, NULL)<0)
            {
                qCritical()<<"AttachCurrentThread failed";

            }
            m_PockemulObject = env->NewGlobalRef(env->NewObject(s_PockemulObjectClassID, s_PockemulObjectConstructorMethodID));

            jstring parameter = fromQString(env,&param);
            jstring nameparam = fromQString(env,&name);
            env->CallVoidMethod(m_PockemulObject, s_PockemulObjectaddShortcutMethodID,nameparam,parameter);

            // Don't forget to detach from current thread
            s_javaVM->DetachCurrentThread();
            qWarning()<<"End addShortcut";
#else
    Q_UNUSED(name)
    Q_UNUSED(param)
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
