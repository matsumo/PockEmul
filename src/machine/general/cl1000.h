#ifndef CL1000_H
#define CL1000_H

#include "ce515p.h"

class DialogConsole;

class Ccl1000:public Cce515p{
Q_OBJECT
public:

    virtual bool run(void);
    virtual bool init(void);				//initialize
    virtual bool exit(void);				//end
    //virtual void ExportByte(qint8);
    virtual void ComputeKey(void);
    virtual bool UpdateFinalImage(void);

    Cconnector *pCONNECTOR; qint64 pCONNECTOR_value;
    Cconnector *pSavedCONNECTOR;


    Ccl1000(CPObject *parent = 0);
    virtual ~Ccl1000();

    bool GoDown(int pin);
    bool GoUp(int pin);
    bool Change(int pin);
protected:
    void paintEvent(QPaintEvent *);
protected slots:
    void contextMenuEvent ( QContextMenuEvent * );
    void CommandSlot(qint8);
private:

    bool printerACK;
    bool printerBUSY;

};



#endif // CL1000_H
