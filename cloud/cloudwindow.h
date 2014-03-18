#ifndef MAINWINDOW_H
#define MAINWINDOW_H


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

protected:
    void wheelEvent(QWheelEvent *event);

public slots:
    Q_INVOKABLE void refresh();
    Q_INVOKABLE QString save();
    Q_INVOKABLE void showFileDialog();
    Q_INVOKABLE void warning(QString msg);

private slots:

    void sendPML(const QString &filePath);
    void downloadFinished();
    void downloadFinished2();
    void finishedSave(QNetworkReply* reply);

private:
    QDeclarativeView *view;

    QNetworkReply *m_reply;
    QFileDialog *m_fileDialog;
    QObject *object;

    CloudImageProvider *imgprov;
    void resizeEvent(QResizeEvent *e);
};

#endif // MAINWINDOW_H
