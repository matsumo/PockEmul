
#include <QQmlContext>

#include "renderView.h"
#include "cloud/cloudimageprovider.h"
#include "mainwindowpockemul.h"
#include "pobject.h"

extern MainWindowPockemul *mainwindow;
extern int ask(QWidget *parent, QString msg, int nbButton);
//extern void m_addShortcut(QString name,QString param);
//extern bool soundEnabled;
//extern bool hiRes;
extern QList<CPObject *> listpPObject;

CrenderView::CrenderView(QWidget *parent)
{
    this->parent = parent;
    engine()->addImageProvider(QLatin1String("Pocket"),new PocketImageProvider(this) );
    rootContext()->setContextProperty("render", this);
    setSource(QUrl("qrc:/Test.qml"));
    setResizeMode(QQuickWidget::SizeRootObjectToView);//QQuickWidget::SizeRootObjectToView);
    connect(engine(), SIGNAL(quit()), this,SLOT(hide()));
    object = (QObject*) rootObject();

    QObject::connect(object, SIGNAL(sendWarning(QString)), this, SLOT(warning(QString)));
    QObject::connect(object, SIGNAL(sendKeyPressed(QString,int,int,int)), this, SLOT(keypressed(QString,int,int,int)));
    QObject::connect(object, SIGNAL(sendKeyReleased(QString,int,int,int)), this, SLOT(keyreleased(QString,int,int,int)));
    QObject::connect(object, SIGNAL(sendContextMenu(QString,int,int)), this, SLOT(contextMenu(QString,int,int)));
    QObject::connect(object, SIGNAL(sendClick(QString,int,int)), this, SLOT(click(QString,int,int)));
    QObject::connect(object, SIGNAL(sendUnClick(QString,int,int)), this, SLOT(unclick(QString,int,int)));
    QObject::connect(object, SIGNAL(sendMovePocket(QString,int,int)), this, SLOT(movepocket(QString,int,int)));
    QObject::connect(object, SIGNAL(sendMoveAllPocket(int,int)), this, SLOT(moveallpocket(int,int)));
    QObject::connect(object, SIGNAL(setZoom(int,int,int)), this, SLOT(setzoom(int,int,int)));

    connect(mainwindow,SIGNAL(NewPObjectsSignal(CPObject*)),this,SLOT(newPObject(CPObject*)));
    connect(mainwindow,SIGNAL(DestroySignal(CPObject *)),this,SLOT(delPObject(CPObject*)));

}

void CrenderView::warning(QString msg) {
    ask(mainwindow, msg, 1);
}

void CrenderView::keypressed(QString Id, int k,int m,int scan)
{
    CPObject *pc = ((CPObject*)Id.toULongLong());
    qWarning()<<"key pressed:"<<k<<m<<(quint32)scan;
    // Send thee MouseButtonPress event
    QKeyEvent *e=new QKeyEvent( QEvent::KeyPress, k,static_cast<Qt::KeyboardModifiers>(m));
    QApplication::sendEvent(pc, e);
    delete e;
}
void CrenderView::keyreleased(QString Id, int k,int m,int scan)
{
    CPObject *pc = ((CPObject*)Id.toULongLong());
    qWarning()<<"key pressed:"<<k<<m<<(quint32)scan;
    // Send thee MouseButtonPress event
    QKeyEvent *e=new QKeyEvent( QEvent::KeyRelease, k, static_cast<Qt::KeyboardModifiers>(m));
    QApplication::sendEvent(pc, e);
    delete e;
}
void CrenderView::movepocket(QString Id, int x, int y)
{
//    qWarning()<<"movepocket:"<<Id<<x<<y;
    CPObject *pc = ((CPObject*)Id.toULongLong());
    QPoint pts(x , y);
    pc->MoveWithLinkedAbs(pts);
 }

void CrenderView::moveallpocket(int x, int y)
{
//    qWarning()<<"moveallpocket:"<<x<<y;
    QPoint pts(x , y);
    mainwindow->MoveAll(pts);
}

void CrenderView::contextMenu(QString Id, int x, int y)
{
    qWarning()<<"contextMenu"<<x<<y;
    CPObject *pc = ((CPObject*)Id.toULongLong());
    QPoint pts(x , y);
    QContextMenuEvent *cme = new QContextMenuEvent(
                QContextMenuEvent::Mouse,
                pts,
                pc->mapToGlobal(pts));
    //contextMenuEvent(cme);
    QApplication::sendEvent(pc,cme);
    delete cme;

}

