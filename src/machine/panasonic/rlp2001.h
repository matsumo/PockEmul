#ifndef RLP2001_H
#define RLP2001_H


#include "pobject.h"

#include "mainwindowpockemul.h"

class Crlp2001:public CPObject {
    Q_OBJECT
public:
    const char*	GetClassName(){ return("Crlp2001");}

    Cconnector	*pCONNECTOR;


    bool run(void);
    bool init(void);				//initialize
    bool exit(void);				//end
    virtual bool    SaveSession_File(QXmlStreamWriter *xmlOut);
    virtual bool	LoadSession_File(QXmlStreamReader *xmlIn);
    virtual bool	InitDisplay(void);

    Crlp2001(CPObject *parent = 0);
    virtual ~Crlp2001();

    bool Power;

protected:
    void paintEvent(QPaintEvent *);
protected slots:
    void contextMenuEvent ( QContextMenuEvent * );
    void Rotate();

private:

    bool rotate;

};


#endif // RLP2001_H
