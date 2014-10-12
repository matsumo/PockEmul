#ifndef _PC1500_H
#define _PC1500_H

#include <stdlib.h>
#include <stdio.h>

#include <QMenu>
#include <QMutex>

#include "common.h"

#include "pcxxxx.h"
#include "lh5801.h"
#include "lh5810.h"
#include "pd1990ac.h"
#include "Keyb.h"


class CPD1990AC;
class CbusPc1500;

class CLH5810_PC1500:public CLH5810{
    Q_OBJECT
public:
	bool	init(void);						//initialize
	bool	step(void);
    const char*	GetClassName(){ return("CLH5810_PC1500");}
	CPD1990AC	*pPD1990AC;

	CLH5810_PC1500(CPObject *parent)	: CLH5810(parent)
	{
		pPD1990AC	= new CPD1990AC(parent);
    }
	~CLH5810_PC1500()
	{
		delete pPD1990AC;
    }


};


class Cpc15XX:public CpcXXXX{

Q_OBJECT


public:
    const char*	GetClassName(){ return("Cpc15XX");}

    bool	LoadConfig(QXmlStreamReader *);
    bool	SaveConfig(QXmlStreamWriter *);
	bool	InitDisplay(void);
	bool	CompleteDisplay(void);
    virtual void ComputeKey();

    virtual bool	run(void);				// emulator main
	void	Set_Port(PORTS Port,BYTE data);
	BYTE	Get_Port(PORTS Port);

	virtual bool	Mem_Mirror(UINT32 *d); 
    virtual void	TurnON(void);

	void	Regs_Info(UINT8 Type);

    bool		lh5810_write(UINT32 d, UINT32 data);
    quint8		lh5810_read(UINT32 d);

	virtual bool		Chk_Adr(UINT32 *d,UINT32 data);
    virtual bool		Chk_Adr_R(UINT32 *d, UINT32 *data);
    UINT8		in(UINT8 address);
    UINT8 out(UINT8 ,UINT8 ){return(1);}
    virtual bool Set_Connector(void);
    virtual bool Get_Connector(void);

    CLH5810_PC1500	*pLH5810;
	
    bool		lh5810_Access;

	void		InitCE150(void);
	
	void		initExtension(void);
	bool	init(void);				// initialize
	
	Cpc15XX(CPObject *parent = 0);

    virtual ~Cpc15XX();


protected slots:
	void contextMenuEvent ( QContextMenuEvent * event );
};

class Cpc1500:public Cpc15XX{						//PC1500 emulator main class
public:
    const char*	GetClassName(){ return("Cpc1500");}

	Cpc1500(CPObject *parent = 0)	: Cpc15XX(this)
	{								//[constructor]
        Q_UNUSED(parent)

        setcfgfname(QString("pc1500"));

		SlotList.clear();
		SlotList.append(CSlot(8 , 0x0000 ,	""								, "" , CSlot::RAM , "RAM"));
		SlotList.append(CSlot(8 , 0x2000 ,	""								, "" , CSlot::ROM , "ROM"));
		SlotList.append(CSlot(16, 0x4000 ,	""								, "" , CSlot::RAM , "RAM"));
        SlotList.append(CSlot(8 , 0x8000 ,	""								, "" , CSlot::NOT_USED , "NOT USED"));
		SlotList.append(CSlot(8 , 0xA000 ,	""								, "" , CSlot::ROM , "ROM"));
        SlotList.append(CSlot(16, 0xC000 ,	P_RES(":/pc1500/SYS1500.ROM")	, "" , CSlot::ROM , "SYSTEM ROM"));
//		SlotList.append(CSlot(64, 0x10000 ,	""								, "" , CSlot::RAM , "RAM"));
//		SlotList.append(CSlot(8 , 0x20000 ,	""								, "" , CSlot::ROM , "ROM"));
//		SlotList.append(CSlot(8 , 0x22000 ,	""								, "" , CSlot::ROM , "ROM"));
//        SlotList.append(CSlot(8 , 0x24000 ,	P_RES(":/pc1500/CE-150.ROM"), "" , CSlot::ROM , "CE-150 ROM"));
	}

	~Cpc1500()
	{								//[constructor]
	}
private:

};

class Cpc1500A:public Cpc15XX{						//PC1500 emulator main class
public:
    const char*	GetClassName(){ return("Cpc1500A");}


    virtual bool Chk_Adr(UINT32 *d,UINT32 data);
    virtual bool Chk_Adr_R(UINT32 *d, UINT32 *data);
    virtual bool Mem_Mirror(UINT32 *d);

    Cpc1500A(CPObject *parent = 0);

	~Cpc1500A()
	{								//[constructor]
	}
private:

};

class Ctrspc2:public Cpc1500{
public:
    const char*	GetClassName(){ return("Ctrspc2");}

    Ctrspc2(CPObject *parent = 0);

	~Ctrspc2()
	{								//[constructor]
	}
};


#endif