void CrenderView::click(QString Id, int x, int y)
{
    qWarning()<<"click:"<<Id<<x<<y;
    CPObject *pc = ((CPObject*)Id.toULongLong());
    QPoint pts(x , y);
//    if ((pc->pKEYB) &&(pc->pKEYB->KeyClick(pts)))
    {
        // Send thee MouseButtonPress event
        QMouseEvent *e=new QMouseEvent(QEvent::MouseButtonPress, pts, Qt::LeftButton, Qt::LeftButton,Qt::NoModifier);
        QApplication::sendEvent(pc, e);
        delete e;
        return;
    }
}
void CrenderView::unclick(QString Id, int x, int y)
{
    qWarning()<<"unclick:"<<Id<<x<<y;
    CPObject *pc = ((CPObject*)Id.toULongLong());
    QPoint pts(x , y);


    // Send thee MouseButtonRelease event
    QMouseEvent *e=new QMouseEvent(QEvent::MouseButtonRelease, pts, Qt::LeftButton, Qt::LeftButton,Qt::NoModifier);
    QApplication::sendEvent(pc, e);
    delete e;
    return;

}

void CrenderView::setzoom(int x,int y,int z)
{
    mainwindow->doZoom(QPoint(x,y),z);
}
void CrenderView::pocketUpdated(CPObject * pObject)
{

    QMetaObject::invokeMethod(object, "refreshPocket",
                              Q_ARG(QVariant, QString("%1").arg((long)pObject))
                              );
}

void CrenderView::newPObject(CPObject *pObject) {
//    qWarning()<<"Add Pocket"<<pObject->getName()<<
//                pObject->pos().x()<<
//                pObject->pos().y()<<
//                pObject->size();

    connect (pObject,SIGNAL(movePObject(CViewObject*,QPoint)),this,SLOT(movePObject(CViewObject*,QPoint)));
    connect (pObject,SIGNAL(sizePObject(CViewObject*,QSize)),this,SLOT(sizePObject(CViewObject*,QSize)));
    connect (pObject,SIGNAL(stackPosChanged()),this,SLOT(stackPosChanged()));
    connect( pObject,SIGNAL(updatedPObject(CPObject*)),this,SLOT(pocketUpdated(CPObject*)));

    QMetaObject::invokeMethod(object, "addPocket",
                              Q_ARG(QVariant, QString("name")),
                              Q_ARG(QVariant, "qrc"+pObject->BackGroundFname),
                              Q_ARG(QVariant, QString("%1").arg((long)pObject)),
                              Q_ARG(QVariant, pObject->pos().x()),
                              Q_ARG(QVariant, pObject->pos().y()),
                              Q_ARG(QVariant, pObject->width()),
                              Q_ARG(QVariant, pObject->height())
                              );
}

void CrenderView::delPObject(CPObject *pObject)
{
//    qWarning()<<"delPObject"<<pObject;
    QMetaObject::invokeMethod(object, "delPocket",
                              Q_ARG(QVariant, QString("%1").arg((long)pObject))
                              );
}

void CrenderView::movePObject(CViewObject *pObject, QPoint pos)
{
//    qWarning()<<"movePocket:"<<pos;
    QMetaObject::invokeMethod(object, "movePocket",
                              Q_ARG(QVariant, QString("%1").arg((long)pObject)),
                              Q_ARG(QVariant, pos.x()),
                              Q_ARG(QVariant, pos.y())
                              );
}
void CrenderView::sizePObject(CViewObject *pObject, QSize size)
{
//    qWarning()<<"sizePObject:"<<size;
    QMetaObject::invokeMethod(object, "sizePocket",
                              Q_ARG(QVariant, QString("%1").arg((long)pObject)),
                              Q_ARG(QVariant, size.width()),
                              Q_ARG(QVariant, size.height())
                              );
}

void CrenderView::stackPosChanged()
{
    // fetch all main windows children
    // set zorder
    QObjectList list = mainwindow->centralWidget()->children();

    for (int i=0; i<list.count();i++) {
        CPObject * pobj = (CPObject*)(list.at(i));
        QMetaObject::invokeMethod(object, "orderPocket",
                                  Q_ARG(QVariant, QString("%1").arg((long)pobj)),
                                  Q_ARG(QVariant, i)
                                  );
    }
}
