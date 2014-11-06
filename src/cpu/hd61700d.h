#ifndef HD61700D_H
#define HD61700D_H

#include "Debug.h"

typedef quint32 UINT32;
typedef struct
{
    const char *str;
    UINT8		arg1;
    UINT8		arg2;
    bool		optjr;
} hd61700_dasm;

class Cdebug_hd61700:public Cdebug{
    Q_OBJECT
public:
    UINT32 DisAsm_1(UINT32 adr);			//disasm 1 line to Buffer

        Cdebug_hd61700(CPObject *parent)	: Cdebug(parent)
        {
            AsmTbl = AsmTbl_sc61860;
        }
        virtual ~Cdebug_hd61700(){}

        UINT8 getMem(int);
        UINT32 get_dasmflags(UINT8 op);
        int dasm_arg(char *buffer, UINT8 op, UINT16 pc, int arg, const UINT8 *oprom, int &pos);
        int dasm_im8(char *buffer, UINT16 pc, int arg, int arg1, const UINT8 *oprom, int &pos);
        int dasm_im8(char *buffer, UINT16 pc, int arg, const UINT8 *oprom, int &pos, int type);

        static const hd61700_dasm hd61700_ops[256];
        static const char *const reg_5b[4];
        static const char *const reg_8b[8];
        static const char *const reg_16b[8];
        static const char *const jp_cond[8];


};

#endif // HD61700D_H
