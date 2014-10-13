// FIXME: test with no config.ini. Seems to crash

#include <QApplication>
#if QT_VERSION >= 0x050000
// Qt5 code
#   include <QtWidgets>
#else
// Qt4 code
#   include <QtCore>
#   include <QFileDialog>
#   include <QGraphicsObject>
#   include <QtGui>
#endif
#include <QDebug>
#include <QFrame>
#include <QLabel>
#include <QListView>
#include <QMenu>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QPushButton>
#include <QTimer>
#include <QVBoxLayout>
#include <QUrl>
#include <QSettings>
#include <QCryptographicHash>


//#include <QtDeclarative/QDeclarativeView>
//#include <QtDeclarative/QDeclarativeContext>
//#include <QtDeclarative/QDeclarativeEngine>

#include <QQmlEngine>
#include <QQmlContext>
#include <QQuickView>
#include <QtQuickWidgets/QQuickWidget>
#include <QKeyEvent>


#include "applicationconfig.h"
#include "cloudwindow.h"
#include "cloudimageprovider.h"

#include "mainwindowpockemul.h"
#include "pobject.h"
#include "Keyb.h"

extern MainWindowPockemul *mainwindow;
extern int ask(QWidget *parent, QString msg, int nbButton);
extern void m_addShortcut(QString name,QString param);
extern bool soundEnabled;
extern bool hiRes;
extern QList<CPObject *> listpPObject;

CloudWindow::CloudWindow(QWidget *parent)
    : QWidget(parent)
{
    imgprov = new CloudImageProvider(this);
    if (getValueFor("serverURL","")=="") saveValueFor("serverURL","http://pockemul.dscloud.me/elgg/");


    view = new QQuickView;//QQuickWidget;//QDeclarativeView(this);
    view->engine()->addImageProvider(QLatin1String("PockEmulCloud"),imgprov );
    view->engine()->addImageProvider(QLatin1String("Pocket"),new PocketImageProvider(this) );
    view->rootContext()->setContextProperty("cloud", this);
    view->setSource(QUrl("qrc:/main.qml"));
    view->setResizeMode(QQuickView::SizeRootObjectToView);//QQuickWidget::SizeRootObjectToView);
    connect(view->engine(), SIGNAL(quit()), this,SLOT(hide()));
    object = (QObject*) view->rootObject();

    QObject::connect(object, SIGNAL(sendWarning(QString)), this, SLOT(warning(QString)));
    QObject::connect(object, SIGNAL(sendKeyPressed(QString,QVariant)), this, SLOT(keypressed(QString,QVariant)));
    QObject::connect(object, SIGNAL(sendClick(QString,int,int)), this, SLOT(click(QString,int,int)));
    QObject::connect(object, SIGNAL(sendUnClick(QString,int,int)), this, SLOT(unclick(QString,int,int)));
    QObject::connect(object, SIGNAL(sendMovePocket(QString,int,int)), this, SLOT(movepocket(QString,int,int)));
    QObject::connect(object, SIGNAL(sendMoveAllPocket(int,int)), this, SLOT(moveallpocket(int,int)));
    QObject::connect(object, SIGNAL(setZoom(int,int,int)), this, SLOT(setzoom(int,int,int)));

    m_fileDialog = new QFileDialog(this);
    m_fileDialog->setFileMode(QFileDialog::ExistingFile);
    m_fileDialog->setNameFilter("PockEmul Session files (*.pml)");
    connect(m_fileDialog, SIGNAL(fileSelected(QString)),
            this, SLOT(sendPML(QString)));

    QVBoxLayout *windowLayout = new QVBoxLayout(this);
    QWidget *container = QWidget::createWindowContainer(view,this);
//    container->setMinimumSize(...);
//    container->setMaximumSize(...);
//    container->setFocusPolicy(Qt::TabFocus);

    windowLayout->addWidget(container);
//    windowLayout->addWidget(view);
    windowLayout->setMargin(0);


    connect(parent,SIGNAL(NewPObjectsSignal(CPObject*)),this,SLOT(newPObject(CPObject*)));
    connect(parent,SIGNAL(DestroySignal(CPObject *)),this,SLOT(delPObject(CPObject*)));
}

