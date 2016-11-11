
#include <QQmlContext>
#include <QSettings>

#include "renderView.h"
#include "cloud/cloudimageprovider.h"
#include "cloud/pocketimageprovider.h"

#include "mainwindowpockemul.h"
#include "pobject.h"
#include "launchbuttonwidget.h"
#include "dialogkeylist.h"
#include "Keyb.h"
#include "QZXing.h"
#include "vibrator.h"
#include "downloadmanager.h"
#include "dialoganalog.h"
#include "ui/windowide.h"
#include "binarydata.h"
#include "laserfilter.h"

#include "ganalytics.h"
extern GAnalytics *tracker;

extern MainWindowPockemul *mainwindow;
extern int ask(QWidget *parent, QString msg, int nbButton);
extern void m_addShortcut(QString name,QString param);
extern bool soundEnabled;
extern bool hiRes;
extern QList<CPObject *> listpPObject;
extern DownloadManager *downloadManager;

using namespace zxing;

CrenderView::CrenderView(QWidget *parent):cloud(this)
{
    this->parent = parent;

    parse.setHostURI(Cloud::getValueFor("serverURL",""));
    parse.setApplicationId("PockEmul");

//    this->setAttribute(Qt::WA_AcceptTouchEvents);

//    grabKeyboard();
    setFocusPolicy(Qt::ClickFocus);

    if (mainwindow->dialoganalogic==0) mainwindow->dialoganalogic = new dialogAnalog(11,this);

    qmlRegisterType<BinaryData>("HexEditor", 1, 0, "HexModel");
    qmlRegisterType<laserfilter>("LaserFilter", 2, 3, "laserfilter");
    QZXing::registerQMLTypes();

    engine()->addImageProvider(QLatin1String("Pocket"),new PocketImageProvider(this) );
    engine()->addImageProvider(QLatin1String("Logic"),mainwindow->dialoganalogic );
    engine()->addImageProvider(QLatin1String("PockEmulCloud"),cloud.imgprov );

    rootContext()->setContextProperty("Vibrator", &vibrator);
    rootContext()->setContextProperty("cloud", &cloud);
    rootContext()->setContextProperty("main", this);
    rootContext()->setContextProperty("logicObj", mainwindow->dialoganalogic);
    rootContext()->setContextProperty("parse", &parse);

    setSource(QUrl("qrc:/Test.qml"));
    setResizeMode(QQuickWidget::SizeRootObjectToView);
//    connect(engine(), SIGNAL(quit()), this,SLOT(hide()));
    cloud.object = (QObject*) rootObject();
    parse.object = (QObject*) rootObject();

    QObject::connect(cloud.object, SIGNAL(toggleFullscreen()), mainwindow, SLOT(toggleFullscreen()));
    QObject::connect(cloud.object, SIGNAL(analyser()), mainwindow, SLOT(Analogic()));

    QObject::connect(cloud.object, SIGNAL(sendNewPocket()), this, SLOT(newpocketSlot()));
    QObject::connect(cloud.object, SIGNAL(sendNewExt()), this, SLOT(newextSlot()));
    QObject::connect(cloud.object, SIGNAL(sendDev()),mainwindow,SLOT(IDE()));
    QObject::connect(cloud.object, SIGNAL(sendSave()), this, SLOT(saveSlot()));
    QObject::connect(cloud.object, SIGNAL(sendLoad()), this, SLOT(loadSlot()));
    QObject::connect(cloud.object, SIGNAL(sendBook()), this, SLOT(bookcaseSlot()));
    QObject::connect(cloud.object, SIGNAL(sendCloseAll()), mainwindow, SLOT(Close_All()));
    QObject::connect(cloud.object, SIGNAL(sendCheck()), this, SLOT(checkSlot()));
    QObject::connect(cloud.object, SIGNAL(sendExit()), mainwindow, SLOT(close()));

    QObject::connect(cloud.object, SIGNAL(sendDownloadCancel()), downloadManager, SLOT(abort()));

    QObject::connect(&cloud,SIGNAL(downloadEnd()),this,SLOT(cloudClose()));
    connect(mainwindow,SIGNAL(NewPObjectsSignal(CPObject*)),this,SLOT(newPObject(CPObject*)));
    connect(mainwindow,SIGNAL(DestroySignal(CPObject *)),this,SLOT(delPObject(CPObject*)));

//    connect(mainwindow->dialoganalogic,SIGNAL(refreshLogic()),this,SLOT(refreshLogic()));
}
void CrenderView::loadPocket(QString id) {
    CPObject *_pc=0;
    int _result = 0;

    if (mainwindow->objtable.contains(id))
        _result = mainwindow->objtable.value(id);

    if (_result != 0)	{
        _pc=mainwindow->LoadPocket(_result);
    }

    emit Launched(id,_pc);
}

