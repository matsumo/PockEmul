#ifndef CX710_H
#define CX710_H




#include "ce515p.h"

class DialogConsole;

class Cx710:public Cce515p{
Q_OBJECT
public:

    virtual bool run(void);
    virtual bool init(void);				//initialize
    virtual bool exit(void);				//end
    //virtual void ExportByte(qint8);
    virtual void ComputeKey(void);
//    virtual bool UpdateFinalImage(void);

    Cconnector *pCONNECTOR; qint64 pCONNECTOR_value;
    Cconnector *pSavedCONNECTOR;


    Cx710(CPObject *parent = 0);
    virtual ~Cx710();

    bool GoDown(int pin);
    bool GoUp(int pin);
    bool Change(int pin);
protected:
//    void paintEvent(QPaintEvent *);
protected slots:
    void contextMenuEvent ( QContextMenuEvent * );
    void CommandSlot(qint8);
private:

};


#endif // CX710_H
