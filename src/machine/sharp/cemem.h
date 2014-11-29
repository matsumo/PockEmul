#ifndef CEMEM
#define CEMEM


#include "pobject.h"
#include "modelids.h"

class CbusPc1500;

class Ccemem:public CPObject{
    Q_OBJECT
public:
    Ccemem(CPObject *parent=0, Models mod=CE151);
    ~Ccemem();

    virtual bool init(void);				//initialize
    virtual bool exit(void);				//end
    virtual	bool run(void);					//check access
    virtual bool    SaveSession_File(QXmlStreamWriter *xmlOut);
    virtual bool	LoadSession_File(QXmlStreamReader *xmlIn);

    Cconnector	*pCONNECTOR;		qint64 pCONNECTOR_value;

private:
    Models model;
};

#endif // CEMEM

