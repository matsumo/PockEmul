#ifndef _PC1401_H
#define _PC1401_H

#include <stdlib.h>
#include <stdio.h>


#include "pcxxxx.h"


class Cpc1401:public CpcXXXX{						//PC1403 emulator main class
Q_OBJECT
public:
    const char*	GetClassName(){ return("Cpc1401");}

	void	TurnON(void);

    virtual bool    init(void);
	virtual bool	Set_Connector(void);
	virtual bool	Get_Connector(void);
    virtual BYTE	Get_PortA(void);
    virtual void	Set_PortA(BYTE data);
    virtual void	Set_PortB(BYTE data);

    virtual bool	Chk_Adr(UINT32 *d,UINT32 data);
    virtual bool	Chk_Adr_R(UINT32 *d, UINT32 *data);
    virtual UINT8   in(UINT8 ){return(1);}
    virtual UINT8   out(UINT8 ,UINT8 ){return(1);}
	
	virtual bool CheckUpdateExtension(CExtension *ext);
			
    Cpc1401(CPObject *parent = 0);

    virtual ~Cpc1401()
	{								//[constructor]
	}
private:
    BYTE cnt;
};




#endif
