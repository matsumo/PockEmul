#ifndef CLOUDWINDOW_H
#define CLOUDWINDOW_H


#include <QMainWindow>
#include <QDialog>
#include <QMap>
#include <QModelIndex>

class ImageDownloader;
class ImageObject;
class QFileDialog;
class QListView;
class QNetworkReply;
class QPushButton;
class CloudImageProvider;
class QDeclarativeView;
class QQuickView;
class QQuickWidget;
class CViewObject;
class CPObject;


class Cloud : public QObject {
    Q_OBJECT
public:
    Cloud(QWidget *parent=0);

    Q_INVOKABLE int askDialog(QString msg, int nbButton);
    Q_INVOKABLE void getPML(int id, int version=0, QString auth_token = QString(), QString type=QString("pml"));
    Q_INVOKABLE static QString getValueFor(const QString &objectName, const QString &defaultValue = QString());
    Q_INVOKABLE static void saveValueFor(const QString &objectName, const QString &inputValue);
    Q_INVOKABLE QByteArray generateKey(QString username, QString password);

    Q_INVOKABLE void saveCache(QString fileName,QString xml);
    Q_INVOKABLE QString loadCache(QString fileName);
    Q_INVOKABLE bool isPortraitOrientation();
    Q_INVOKABLE void addShortcut(QString param);

    Q_INVOKABLE void clearCache(QString s);

signals:
    void downloadEnd();
public slots:
    Q_INVOKABLE void refresh();
    Q_INVOKABLE QString save();
    Q_INVOKABLE void showFileDialog();
    Q_INVOKABLE void warning(QString msg);

    void sendPML(const QString &filePath);
    void downloadFinishedPml();
    void downloadFinishedPsk();
    void downloadFinished2();
    void finishedSave(QNetworkReply* reply);
public:
    CloudImageProvider *imgprov;
    QFileDialog *m_fileDialog;
    QNetworkReply *m_reply;
    QWidget *parent;
    QObject *object;
};

class CloudWindow : public QWidget
{
    Q_OBJECT
    
public:
    CloudWindow(QWidget *parent = 0, QString src=QString("qrc:/Main.qml"));
    virtual QSize sizeHint() const;

    Cloud cloud;

    void addPocket(QString _name, QString url, QString pocketId, int left, int top, int width, int height);

protected:
    void wheelEvent(QWheelEvent *event);




private slots:

    Q_INVOKABLE void closeQuick();



private:
    QDeclarativeView *declarativeView;
//    QQuickView *view;
    QQuickWidget *cloudView;






    void resizeEvent(QResizeEvent *e);
};

#endif // CLOUDWINDOW_H
