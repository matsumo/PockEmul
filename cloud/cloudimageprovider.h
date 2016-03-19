#ifndef CLOUDIMAGEPROVIDER_H
#define CLOUDIMAGEPROVIDER_H


//#include <QtDeclarative/QDeclarativeImageProvider>
#include <QQuickImageProvider>
#include <QMap>
#include <QNetworkReply>
#include <QMutex>
#include <QCryptographicHash>

class CloudImageProvider : public QObject, public QQuickImageProvider //QDeclarativeImageProvider
{
Q_OBJECT

public:
    CloudImageProvider(QObject*);
    ~CloudImageProvider();
    QImage requestImage(const QString& id, QSize* size, const QSize& requestedSize);
    QPixmap requestPixmap(const QString& id, QSize* size, const QSize& requestedSize);

    QMap<QString,QImage> cache;
    QNetworkAccessManager * mgr;

    static QString toKey(QString s) {
        return  QString(QCryptographicHash::hash ( s.toUtf8(), QCryptographicHash::Md5).toBase64().toHex());
    }


    QMutex mutex;
    void clearCache(QString s);
signals:
    Q_INVOKABLE void cacheUpdated();

public slots:
    void loadfinished(QNetworkReply *reply);
};



#endif // CLOUDIMAGEPROVIDER_H