void CloudWindow::resizeEvent(QResizeEvent *e) {
    Q_UNUSED(e)

//    view->setGeometry(this->geometry());
}

QSize CloudWindow::sizeHint() const
{
    return QSize(500, 700);
}

int CloudWindow::askDialog(QString msg, int nbButton)
{
    return ask(this, msg, nbButton);
}

void CloudWindow::wheelEvent(QWheelEvent *event)
{
    event->accept();
}

void CloudWindow::refresh()
{

}


QString CloudWindow::save()
{
    hide();
    mainwindow->repaint();

    QString s = mainwindow->saveassessionString().remove(0,1);
//    qWarning()<<"session saved";
//    qWarning()<<s.left(500);
//    qWarning()<<"session saved";
    show();
    return s;

//    sendPML(filePath);
//    show();
//    refresh();
}






void CloudWindow::sendPML(const QString &filePath)
{
//    qWarning()<<"sendPML";
    if (filePath.isEmpty())
        return;

    QFile file(filePath);

    file.open(QIODevice::ReadOnly);
    QByteArray data = file.readAll();
    file.close();
    QNetworkAccessManager *mgr = new QNetworkAccessManager();
    // the HTTP request

    QString apikey = getValueFor("apikey","");
    QString server = getValueFor("serverURL","")+"savePML";
    // Check if apikey exists

#if QT_VERSION >= 0x050000
// Qt5 code
    QUrlQuery qu;
    qu.addQueryItem("apikey",apikey);
    qu.addQueryItem("content",QString(data).replace("+","%2B"));
//    qWarning()<<qu.query(QUrl::FullyEncoded).toUtf8();
    // some parameters for the HTTP request

    QNetworkRequest req(server);
//    qWarning()<<req.url();
    connect(mgr,SIGNAL(finished(QNetworkReply*)),this,SLOT(finishedSave(QNetworkReply*)));
    QNetworkReply *reply = mgr->post(req, qu.query(QUrl::FullyEncoded).toUtf8());
    Q_UNUSED(reply)
#else
// Qt4 code
#endif
}

void CloudWindow::finishedSave(QNetworkReply *reply)
{

    QByteArray xmlData = reply->readAll();

//    qWarning()<<"received id:"<<xmlData;

    QString pmlid,username,ispublic,isdeleted,title,description,objects,listobjects;

    QXmlStreamReader *xml = new QXmlStreamReader(xmlData);
    if (xml->readNextStartElement()) {
        if (xml->name() == "item") {
            while (!xml->atEnd()) {
                while (xml->readNextStartElement()) {
                    QString eltname = xml->name().toString();
                    if (eltname == "pmlid") pmlid = xml->readElementText();
                    if (eltname == "username") username = xml->readElementText();
                    if (eltname == "ispublic") ispublic = xml->readElementText();
                    if (eltname == "isdeleted") isdeleted = xml->readElementText();
                    if (eltname == "title") title = xml->readElementText();
                    if (eltname == "description") description = xml->readElementText();
                    if (eltname == "objects") objects = xml->readElementText();
                    if (eltname == "listobjects") listobjects = xml->readElementText();
                }
            }
        }
    }
//    qWarning()<<pmlid<<username<<ispublic<<isdeleted<<title<<description<<objects<<listobjects;

    QMetaObject::invokeMethod(object, "addRefPmlModel",
                              Q_ARG(QVariant, pmlid),
                              Q_ARG(QVariant, username),
                              Q_ARG(QVariant, objects),
                              Q_ARG(QVariant, listobjects),
                              Q_ARG(QVariant, ispublic),
                              Q_ARG(QVariant, isdeleted),
                              Q_ARG(QVariant, title),
                              Q_ARG(QVariant, description)
                              );


    reply->deleteLater();
}


void CloudWindow::downloadFinished()
{
//    qWarning()<<"CloudWindow::downloadFinished - ";
    QByteArray xmlData = m_reply->readAll();
//    qWarning() << "data="<<xmlData.left(200);
    QXmlStreamReader *xml = new QXmlStreamReader(xmlData);


    mainwindow->opensession(xml);
//    emit imageChanged(m_object.value("id").toString());

    m_reply->deleteLater();
    this->hide();

}

