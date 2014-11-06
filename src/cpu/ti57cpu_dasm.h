#ifndef TI57CPU_DASM_H
#define TI57CPU_DASM_H

#include "Debug.h"

#include "ti57cpu.h"


class Cdebug_ti57cpu:public Cdebug{
    Q_OBJECT
public:
    UINT32 DisAsm_1(UINT32 adr);			//disasm 1 line to Buffer

        Cdebug_ti57cpu(CPObject *parent)	: Cdebug(parent)
        {
        }
        virtual ~Cdebug_ti57cpu(){}


        static QString IntToHex(int val, int nb = 3);
        QString BranchOP(TI57regs *r);
        QString CallOP(TI57regs *r);
        QString FlagOP(TI57regs *r);
        QString MaskOP(TI57regs *r);
        QString MiscOP(TI57regs *r);
        QString Decode(TI57regs *r);
        static QString Reg(TRegister R);
        void Analyze(TI57regs *r);
        void Disassemble(TI57regs *r);
        QString Debugging(TI57regs *r);
        QString Tracing(TI57regs *r);
};

#endif // TI57CPU_DASM_H
