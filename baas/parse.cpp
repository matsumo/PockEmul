#include <QFile>
#include <QMimeDatabase>
#include <QXmlStreamWriter>
#include <QUuid>
#include <QFileDialog>
#include <QDomDocument>

#include "parse.h"
#include "mainwindowpockemul.h"
#include "cloud/cloudwindow.h"

#include "quazip/quazip.h"
#include "quazip/quazipfile.h"
#include "quazip/JlCompress.h"

extern MainWindowPockemul *mainwindow;
extern int ask(QWidget *parent, QString msg, int nbButton);

Parse::Parse(): BaaS()
{
    //Defines default headers
    setDefaultExtraHostURI("parse");
}

void Parse::initHeaders( )
{
    setRawHeader("X-Parse-Application-Id",applicationId.toUtf8());
    setRawHeader("Accept","application/json");
    setHeader(QNetworkRequest::ContentTypeHeader, "application/Json");
}

QString Parse::getApplicationId() const
{
    return applicationId;
}

void Parse::setApplicationId(const QString& res)
{
    applicationId = res;
    emit applicationIdChanged();
    emit readyChanged();
}
QString Parse::getSessionId() const
{
    return sessionId;
}

void Parse::setSessionId(const QString& res)
{
    sessionId = res;
    emit sessionIdChanged();
    emit readyChanged();
}
QString Parse::getUserId() const
{
    return userId;
}

void Parse::setUserId(const QString& res)
{
    userId = res;
    emit userIdChanged();
    emit readyChanged();
}

QString Parse::getMasterKey() const
{
    return masterKey;
}

bool Parse::isReady() const
{
    return ( (!applicationId.isEmpty()) && (!getHostURI().isEmpty()) && (getPercComplete()==100) );
}


void Parse::setMasterKey(const QString& res)
{
    masterKey = res;
    emit masterKeyChanged();
}


void Parse::signup( QString username, QString password, QString email )
{
    if (!isReady()) return;

    setEndPoint("users");

    QJsonObject obj{
      {"username", username},
      {"password", password},
      {"email", email}
    };

    setMasterKey("Remy");

    m_conn = connect(this, &BaaS::replyFinished, [=]( QJsonDocument json){
        disconnect(m_conn);
        if ( getHttpCode() == 201 ){
            QJsonObject obj = json.object();
            sessionId = obj.value("sessionToken").toString();
            userId = obj.value("objectId").toString();
            userName = obj.value("username").toString();
            qDebug() << "objectId" << obj.value("objectId").toString();
            qDebug() << "sessionToken" << sessionId;
            qDebug() << "res" << obj;
            emit loginChanged();
        }

    } );

    initHeaders();
    request( BaaS::POST, QJsonDocument(obj).toJson());

    setMasterKey("");
}

void Parse::deleteUser( QString objectId)
{
    if (!isReady() || sessionId.isEmpty()) return;

    setEndPoint("users/" + objectId );

    initHeaders();
    setRawHeader("X-Parse-Session-Token", sessionId.toUtf8());
    request( BaaS::DELETE);
}

void Parse::login( QString username, QString password )
{
    setHostURI(Cloud::getValueFor("serverURL",""));

    if (!isReady()) return;

    QUrlQuery postData;
    postData.addQueryItem("username", username);
    postData.addQueryItem("password", password);

    setEndPoint( "login?" + postData.toString() ); //TODO improve me : not use endpoint to give url encoded params

    initHeaders();
    if (registerInstallationOnLogin)
        setRawHeader("X-Parse-Installation-Id", "1");

    m_conn = connect(this, &BaaS::replyFinished, [=]( QJsonDocument json){
        disconnect(m_conn);
        if ( isLastRequestSuccessful() ) { //getHttpCode() == 201 ){
            QJsonObject obj = json.object();
            sessionId = obj.value("sessionToken").toString();
            userId = obj.value("objectId").toString();
            userName = obj.value("username").toString();
//            qWarning()<<obj;
            qWarning() << "objectId" << obj.value("objectId").toString();
            qWarning() << "sessionToken" << sessionId;
            setRawHeader("X-Parse-Session-Token", sessionId.toUtf8());
            emit loginChanged();
        }

    } );

    request( BaaS::GET);


}

