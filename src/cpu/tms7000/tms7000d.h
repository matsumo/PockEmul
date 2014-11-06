#ifndef TMS7000D_H
#define TMS7000D_H

#include "Debug.h"



class Cdebug_tms7000:public Cdebug{
    Q_OBJECT
public:

    UINT32 DisAsm_1(UINT32 adr);			//disasm 1 line to Buffer

    Cdebug_tms7000(CCPU *parent);

    quint16 Dasm_tms7000(char *buffer, quint16 pc, const UINT8 *oprom, const UINT8 *opram);

};

#endif // TMS7000D_H