void CloudWindow::downloadFinished2()
{
//    qWarning()<<"CloudWindow::downloadFinished - ";
    QByteArray xmlData = m_reply->readAll();
//    qWarning() << "data="<<xmlData.left(200);
    QXmlStreamReader *xml = new QXmlStreamReader(xmlData);

    if (xml->readNextStartElement() && (xml->name() == "elgg")) {
        if (xml->readNextStartElement() &&
                (xml->name() == "status") &&
                (xml->readElementText().toInt()==0)) {
            if (xml->readNextStartElement() &&
                    (xml->name() == "result")) {
                QString pmlData = xml->readElementText();
                qWarning() << "data="<<pmlData.left(200);
                QXmlStreamReader *pmlxml = new QXmlStreamReader(pmlData);
                mainwindow->opensession(pmlxml);
            }
        }
    }

    m_reply->deleteLater();
    this->hide();

}


void CloudWindow::showFileDialog()
{
    m_fileDialog->show();
}

void CloudWindow::getPML(int id,int version,QString auth_token) {
    QNetworkAccessManager *mgr = new QNetworkAccessManager();
    QString url;
    if (version==0) {
        QString apikey = getValueFor("apikey","");
        QString server = getValueFor("serverURL","http://pockemul.dscloud.me/pocketcloud/")+"getPML";
        url = server+QString("/%1/%2").arg(getValueFor("apikey","0")).arg(id);
    }
    else {
        url = getValueFor("serverURL","http://pockemul.dscloud.me/elgg/")+
                QString("services/api/rest/xml/?method=file.get_pml")+
                QString("&file_guid=%1").arg(id)+
                "&api_key=7118206e08fed2c5ec8c0f2db61bbbdc09ab2dfa"+
                "&auth_token="+auth_token;
        qWarning()<<url;
    }
    QNetworkRequest req(url);

//    qWarning()<<req.url();
    m_reply = mgr->get(req);
    if (version==0)
        connect(m_reply, SIGNAL(finished()), this, SLOT(downloadFinished()));
    else
        connect(m_reply, SIGNAL(finished()), this, SLOT(downloadFinished2()));
}

extern QString workDir;

QString CloudWindow::getValueFor(const QString &objectName, const QString &defaultValue)
{
    QSettings settings(workDir+"config.ini",QSettings::IniFormat);
    if (settings.value(objectName).isNull()) {
//        qWarning()<<"getValue("<<objectName<<","<<defaultValue<<")";
        return defaultValue;
    }
//    qWarning()<<"getValue("<<objectName<<","<<settings.value(objectName).toString()<<") in "<<workDir+"config.ini";
    return settings.value(objectName).toString();
}

void CloudWindow::saveValueFor(const QString &objectName, const QString &inputValue)
{
    QSettings settings(workDir+"config.ini",QSettings::IniFormat);
    settings.setValue(objectName, QVariant(inputValue));
//    qWarning()<<"saveValue("<<objectName<<","<<inputValue<<") in "<<workDir+"config.ini";

    if (objectName == "soundEnabled") soundEnabled =  (inputValue=="on") ? true : false;
    if (objectName == "hiRes") hiRes =  (inputValue=="on") ? true : false;
}

QByteArray CloudWindow::generateKey(QString username,QString password) {
    QString key = QString("PockEmul"+username+"A"+password+"TRFGHUIJ");
//    qWarning()<<"KEY:"<<key;
    return QCryptographicHash::hash ( key.toUtf8(), QCryptographicHash::Sha1).toBase64();
}