void Parse::logout( )
{
    if (!isReady()) return;

    setEndPoint("logout");

    m_conn = connect(this, &BaaS::replyFinished, [=]( QJsonDocument json){
        disconnect(m_conn);
        Q_UNUSED(json);
        if ( isLastRequestSuccessful() ) {
            sessionId = "";
            userId = "";
            userName = "";

            removeRawHeader("X-Parse-Session-Token");
            emit loginChanged();
        }

    } );

    initHeaders();
    request( BaaS::POST);
}

void Parse::passwordReset( QString email)
{
    if (!isReady()) return;

    setEndPoint("requestPasswordReset");

    QJsonObject obj{
      {"email", email}
    };

    m_conn = connect(this, &BaaS::replyFinished, [=]( QJsonDocument json){
        disconnect(m_conn);
        Q_UNUSED(json);
        if ( isLastRequestSuccessful() ) {

        }

    } );

    initHeaders();
    request( BaaS::POST, QJsonDocument(obj).toJson() );
}

QNetworkReply *Parse::query(QString endPoint, QUrlQuery extraParams)
{
    if (!isReady()) return NULL;

    if (extraParams.isEmpty())
        setEndPoint(endPoint);
    else setEndPoint( endPoint + "?" + extraParams.toString() ); //TODO improve me : not use endpoint to give url encoded params

    ensureEndPointHasPrefix("classes");

    m_conn = connect(this, &BaaS::replyFinished, [=](QJsonDocument json, QNetworkReply *reply){
        disconnect(m_conn);
        if (isLastRequestSuccessful()){
                //structure to return from parse
                QHash<int, QByteArray> roles = QHash<int, QByteArray>();
                QVector<QVariantMap> data = QVector<QVariantMap>();

                Q_ASSERT(json.isObject());
                QJsonObject obj = json.object();
                QJsonValue tmp = obj.value("results");

                if (tmp.isArray()){
                    QJsonArray res = tmp.toArray();
                    for ( QJsonValue elem: res){
                        if (elem.isObject()){
                            obj = elem.toObject();

                            //Update roles
                            QStringList keys = obj.keys();
                            for (QString roleName : keys )
                            {
                                if ( roles.key( roleName.toUtf8(), -1) == -1)
                                    roles[roles.size()] = roleName.toUtf8();
                            }
                            //Add values
                            data.push_back( obj.toVariantMap());


                        }
                    }

                    emit querySucceeded(roles, data, reply);
                }
                //else if (tmp.isObject()){
                    //obj = tmp.toObject();//Update roles
                else {
                    QStringList keys = obj.keys();
                    for (QString roleName : keys )
                    {
                        if ( roles.key( roleName.toUtf8(), -1) == -1)
                            roles[roles.size()] = roleName.toUtf8();
                    }
                    //Add values
                    data.push_back( obj.toVariantMap());
                    emit querySucceeded(roles, data, reply);
                }
        }
    } );

    initHeaders();
    return request( BaaS::GET);

}

bool Parse::ensureEndPointHasPrefix(QString prefix)
{
    QString endpt = getEndPoint();
    //if ( endpt.left( prefix.length() ) == prefix )
    if (endpt.startsWith( prefix ))
        return true;
    else{
        setEndPoint( prefix + "/" + endpt);
    }
    return false;
}

//Objects related
void Parse::create(QString doc)
{
    if (!isReady()) {
        m_createQueue.append(doc);
        return;
    }

    qWarning()<<"create:"<<doc;

    ensureEndPointHasPrefix("classes");

    m_conn = connect(this, &BaaS::replyFinished, [=]( QJsonDocument json){
        disconnect(m_conn);
        if ( getHttpCode() == 201 ){
            currentObject = json.object();
            emit currentObjectChanged(currentObject);
            if(!m_createQueue.isEmpty()) {
                create(m_createQueue.takeFirst());
            }
        }
        else {
            // error, drop the queue
            m_createQueue.empty();
        }
    } );

    initHeaders();
    request( BaaS::POST, doc.toUtf8() );
}

