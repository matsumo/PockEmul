#ifndef _PC1260_H_
#define _PC1260_H_

#include "pc1250.h"

class Cpc1260:public Cpc1250{
public:
    const char*	GetClassName(){ return("Cpc1260");}

    virtual BYTE	Get_PortB(void);

    virtual bool	Chk_Adr(UINT32 *d,UINT32 data);

    Cpc1260(CPObject *parent = 0);

private:
};

class Cpc1261:public Cpc1260{
public:
    const char*	GetClassName(){ return("Cpc1261");}
	bool	Chk_Adr(UINT32 *d,UINT32 data);

    Cpc1261(CPObject *parent = 0);

private:
};

class Cpc1262:public Cpc1261{						//PC1360 emulator main class
public:
    const char*	GetClassName(){ return("Cpc1262");}


    Cpc1262(CPObject *parent = 0);

private:
};
#endif
