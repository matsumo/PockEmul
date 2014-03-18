#ifndef IMAGEOBJECT_H
#define IMAGEOBJECT_H

#include <QtCore>
#include <QtGui>

class QNetworkReply;
class EnginioClient;
class EnginioReply;
class ImageObject : public QObject
{
    Q_OBJECT
public:
    explicit ImageObject(EnginioClient *enginio);

    void setObject(const QJsonObject &object);

    QImage image() { return m_image; }
    QPixmap thumbnail();

signals:
    void imageChanged(const QString &id);

private slots:
    void replyFinished(EnginioReply *enginioReply);
    void downloadFinished();

private:
    QImage m_image;
    QPixmap m_thumbnail;
    QNetworkReply *m_reply;
    EnginioClient *m_enginio;
    QJsonObject m_object;
};

class PMLObject : public QObject
{
    Q_OBJECT
public:
    explicit PMLObject(EnginioClient *enginio);

    void setObject(const QJsonObject &object);

    QString fileid() { return m_fileid;}
    QString id() { return m_id;}
    QImage screenshot() { return m_screenshot; }
    QString description() { return m_description;}
    QString name() { return m_name;}
    int version() { return m_version;}
    QString xmldata() { return m_xmldata;}
    QPixmap thumbnail();

    void getFile(QString id);
    void updateFileId(QString id);
signals:
    void PMLChanged(const QString &id);

private slots:
    void replyFinished(EnginioReply *enginioReply);
    void downloadFinished();

private:
    QString m_fileid;
    QString m_id;
    QString m_xmldata;
    QString m_description;
    QString m_name;
    int m_version;
    QImage m_screenshot;
    QPixmap m_thumbnail;
    QNetworkReply *m_reply;
    EnginioClient *m_enginio;
    QJsonObject m_object;
};

#endif // IMAGEOBJECT_H