QNetworkReply* Parse::get( QString include)
{
    Q_UNUSED(include); //TODO implement include

    if (!isReady()) return NULL;

    ensureEndPointHasPrefix("classes");

    initHeaders();

    m_conn = connect(this, &BaaS::replyFinished, [=]( QJsonDocument json){
        disconnect(m_conn);
        if ( isLastRequestSuccessful() ) {
            QJsonObject obj = json.object();
            currentObject = obj;
            postProcessGet(obj);
        }
    } );

    return request( BaaS::GET);

}

void Parse::postProcessGet(QJsonObject obj) {

    qWarning()<<getEndPoint();


    if (getEndPoint() == "classes/Pml?include=owner&order=-updatedAt&limit=2000") {
        qWarning()<<"obj:"<<obj;
        generatePmlXml(obj);
    }

    emit currentObjectChanged( obj);
}

void Parse::update(QString doc)
{
    if (!isReady()) {
        m_updateQueue.append(doc);
        return;
    }

    ensureEndPointHasPrefix("classes");

    m_conn = connect(this, &BaaS::replyFinished, [=]( QJsonDocument json){
        disconnect(m_conn);
        if ( isLastRequestSuccessful() ) {
            currentObject = json.object();
            emit currentObjectChanged(currentObject);
            if(!m_updateQueue.isEmpty()) {
                update(m_updateQueue.takeFirst());
            }
        }
        else {
            // error, drop the queue
            m_updateQueue.empty();
        }
    } );

    initHeaders();
    request( BaaS::PUT, doc.toUtf8() );

}

void Parse::deleteObject(QString doc)
{
    if (!isReady()) {
        m_deleteQueue.append(doc);
        return;
    }

    ensureEndPointHasPrefix("classes");

    //Get objectId to be deleted
    QString deletedObjectId = getEndPoint();
    int found = deletedObjectId.lastIndexOf('/');
    int length = deletedObjectId.length();
    deletedObjectId = deletedObjectId.right( length - found -1);


    m_conn = connect(this, &BaaS::replyFinished, [=]( QJsonDocument json){
        Q_UNUSED(json);
        disconnect(m_conn);
        if ( isLastRequestSuccessful() ) {
            emit objectDeleted( deletedObjectId );
            if(!m_deleteQueue.isEmpty()) {
                deleteObject(m_deleteQueue.takeFirst());
            }
        }
        else {
            // error, drop the queue
            m_deleteQueue.empty();
        }
    } );

    initHeaders();
    request( BaaS::DELETE, doc.toUtf8() );
}


QNetworkReply* Parse::uploadFile(QUrl url, QString name)
{
    QString filePath = url.toLocalFile();
    if (!isReady() || !QFile::exists(filePath)) return NULL;

    if (name.isEmpty()) name = url.fileName();
    setEndPoint( "files/"+name);

    QMimeDatabase db;
    QMimeType mime = db.mimeTypeForFile(filePath);

    QFile file(filePath);
    if (mime.inherits("text/plain")){
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
            return NULL;
    }
    else{
        if (!file.open(QIODevice::ReadOnly ))
            return NULL;
    }

    initHeaders();
    setHeader(QNetworkRequest::ContentTypeHeader, mime.name().toUtf8());

    m_conn = connect(this, &BaaS::replyFinished, [=]( QJsonDocument json){
        disconnect(m_conn);
        if ( getHttpCode() == 201 ){
            currentObject = json.object();
            emit fileUploaded( currentObject);
        }

    } );

    return request( BaaS::POST, file.readAll() );
}

