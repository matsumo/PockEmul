#ifndef HD61710_H
#define HD61710_H

#include <QFile>


#include "pobject.h"
#include "Log.h"

typedef struct {
    bool printerACK,printerBUSY,printerSTROBE,printerINIT,printerERROR,prev_printerSTROBE,prev_printerINIT;
    BYTE printerDATA;
    BYTE prev_printerStatusPort;
} HD61710info;

class CHD61710:public QObject{



public:
    CHD61710(CPObject *parent, Cconnector *pCENT, Cconnector* pTAPE, Cconnector *pSIO);
    ~CHD61710();

    const char*	GetClassName(){ return("CHD61710");}
    CPObject		*pPC;

    bool	init(void);						//initialize
    bool	exit(void);						//end
    void	Reset(void);
    bool	step(void);

    void    regWrite(qint8 adr, quint8 data);
    BYTE    regRead(qint8 adr);

    HD61710info info;

    void	Load_Internal(QXmlStreamReader *);
    void	save_internal(QXmlStreamWriter *);

    void Set_CentConnector(void);
    void Get_CentConnector(void);
    void Set_TAPEConnector(void);
    void Get_TAPEConnector(void);
    void Set_SIOConnector(void);
    void Get_SIOConnector(void);

    Cconnector	*pCENTCONNECTOR;
    Cconnector	*pSIOCONNECTOR;
    Cconnector  *pTAPECONNECTOR;

private:
    void printerControlPort(BYTE);
    BYTE printerStatusPort();
    void printerDataPort(BYTE);


};


#endif // HD61710_H
