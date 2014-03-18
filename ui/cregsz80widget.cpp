#include "cregsz80widget.h"
#include "ui_cregsz80widget.h"
#include "ui/cregcpu.h"
#include "cpu.h"
#include "z80.h"

Cregsz80Widget::Cregsz80Widget(CPObject *parent,CCPU *pCPU) :
    CregCPU(parent,pCPU),
    ui(new Ui::Cregsz80Widget)
{
    ui->setupUi(this);
}

Cregsz80Widget::~Cregsz80Widget()
{
    delete ui;
}

void Cregsz80Widget::refresh()
{
    if (!pCPU) return;

//    CZ80 *cpu = (CZ80 *) pCPU;
    CCPU *cpu =  pCPU;

    cpu->Regs_Info(0);
    ui->regsTextEdit->setText(QString(cpu->Regs_String));
    update();
}
