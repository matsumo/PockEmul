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
    virtual void ComputeKey(KEYEVENT ke = KEY_PRESSED,int scancode=0,QMouseEvent *event=0);
	
    Cce125tape(CPObject *parent = 0);

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
    virtual void ComputeKey(KEYEVENT ke = KEY_PRESSED,int scancode=0,QMouseEvent *event=0);

    Cce125(CPObject *parent = 0);
    virtual ~Cce125();
	
private:
    Cce125tape Tape;
};

class Cmp220:public Cce125
{
public:

    Cmp220(CPObject *parent = 0);

};



#endif
