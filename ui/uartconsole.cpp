#include "uartconsole.h"
#include "ui_uartconsole.h"
#include "uart.h"

CUartConsole::CUartConsole(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CUartConsole)
{
    ui->setupUi(this);
    pUart = (Cuart*) parent;

    //connect(ui->textEdit_in,SIGNAL(textChanged()),this,SLOT(newInputByte()));
    connect(ui->pbSend,SIGNAL(clicked()),this,SLOT(newInputByte()));
}

CUartConsole::~CUartConsole()
{
    delete ui;
}

void CUartConsole::newOutputByte(qint8 data) {
    if (data != 0x0D) {
        QString buf = ui->textEdit_out->toPlainText();
        ui->textEdit_out->setPlainText(buf + QString(data));
    }
}

void CUartConsole::newInputByte()
{
    pUart->clearInputBuffer();
    pUart->newInputArray(ui->textEdit_in->toPlainText().toLatin1().replace(0x0a,0x0d));
}