QNetworkReply* Parse::uploadFileData(QByteArray data, QString name)
{

    if (!isReady() || data.isEmpty()) return NULL;

    if (name.isEmpty()) {
        // compute name from content
    }
    setEndPoint( "files/"+name);

    QMimeDatabase db;
    QMimeType mime = db.mimeTypeForData(data);

    initHeaders();
    setHeader(QNetworkRequest::ContentTypeHeader, mime.name().toUtf8());

    m_conn = connect(this, &BaaS::replyFinished, [=]( QJsonDocument json){
        disconnect(m_conn);
        if ( getHttpCode() == 201 ){
            currentObject = json.object();
            // Create fileLit object
            QString mainObj = "{\"url\": \""+currentObject.value("url").toString()+
                    "\",\"file\": {\"name\": \""+currentObject.value("name").toString()+
                                ",\"__type\": \"File\"}}";

            setEndPoint("classes/FilesList");
            create( mainObj);

            emit fileUploaded( currentObject);
        }

    } );

    return request( BaaS::POST, data );
}


QNetworkReply* Parse::deleteFile(QString fileName)
{
    if (!isReady() || getMasterKey().isEmpty()) return NULL;

    setEndPoint( "files/" + fileName);

    m_conn = connect(this, &BaaS::replyFinished, [=]( QJsonDocument json){
        Q_UNUSED(json);
        disconnect(m_conn);
        if ( isLastRequestSuccessful() ) {
            emit objectDeleted( fileName );

        }
    } );

    initHeaders();
    setRawHeader("X-Parse-Master-Key", getMasterKey().toUtf8());

    return request( BaaS::DELETE );

}

void Parse::uploadPML(QString type) {
    QString _filter;
    if (type == "pml") _filter = "PockEmul Session files (*.pml)";
    if (type == "psk") _filter = "PockEmul Skin files (*.psk)";

    m_uploadQueue = QFileDialog::getOpenFileNames(0,
                                                    tr("Open Session files"),
                                                    ".",
                                                    _filter);

    m_conn_uploadPML = connect(this,&Parse::pmlUploaded,this,[=](){
        if (!m_uploadQueue.isEmpty()) {
            qWarning()<<"**"<<m_uploadQueue.count()<<" files remaining"<<m_uploadQueue.first();
            processPML(type,m_uploadQueue.takeFirst());
        }
        else {
            disconnect(m_conn_uploadPML);
            QMetaObject::invokeMethod(object, "hideWorkingScreen");
            pmlList();
        }
    });

    QMetaObject::invokeMethod(object, "showWorkingScreen");
    processPML(type,m_uploadQueue.takeFirst());
}
void Parse::saveCurrentSession(QString title, QString description, QString pml_file) {

    connect(this,&Parse::pmlUploaded,this,[=](){
            QMetaObject::invokeMethod(object, "hideWorkingScreen");
            pmlList();
    });

    QMetaObject::invokeMethod(object, "showWorkingScreen");
    postPML(  "pml", title,  description,  pml_file.toLatin1() );
}

void Parse::processPML(QString type,QString pmlFileName) {
    QFile _file(pmlFileName);
    _file.open(QIODevice::ReadOnly);

    QByteArray _ba = _file.readAll();
    _file.close();

    postPML( type,"Titre", "Description", _ba);

}


