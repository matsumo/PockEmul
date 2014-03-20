// AutoUpdater.h: interface for the CAutoUpdater class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _AUTOUPDATER_H_
#define _AUTOUPDATER_H_



#include <QDialog>
#include <QNetworkAccessManager>

class QDialogButtonBox;
class QPushButton;
class QLabel;

class CAutoUpdater : public QDialog
 {
     Q_OBJECT

 public:
     CAutoUpdater(QWidget *parent = 0);

 private slots:
     void downloadFile();
     void cancelDownload();
     void downloadFinished(QNetworkReply *reply);

 private:
     QPushButton *checkButton;
     QPushButton *quitButton;
     QDialogButtonBox *buttonBox;
     QLabel		*labelmsg;

     QNetworkAccessManager manager;

     int httpGetId;
     bool httpRequestAborted;
 };

#endif
