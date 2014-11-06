#ifndef UPD7907D_H
#define UPD7907D_H

#include "Debug.h"


class Cdebug_upd7810:public Cdebug{
    Q_OBJECT
public:
    struct dasm_s {
        UINT8 token;
        const void *args;
    };


    UINT32 DisAsm_1(UINT32 adr);			//disasm 1 line to Buffer

        Cdebug_upd7810(CCPU *parent);

        quint16 Dasm_upd7810(char *buffer, quint16 pc, const dasm_s *dasmXX, const UINT8 *oprom, const UINT8 *opram, int is_7810);
};

#endif // UPD7907D_H
