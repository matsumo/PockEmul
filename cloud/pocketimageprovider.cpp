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

//qWarning()<<"PocketImageProvider::requestImage:"<<id<<(_id[0].toULongLong());
    if (id.isEmpty()) return QImage();


    CPObject *pc = (CPObject*)(_id[0].toULongLong());

    pc->paintingImage.lock();

    if ( (pc->flipping | pc->closed) && pc->AnimatedImage){
//        qWarning()<<"image prov:"<<mainwindow->rawclk;
       providedImage = *(pc->AnimatedImage);

       pc->paintingImage.unlock();
       return providedImage;
    }

    View _v = pc->currentView;

    if (_id[1]=="LEFT")     _v = LEFTview;
    if (_id[1]=="RIGHT")    _v = RIGHTview;
    if (_id[1]=="TOP")      _v = TOPview;
    if (_id[1]=="BOTTOM")   _v = BOTTOMview;
    if (_id[1]=="BACK")     _v = BACKview;
    if (_id[1]=="FRONT")    _v = FRONTview;

//qWarning()<<"image prov:"<<mainwindow->rawclk;
    QImage * _ret = pc->getViewImage(_v);
    if (_ret) {
        providedImage = *_ret;
    }
    else {
        providedImage = QImage();
    }

    pc->paintingImage.unlock();

    return providedImage;
}
