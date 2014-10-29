/********************************************************************************************************
 * PROGRAM      : test
 * DATE - TIME  : samedi 28 octobre 2006 - 12h40
 * AUTHOR       :  (  )
 * FILENAME     : Ce126.h
 * LICENSE      : GPL
 * COMMENTARY   : printer(CE-126P) emulation class
 ********************************************************************************************************/
#ifndef _CE126_H
#define _CE126_H

#include <QTime>
#ifndef NO_SOUND
#include <QSound>
#endif

#include "common.h"
#include "cprinter.h"

#include "Inter.h"

class Cconnector;

class Cce126:public Cprinter{
public:

	virtual bool init(void);				//initialize
	virtual bool exit(void);				//end
	virtual	bool run(void);					//check access

    virtual bool	Set_Connector(Cbus *_bus = 0);
    virtual bool	Get_Connector(Cbus *_bus = 0);

	virtual void clearPaper(void);
	virtual void SaveAsText(void);

    virtual void ComputeKey(KEYEVENT ke = KEY_PRESSED,int scancode=0);

    //virtual void resizeEvent ( QResizeEvent * );
	
	Cconnector	*pCONNECTOR;		qint64 pCONNECTOR_value;
	Cconnector	*pTAPECONNECTOR;	qint64 pTAPECONNECTOR_value;
	
	QImage *ce126buf;
	QImage *ce126display;
	
	void RefreshCe126(qint8 data);

#ifndef NO_SOUND
	QSound *bells;
#endif
	
	bool	ToDestroy;
		
    void	settop(int value){ top = value; }
    void	setposX(int value) { posX = value; }
int ce126_Mode;
					
    Cce126(CPObject *parent = 0);
	
    virtual ~Cce126();

public:
    bool    rmtSwitch;
    int     internal_device_code;
    void	Printer(qint8 d);			//printer emulator
    QImage *charTable;
    int margin;
    int		top;

private:

	void	pulldownsignal(void);
	
	int		posX;

    bool	Previous_BUSY;
    bool	Previous_MT_OUT1;
	QTime	time;
	bool	ctrl_char;
	BYTE	t,c;
    quint64	run_oldstate;
    int     code_transfer_step;
    int     device_code;
    quint64  lastState;


public:
    bool MT_OUT2;
    bool BUSY;
    bool D_OUT;
    bool MT_IN;
    bool MT_OUT1;
    bool D_IN;
    bool ACK;
    bool SEL2;
    bool SEL1;

};



class Cce123:public Cce126
{
public:
    Cce123();
    virtual bool init();
};

class Cce129:public Cce126
{
public:
    Cce129();
    virtual bool init();
};

class C263591:public Cce126
{
public:

    C263591();
    virtual bool init();

private:
};

#endif
