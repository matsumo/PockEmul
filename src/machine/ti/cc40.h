#ifndef CC40_H
#define CC40_H

class CPObject;
class Ctms7000;

#include "pcxxxx.h"

class Ccc40 : public CpcXXXX
{
    Q_OBJECT

public:
    Ccc40(CPObject *parent = 0);
    virtual ~Ccc40();

    virtual bool	Chk_Adr(UINT32 *d,UINT32 data);
    virtual bool	Chk_Adr_R(UINT32 *d, UINT32 *data);
    virtual UINT8 in(UINT8 address);
    virtual UINT8 out(UINT8 address,UINT8 value);
//    virtual UINT8 in8(UINT16 Port);
//    virtual UINT8 out8(UINT16 Port,UINT8 x);
//    virtual UINT16 in16(UINT16 Port);
//    virtual UINT16 out16(UINT16 Port, UINT16 x);

    virtual bool	Set_Connector(void);
    virtual bool	Get_Connector(void);

    bool init();

    virtual bool run();

    virtual void Reset();

    void TurnON();
    void TurnOFF();
    bool SaveConfig(QXmlStreamWriter *xmlOut);
    bool LoadConfig(QXmlStreamReader *xmlIn);

    Ctms7000 *tms7000cpu;

    quint8 ks;
    quint16 getKey();
    bool Cetl;
    quint8 sid;

    int lastKeyBufSize;
    bool newKey;
    UINT8 matrixResult;
    virtual void initExtension(void);



};


#endif // CC40_H
