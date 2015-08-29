#ifndef M160_H
#define M160_H


#include <QTime>
#ifndef NO_SOUND
#include <QSound>
#endif

#include "common.h"
#include "cprinter.h"

#include "Inter.h"

class Cconnector;

class Cm160:public Cprinter{
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

    QImage *m160buf;
    QImage *m160display;

    void Refreshm160(qint8 data);
#ifndef NO_SOUND
    QSound *bells;
#endif

    bool	ToDestroy;

    void	settop(int value){ top = value; }
    void	setposX(int value) { posX = value; }
int pc2021_Mode;

    Cm160(CPObject *parent = 0);

    virtual ~Cm160();

public:
    bool    rmtSwitch;
    int     internal_device_code;
    void	Printer(qint8 d);			//printer emulator
    QImage *charTable;
    int margin;
    int		top;

private:

    bool H1,H2,H3,H4,Motor,MotorBreak,TS,RS;
    int TPIndex;


    int		posX;

    bool	ctrl_char;
    BYTE	t,c;
    quint64	run_oldstate;
    quint64  lastState;


public:

};



#endif // M160_H

