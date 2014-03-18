#ifndef TP83_H
#define TP83_H

#include "ce515p.h"

class DialogConsole;

class Ctp83:public Cce515p{
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


    Ctp83(CPObject *parent = 0);
    virtual ~Ctp83();

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




#endif // TP83_H
