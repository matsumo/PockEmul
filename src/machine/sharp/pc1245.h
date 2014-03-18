#ifndef _PC1245_H_
#define _PC1245_H_

#include "pc1250.h"

class Cpc1245:public Cpc1250{
public:
    const char*	GetClassName(){ return("Cpc1245");}
	BYTE	Get_PortB(void);
    virtual bool UpdateFinalImage(void);
    virtual bool InitDisplay(void);

	Cpc1245(CPObject *parent = 0);

private:
};

class Cmc2200:public Cpc1245{
public:
    const char*	GetClassName(){ return("Cmc2200");}

    Cmc2200(CPObject *parent = 0);

private:
};
#endif
