#ifndef Z1_H
#define Z1_H

class CPObject;
class CHD66108;
class Ci80L188EB;
class CCF79107PJ;
class Cctronics;

#include "pcxxxx.h"
#include "modelids.h"

class Cz1 : public CpcXXXX
{
    Q_OBJECT

public:
    Cz1(CPObject *parent = 0,Models mod=Z1);
    virtual ~Cz1();

    virtual bool	Chk_Adr(UINT32 *d,UINT32 data);
    virtual bool	Chk_Adr_R(UINT32 *d, UINT32 *data);
    virtual UINT8 in(UINT8 address);
    virtual UINT8 out(UINT8 address,UINT8 value);
    virtual UINT8 in8(UINT16 Port);
    virtual UINT8 out8(UINT16 Port,UINT8 x);
    virtual UINT16 in16(UINT16 Port);
    virtual UINT16 out16(UINT16 Port, UINT16 x);
    virtual bool Set_Connector(Cbus *_bus = 0);
    virtual bool Get_Connector(Cbus *_bus = 0);
    virtual void Get_SerialConnector();
    virtual void Set_SerialConnector();
    virtual void Get_CentConnector();
    virtual void Set_CentConnector();
    virtual void Get_SIOConnector();
    virtual void Set_SIOConnector();
    virtual void initExtension(void);
    virtual bool UpdateFinalImage(void);
    bool init();

    CHD66108 *pHD66108;
    CCF79107PJ *pFPU;
    Cctronics *pCENT;


    virtual bool run();

    virtual void Reset();

    void TurnON();
    void TurnOFF();
    bool SaveConfig(QXmlStreamWriter *xmlOut);
    bool LoadConfig(QXmlStreamReader *xmlIn);

    UINT16 getKey();

    Ci80L188EB *i80l188ebcpu;

    Cconnector *pCENTCONNECTOR;    qint64      pCENTCONNECTOR_value;
    Cconnector *pSERIALCONNECTOR;    qint64      pSERIALCONNECTOR_value;



private:
    UINT16 ks;

    UINT8 io_b8;
    UINT16 timer0Control;
    UINT16 timer1Control;
    UINT16 timer2Control;

    int intPulseId;
    bool lastIntPulse;
    int lastKeyBufSize;
    bool newKey;
    Models model;

    bool pCENTflip;

};

#endif // Z1_H
