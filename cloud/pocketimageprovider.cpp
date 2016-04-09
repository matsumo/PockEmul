#include <QObject>
#include <QDebug>

#include "pocketimageprovider.h"
#include "cloudwindow.h"
#include "pobject.h"
#include "mainwindowpockemul.h"

extern QString workDir;
extern MainWindowPockemul *mainwindow;



PocketImageProvider::PocketImageProvider(QObject *parent) : QObject(parent),
    QQuickImageProvider(QQuickImageProvider::Image)
{
    // This space intentionally left blank.
}

PocketImageProvider::~PocketImageProvider()
{
    // This space intentionally left blank.
}
QImage PocketImageProvider::requestImage(const QString& id, QSize* size, const QSize& requestedSize)
{
    Q_UNUSED(size)
    Q_UNUSED(requestedSize)

    QStringList _id = id.split('/');

//qWarning()<<"PocketImageProvider::requestImage:"<<id<<_id<<_id[1];
    if (id.isEmpty()) return QImage();
    CPObject *pc = (CPObject*)_id[0].toULongLong();

    if ( (pc->flipping | pc->closed) && pc->AnimatedImage){
//        qWarning()<<"image prov:"<<mainwindow->rawclk;
       return *(pc->AnimatedImage);
    }

    View _v = pc->currentView;

    if (_id[1]=="LEFT")     _v = LEFTview;
    if (_id[1]=="RIGHT")    _v = RIGHTview;
    if (_id[1]=="TOP")      _v = TOPview;
    if (_id[1]=="BOTTOM")   _v = BOTTOMview;
    if (_id[1]=="BACK")     _v = BACKview;
    if (_id[1]=="FRONT")    _v = FRONTview;

//qWarning()<<"image prov:"<<mainwindow->rawclk;
    return *(pc->getViewImage(_v));
}
