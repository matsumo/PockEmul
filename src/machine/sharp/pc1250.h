#ifndef _PC1250_H
#define _PC1250_H

#include <stdlib.h>
#include <stdio.h>

#include <QImage>


#include "pcxxxx.h"




// WARNING: -- TO MODIFY !!!!!!!!!!!!!
//#define KEY(c)	( asfocus && ( GetAsyncKeyState(c) || (TOUPPER(pPC->pKEYB->LastKey)==TOUPPER(c)) || pPC->pKEYB->LastKey == c) )
#define KEY(c)	( TOUPPER(pKEYB->LastKey) == TOUPPER(c) )

class Cpc1250:public CpcXXXX{						//PC1250 emulator main class

public:
    const char*	GetClassName(){ return("Cpc1250");}

	void	TurnON(void);

    virtual bool	run(void);
    virtual bool init(void);
    virtual bool	Set_Connector(Cbus *_bus = 0);
    virtual bool	Get_Connector(Cbus *_bus = 0);
    virtual BYTE	Get_PortA(void);
	virtual BYTE	Get_PortB(void);

    virtual bool	InitDisplay(void);
    //bool	CompleteDisplay(void);
    virtual bool UpdateFinalImage(void);

	bool	Mem_Mirror(UINT32 *d);
    virtual bool	Chk_Adr(UINT32 *d,UINT32 data);
    virtual bool	Chk_Adr_R(UINT32 *d, UINT32 *data);
    UINT8 in(UINT8 ,QString sender=QString()){return(1);}
    UINT8 out(UINT8 ,UINT8,QString sender=QString() ){return(1);}


	virtual bool	LoadExtra(QFile *);
	virtual bool	SaveExtra(QFile *);

	Cpc1250(CPObject *parent = 0);

	virtual ~Cpc1250()
	{								//[constructor]
    }

protected:
	BYTE previous_key;

    QImage iPowerOFF;
    QImage iPowerRUN;
    QImage iPowerPRO;
    QImage iPowerRSV;
};



class Ctrspc3:public Cpc1250{						//PC1250 emulator main class
public:

    Ctrspc3(CPObject *parent = 0);


};

#endif


