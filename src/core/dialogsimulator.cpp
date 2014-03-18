#include <QTextCursor>
#include <QCloseEvent>
#include <QAbstractButton>
#include <QFileDialog>
#include <QTextStream>

#include "dialogsimulator.h"
#include "common.h"
#include "pcxxxx.h"
#include "Connect.h"
#include "mainwindowpockemul.h"

extern	MainWindowPockemul* mainwindow;

//
DialogSimulator::DialogSimulator( QWidget * parent, Qt::WindowFlags f) : QDialog(parent, f)
{
    setupUi(this);
    setFocusPolicy(Qt::StrongFocus);

    connect(buttonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(clicked(QAbstractButton*)));

    currentIndex=0;
    textEdit->setTabStopWidth(40);

    pCeSimu = (Ccesimu *) parent;

}
//
void DialogSimulator::refresh( void)
{



}



void DialogSimulator::closeEvent(QCloseEvent *event)
{
    event->accept();
}

void DialogSimulator::clicked( QAbstractButton * button) {
    if ( buttonBox->standardButton(button) == QDialogButtonBox::Apply ) {
            // code pour Apply

            QString header = " \
const MT_OUT2=1; \
const GND=2; \
const VGG=3; \
const BUSY=4; \
const D_OUT=5; \
const MT_IN=6; \
const MT_OUT1=7; \
const D_IN=8; \
const ACK=9; \
const SEL2=10; \
const SEL1=	11;";
            QString footer = "";// return 'OK';}";
            pCeSimu->script = new QScriptValue(pCeSimu->engine->evaluate(header + textEdit->toPlainText() + footer));
            if (pCeSimu->script->isError()) {
                    MSG_ERROR(pCeSimu->script->property("message").toString());
                }
            pCeSimu->mainfunction = new QScriptValue(pCeSimu->engine->globalObject().property("run"));
            QString s = pCeSimu->mainfunction->call(QScriptValue()).toString();

            TRACE("Script loaded.");

        }
    else
    if ( buttonBox->standardButton(button) == QDialogButtonBox::Open ) {
        // code pour Apply

        QString fileName = QFileDialog::getOpenFileName(
                mainwindow,
                tr("Choose a file"),
                ".",
                tr("PockEmul sessions (*.qs)"));

        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly)) {
            QMessageBox::warning(mainwindow,tr("PockEmul"),
                                 tr("Cannot read file %1:\n%2.")
                                 .arg(file.fileName())
                                 .arg(file.errorString()));
            return ;
        }
        textEdit->setPlainText(QString(file.readAll()));
    }
    else
    if ( buttonBox->standardButton(button) == QDialogButtonBox::Save ) {
        // code pour Apply
        QString fn = QFileDialog::getSaveFileName(
                mainwindow,
                tr("Choose a filename to save the script"),
                ".",
                tr("Session File (*.qs)"));

        QFile f(fn);
        if (f.open(QFile::WriteOnly | QFile::Truncate)) {
            QTextStream out(&f);
            out << textEdit->toPlainText();
        }else {
            MSG_ERROR("ERROR saving script")
        }

    }
    else
    if ( buttonBox->standardButton(button) == QDialogButtonBox::Reset ) {
        pCeSimu->pCONNECTOR->Set_values(0);
        pCeSimu->pSavedCONNECTOR->Set_values(0);
    }
    else {
        // code pour Reset
    }
}
