#ifndef PC100_H
#define PC100_H


#include <QTime>
#ifndef NO_SOUND
#include <QSound>
#endif

#include "common.h"
#include "cprinter.h"

#include "Inter.h"

class Cconnector;

class Cpc100:public Cprinter{
public:

    virtual bool init(void);				//initialize
    virtual bool exit(void);				//end
    virtual	bool run(void);					//check access

    virtual bool	Set_Connector(Cbus *_bus = 0);
    virtual bool	Get_Connector(Cbus *_bus = 0);

    virtual void clearPaper(void);
    virtual void SaveAsText(void);

    virtual void ComputeKey(KEYEVENT ke = KEY_PRESSED,int scancode=0,QMouseEvent *event=0);

    //virtual void resizeEvent ( QResizeEvent * );

    Cconnector	*pCONNECTOR;		qint64 pCONNECTOR_value;

    QImage *pc100buf;
    QImage *pc100display;

    void Refreshpc100(quint8 data);
#ifndef NO_SOUND
    QSound *bells;
#endif

    bool	ToDestroy;

    void	settop(int value){ top = value; }
    void	setposX(int value) { posX = value; }
int pc100_Mode;

    Cpc100(CPObject *parent = 0);

    virtual ~Cpc100();

public:
    void	Printer(qint8 d);			//printer emulator
    QImage *charTable;
    int margin;
    int		top;

private:




    int		posX;

    bool	ctrl_char;
    BYTE	t,c;
    quint64	run_oldstate;
    quint64  lastState;

    QByteArray lineBuffer;
    bool print,trace,adv;

public:

    void drawChar(quint8 code);
};




#endif // PC100_H

