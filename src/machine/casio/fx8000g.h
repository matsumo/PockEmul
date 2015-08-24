#ifndef FX8000G_H
#define FX8000G_H



class CPObject;
class CUPD1007;
class CHD44352;
class Cconnector;

#include "pcxxxx.h"



class Cfx8000g : public CpcXXXX
{
    Q_OBJECT

public:
    Cfx8000g(CPObject *parent = 0);
    virtual ~Cfx8000g();

    virtual bool	Chk_Adr(UINT32 *d,UINT32 data);
    virtual bool	Chk_Adr_R(UINT32 *d, UINT32 *data);
    virtual UINT8 in(UINT8 address,QString sender=QString());
    virtual UINT8 out(UINT8 address,UINT8 value,QString sender=QString());

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

    UINT8 getKey();
    virtual bool UpdateFinalImage(void);

    CUPD1007 *fx8000gcpu;
    CHD44352 *pHD44352;

private:
    bool hdFlag;
};

#endif // FX8000G_H
