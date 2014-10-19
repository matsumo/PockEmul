#include "cloudimageprovider.h"
#include <QImage>
#include <QPixmap>

#include <QObject>
#include <QDebug>
#include <QEventLoop>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QTimer>
#include <QApplication>
#include <QDir>
#include <QCryptographicHash>
#include <QXmlStreamReader>

#include "cloudwindow.h"
#include "pobject.h"

extern QString workDir;


CloudImageProvider::CloudImageProvider(QObject *parent) : QObject(parent),
    QQuickImageProvider(QQuickImageProvider::Image)
//    QDeclarativeImageProvider(QDeclarativeImageProvider::Image)
{
    // This space intentionally left blank.

    mgr = new QNetworkAccessManager;
    connect(mgr,SIGNAL(finished(QNetworkReply*)),this,SLOT(loadfinished(QNetworkReply*)));

    // load cache
    // fetch workdir+"/imgcache" directory
    QDir dir;
    dir.mkpath(workDir+"/imgcache/");
    dir.setPath(workDir+"/imgcache/");
    dir.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);
    dir.setSorting(QDir::Size | QDir::Reversed);

    QFileInfoList list = dir.entryInfoList();
    //std::cout << "     Bytes Filename" << std::endl;
    for (int i = 0; i < list.size(); ++i) {
        QFileInfo fileInfo = list.at(i);
        if (fileInfo.suffix() == "jpg") {
//            qWarning()<<"load ["<< fileInfo.baseName()<<"]="+fileInfo.absoluteFilePath();
            cache[fileInfo.baseName()] = QImage(fileInfo.absoluteFilePath());
        }

    }
}

CloudImageProvider::~CloudImageProvider()
{
    // This space intentionally left blank.
}

PocketImageProvider::PocketImageProvider(QObject *parent) : QObject(parent),
    QQuickImageProvider(QQuickImageProvider::Image)
//    QDeclarativeImageProvider(QDeclarativeImageProvider::Image)
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

//qWarning()<<"PocketImageProvider::requestImage:"<<_id;
    if (id.isEmpty()) return QImage();
    CPObject *pc = (CPObject*)_id[0].toULongLong();

//    if (pc->currentView!=FRONTview)
//    qWarning()<<"currentView="<<pc->currentView;

    if (pc->flipping | pc->closed)
        return *(pc->AnimatedImage);

    return *(pc->getViewImage(pc->currentView));

}

// CACHE MODULE
QImage CloudImageProvider::requestImage(const QString& id, QSize* size, const QSize& requestedSize)
{
    Q_UNUSED(size)
    Q_UNUSED(requestedSize)

//    qWarning()<<id<<"   auth_token="<<CloudWindow::getValueFor("auth_token")<<" size="<<requestedSize;
    QByteArray _ba = "api_key=7118206e08fed2c5ec8c0f2db61bbbdc09ab2dfa&auth_token=" +
            Cloud::getValueFor("auth_token").toUtf8();

    QString _id = id;
    _id.remove(QChar('#'));

    QNetworkRequest req("http://"+_id);
//    qWarning()<<req.url();
//    req.setHeader(QNetworkRequest::ContentTypeHeader,"application/x-www-form-urlencoded");
    QString key = toKey( req.url().toString());

    if (cache.contains(key))
        return cache[key];

qWarning()<<"YES!!1:"<<req.url()<<_ba;
    cache[key] = QImage();

    QNetworkAccessManager *_mgr = new QNetworkAccessManager;
    connect(_mgr,SIGNAL(finished(QNetworkReply*)),this,SLOT(loadfinished(QNetworkReply*)));

    QNetworkReply *_reply = _mgr->post(req, _ba);
    Q_UNUSED(_reply)
//    qWarning()<<_reply;
qWarning()<<"YES!!2";
    return QImage();
}


QPixmap CloudImageProvider::requestPixmap(const QString& id, QSize* size, const QSize& requestedSize)
{
    QString rsrcid = ":/" + id;
    QPixmap image(rsrcid);
    QPixmap result;

    if (requestedSize.isValid()) {
        result = image.scaled(requestedSize, Qt::KeepAspectRatio);
    } else {
        result = image;
    }
    *size = result.size();
    return result;
}

void CloudImageProvider::clearCache(QString s) {
    qWarning()<<"CloudImageProvider::clearCache:"<<s;
    QString key = toKey(s);
    cache.remove(key);

    QString fn =workDir+"imgcache/"+key+".jpg";
    qWarning()<<fn;
    QFile file(fn);
    if (file.remove()) qWarning()<<"removed";
}

void CloudImageProvider::loadfinished(QNetworkReply *reply)
{
    qWarning()<<"Downloas finished*******";

    QByteArray xmlData = reply->readAll();
//    qWarning() << "data="<<xmlData.left(200);
    QXmlStreamReader *xml = new QXmlStreamReader(xmlData);

    if (xml->readNextStartElement() && (xml->name() == "elgg")) {
        if (xml->readNextStartElement() &&
                (xml->name() == "status") &&
                (xml->readElementText().toInt()==0)) {
            if (xml->readNextStartElement() &&
                    (xml->name() == "result")) {
                QByteArray snapData = xml->readElementText().toLatin1();
//                qWarning() << "data="<<snapData.left(200);
                QImage image;
                image.loadFromData(QByteArray::fromBase64(snapData));

                QString key = toKey( reply->url().toString());
//                qWarning()<<key<<" <- "<<reply->url().toString();
                cache[key] = image;

                image.save(workDir+"imgcache/"+key+".jpg");
            }
        }
    }

    reply->manager()->deleteLater();
    reply->deleteLater();

    emit cacheUpdated();
}

