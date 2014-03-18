#ifndef _CE125_H_
#define _CE125_H_

#include "common.h"
#include "Ce126.h"
#include "ce152.h"

extern TransMap KeyMapce125tape[];
extern int KeyMapce125tapeLenght;

class Cce125tape:public Cce152
{
public:
	virtual void ComputeKey(void);
	
	Cce125tape(CPObject *parent = 0)	: Cce152(parent)
	{									//[constructor]
        BackGroundFname	= P_RES(":/ext/ce-125tape.png");
        playImage = P_RES(":/ext/ce-125play.png");
        recordImage=P_RES(":/ext/ce-125record.png");
        loadImage = P_RES(":/ext/ce-125load.png");
        setDX(249);
        setDY(299);
		KeyMap		= KeyMapce125tape;
		KeyMapLenght= KeyMapce125tapeLenght;
		pKEYB		= new Ckeyb(this,"ce125tape.map");
    }

    QString playImage;
    QString recordImage;
    QString loadImage;

};

class Cce125:public Cce126
{
public:
	virtual bool init(void);				//initialize
	virtual bool exit(void);				//end
	virtual	bool run(void);					//check access
    virtual void resizeEvent ( QResizeEvent * );
    virtual bool UpdateFinalImage(void);

    Cce125(CPObject *parent = 0);
    virtual ~Cce125();
	
private:
	Cce125tape* pTAPE;
};

class Cmp220:public Cce125
{
public:

    Cmp220(CPObject *parent = 0);

};



#endif
