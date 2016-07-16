// AutoUpdater.cpp: implementation of the CAutoUpdater class.

#include <qglobal.h>
#if QT_VERSION >= 0x050000
#   include <QtWidgets>
#   include <QVBoxLayout>
#else
#   include <QtCore>
#   include <QtGui>

#endif

#include <QPushButton>
#include <QDialogButtonBox>
#include <QLabel>

#include <QMainWindow>
#include <QNetworkRequest>
#include <QNetworkReply>

#include "version.h"
#include "common.h" 
#include "autoupdater.h"
#include "mainwindowpockemul.h"

#define POCKEMUL_UPDATE_FILE "http://pockemul.free.fr/update/autoupdater.txt"

extern MainWindowPockemul* mainwindow;
extern int ask(QWidget *parent,QString msg,int nbButton);

CAutoUpdater::CAutoUpdater(QWidget *parent)
     : QDialog(parent)
{
    setAttribute(Qt::WA_DeleteOnClose, true);

	checkButton = new QPushButton(tr("Retry"));
	quitButton = new QPushButton(tr("Quit"));
	checkButton->setDefault(true);
	
	buttonBox = new QDialogButtonBox;
	buttonBox->addButton(checkButton, QDialogButtonBox::ActionRole);
	buttonBox->addButton(quitButton, QDialogButtonBox::RejectRole);

	labelmsg = new QLabel(tr("Checking Updates"));
	
    connect(&manager, SIGNAL(finished(QNetworkReply*)),
            SLOT(downloadFinished(QNetworkReply*)));
	connect(checkButton, SIGNAL(clicked()), this, SLOT(downloadFile()));
	connect(quitButton, SIGNAL(clicked()), this, SLOT(close()));
	
	QVBoxLayout *mainLayout = new QVBoxLayout;
	mainLayout->addWidget(labelmsg);
	mainLayout->addWidget(buttonBox);
	setLayout(mainLayout);
	
	setWindowTitle(tr("Check Updates"));
	downloadFile();
}

void CAutoUpdater::downloadFile()
{
	QUrl url(POCKEMUL_UPDATE_FILE);
	labelmsg->setText(tr("Ckecking Updates..."));

    QNetworkRequest request(url);
    QNetworkReply *reply = manager.get(request);
    Q_UNUSED(reply)

	checkButton->setEnabled(false);
}

void CAutoUpdater::cancelDownload()
{
    httpRequestAborted = true;
	checkButton->setEnabled(true);
}


void CAutoUpdater::downloadFinished(QNetworkReply *reply)
{
    QUrl url = reply->url();
    if (reply->error()) {
        ask(mainwindow,tr("Download of %1 failed: %2").
            arg(url.toEncoded().constData()).arg(reply->errorString()),1);
    } else {

        QString result(reply->readAll());
        qWarning()<<result;
        if (result.replace(".","").toInt() > QString(POCKEMUL_VERSION).replace(".","").toInt()){
            QMessageBox::about(this, tr("New Release"),tr("A new release is available\nCheck Web Site : http://pockemul.free.fr"));
            close();
        }
        else {
            QMessageBox::about(this, tr("New Release"),tr("PockEmul is up-to-date"));
            close();
        }
    }
}
