#ifndef MC6800D_H
#define MC6800D_H


#include "Debug.h"


class Cdebug_mc6800:public Cdebug{
    Q_OBJECT
public:
    quint32 DisAsm_1(quint32 pc);			//disasm 1 line to Buffer

        Cdebug_mc6800(CCPU *parent);
        virtual ~Cdebug_mc6800(){}

        int DasmOpe(char *S,quint8 *A,unsigned long PC);
};

#endif // MC6800D_H

