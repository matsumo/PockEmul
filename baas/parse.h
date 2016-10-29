#ifndef PARSE_H
#define PARSE_H
#include "baas.h"
#include <QObject>

class Parse : public BaaS
{
    Q_OBJECT
    Q_PROPERTY( QString applicationId READ getApplicationId  WRITE setApplicationId NOTIFY applicationIdChanged)
    Q_PROPERTY( QString sessionId READ getSessionId  WRITE setSessionId NOTIFY sessionIdChanged)
    Q_PROPERTY( QString userId READ getUserId  WRITE setUserId NOTIFY userIdChanged)
    Q_PROPERTY( QString masterKey READ getMasterKey  WRITE setMasterKey NOTIFY masterKeyChanged)
    Q_PROPERTY( bool useMaster READ getUseMaster WRITE SetMasterMode NOTIFY masterModeChanged)
    Q_PROPERTY( bool loggedIn READ isLoggedIn NOTIFY loginChanged)
    Q_PROPERTY( QString userName MEMBER userName NOTIFY loginChanged)

    Q_PROPERTY(QString xmlPmlList MEMBER xmlPmlList NOTIFY xmlPmlListChanged)

public:
    Parse();

    //Users related
    Q_INVOKABLE void signup(QString username, QString password , QString email);
    Q_INVOKABLE void deleteUser( QString objectId);
    Q_INVOKABLE void login( QString username, QString password );
    Q_INVOKABLE void logout( );
    Q_INVOKABLE void passwordReset(QString email);

    //Objects related
    Q_INVOKABLE void create(QString doc);
    Q_INVOKABLE QNetworkReply *get( QString include = "");
    Q_INVOKABLE void update(QString doc);
    Q_INVOKABLE void deleteObject(QString doc = "");

    Q_INVOKABLE bool ensureEndPointHasPrefix(QString prefix);

    Q_INVOKABLE void pmlList();
    Q_INVOKABLE void uploadPML();
    Q_INVOKABLE void postPML(QString title, QString description, QString pml_file);
    Q_INVOKABLE void updatePMLfile(QString objectId, QString pml_file);
    Q_INVOKABLE void saveCurrentSession(QString title, QString description, QString pml_file);

    //Files related
    Q_INVOKABLE QNetworkReply *uploadFile(QUrl url, QString name = QString());
    Q_INVOKABLE QNetworkReply *uploadFileData(QByteArray data, QString name);
    Q_INVOKABLE QNetworkReply *deleteFile(QString fileName);

    //Query - this is used by the BaasModel
    virtual QNetworkReply *query(QString endPoint, QUrlQuery extraParams)  Q_DECL_OVERRIDE;


public: // property access
    QString getApplicationId() const;
    void setApplicationId(const QString& res);
    QString getSessionId() const;
    void setSessionId(const QString& res);
    QString getUserId() const;
    void setUserId(const QString& res);
    QString getMasterKey() const;
    void setMasterKey(const QString& res);
    bool isLoggedIn(){ return !sessionId.isEmpty();}
    bool isReady() const Q_DECL_OVERRIDE;
    bool getUseMaster() const{ return useMaster;}
    void SetMasterMode(bool _useMaster){
        if (masterKey.isEmpty()) return;

        useMaster = _useMaster;

        if (useMaster){
            setRawHeader("X-Parse-Master-Key", masterKey.toUtf8());
        }
        else removeRawHeader("X-Parse-Master-Key");
        emit masterModeChanged();
    }


    void postProcessGet(QJsonObject obj);

    QString generatePmlXml(QJsonObject json);

    static QString toKey(QString s) {
        return  QString(QCryptographicHash::hash ( s.toUtf8(), QCryptographicHash::Md5).toBase64().toHex());
    }

    QObject *object;

    void processPML(QString pmlFileName);
    void updatePML(QString doc);
    void updateSnapfile(QString objectId, QString pml_file, QJsonObject obj);
signals:
    void applicationIdChanged();
    void sessionIdChanged();
    void userIdChanged();
    void masterKeyChanged();
    void loginChanged();
    void pmlListChanged(QString xml);
    void masterModeChanged();
    void objectDeleted( QString deletedObjectId );
    void currentObjectChanged( QJsonObject currentObject);
    void fileUploaded( QJsonObject currentObject);
    void xmlPmlListChanged(const QString &newXml);
    void pmlUploaded();



private:
    void initHeaders( );

    QString applicationId ="";
    QString sessionId ="";
    QString userId ="";
    QString userName = "";
    QJsonObject currentObject = QJsonObject();
    QString masterKey = "";

    QString xmlPmlList;

    QMetaObject::Connection m_conn;             //temporary use for terminating connection with lambda
    bool registerInstallationOnLogin = false;
    bool useMaster = false;

    QList<QString> m_createQueue;
    QList<QString> m_updateQueue;
    QList<QString> m_deleteQueue;
    QList<QString> m_uploadQueue;


};

#endif // PARSE_H
