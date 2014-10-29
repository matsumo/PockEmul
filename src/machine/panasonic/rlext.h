#ifndef RLEXT_H
#define RLEXT_H


#include "common.h"

#include "Inter.h"

class Cconnector;

class CrlExt:public CPObject{
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

    CrlExt(CPObject *parent = 0);

    virtual ~CrlExt();

public:

    bool INTrequest;
    virtual bool writeLine0();
    virtual bool readLine0();
    virtual bool writeLine1();
    virtual bool readLine1();
    virtual bool writeLine2();
    virtual bool readLine2();
    virtual bool writeLine3();
    virtual bool readLine3();
    virtual void readData(quint32 adr);
    virtual void writeData(quint32 adr,quint8 val);
protected:
    void paintEvent(QPaintEvent *);
protected slots:
    void contextMenuEvent ( QContextMenuEvent * );
    void Rotate();

private:


    bool rotate;

};


#endif // RLEXT_H