void Parse::updatePML(QString doc)
{
    if (!isReady()) {
        m_updateQueue.append(doc);
        return;
    }

    ensureEndPointHasPrefix("classes");

    m_conn = connect(this, &BaaS::replyFinished, [=]( QJsonDocument json){
        disconnect(m_conn);
        if ( isLastRequestSuccessful() ) {
            qWarning()<<"updatePML Result:"<<json.object();
            currentObject = json.object();
            emit currentObjectChanged(currentObject);

            emit pmlUploaded();
            if(!m_updateQueue.isEmpty()) {
                update(m_updateQueue.takeFirst());
            }
            else {
                QMetaObject::invokeMethod(object, "hideWorkingScreen");
                pmlList();
            }
        }
        else {
            // error, drop the queue
            m_updateQueue.empty();
        }
    } );

    initHeaders();
    request( BaaS::PUT, doc.toUtf8() );

}
void Parse::postPML( QString type, QString title, QString description, QByteArray pml_file, QString keywords )
{
    qWarning()<<"postPML, ready:"<<isReady();
    while (!isReady()) { }

    setEndPoint("classes/Pml");

    QJsonObject obj;

    QJsonObject owner{
                {"__type","Pointer"},
                {"className","_User"},
                {"objectId",userId}
            };

    obj.insert("owner",owner);
    obj.insert("type",type);

    QByteArray zipdata;

    if (type=="pml") {
        QBuffer buf(&zipdata);
        QuaZip zip(&buf);

        zip.open(QuaZip::mdCreate);
        QuaZipFile file(&zip);
        file.open(QIODevice::WriteOnly, QuaZipNewInfo("session.pml"));

        file.write(pml_file);

        file.close();
        zip.close();
    }

    if (type=="psk") {
        zipdata = pml_file;

        QBuffer buf(&zipdata);
        QuaZip zip(&buf);
        zip.open(QuaZip::mdUnzip);

        // check Package.json exist
        if (!zip.setCurrentFile("package.json")) {
            // ERROR
            qWarning()<<"ERROR - package.json missing";
            ask(mainwindow,"Invalid skin package.\n'package.json' file missing",1);
            return;
        }

        // open Package.json
        QuaZipFile file(&zip);
        file.open(QIODevice::ReadOnly);
        QByteArray _ba =   file.readAll();
        QJsonDocument json = QJsonDocument::fromJson(_ba);


        if (json.object().value("model").isUndefined()) {
            // ERROR
            qWarning()<<"ERROR model empty";
            ask(mainwindow,"Invalid skin package.\nModel not defined in package.json file",1);
            return;
        }

        QString _model = json.object().value("model").toString();
        QString _title = json.object().value("title").toString();
        QString _description = json.object().value("description").toString();
        QString _author = json.object().value("author").toString();
        QString _email = json.object().value("email").toString();
        QString _url = json.object().value("url").toString();

        title = _title;
        description = _description + "\r\n" +
                "Author : "+_author + "\r\n" +
                "Email : "+_email + "\r\n" +
                "Url : "+_url;
//                "Email :<a href=\"mailto://"+_email +"\">"+_email+"</a>\r\n" +
//                "Url :<a href=\""+_url +"\">"+_url+"</a>";
        file.close();
    }

    obj.insert("data",QString(zipdata.toBase64()));


    QJsonObject acl{
        { userId,  QJsonObject{
                    {"read", true},
                    {"write", true}
                }
        }
    };
    obj.insert("ACL",acl);

    obj.insert("title",title);
    obj.insert("description",description);
    obj.insert("keywords",keywords);

    // parse pml to generate objetcs list
    // This should be moved to parse.Cloud
    // DONE

//    QDomDocument document;
//    document.setContent( pml_file );
//    QDomElement documentElement = document.documentElement();
//    QDomNodeList elements = documentElement.elementsByTagName( "object" );
//    QString keywords = "|";
//    for (int i=0; i<elements.size();i++ )
//    {
//        QDomElement object = elements.at(i).toElement();
//        QString name = object.attribute("name");
//        keywords.append( name + QString("|") );
//        qWarning()<<keywords;
//    }
//    obj.insert("keywords",keywords);

    m_conn = connect(this, &BaaS::replyFinished, [=]( QJsonDocument json){
//        qWarning()<<"disconnect"<<m_conn;
        disconnect(m_conn);
        if ( getHttpCode() == 201 ){
            qWarning()<<"postPML Result:"<<json.object();

            emit pmlUploaded();
        }

    } );

    qWarning()<<"connect"<<m_conn;
    initHeaders();
    request( BaaS::POST, QJsonDocument(obj).toJson());


}
void Parse::pmlList()
{

    if (!isReady()) return;

    setEndPoint("classes/Pml?include=owner&order=-updatedAt&limit=2000");
    get();

}


