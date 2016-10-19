#ifndef BAAS_H
#define BAAS_H
#include <QObject>

#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>
#include <QJsonDocument>
#include <QJsonValue>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrl>
#include <QUrlQuery>
#include <QDebug>
#include <QBuffer>

class BaaS : public QObject
{
    Q_OBJECT
    Q_PROPERTY( QString hostURI READ getHostURI  WRITE setHostURI NOTIFY hostChanged)
    Q_PROPERTY( QString extraHostURI READ getExtraHostURI  WRITE setExtraHostURI NOTIFY hostChanged)
    Q_PROPERTY( QString endPoint READ getEndPoint WRITE setEndPoint NOTIFY endPointChanged)
    Q_PROPERTY( QString error READ getError WRITE setError NOTIFY errorChanged)
    Q_PROPERTY( qreal percComplete READ getPercComplete WRITE setPercComplete NOTIFY percCompleteChanged)
    Q_PROPERTY( uint httpCode READ getHttpCode WRITE setHttpCode NOTIFY httpCodeChanged)
    Q_PROPERTY( QString httpResponse READ getHttpResponse WRITE setHttpResponse NOTIFY httpResponseChanged)
    Q_PROPERTY( bool ready READ isReady NOTIFY readyChanged)


public:

    explicit BaaS();
    virtual ~BaaS();


    virtual QNetworkReply *query(QString endPoint, QUrlQuery extraParams)=0;
#undef GET
#undef POST
#undef PUT
#undef DELETE
#undef PATCH

    enum Ope{ GET, POST, PUT, DELETE, PATCH};
public: // property access
    QString getHostURI() const;
    void setHostURI(const QString& res);
    QString getExtraHostURI() const;
    void setExtraHostURI(const QString& res);
    QString getError() const;
    void setError(const QString& res);
    QString getEndPoint() const;
    void setEndPoint(const QString& res);
    qreal getPercComplete() const;
    void setPercComplete(qreal res);
    QString getHttpResponse() const;
    void setHttpResponse(const QString& res);
    uint getHttpCode() const;
    void setHttpCode(uint res);
    virtual bool isReady() const = 0;

private slots: // common network operations
    void readReply( QNetworkReply *reply );
    void replyError(QNetworkReply::NetworkError);
    void replyProgress(qint64 bytesSent, qint64 bytesTotal);


protected:

    QNetworkReply* request( BaaS::Ope operation, const QByteArray& data = QByteArray());

    //Use this to manage headers
    void setRawHeader(QByteArray name, QByteArray value){ rawHeaders[name] = value;}
    void removeRawHeader(QByteArray name){ rawHeaders.remove(name);}
    void resetRawHeaders(){ rawHeaders.empty();}
    void setHeader(QNetworkRequest::KnownHeaders name, QByteArray value){ headers[name] = value;}
    void removeHeader(QNetworkRequest::KnownHeaders name){ headers.remove(name);}
    void resetHeaders(){ headers.empty();}
    void initHeaders( ){}
    void applyAllHeaders( QNetworkRequest&) const;

    void setDefaultExtraHostURI(QString res){ extraHostURI = res;}
    bool isLastRequestSuccessful()const{ return lastRequestSuccessful;}


signals: // operation notifications
    void hostChanged();
    void endPointChanged();
    void errorChanged();
    void percCompleteChanged();
    void httpCodeChanged();
    void httpResponseChanged();
    void replyFinished(QJsonDocument doc, QNetworkReply *reply);                       //emited when a request gets replied
    void queryFailed(QString msg);
    void querySucceeded(QHash<int, QByteArray> roles, QVector<QVariantMap> data, QNetworkReply *reply);
    void readyChanged();

private:
    QNetworkAccessManager *_NAM = nullptr;
    QMap<QByteArray, QByteArray> rawHeaders;
    QMap<QNetworkRequest::KnownHeaders, QByteArray> headers;
    QNetworkReply* processRequest( BaaS::Ope operation, const QUrl& url , const QByteArray& data);
    QString hostURI = "";
    QString extraHostURI = "";
    QString endPoint = "";
    QString error = "";
    qreal percComplete = 100.;
    bool lastRequestSuccessful = false;

    uint httpCode = 0;
    QString httpResponse = "";
};

#endif // BAAS_H

