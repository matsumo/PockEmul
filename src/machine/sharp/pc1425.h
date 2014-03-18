#ifndef PC1425_H
#define PC1425_H

#include "pc1403.h"

class Cpc1425:public Cpc1403{
Q_OBJECT
public:
    const char*	GetClassName(){ return("Cpc1425");}


    virtual bool Chk_Adr(UINT32 *d,UINT32 data);
    virtual bool Chk_Adr_R(UINT32 *d, UINT32 *data);

    Cpc1425(CPObject *parent=0);

    ~Cpc1425()
    {								//[constructor]
    }

private:
};

#endif // PC1425_H
