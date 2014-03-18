
#ifndef _PC1251_H_
#define _PC1251_H_

#include "pc1250.h"

class Cpc1251:public Cpc1250{						//PC1360 emulator main class
public:
    const char*	GetClassName(){ return("Cpc1251");}


	bool	Chk_Adr(UINT32 *d,UINT32 data);
    bool	Chk_Adr_R(UINT32 *d, UINT32 *data);


    Cpc1251(CPObject *parent = 0);


private:
};

class Ctrspc3Ext:public Cpc1251{						//PC1250 emulator main class
public:

    Ctrspc3Ext(CPObject *parent = 0)	: Cpc1251(this)
    {								//[constructor]
        Q_UNUSED(parent)

        setcfgfname("trspc3ext");

        SessionHeader	= "TRSPC3EXTPKM";
        Initial_Session_Fname ="trspc3ext.pkm";

        BackGroundFname	= P_RES(":/pc1250/pc31k.png");
    }


};

#endif