void CloudWindow::saveCache(QString fileName, QString xml)
{
    QFile file(workDir+fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
             return;
    file.write(xml.toUtf8());
    file.close();
}

QString CloudWindow::loadCache(QString fileName)
{
    QFile file(workDir+fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return"";
    QString xml(file.readAll());
    file.close();
    return xml;
}


void CloudWindow::warning(QString msg) {
    ask(this, msg, 1);
}

void CloudWindow::keypressed(QString Id, QVariant v)
{
    CPObject *pc = ((CPObject*)Id.toULongLong());
    qWarning()<<"key pressed:"<<v;
}

void CloudWindow::movepocket(QString Id, int x, int y)
{
//    qWarning()<<"movepocket:"<<Id<<x<<y;
    CPObject *pc = ((CPObject*)Id.toULongLong());
    QPoint pts(x , y);
    pc->MoveWithLinkedAbs(pts);
 }

void CloudWindow::moveallpocket(int x, int y)
{
//    qWarning()<<"moveallpocket:"<<x<<y;
    QPoint pts(x , y);
    mainwindow->MoveAll(pts);
}

void CloudWindow::click(QString Id, int x, int y)
{
//    qWarning()<<"click:"<<Id<<x<<y;
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
void CloudWindow::unclick(QString Id, int x, int y)
{
//    qWarning()<<"unclick:"<<Id<<x<<y;
    CPObject *pc = ((CPObject*)Id.toULongLong());
    QPoint pts(x , y);


    // Send thee MouseButtonRelease event
    QMouseEvent *e=new QMouseEvent(QEvent::MouseButtonRelease, pts, Qt::LeftButton, Qt::LeftButton,Qt::NoModifier);
    QApplication::sendEvent(pc, e);
    delete e;
    return;

}

void CloudWindow::setzoom(int x,int y,int z)
{
    mainwindow->doZoom(QPoint(x,y),z);
}
void CloudWindow::pocketUpdated(CPObject * pObject)
{

    QMetaObject::invokeMethod(object, "refreshPocket",
                              Q_ARG(QVariant, QString("%1").arg((long)pObject))
                              );
}

void CloudWindow::newPObject(CPObject *pObject) {
    qWarning()<<"Add Pocket"<<pObject->getName()<<
                pObject->pos().x()<<
                pObject->pos().y()<<
                pObject->size();

    connect (pObject,SIGNAL(movePObject(CViewObject*,QPoint)),this,SLOT(movePObject(CViewObject*,QPoint)));
    connect (pObject,SIGNAL(sizePObject(CViewObject*,QSize)),this,SLOT(sizePObject(CViewObject*,QSize)));
    connect (pObject,SIGNAL(stackPosChanged()),this,SLOT(stackPosChanged()));

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

void CloudWindow::delPObject(CPObject *pObject)
{
    qWarning()<<"delPObject"<<pObject;
    QMetaObject::invokeMethod(object, "delPocket",
                              Q_ARG(QVariant, QString("%1").arg((long)pObject))
                              );
}

void CloudWindow::movePObject(CViewObject *pObject, QPoint pos)
{
    qWarning()<<"movePocket:"<<pos;
    QMetaObject::invokeMethod(object, "movePocket",
                              Q_ARG(QVariant, QString("%1").arg((long)pObject)),
                              Q_ARG(QVariant, pos.x()),
                              Q_ARG(QVariant, pos.y())
                              );
}
void CloudWindow::sizePObject(CViewObject *pObject, QSize size)
{
    qWarning()<<"sizePObject:"<<size;
    QMetaObject::invokeMethod(object, "sizePocket",
                              Q_ARG(QVariant, QString("%1").arg((long)pObject)),
                              Q_ARG(QVariant, size.width()),
                              Q_ARG(QVariant, size.height())
                              );
}

void CloudWindow::stackPosChanged()
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

void CloudWindow::addShortcut(QString param) {
    m_addShortcut("test",param);
}

void CloudWindow::clearCache(QString s)
{
    s.replace("image://pockemulcloud/","http://");
    qWarning()<<"CloudWindow::clearCache:"<<s;
    imgprov->clearCache(s);
}

bool CloudWindow::isPortraitOrientation() {
#if QT_VERSION >= 0x050000

    return  (QGuiApplication::primaryScreen()->orientation() == Qt::PortraitOrientation) ||
            (QGuiApplication::primaryScreen()->orientation() == Qt::InvertedPortraitOrientation) ||
             ( (QGuiApplication::primaryScreen()->orientation() == Qt::LandscapeOrientation ||
              QGuiApplication::primaryScreen()->orientation() == Qt::InvertedLandscapeOrientation) &&
             (this->height()>this->width())) ;

#endif
}
