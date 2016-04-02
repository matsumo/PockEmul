#ifndef _PC1450_H_
#define _PC1450_H_

#include <stdlib.h>
#include <stdio.h>


#include "pc1350.h"



class Cpc1450:public Cpc1350{						//PC1350 emulator main class
Q_OBJECT
public:
    const char*	GetClassName(){ return("Cpc1450");}


    virtual void	initExtension(void);
    virtual BYTE	Get_PortA(void);

    virtual bool	Chk_Adr(UINT32 *d,UINT32 data);
    virtual bool	Chk_Adr_R(UINT32 *d, UINT32 *data);
    UINT8	in(UINT8 adr,QString sender=QString()){ Q_UNUSED(adr) Q_UNUSED(sender); return(1);}
    UINT8 out(UINT8 adr,UINT8 val,QString sender=QString()){ Q_UNUSED(adr) Q_UNUSED(val) Q_UNUSED(sender); return(1);}
	

    Cpc1450(CPObject *parent = 0);

    virtual ~Cpc1450()
	{								//[constructor]
	}
private:
};

#endif
