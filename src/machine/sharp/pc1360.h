#ifndef _PC1360_H_
#define _PC1360_H_

#include <stdlib.h>
#include <stdio.h>

#include "pc1350.h"
#include "Keyb.h"
#include "Connect.h"




class Cpc1360:public Cpc13XX{						//PC1360 emulator main class
Q_OBJECT
public:
    const char*	GetClassName(){ return("Cpc1360");}

    virtual BYTE	Get_PortA(void);
    virtual BYTE	Get_PortB(void);
//	virtual void	Set_PortB(BYTE data);
    virtual void	Set_PortF(BYTE data);
    virtual void	Set_PortC(BYTE data);
	virtual bool	Chk_Adr(UINT32 *d,UINT32 data);
    virtual bool	Chk_Adr_R(UINT32 *d, UINT32 *data);

	virtual bool	Set_Connector(void);
	virtual bool	Get_Connector(void);

    virtual void	initExtension(void);

    virtual bool	init(void);
    virtual bool	run(void);

    Cconnector	*pS2CONNECTOR;		qint64 pS2CONNECTOR_value;
    Cbus *busS2;

    Cpc1360(CPObject *parent = 0);
    virtual ~Cpc1360();

private:

};
#endif
