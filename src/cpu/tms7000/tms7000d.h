#ifndef TMS7000D_H
#define TMS7000D_H

#include "Debug.h"



class Cdebug_tms7000:public Cdebug{
    Q_OBJECT
public:

    quint32 DisAsm_1(quint32 adr);			//disasm 1 line to Buffer

    Cdebug_tms7000(CCPU *parent);

    quint16 Dasm_tms7000(char *buffer, quint16 pc, const quint8 *oprom, const quint8 *opram);

};

#endif // TMS7000D_H
