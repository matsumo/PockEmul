#ifndef RLP2001_H
#define RLP2001_H


#include "pobject.h"

#include "mainwindowpockemul.h"

class MC6847;

class Crlp2001:public CPObject {
    Q_OBJECT
public:
    const char*	GetClassName(){ return("Crlp2001");}

    Cconnector	*pCONNECTOR;
    MC6847      *pMC6847;

    bool run(void);
    bool init(void);				//initialize
    bool exit(void);				//end
    virtual bool    SaveSession_File(QXmlStreamWriter *xmlOut);
    virtual bool	LoadSession_File(QXmlStreamReader *xmlIn);
    virtual bool	InitDisplay(void);
    virtual bool UpdateFinalImage(void);

    Crlp2001(CPObject *parent = 0);
    virtual ~Crlp2001();

    bool Power;

protected:
    void paintEvent(QPaintEvent *);
protected slots:
    void contextMenuEvent ( QContextMenuEvent * );
    void Rotate();

private:
    bool INTrequest;
    bool INTEnabled;
    bool rotate;
    quint8 controlReg;
    QImage *screen;

};


#endif // RLP2001_H
