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


#include "version.h"
#include "common.h" 
#include "autoupdater.h"
#include "qhttp/qhttp.h"

#define POCKEMUL_UPDATE_FILE "http://pockemul.free.fr/update/autoupdater.txt"

extern QMainWindow *mainwindow;


CAutoUpdater::CAutoUpdater(QWidget *parent)
     : QDialog(parent)
{

	checkButton = new QPushButton(tr("Retry"));
	quitButton = new QPushButton(tr("Quit"));
	checkButton->setDefault(true);
	
	buttonBox = new QDialogButtonBox;
	buttonBox->addButton(checkButton, QDialogButtonBox::ActionRole);
	buttonBox->addButton(quitButton, QDialogButtonBox::RejectRole);

	labelmsg = new QLabel(tr("Checking Updates"));
	
	http = new QHttp(this);
	
	connect(http, SIGNAL(done(bool)), this, SLOT(done(bool)));
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
	http->setHost(url.host(), url.port() != -1 ? url.port() : 80);
	if (!url.userName().isEmpty())
		http->setUser(url.userName(), url.password());
	
	httpRequestAborted = false;
	httpGetId = http->get(url.path(), 0);
	checkButton->setEnabled(false);
}

void CAutoUpdater::cancelDownload()
{
	httpRequestAborted = true;
	http->abort();
	checkButton->setEnabled(true);
}

void CAutoUpdater::done(bool error)
{
	if (error){
		MSG_ERROR(http->errorString());
	}
	else {
		QString result(http->readAll());
//		MSG_ERROR(result)
        if (result != POCKEMUL_VERSION){
			QMessageBox::about(this, tr("New Release"),tr("A new release is available\nCheck Web Site : http://pockemul.free.fr"));
			close();
		}
		else {
			QMessageBox::about(this, tr("New Release"),tr("PockEmul is up-to-date"));
			close();		
		}
	}
	QApplication::restoreOverrideCursor();
	checkButton->setEnabled(true);
}
