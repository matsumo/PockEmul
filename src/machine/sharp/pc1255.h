
#ifndef _PC1255_H_
#define _PC1255_H_

#include "pc1250.h"

class Cpc1255:public Cpc1250{
public:
    const char*	GetClassName(){ return("Cpc1255");}


    virtual bool	Chk_Adr(UINT32 *d,UINT32 data);


    Cpc1255(CPObject *parent = 0);

private:
};

class Cpc1251H:public Cpc1250{
public:
    const char*	GetClassName(){ return("Cpc1251H");}

    virtual bool	Chk_Adr(UINT32 *d,UINT32 data);


    Cpc1251H(CPObject *parent = 0);

private:
};


#endif
