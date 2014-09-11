#ifndef I8085_DASM_H
#define I8085_DASM_H

#include "Debug.h"
#include "i8085.h"

class Cdebug_i8085:public Cdebug{
    Q_OBJECT
public:
    UINT32 DisAsm_1(UINT32 adr);			//disasm 1 line to Buffer

        Cdebug_i8085(CPObject *parent)	: Cdebug(parent)
        {
            AsmTbl = AsmTbl_sc61860;
            i8085 = (Ci8085*)(pPC->pCPU);
        }
        Ci8085 *i8085;

        char *i85regs(char *buf, const I85stat *i85);

        static const char   *Op_Code[256];
        static char         Op_Code_Size[256];
};

#endif // I8085_DASM_H
