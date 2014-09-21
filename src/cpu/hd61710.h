#ifndef HD61710_H
#define HD61710_H

#include <QFile>


#include "pobject.h"
#include "Log.h"
#include "uart.h"

typedef struct {
    bool printerACK,printerBUSY,printerSTROBE,printerINIT,printerERROR,prev_printerSTROBE,prev_printerINIT;
    BYTE printerDATA;
    BYTE prev_printerStatusPort;
    BYTE sioCTRLReg;
    BYTE sioMode;
    BYTE sioParity;
    BYTE sioParityEnabled;
    BYTE sioDataLength;
    BYTE sioStopBit;
    bool sioTransEnabled,sioReceiEnabled;
    UINT16 sioBds;
} HD61710info;


class CHD61710:public QObject{



public:
    CHD61710(CPObject *parent, Cconnector *pCENT=NULL, Cconnector* pTAPE=NULL, Cconnector *pSIO=NULL);
    ~CHD61710();

    void linkConnectors(Cconnector *pCENT=NULL, Cconnector* pTAPE=NULL, Cconnector *pSIO=NULL);
    const char*	GetClassName(){ return("CHD61710");}
    CPObject		*pPC;

    bool	init(void);						//initialize
    bool	exit(void);						//end
    void	Reset(void);
    bool	step(void);

    void    regWrite(qint8 adr, quint8 data);
    BYTE    regRead(qint8 adr);

    HD61710info info;

    void	Load_Internal(QXmlStreamReader *xmlIn);
    void	save_internal(QXmlStreamWriter *xmlOut);

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
    void WriteSioInitReg(BYTE data);

    void WritePrtCrlPort(BYTE);
    BYTE ReadPrtStatusPort();
    void WritePrtDataPort(BYTE);

    Cuart uart;
};


#endif // HD61710_H
