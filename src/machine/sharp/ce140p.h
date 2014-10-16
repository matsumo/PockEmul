#ifndef CE140P_H
#define CE140P_H



#include "ce515p.h"

class DialogConsole;
class Csio;

class Cce140p:public Cce515p{
Q_OBJECT
public:

    virtual bool run(void);
    virtual bool init(void);				//initialize
    virtual bool exit(void);				//end
    //virtual void ExportByte(qint8);
    virtual void ComputeKey(void);
    virtual bool UpdateFinalImage(void);

    Csio *pSIO;
    Cconnector *pSIOCONNECTOR; qint64 pSIOCONNECTOR_value;
    Cconnector *pSIOCONNECTOR_OUT; qint64 pSIOCONNECTOR_OUT_value;


    Cce140p(CPObject *parent = 0);
    virtual ~Cce140p();

    virtual void BuildContextMenu(QMenu *menu);
protected:
    void paintEvent(QPaintEvent *);
protected slots:

    void CommandSlot(qint8);
//    void ShowConsole(void);
//    void HideConsole(void);
private:
    bool    printerSwitch;
};

#endif // CE140P_H
