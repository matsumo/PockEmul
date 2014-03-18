#ifndef DOWNLOADMANAGER_H
#define DOWNLOADMANAGER_H

#include <QCoreApplication>
#include <QFile>
#include <QFileInfo>
#include <QList>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QStringList>
#include <QTimer>
#include <QUrl>
#include <QProgressBar>
#include <QPushButton>

class DownloadManager: public QObject
{
    Q_OBJECT
    QNetworkAccessManager manager;
    QList<QNetworkReply *> currentDownloads;
    QMap<QNetworkReply *,qint64> bytesReceived;
    QMap<QNetworkReply *,qint64> bytesTotal;

public:
    DownloadManager();
    void doDownload(const QUrl &url);
    QString saveFileName(const QUrl &url);
    bool saveToDisk(const QString &filename, QIODevice *data);
    QString targetDir;
    QProgressBar *progress;
    QPushButton *abortPB;

    void resize();

public slots:
    void abort();
    void downloadFinished(QNetworkReply *reply);
    void downloadProgress(qint64 received, qint64 total);
};

#endif // DOWNLOADMANAGER_H
