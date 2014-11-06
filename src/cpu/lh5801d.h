#ifndef LH5801D_H
#define LH5801D_H

#include "Debug.h"

class Parser;

class Cdebug_lh5801:public Cdebug{
    Q_OBJECT
public:
    UINT32 DisAsm_1(UINT32 adr);			//disasm 1 line to Buffer
    virtual void loadSymbolMap();

    virtual void injectReg(Parser *p);
    Cdebug_lh5801(CCPU *parent);


};

#endif // LH5801D_H
