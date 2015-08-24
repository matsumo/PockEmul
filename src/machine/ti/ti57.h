#ifndef TI57_H
#define TI57_H


class CPObject;
class Cti57cpu;
class Cconnector;

#include "pcxxxx.h"



class Cti57 : public CpcXXXX
{
    Q_OBJECT

public:
    Cti57(CPObject *parent = 0);
    virtual ~Cti57();

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

    Cti57cpu *ti57cpu;

    QString Display();
private:

};

#endif // TI57_H
