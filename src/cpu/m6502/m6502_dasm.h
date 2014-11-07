#ifndef M6502_DASM_H
#define M6502_DASM_H

#include "Debug.h"


class Cdebug_m6502:public Cdebug{
    Q_OBJECT
public:
    quint32 DisAsm_1(quint32 adr);			//disasm 1 line to Buffer

        Cdebug_m6502(CCPU *parent);
        virtual ~Cdebug_m6502(){}

        int DasmOpe(char *S,quint8 *A,unsigned long PC);
};

#endif // M6502_DASM_H
