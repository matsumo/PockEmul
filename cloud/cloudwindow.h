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

class CloudWindow : public QWidget
{
    Q_OBJECT
    
public:
    CloudWindow(QWidget *parent = 0);
    virtual QSize sizeHint() const;

    Q_INVOKABLE int askDialog(QString msg, int nbButton);
    Q_INVOKABLE void getPML(int id, int version=0, QString auth_token = QString());
    Q_INVOKABLE static QString getValueFor(const QString &objectName, const QString &defaultValue = QString());
    Q_INVOKABLE static void saveValueFor(const QString &objectName, const QString &inputValue);
    Q_INVOKABLE QByteArray generateKey(QString username, QString password);

    Q_INVOKABLE void saveCache(QString fileName,QString xml);
    Q_INVOKABLE QString loadCache(QString fileName);
    Q_INVOKABLE bool isPortraitOrientation();
    Q_INVOKABLE void addShortcut(QString param);

    Q_INVOKABLE void clearCache(QString s);

    void addPocket(QString _name, QString url, QString pocketId, int left, int top, int width, int height);

protected:
    void wheelEvent(QWheelEvent *event);

public slots:
    Q_INVOKABLE void refresh();
    Q_INVOKABLE QString save();
    Q_INVOKABLE void showFileDialog();
    Q_INVOKABLE void warning(QString msg);
    Q_INVOKABLE void click(QString Id, int x, int y);
    Q_INVOKABLE void unclick(QString Id, int x, int y);
    Q_INVOKABLE void setzoom(int x,int y,int z);
    Q_INVOKABLE void movepocket(QString Id, int x, int y);
    void pocketUpdated(CPObject* pObject);
    void newPObject(CPObject *pObject);
    void delPObject(CPObject *pObject);
    void movePObject(CViewObject*pObject, QPoint pos);
    void sizePObject(CViewObject*pObject, QSize size);
    void stackPosChanged();


private slots:

    void sendPML(const QString &filePath);
    void downloadFinished();
    void downloadFinished2();
    void finishedSave(QNetworkReply* reply);

private:
//    QDeclarativeView *view;
    QQuickView *view;
//    QQuickWidget *view;

    QNetworkReply *m_reply;
    QFileDialog *m_fileDialog;
    QObject *object;

    CloudImageProvider *imgprov;
    void resizeEvent(QResizeEvent *e);
};

#endif // CLOUDWINDOW_H