/*
 * <guid type="integer">494</guid>
<title type="string">Quelques jeux</title>
<description type="string">une description provisoire</description>
<snapshot_small type="string">
http://ds409/elgg/mod/file/thumbnail.php?file_guid=494&size=small&icontime=1464205945
</snapshot_small>
<snapshot_medium type="string">
http://ds409/elgg/mod/file/thumbnail.php?file_guid=494&size=medium&icontime=1464205945
</snapshot_medium>
<snapshot_large type="string">
http://ds409/elgg/mod/file/thumbnail.php?file_guid=494&size=large&icontime=1464205945
</snapshot_large>
<owner_id type="integer">33</owner_id>
<owner_username type="string">remy</owner_username>
<owner_name type="string">Remy</owner_name>
<owner_avatar_url type="string">
http://ds409/elgg/mod/profile/icondirect.php?lastcache=1382668274&joindate=1382664806&guid=33&size=small
</owner_avatar_url>
<isfriend type="integer">0</isfriend>
<container_guid type="string">33</container_guid>
<access_id type="string">2</access_id>
<ispublic type="integer">1</ispublic>
<time_created type="integer">1464205944</time_created>
<time_updated type="integer">1471757772</time_updated>
<last_action type="integer">1464206116</last_action>
<MIMEType type="string">pml/pml</MIMEType>
<objects type="string">466|HX-20;467|Epson H20RC</objects>
<listobjects type="string">,466,467,</listobjects>
<isdeleted type="integer">0</isdeleted>
*/

QString Parse::generatePmlXml(QJsonObject obj) {

    QString _xml;

    QXmlStreamWriter *xml = new QXmlStreamWriter(&_xml);

    xml->writeStartElement("pmllist");
    //        xml->writeAttribute("version", "2.0");
    QJsonArray results = obj["results"].toArray();
    qWarning()<<"json count"<<results.count();
    foreach (const QJsonValue & value, results) {

        xml->writeStartElement("pml_item");
        QJsonObject pml_item = value.toObject();
        xml->writeTextElement("guid",pml_item["objectId"].toString());
        xml->writeTextElement("title",pml_item["title"].toString());
        xml->writeTextElement("description",pml_item["description"].toString());
        xml->writeTextElement("snapshot_small",(pml_item["snap"].toObject())["url"].toString());
        xml->writeTextElement("pmlfile",(pml_item["pmlfile"].toObject())["url"].toString());
        xml->writeTextElement("owner_username",(pml_item["owner"].toObject())["username"].toString());
        xml->writeTextElement("access_id",pml_item["ACL"].toObject().contains("*")?"2":"0");
        xml->writeTextElement("ispublic",pml_item["ACL"].toObject().contains("*")?"1":"0");
        xml->writeTextElement("keywords",pml_item["keywords"].toString());
        xml->writeTextElement("type",pml_item["type"].toString());
        xml->writeTextElement("createdAt",pml_item["createdAt"].toString());
        xml->writeTextElement("updatedAt",pml_item["updatedAt"].toString());


        xml->writeEndElement();  // pml_item
    }
    xml->writeEndElement();  // pmllist

    xmlPmlList = _xml;
    emit xmlPmlListChanged(xmlPmlList);

    qWarning()<<xmlPmlList;

    return xmlPmlList;
}

// NOT USED anymore
void Parse::updatePMLfile( QString objectId, QString pml_file)
{
    if (!isReady()) return;

    setEndPoint("classes/Pml/"+objectId);

    QByteArray zipdata;
    QBuffer buf(&zipdata);
    QuaZip zip(&buf);

    zip.open(QuaZip::mdCreate);
    QuaZipFile file(&zip);
    file.open(QIODevice::WriteOnly, QuaZipNewInfo("session.pml"));

    file.write(pml_file.toLatin1());

    file.close();
    zip.close();

    QJsonObject obj;
    obj.insert("data",QString(zipdata.toBase64()));

    updatePML( QJsonDocument(obj).toJson());

}

