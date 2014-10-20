#ifndef FP100_H
#define FP100_H

#include "ce515p.h"

class DialogConsole;

class Cfp100:public Cce515p{
Q_OBJECT
public:

    virtual bool run(void);
    virtual bool init(void);				//initialize
    virtual bool exit(void);				//end
    //virtual void ExportByte(qint8);
    virtual void ComputeKey(KEYEVENT ke = KEY_PRESSED,int scancode=0);
    virtual bool UpdateFinalImage(void);
    virtual bool InitDisplay(void);

    Cconnector *pCONNECTOR; qint64 pCONNECTOR_value;
    Cconnector *pSavedCONNECTOR;


    Cfp100(CPObject *parent = 0);
    virtual ~Cfp100();

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
    QImage *capot;
    QImage *head;
    QImage *cable;

};



#endif // FP100_H
