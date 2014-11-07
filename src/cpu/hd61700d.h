#ifndef HD61700D_H
#define HD61700D_H

#include "Debug.h"

typedef quint32 UINT32;
typedef struct
{
    const char *str;
    quint8		arg1;
    quint8		arg2;
    bool		optjr;
} hd61700_dasm;

class Cdebug_hd61700:public Cdebug{
    Q_OBJECT
public:
    quint32 DisAsm_1(quint32 adr);			//disasm 1 line to Buffer

        Cdebug_hd61700(CCPU *parent);
        virtual ~Cdebug_hd61700(){}

        quint8 getMem(int);
        UINT32 get_dasmflags(quint8 op);
        int dasm_arg(char *buffer, quint8 op, quint16 pc, int arg, const quint8 *oprom, int &pos);
        int dasm_im8(char *buffer, quint16 pc, int arg, int arg1, const quint8 *oprom, int &pos);
        int dasm_im8(char *buffer, quint16 pc, int arg, const quint8 *oprom, int &pos, int type);

        static const hd61700_dasm hd61700_ops[256];
        static const char *const reg_5b[4];
        static const char *const reg_8b[8];
        static const char *const reg_16b[8];
        static const char *const jp_cond[8];


};

#endif // HD61700D_H
