#ifndef _PC1402_H_
#define _PC1402_H_

#include "pc1401.h"

class Cpc1402:public Cpc1401{						//PC1360 emulator main class
Q_OBJECT
public:
    const char*	GetClassName(){ return("Cpc1402");}


    virtual bool	Chk_Adr(UINT32 *d,UINT32 data);
    virtual bool	Chk_Adr_R(UINT32 *d, UINT32 *data);


    Cpc1402(CPObject *parent = 0);

};
#endif