extern LaunchButtonWidget* launch1;
void CrenderView::newpocketSlot()
{
    qWarning()<<"FO"<<launch1;
    launch1->mousePressEvent( new QMouseEvent(QEvent::MouseButtonPress, QPoint(0,0), Qt::LeftButton, Qt::LeftButton,Qt::NoModifier));
}

extern LaunchButtonWidget* launch2;
void CrenderView::newextSlot()
{
    qWarning()<<"FO"<<launch2;
    launch2->mousePressEvent( new QMouseEvent(QEvent::MouseButtonPress, QPoint(0,0), Qt::LeftButton, Qt::LeftButton,Qt::NoModifier));
}

extern LaunchButtonWidget* load;
extern QString workDir;
void CrenderView::loadSlot()
{
    if (load==0) {
        load = new LaunchButtonWidget(mainwindow->centralwidget,
                                                          LaunchButtonWidget::FileBrowser,
                                                          QStringList()<<(workDir+"sessions")<<"*.pml",
                                                          ":/core/load.png");
    }
    load->mousePressEvent( new QMouseEvent(QEvent::MouseButtonPress, QPoint(0,0), Qt::LeftButton, Qt::LeftButton,Qt::NoModifier));
}

void CrenderView::saveSlot()
{
    mainwindow->saveassession();
}

extern LaunchButtonWidget* bookcase;
void CrenderView::bookcaseSlot()
{
    if (bookcase==0) {
        bookcase = new LaunchButtonWidget(mainwindow->centralwidget,
                                                          LaunchButtonWidget::FileBrowser,
                                                              QStringList()<< (workDir+"documents")<<"*.pdf",
                                                          ":/core/bookcase.png");
    }
    bookcase->mousePressEvent( new QMouseEvent(QEvent::MouseButtonPress, QPoint(0,0), Qt::LeftButton, Qt::LeftButton,Qt::NoModifier));
}

void CrenderView::checkSlot()
{
    mainwindow->CheckUpdates();
}

void CrenderView::warning(QString msg) {
    ask(mainwindow, msg, 1);
}

void CrenderView::keypressed(QString Id, int k,int m,int scan)
{
    Q_UNUSED(scan)

    CPObject *pc = ((CPObject*)Id.toULongLong());
    qWarning()<<"CrenderView::keypressed:"<<k<<m<<(quint32)scan;
    // Send thee MouseButtonPress event
    QKeyEvent _e( QEvent::KeyPress, k,static_cast<Qt::KeyboardModifiers>(m));
//    QApplication::sendEvent(pc, &_e);
    pc->keyPressEvent(&_e);
}
void CrenderView::keyreleased(QString Id, int k,int m,int scan)
{
    Q_UNUSED(scan)

    CPObject *pc = ((CPObject*)Id.toULongLong());
//    qWarning()<<"key pressed:"<<k<<m<<(quint32)scan;
    // Send thee MouseButtonPress event
    QKeyEvent _e( QEvent::KeyRelease, k, static_cast<Qt::KeyboardModifiers>(m));
//    QApplication::sendEvent(pc, &_e);
    pc->keyReleaseEvent(&_e);

}
void CrenderView::movepocket(QString Id, int x, int y)
{
//    qWarning()<<"movepocket:"<<Id<<x<<y;
    CPObject *pc = ((CPObject*)Id.toULongLong());
    QPointF pts(x , y);

    pc->MoveWithLinkedAbs(pts);
 }

void CrenderView::moveallpocket(int x, int y)
{
//    qWarning()<<"moveallpocket:"<<x<<y;
    QPointF pts(x , y);
    mainwindow->MoveAll(pts);
}

void CrenderView::maximize(QString Id)
{
    qWarning()<<"maximize:"<<Id;
    CPObject *pc = ((CPObject*)Id.toULongLong());
    pc->maximize(pc->RectWithLinked().center().toPoint());
}

void CrenderView::minimize(QString Id)
{
    CPObject *pc = ((CPObject*)Id.toULongLong());
    pc->minimize(pc->RectWithLinked().center().toPoint());
}


