#include "cregshd61700widget.h"
#include "ui_cregshd61700widget.h"
#include "ui/cregcpu.h"
#include "cpu.h"

Cregshd61700Widget::Cregshd61700Widget(CPObject *parent,CCPU *pCPU) :
    CregCPU(parent,pCPU),
    ui(new Ui::Cregshd61700Widget)
{
    ui->setupUi(this);
}

Cregshd61700Widget::~Cregshd61700Widget()
{
    delete ui;
}

void Cregshd61700Widget::refresh()
{
    if (!pCPU) return;

//    CZ80 *cpu = (CZ80 *) pCPU;
    CCPU *cpu =  pCPU;

    cpu->Regs_Info(0);
    ui->regsTextEdit->setText(QString(cpu->Regs_String));
    update();

}
