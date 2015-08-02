#ifndef TMC0501_DASM_H
#define TMC0501_DASM_H

#include "Debug.h"

#include "tmc0501.h"


class Cdebug_tmc0501:public Cdebug{
    Q_OBJECT
public:
    UINT32 DisAsm_1(UINT32 adr);			//disasm 1 line to Buffer

        Cdebug_tmc0501(CCPU *parent);
        virtual ~Cdebug_tmc0501(){}


        void disasm(unsigned addr, unsigned opcode);
};



#endif // TMC0501_DASM

