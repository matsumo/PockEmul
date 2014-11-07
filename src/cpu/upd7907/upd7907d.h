#ifndef UPD7907D_H
#define UPD7907D_H

#include "Debug.h"


class Cdebug_upd7810:public Cdebug{
    Q_OBJECT
public:
    struct dasm_s {
        quint8 token;
        const void *args;
    };


    quint32 DisAsm_1(quint32 adr);			//disasm 1 line to Buffer

        Cdebug_upd7810(CCPU *parent);

        quint16 Dasm_upd7810(char *buffer, quint16 pc, const dasm_s *dasmXX, const quint8 *oprom, const quint8 *opram, int is_7810);
};

#endif // UPD7907D_H
