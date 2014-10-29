#ifndef LBC1100_H
#define LBC1100_H

class CPObject;
class Cupd7907;
class CUPD16434;
class CPD1990AC;
class Cconnector;

#include "pcxxxx.h"


class Clbc1100 : public CpcXXXX
{
    Q_OBJECT

public:
    Clbc1100(CPObject *parent = 0);
    virtual ~Clbc1100();

    virtual bool	Chk_Adr(UINT32 *d,UINT32 data);
    virtual bool	Chk_Adr_R(UINT32 *d, UINT32 *data);
    virtual UINT8 in(UINT8 address);
    virtual UINT8 out(UINT8 address,UINT8 value);
    virtual UINT16 out16(UINT16 address,UINT16 value);

    virtual bool	Set_Connector(Cbus *_bus = 0);
    virtual bool	Get_Connector(Cbus *_bus = 0);

    quint16 kstrobe;

    bool init();

    virtual bool run();

    virtual void Reset();

    void TurnON();
    void TurnOFF();
    bool SaveConfig(QXmlStreamWriter *xmlOut);
    bool LoadConfig(QXmlStreamReader *xmlIn);

    UINT16 getKey();

    Cupd7907 *upd7907;
    CUPD16434 *upd16434[4];
    CPD1990AC *pd1990ac;

//    Cconnector	*pTAPECONNECTOR;	qint64 pTAPECONNECTOR_value;
//    Cconnector	*pPRINTERCONNECTOR;	qint64 pPRINTERCONNECTOR_value;
private:
    quint8 portB;
    quint8 sendToPrinter;
    quint8 currentLCDctrl;
    bool initcmd;

};

#endif // LBC1100_H
