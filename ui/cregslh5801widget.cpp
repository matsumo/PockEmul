#include <QDebug>

#include "cregslh5801widget.h"
#include "ui_cregslh5801widget.h"
#include "ui/cregcpu.h"
#include "cpu.h"
#include "lh5801.h"
#include "pcxxxx.h"


Cregslh5801Widget::Cregslh5801Widget(CPObject *parent,CCPU *pCPU) :
    CregCPU(parent,pCPU),
    ui(new Ui::Cregslh5801Widget)
{
    ui->setupUi(this);

    connect(ui->regP,SIGNAL(textEdited(QString)),this,SLOT(updateReg()));
    connect(ui->regS,SIGNAL(textEdited(QString)),this,SLOT(updateReg()));
    connect(ui->regU,SIGNAL(textEdited(QString)),this,SLOT(updateReg()));
    connect(ui->regX,SIGNAL(textEdited(QString)),this,SLOT(updateReg()));
    connect(ui->regY,SIGNAL(textEdited(QString)),this,SLOT(updateReg()));
    connect(ui->regA,SIGNAL(textEdited(QString)),this,SLOT(updateReg()));
    connect(ui->regT,SIGNAL(textEdited(QString)),this,SLOT(updateReg()));
    connect(ui->regTM,SIGNAL(textEdited(QString)),this,SLOT(updateReg()));
    connect(ui->cbPU,SIGNAL(clicked(bool)),this,SLOT(updateReg()));
    connect(ui->cbPV,SIGNAL(clicked(bool)),this,SLOT(updateReg()));
    connect(ui->cbBF,SIGNAL(clicked(bool)),this,SLOT(updateReg()));
    connect(ui->cbDP,SIGNAL(clicked(bool)),this,SLOT(updateReg()));
    connect(ui->cbF0,SIGNAL(clicked(bool)),this,SLOT(updateReg()));
    connect(ui->cbF1,SIGNAL(clicked(bool)),this,SLOT(updateReg()));
    connect(ui->cbF2,SIGNAL(clicked(bool)),this,SLOT(updateReg()));
    connect(ui->cbF3,SIGNAL(clicked(bool)),this,SLOT(updateReg()));
    connect(ui->cbF4,SIGNAL(clicked(bool)),this,SLOT(updateReg()));
    connect(ui->cbF5,SIGNAL(clicked(bool)),this,SLOT(updateReg()));
    connect(ui->cbF6,SIGNAL(clicked(bool)),this,SLOT(updateReg()));
    connect(ui->cbF7,SIGNAL(clicked(bool)),this,SLOT(updateReg()));

}

Cregslh5801Widget::~Cregslh5801Widget()
{
    delete ui;
}

void Cregslh5801Widget::refresh()
{
    if (!pCPU) return;

    CLH5801 *cpu = (CLH5801 *) pCPU;

    cpu->pPC->Regs_Info(0);
    ui->textEdit->setText(QString(cpu->pPC->Regs_String));

    ui->regP->setText(QString("%1").arg(cpu->lh5801.p.w,4,16,QChar('0')));
    ui->regS->setText(QString("%1").arg(cpu->lh5801.s.w,4,16,QChar('0')));
    ui->regU->setText(QString("%1").arg(cpu->lh5801.u.w,4,16,QChar('0')));
//    ui->regIN->setText(QString("%1").arg(cpu->pPC->pKEYB->KStrobe,2,16,QChar('0')));
    ui->regX->setText(QString("%1").arg(cpu->lh5801.x.w,4,16,QChar('0')));
    ui->regY->setText(QString("%1").arg(cpu->lh5801.y.w,4,16,QChar('0')));
    ui->regA->setText(QString("%1").arg(cpu->lh5801.a,2,16,QChar('0')));
    ui->regT->setText(QString("%1").arg(cpu->lh5801.t,2,16,QChar('0')));
    ui->regTM->setText(QString("%1").arg(cpu->lh5801.tm,2,16,QChar('0')));
    ui->cbPV->setChecked(cpu->lh5801.pv);
    ui->cbPU->setChecked(cpu->lh5801.pu);
    ui->cbBF->setChecked(cpu->lh5801.bf);
    ui->cbDP->setChecked(cpu->lh5801.dp);
    ui->cbF7->setChecked(cpu->lh5801.t&0x80);
    ui->cbF6->setChecked(cpu->lh5801.t&0x40);
    ui->cbF5->setChecked(cpu->lh5801.t&0x20);
    ui->cbF4->setChecked(cpu->lh5801.t&0x10);
    ui->cbF3->setChecked(cpu->lh5801.t&8);
    ui->cbF2->setChecked(cpu->lh5801.t&4);
    ui->cbF1->setChecked(cpu->lh5801.t&2);
    ui->cbF0->setChecked(cpu->lh5801.t&1);

    update();
}


void Cregslh5801Widget::updateReg() {
    qWarning()<<"UPDATED S="<<ui->regS->text();
    if (!pCPU) return;
    CLH5801 *cpu = (CLH5801 *) pCPU;

    cpu->lh5801.p.w = ui->regP->text().toUInt(0,16);
    cpu->lh5801.s.w = ui->regS->text().toUInt(0,16);
    cpu->lh5801.u.w = ui->regU->text().toUInt(0,16);
    //    ui->regIN->setText(QString("%1").arg(cpu->pPC->pKEYB->KStrobe,2,16,QChar('0')));
    cpu->lh5801.x.w = ui->regX->text().toUInt(0,16);
    cpu->lh5801.y.w = ui->regY->text().toUInt(0,16);
    cpu->lh5801.a = ui->regA->text().toUInt(0,16);
    cpu->lh5801.t = ui->regT->text().toUInt(0,16);
    cpu->lh5801.tm = ui->regTM->text().toUInt(0,16);
    cpu->lh5801.pv = ui->cbPV->isChecked();
    cpu->lh5801.pu = ui->cbPU->isChecked();
    cpu->lh5801.bf = ui->cbBF->isChecked();
    cpu->lh5801.dp = ui->cbDP->isChecked();

    cpu->lh5801.t = 0x00;
    cpu->lh5801.t |= ui->cbF7->isChecked() ? 0x80 : 0x00;
    cpu->lh5801.t |= ui->cbF6->isChecked() ? 0x40 : 0x00;
    cpu->lh5801.t |= ui->cbF5->isChecked() ? 0x20 : 0x00;
    cpu->lh5801.t |= ui->cbF4->isChecked() ? 0x10 : 0x00;
    cpu->lh5801.t |= ui->cbF3->isChecked() ? 0x08 : 0x00;
    cpu->lh5801.t |= ui->cbF2->isChecked() ? 0x04 : 0x00;
    cpu->lh5801.t |= ui->cbF1->isChecked() ? 0x02 : 0x00;
    cpu->lh5801.t |= ui->cbF0->isChecked() ? 0x01 : 0x00;

}
