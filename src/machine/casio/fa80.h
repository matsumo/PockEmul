#ifndef FA80_H
#define FA80_H


#include <QTime>
#ifndef NO_SOUND
#include <QSound>
#endif
#include <QDir>

#include "common.h"
#include "pobject.h"


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




    Cfa80(CPObject *parent = 0);

    virtual ~Cfa80();

public:


private:
    bool Get_MainConnector(void);
    bool Set_MainConnector(void);
    void Get_CentConnector(void);
    void Set_CentConnecor(void);

    QDir    directory;
    typedef BYTE (Cfa80::* funcPtr)(BYTE);
    static const funcPtr cmdtab[55];

    int count;
    int index;
    int bufindex;
    int opstatus;

    BYTE port,data;
    bool sendData;

    qint64 adrBus,prev_adrBus,out_adrBus;

    void printerControlPort(BYTE);
    BYTE printerStatusPort();
    void printerDataPort(BYTE);

    bool printerACK,printerBUSY,printerSTROBE,printerINIT,printerERROR,prev_printerSTROBE,prev_printerINIT;
    BYTE printerDATA;
    BYTE prev_printerStatusPort;
};



#endif // FA80_H
