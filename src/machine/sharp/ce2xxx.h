#ifndef CE2XXX_H
#define CE2XXX_H

#include "pobject.h"
#include "modelids.h"

class CbusPc1500;

class Cce2xxx:public CPObject{
    Q_OBJECT
public:
    Cce2xxx(CPObject *parent=0, Models mod=CE201M, QString _cfg=QString());
    ~Cce2xxx();

    virtual bool init(void);				//initialize
    virtual bool exit(void);				//end
    virtual	bool run(void);					//check access
    virtual bool    SaveSession_File(QXmlStreamWriter *xmlOut);
    virtual bool	LoadSession_File(QXmlStreamReader *xmlIn);

    Cconnector	*pCONNECTOR;		qint64 pCONNECTOR_value;

private:
    Models model;
};

#endif // CE2XXX_H
