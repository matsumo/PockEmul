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