void CrenderView::fit()
{

    if (listpPObject.isEmpty()) return;

    // Compute global rect
    QRectF _globalRect(0,0,0,0);
    for (int k = 0; k < listpPObject.size(); k++)
    {
        _globalRect = _globalRect.united(listpPObject.at(k)->rect());
    }

    float _cww = mainwindow->centralwidget->rect().width();
    float _cwh = mainwindow->centralwidget->rect().height();
    float _grw = _globalRect.width();
    float _grh = _globalRect.height();
    float rw= _cww/_grw;
    float rh= _cwh/_grh;
    float r=0;
    if ((rw>1) && (rh>1)) r = MIN(rw,rh);
    else if ((rw>1) && (rh<=1)) r = rh;
    else if ((rw<=1) && (rh>1)) r = rw;
    else if ((rw<=1) && (rh<=1)) r = MAX(rw,rh);

    mainwindow->doZoom(_globalRect.topLeft().toPoint(),r);
    //move to upper left
    // Fetch all_object and move them
    _globalRect.setCoords(0,0,0,0);
    for (int k = 0; k < listpPObject.size(); k++)
    {
        _globalRect = _globalRect.united(listpPObject.at(k)->rect());
    }
    mainwindow->MoveAll(- _globalRect.topLeft());
}

void CrenderView::disableKeyboard(QString Id) {
    CPObject *pc = ((CPObject*)Id.toULongLong());
    pc->pKEYB->enabled = false;
}
void CrenderView::enableKeyboard(QString Id) {
    CPObject *pc = ((CPObject*)Id.toULongLong());
    pc->pKEYB->enabled = true;
}

void CrenderView::contextMenu(QString Id, int x, int y)
{
//    qWarning()<<"contextMenu"<<x<<y;
    CPObject *pc = ((CPObject*)Id.toULongLong());
    QPoint pts(x , y);
    QContextMenuEvent cme(
                QContextMenuEvent::Mouse,
                pts,
                pc->mapToGlobal(pts));
    //contextMenuEvent(cme);
//    QApplication::sendEvent(pc,&cme);
    pc->contextMenuEvent(&cme);


}

void CrenderView::rotpocket(QString Id, int x)
{
    CPObject *pc = ((CPObject*)Id.toULongLong());
    pc->setRotation(x);
}

void CrenderView::click(QString Id, int touchId,int x, int y)
{

    lockClick.lock();

//    qWarning()<<"click:"<<Id<<x<<y;
    CPObject *pc = ((CPObject*)Id.toULongLong());
    QPoint pts(x , y);

    mapTouch[touchId] = QPoint(x,y);

    // Send thee MouseButtonPress event
    QMouseEvent event(QEvent::MouseButtonPress, pts, Qt::LeftButton, Qt::LeftButton,Qt::NoModifier);

//    QApplication::sendEvent(pc, &event);

    pc->mousePressEvent(&event);
    lockClick.unlock();

    return;
}

void CrenderView::unclick(QString Id, int touchId,int x, int y)
{
    Q_UNUSED(x)
    Q_UNUSED(y)
//    qWarning()<<"unclick:"<<Id<<x<<y;


//    QEventLoop eventLoop;
//    QTimer::singleShot (50, &eventLoop, SLOT (quit ()));
//    eventLoop.exec ();

    lockClick.lock();

    CPObject *pc = ((CPObject*)Id.toULongLong());
    //QPoint pts(x , y);

    QPoint pts = mapTouch[touchId];
    mapTouch.remove(touchId);
    // Send thee MouseButtonRelease event
    QMouseEvent event(QEvent::MouseButtonRelease, pts, Qt::LeftButton, Qt::LeftButton,Qt::NoModifier);
//    QApplication::sendEvent(pc, &event);

    pc->mouseReleaseEvent(&event);
    lockClick.unlock();

    return;

}

bool CrenderView::keyAt(QString Id, int x, int y)
{
//    qWarning()<<"click:"<<Id<<x<<y;
    CPObject *pc = ((CPObject*)Id.toULongLong());
    QPoint pts(x , y);

    if (pc->pKEYB) return pc->pKEYB->KeyClick(pts);

    return false;
}

void CrenderView::flip(QString Id, int dir)
{
    CPObject *pc = ((CPObject*)Id.toULongLong());
    Direction _d = static_cast<Direction>(dir);
    pc->flip(_d);
}

double CrenderView::getZoom()
{
    return mainwindow->zoom;
}

