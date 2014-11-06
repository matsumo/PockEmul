#ifndef Z80_DASM_H
#define Z80_DASM_H

#include "Debug.h"



class Cdebug_z80:public Cdebug{
    Q_OBJECT
public:
    UINT32 DisAsm_1(UINT32 adr);			//disasm 1 line to Buffer

        Cdebug_z80(CCPU *parent);

};

#endif // Z80_DASM_H
