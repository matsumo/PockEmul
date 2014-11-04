#ifndef PC1253_H
#define PC1253_H

#include "pc1251.h"

class Cpc1253:public Cpc1251{						//PC1360 emulator main class
public:
    const char*	GetClassName(){ return("Cpc1253");}

    virtual BYTE	Get_PortA(void);

    Cpc1253(CPObject *parent = 0);


    virtual bool Chk_Adr(UINT32 *d, UINT32 data);
    virtual bool Chk_Adr_R(UINT32 *d, UINT32 *data);
    virtual bool UpdateFinalImage();
private:
};

#endif // PC1253_H