void CrenderView::test()
{
    QZXing decoder;
    try {
        qWarning()<<"throw";
        throw std::runtime_error("Gak!");
        qWarning()<<"throw2";
    }
    catch (const std::exception &e) {
        qWarning()<<"CATCHED:"<<e.what();
    }

    return;
    decoder.setDecoder( QZXing::DecoderFormat_QR_CODE );
    QImage img("C:/Users/Remy/Pictures/remy.jpg");
    QString res = decoder.decodeImage(img);
    qWarning()<<res;

}

QString CrenderView::getRes(QString _fn)
{
    QString _res = P_RES(_fn);

//    qWarning()<<_res;

    if (_res.startsWith(':')) {
        _res = "qrc" + _res;
    }
    else {
        _res = "file:///"+_res;
    }

    return _res;
}

void CrenderView::changeGeo(int x, int y, int w, int h)
{
    Q_UNUSED(x)
    Q_UNUSED(y)
    Q_UNUSED(w)
    Q_UNUSED(h)
//    if (mainwindow->windowide==0) mainwindow->windowide = new WindowIDE(this);
//    mainwindow->windowide->show();

//    if (mainwindow->windowide)
//        mainwindow->windowide->setGeometry(QRect(mapToGlobal(QPoint(x,y)),QSize(w,h)));
}

void CrenderView::sendTrackingEvent(const QString &cat,
                                    const QString &action,
                                    const QString &label,
                                    const QVariant &value)
{
    tracker->sendEvent(cat,action,label,value);
    tracker->startSending();
}

void CrenderView::fillSlotList(void)
{

    QMetaObject::invokeMethod(cloud.object, "clearSlotList");

    for (int i = 0; i < listpPObject.size(); i++)
    {
        CPObject *_p = listpPObject.at(i);
        for (int j = 0; j < _p->SlotList.count(); j++)
        {
            CSlot _s =_p->SlotList[j];
            QMetaObject::invokeMethod(cloud.object, "addSlot",
                                      Q_ARG(QVariant, QString("%1").arg((quint64)_p)),
                                      Q_ARG(QVariant, _p->getDisplayName()),
                                      Q_ARG(QVariant, _s.getLabel()),
                                      Q_ARG(QVariant, _s.getSize()),
                                      Q_ARG(QVariant, QString("0x%1").arg(_s.getAdr(),6,16,QChar('0'))),
                                      Q_ARG(QVariant, QString("0x%1").arg(_s.getAdr()+_s.getSize()*1024,6,16,QChar('0'))),
                                      Q_ARG(QVariant, j)
                                      );
        }
    }
}


void CrenderView::loadSlot(QString Id, int slotNumber, BinaryData *display)
{
    CPObject *pc = ((CPObject*)Id.toULongLong());
//    if (listpPObject.isEmpty()) return;
//    CPObject *pc = listpPObject[0];

    int adr = pc->SlotList[slotNumber].getAdr();
    int size = pc->SlotList[slotNumber].getSize() * 1024;
    QByteArray *ba= new QByteArray((const char*)&(pc->mem[adr]),size);
    display->load(*ba);
//    display->setAddress(QString("%1").arg(adr,10,16,QChar('0')));
//    display->setOffset(adr);
}

QString CrenderView::getReleaseNotes(QString _fn)
{
    QFile _file(_fn);
    _file.open( QIODevice::ReadOnly);
    QString _notes = _file.readAll();

    return _notes;
}

void CrenderView::dblclick(QString Id, int x, int y)
{

    CPObject *pc = ((CPObject*)Id.toULongLong());
    QPoint pts(x , y);

    // Send thee MouseButtonPress event
    QMouseEvent event(QEvent::MouseButtonDblClick, pts, Qt::LeftButton, Qt::LeftButton,Qt::NoModifier);
//    QApplication::sendEvent(pc, &event);

    pc->mouseDoubleClickEvent(&event);
    return;

}
void CrenderView::setzoom(int x,int y,double z)
{
    mainwindow->doZoom(QPoint(x,y),z);
}
void CrenderView::pocketUpdated(CViewObject * pObject)
{
//    update();
    QMetaObject::invokeMethod(cloud.object, "refreshPocket",
                              Q_ARG(QVariant, QString("%1").arg((quint64)pObject))
                              );
}

