#include "cregssc61860widget.h"
#include "ui_cregssc61860widget.h"
#include "ui/cregcpu.h"
#include "cpu.h"
#include "sc61860.h"

Cregssc61860Widget::Cregssc61860Widget(CPObject *parent,CCPU *pCPU) :
    CregCPU(parent,pCPU),
    ui(new Ui::Cregssc61860Widget)
{
    ui->setupUi(this);

    connect(ui->reg00,SIGNAL(textEdited(QString)),this,SLOT(updateReg()));
    connect(ui->reg01,SIGNAL(textEdited(QString)),this,SLOT(updateReg()));
    connect(ui->reg02,SIGNAL(textEdited(QString)),this,SLOT(updateReg()));
    connect(ui->reg03,SIGNAL(textEdited(QString)),this,SLOT(updateReg()));
    connect(ui->reg04,SIGNAL(textEdited(QString)),this,SLOT(updateReg()));
    connect(ui->reg05,SIGNAL(textEdited(QString)),this,SLOT(updateReg()));
    connect(ui->reg06,SIGNAL(textEdited(QString)),this,SLOT(updateReg()));
    connect(ui->reg07,SIGNAL(textEdited(QString)),this,SLOT(updateReg()));
    connect(ui->reg08,SIGNAL(textEdited(QString)),this,SLOT(updateReg()));
    connect(ui->reg09,SIGNAL(textEdited(QString)),this,SLOT(updateReg()));

    connect(ui->regP,SIGNAL(textEdited(QString)),this,SLOT(updateReg()));
    connect(ui->regQ,SIGNAL(textEdited(QString)),this,SLOT(updateReg()));
    connect(ui->regR,SIGNAL(textEdited(QString)),this,SLOT(updateReg()));
    connect(ui->regDP,SIGNAL(textEdited(QString)),this,SLOT(updateReg()));
    connect(ui->regPC,SIGNAL(textEdited(QString)),this,SLOT(updateReg()));


//    connect(ui->cbPU,SIGNAL(clicked(bool)),this,SLOT(updateReg()));

}

Cregssc61860Widget::~Cregssc61860Widget()
{
    delete ui;
}

void Cregssc61860Widget::refresh()
{
    if (!pCPU) return;

    CSC61860 *cpu = (CSC61860 *) pCPU;
    ui->reg00->setText(QString("%1").arg(cpu->imem[0],2,16,QChar('0')));
    ui->reg01->setText(QString("%1").arg(cpu->imem[1],2,16,QChar('0')));
    ui->reg02->setText(QString("%1").arg(cpu->imem[2],2,16,QChar('0')));
    ui->reg03->setText(QString("%1").arg(cpu->imem[3],2,16,QChar('0')));
    ui->reg04->setText(QString("%1").arg(cpu->imem[4],2,16,QChar('0')));
    ui->reg05->setText(QString("%1").arg(cpu->imem[5],2,16,QChar('0')));
    ui->reg06->setText(QString("%1").arg(cpu->imem[6],2,16,QChar('0')));
    ui->reg07->setText(QString("%1").arg(cpu->imem[7],2,16,QChar('0')));
    ui->reg08->setText(QString("%1").arg(cpu->imem[8],2,16,QChar('0')));
    ui->reg09->setText(QString("%1").arg(cpu->imem[9],2,16,QChar('0')));

    ui->regP->setText(QString("%1").arg(cpu->get_reg(REG_P),2,16,QChar('0')));
    ui->regQ->setText(QString("%1").arg(cpu->get_reg(REG_Q),2,16,QChar('0')));
    ui->regR->setText(QString("%1").arg(cpu->get_reg(REG_R),2,16,QChar('0')));

    ui->regDP->setText(QString("%1").arg(cpu->get_reg(REG_DP),4,16,QChar('0')));
    ui->regPC->setText(QString("%1").arg(cpu->get_reg(REG_PC),4,16,QChar('0')));

    ui->regC->setText(cpu->get_reg(REG_C)?"1":".");
    ui->regZ->setText(cpu->get_reg(REG_Z)?"1":".");

    ui->reginP->setText(QString("%1").arg(cpu->imem[cpu->get_reg(REG_P)],2,16,QChar('0')));
    ui->reginQ->setText(QString("%1").arg(cpu->imem[cpu->get_reg(REG_Q)],2,16,QChar('0')));
    ui->reginR->setText(QString("%1").arg(cpu->imem[cpu->get_reg(REG_R)],2,16,QChar('0')));

    update();
}

void Cregssc61860Widget::updateReg() {

    if (!pCPU) return;
    CSC61860 *cpu = (CSC61860 *) pCPU;

    cpu->imem[0] = ui->reg00->text().toUInt(0,16);
    cpu->imem[1] = ui->reg01->text().toUInt(0,16);
    cpu->imem[2] = ui->reg02->text().toUInt(0,16);
    cpu->imem[3] = ui->reg03->text().toUInt(0,16);
    cpu->imem[4] = ui->reg04->text().toUInt(0,16);
    cpu->imem[5] = ui->reg05->text().toUInt(0,16);
    cpu->imem[6] = ui->reg06->text().toUInt(0,16);
    cpu->imem[7] = ui->reg07->text().toUInt(0,16);
    cpu->imem[8] = ui->reg08->text().toUInt(0,16);
    cpu->imem[9] = ui->reg09->text().toUInt(0,16);

    cpu->set_reg(REG_P,ui->regP->text().toUInt(0,16));
    cpu->set_reg(REG_Q,ui->regQ->text().toUInt(0,16));
    cpu->set_reg(REG_R,ui->regR->text().toUInt(0,16));

    cpu->set_reg(REG_DP,ui->regDP->text().toUInt(0,16));
    cpu->set_reg(REG_PC,ui->regPC->text().toUInt(0,16));

    cpu->imem[cpu->get_reg(REG_P)] =  ui->reginP->text().toUInt(0,16);
    cpu->imem[cpu->get_reg(REG_Q)] =  ui->reginQ->text().toUInt(0,16);
    cpu->imem[cpu->get_reg(REG_R)] =  ui->reginR->text().toUInt(0,16);

}
