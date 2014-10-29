#ifndef CPB2000_H
#define CPB2000_H

#include "pb1000.h"

class CHD44352;

class Cpb2000 : public Cpb1000
{
    Q_OBJECT

public:
    Cpb2000(CPObject *parent = 0);
    virtual ~Cpb2000();

    virtual bool	LoadConfig(QXmlStreamReader *);
    virtual bool	SaveConfig(QXmlStreamWriter *);

    virtual void initExtension(void);
    virtual void TurnON();
    virtual bool init();
    virtual void Reset();
    virtual bool run();
    virtual bool Chk_Adr(UINT32 *d, UINT32 data);
    virtual bool Chk_Adr_R(UINT32 *d, UINT32 *data);

    virtual bool Set_Connector(Cbus *_bus = 0);
    virtual bool Get_Connector(Cbus *_bus = 0);

    virtual UINT16 getKey();

    virtual bool UpdateFinalImage(void);
    virtual void paintEvent(QPaintEvent *event);
    virtual void TurnCLOSE();
    virtual UINT8 readPort();
    virtual WORD Get_16rPC(UINT32 adr);
    virtual void MemBank(UINT32 *d);

private:
    QImage *overlay;

};

#endif // CPB2000_H