void CrenderView::newPObject(CPObject *pObject) {
    qWarning()<<"Add Pocket"<<pObject->getName()<<
                pObject->pos().x()<<
                pObject->pos().y()<<
                pObject->size();

    connect (pObject,SIGNAL(movePObject(CViewObject*,QPointF)),this,SLOT(movePObject(CViewObject*,QPointF)));
    connect (pObject,SIGNAL(sizePObject(CViewObject*,QSizeF)),this,SLOT(sizePObject(CViewObject*,QSizeF)));
    connect (pObject,SIGNAL(stackPosChanged()),this,SLOT(stackPosChanged()));
    connect( pObject,SIGNAL(updatedPObject(CViewObject*)),this,SLOT(pocketUpdated(CViewObject*)));
    connect( pObject,SIGNAL(rotPObject(CViewObject*,int)),this,SLOT(rotPObject(CViewObject*,int)));

    qWarning()<<pObject;
    QString _a = QString("%1").arg((quint64)pObject);

    qWarning()<<_a;

    QMetaObject::invokeMethod(cloud.object, "addPocket",
                              Q_ARG(QVariant, pObject->getDisplayName()),
                              Q_ARG(QVariant, "qrc"+pObject->BackGroundFname),
                              Q_ARG(QVariant, QString("%1").arg((quint64)pObject)),
                              Q_ARG(QVariant, pObject->pos().x()),
                              Q_ARG(QVariant, pObject->pos().y()),
                              Q_ARG(QVariant, pObject->width()),
                              Q_ARG(QVariant, pObject->height()),
                              Q_ARG(QVariant, pObject->getRotation())
                              );

    fillSlotList();
}

void CrenderView::delPObject(CPObject *pObject)
{
//    qWarning()<<"delPObject"<<pObject;
    QMetaObject::invokeMethod(cloud.object, "delPocket",
                              Q_ARG(QVariant, QString("%1").arg((quint64)pObject))
                              );
    fillSlotList();
}

void CrenderView::movePObject(CViewObject *pObject, QPointF pos)
{
//    qWarning()<<"movePocket:"<<pos;
    QMetaObject::invokeMethod(cloud.object, "movePocket",
                              Q_ARG(QVariant, QString("%1").arg((quint64)pObject)),
                              Q_ARG(QVariant, pos.x()),
                              Q_ARG(QVariant, pos.y())
                              );
}
void CrenderView::sizePObject(CViewObject *pObject, QSizeF size)
{
//    qWarning()<<"sizePObject:"<<size;
    QMetaObject::invokeMethod(cloud.object, "sizePocket",
                              Q_ARG(QVariant, QString("%1").arg((quint64)pObject)),
                              Q_ARG(QVariant, size.width()),
                              Q_ARG(QVariant, size.height())
                              );
}
void CrenderView::hidePObject(CViewObject *pObject)
{
//    qWarning()<<"sizePObject:"<<size;
    QMetaObject::invokeMethod(cloud.object, "hidePocket",
                              Q_ARG(QVariant, QString("%1").arg((quint64)pObject))
                              );
}
void CrenderView::showPObject(CViewObject *pObject)
{
//    qWarning()<<"sizePObject:"<<size;
    QMetaObject::invokeMethod(cloud.object, "showPocket",
                              Q_ARG(QVariant, QString("%1").arg((quint64)pObject))
                              );
}

void CrenderView::pickExtension(QString brand)
{
//    qWarning()<<"sizePObject:"<<size;
    QMetaObject::invokeMethod(cloud.object, "pickExtension",
                              Q_ARG(QVariant, brand)
                              );
}
void CrenderView::pickExtensionConnector(QString brand)
{
//    qWarning()<<"sizePObject:"<<size;
    QMetaObject::invokeMethod(cloud.object, "pickExtensionConnector",
                              Q_ARG(QVariant, brand)
                              );
}
void CrenderView::cloudClose()
{
//    qWarning()<<"sizePObject:"<<size;
    QMetaObject::invokeMethod(cloud.object, "cloudHide");
}

void CrenderView::rotPObject(CViewObject *pObject,int angle)
{
    QMetaObject::invokeMethod(cloud.object, "setRotPocket",
                              Q_ARG(QVariant, QString("%1").arg((quint64)pObject)),
                              Q_ARG(QVariant,angle)
                              );
}

void CrenderView::stackPosChanged()
{
    // fetch all main windows children
    // set zorder
    QObjectList list = mainwindow->centralWidget()->children();

    for (int i=0; i<list.count();i++) {
        CPObject * pobj = (CPObject*)(list.at(i));
        QMetaObject::invokeMethod(cloud.object, "orderPocket",
                                  Q_ARG(QVariant, QString("%1").arg((quint64)pobj)),
                                  Q_ARG(QVariant, i)
                                  );
    }
}


