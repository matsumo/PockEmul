#ifndef I80X86_DASM_H
#define I80X86_DASM_H

#include "Debug.h"
#include "i80x86.h"

class Cdebug_i80x86:public Cdebug{
    Q_OBJECT
public:
    UINT32 DisAsm_1(UINT32 adr);			//disasm 1 line to Buffer

        Cdebug_i80x86(CPObject *parent)	: Cdebug(parent)
        {
            AsmTbl = AsmTbl_sc61860;
            i80x86 = (Ci80x86*)(pPC->pCPU);
        }
        Ci80x86 *i80x86;

        int i86disasm(char *buf, const I86stat *i86, uint16 seg, uint16 off);
        char *s8(int8 val);
        char *s16(int16 val);
        void getsegoff(const I86stat *i86, uint8 rm, uint16 *seg, uint16 *off);
        char *rm8val(const I86stat *i86, uint8 rm);
        char *rm16val(const I86stat *i86, uint8 rm);
        char *i86regs(char *buf, const I86stat *i86);
};


#endif // I80X86_DASM_H
