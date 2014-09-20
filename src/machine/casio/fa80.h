#ifndef FA80_H
#define FA80_H


#include <QTime>
#ifndef NO_SOUND
#include <QSound>
#endif
#include <QDir>

#include "common.h"
#include "pobject.h"

class CHD61710;


class Cfa80:public CPObject{
    Q_OBJECT
public:

    virtual bool init(void);				//initialize
    virtual bool exit(void);				//end
    virtual	bool run(void);					//check access

    virtual bool	Set_Connector(void);
    virtual bool	Get_Connector(void);

    virtual bool UpdateFinalImage(void);
    virtual bool    SaveSession_File(QXmlStreamWriter *xmlOut);
    virtual bool	LoadSession_File(QXmlStreamReader *xmlIn);

    Cconnector	*pCONNECTOR;		qint64 pCONNECTOR_value;
    Cconnector	*pCENTCONNECTOR;	qint64 pCENTCONNECTOR_value;
    Cconnector  *pTAPECONNECTOR;    qint64 pTAPECONNECTOR_value;

    CHD61710 *pHD61710;


    Cfa80(CPObject *parent = 0);

    virtual ~Cfa80();

private:
    bool Get_MainConnector(void);
    bool Set_MainConnector(void);

    BYTE port,data;
    qint64 adrBus;

};



#endif // FA80_H
