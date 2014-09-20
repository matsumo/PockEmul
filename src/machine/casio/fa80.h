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


    Cconnector	*pCONNECTOR;		qint64 pCONNECTOR_value;
    Cconnector	*pCENTCONNECTOR;	qint64 pCENTCONNECTOR_value;
//    Cconnector	*pSIOCONNECTOR;     qint64 pSIOCONNECTOR_value;

    CHD61710 *pHD61710;


    Cfa80(CPObject *parent = 0);

    virtual ~Cfa80();

private:
    bool Get_MainConnector(void);
    bool Set_MainConnector(void);
    void Get_CentConnector(void);
    void Set_CentConnecor(void);

    BYTE port,data;
    qint64 adrBus;

};



#endif // FA80_H
