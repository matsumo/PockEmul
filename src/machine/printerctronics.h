#ifndef PRINTERCTRONICS_H
#define PRINTERCTRONICS_H


#include <QTime>
#ifndef NO_SOUND
#include <QSound>
#endif
#include "common.h"
#include "cprinter.h"

#include "Inter.h"

class Cconnector;

class CprinterCtronics:public Cprinter{
    Q_OBJECT
public:

    virtual bool init(void);				//initialize
    virtual bool exit(void);				//end
    virtual	bool run(void);					//check access

    virtual bool Set_Connector(void);
    virtual bool Get_Connector(void);

    virtual void clearPaper(void);
    virtual void SaveAsText(void);

    virtual void ComputeKey(void);
    virtual bool UpdateFinalImage(void);

    virtual void Printer(quint8 d);			//printer emulator
    //virtual void resizeEvent ( QResizeEvent * );

    Cconnector *pCONNECTOR;		qint64 pCONNECTOR_value;
    Cconnector *pSavedCONNECTOR;

    QImage *printerbuf;
    QImage *printerdisplay;

    void	settop(int value){ top = value; }
    void	setposX(int value) { posX = value; }


    CprinterCtronics(CPObject *parent = 0);

    virtual ~CprinterCtronics();

    bool GoDown(int pin);
    bool GoUp(int pin);
    bool Change(int pin);

public:
    bool printerACK;
    bool printerBUSY;

    QImage *charTable;
    int margin;
    int	top;

    int	posX;
    int paperWidth;
    int charsize;

private:

    bool	ctrl_char;
    BYTE	t,c;
    qint64	run_oldstate;
    qint64  lastState;

public:

};

#endif // PRINTERCTRONICS_H
