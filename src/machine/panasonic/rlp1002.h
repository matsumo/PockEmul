#ifndef RLP1002_H
#define RLP1002_H


#include <QTime>
#ifndef NO_SOUND
#include <QSound>
#endif

#include "common.h"
#include "ce515p.h"

#include "Inter.h"

class Cconnector;

class Crlp1002:public Cce515p{
    Q_OBJECT
public:

    virtual bool init(void);				//initialize
    virtual bool exit(void);				//end
    virtual	bool run(void);					//check access

    virtual bool	Set_Connector(Cbus *_bus = 0);
    virtual bool	Get_Connector(Cbus *_bus = 0);

    virtual void ComputeKey(KEYEVENT ke = KEY_PRESSED,int scancode=0);

    //virtual void resizeEvent ( QResizeEvent * );

    Cconnector	*pCONNECTOR;		qint64 pCONNECTOR_value;

    Crlp1002(CPObject *parent = 0);

    virtual ~Crlp1002();

public:

    int margin;
    int		top;
    bool INTrequest;
    bool INTEnabled;
protected:

protected slots:
    void Rotate();

private:


    bool rotate;

    int		posX;

    bool	ctrl_char;
    BYTE	t,c;
    quint64	run_oldstate;
    quint64  lastState;
    bool    tapeOutput,tapeInput;
    QByteArray buffer;
    bool printing;
    bool receiveMode;
    bool CRLFPending;


};


#endif // RLP1002_H
