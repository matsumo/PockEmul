#ifndef SC61860D_H
#define SC61860D_H

#include "Debug.h"

class Cdebug_sc61860:public Cdebug{
    Q_OBJECT
public:
    UINT32 DisAsm_1(UINT32 adr);			//disasm 1 line to Buffer
    virtual void injectReg(Parser *p);

        Cdebug_sc61860(CCPU *parent);
        virtual ~Cdebug_sc61860(){}

};

#endif // SC61860D_H
