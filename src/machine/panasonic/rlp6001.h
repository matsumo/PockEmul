#ifndef RLP6001_H
#define RLP6001_H

#include "pobject.h"

class Crlp6001:public CPObject {
    Q_OBJECT
public:
    const char*	GetClassName(){ return("Crlp6001");}

    Cconnector	*pMAINCONNECTOR;

    Cconnector *pEXTCONNECTOR[16];

    bool run(void);
    bool init(void);				//initialize
    bool exit(void);				//end

    Crlp6001(CPObject *parent = 0);
    virtual ~Crlp6001();

protected:
    void paintEvent(QPaintEvent *);
protected slots:
    void contextMenuEvent ( QContextMenuEvent * );

private:

};

#endif // RLP6001_H
