#ifndef SC62015D_H
#define SC62015D_H

#include "Debug.h"


class Cdebug_sc62015:public Cdebug{
    Q_OBJECT
public:
    quint32 DisAsm_1(quint32 adr);			//disasm 1 line to Buffer
    virtual void injectReg(Parser *p);

        Cdebug_sc62015(CCPU *parent);
        virtual ~Cdebug_sc62015(){}

};


#endif // SC62015D_H
