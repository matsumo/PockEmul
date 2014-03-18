#include "dialogpotar.h"
#include "ui_dialogpotar.h"

DialogPotar::DialogPotar(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogPotar)
{
    ui->setupUi(this);
    connect(ui->vSlider, SIGNAL(valueChanged( int )), this, SLOT(slotSlider(int)));
    pPotar = (Cpotar *) parent;
}

DialogPotar::~DialogPotar()
{
    delete ui;
}

void DialogPotar::slotSlider(int val) {
    pPotar->set_value(val);
}

void DialogPotar::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

