// AutoUpdater.h: interface for the CAutoUpdater class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _AUTOUPDATER_H_
#define _AUTOUPDATER_H_



#include <QDialog>

class QDialogButtonBox;
class QHttp;
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
     void done(bool error);

 private:
     QPushButton *checkButton;
     QPushButton *quitButton;
     QDialogButtonBox *buttonBox;
     QLabel		*labelmsg;

     QHttp *http;
     int httpGetId;
     bool httpRequestAborted;
 };

#endif
