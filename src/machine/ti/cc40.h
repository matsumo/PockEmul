#ifndef CC40_H
#define CC40_H

class CPObject;
class Ctms7000;
class CHD44780;

#include "pcxxxx.h"

class Ccc40 : public CpcXXXX
{
    Q_OBJECT

public:
    Ccc40(CPObject *parent = 0);
    virtual ~Ccc40();

    virtual bool	Chk_Adr(UINT32 *d,UINT32 data);
    virtual bool	Chk_Adr_R(UINT32 *d, UINT32 *data);
    virtual UINT8 in(UINT8 address,QString sender=QString());
    virtual UINT8 out(UINT8 address,UINT8 value,QString sender=QString());

    virtual bool	Set_Connector(Cbus *_bus = 0);
    virtual bool	Get_Connector(Cbus *_bus = 0);

    bool init();

    virtual bool run();

    virtual void Reset();

    void TurnON();
    void TurnOFF();
    bool SaveConfig(QXmlStreamWriter *xmlOut);
    bool LoadConfig(QXmlStreamReader *xmlIn);

    virtual void ComputeKey(KEYEVENT ke = KEY_PRESSED,int scancode=0,QMouseEvent *event=0);

    Ctms7000 *ptms7000cpu;
    CHD44780 *pHD44780;

    quint8 ks;
    quint8 getKey();
    UINT8 m_clock_control;
    UINT8 m_banks;
    UINT8 m_power;
    UINT8 m_bus_control;

    virtual void initExtension(void);

    UINT8 *m_sysram[2];
    UINT16 m_sysram_size[2];
    UINT16 m_sysram_end[2];
    UINT16 m_sysram_mask[2];

    void clock_w(UINT8 data);
    UINT8 clock_r();
    void power_w(UINT8 data);
    void bus_control_w(UINT8 data);
    UINT8 bus_control_r();
    void sysram_w(UINT16 offset, UINT8 data);
    UINT8 sysram_r(UINT16 offset);
    void init_sysram(int chip, UINT16 size);

    bool slotChanged;
    int currentSlot;

protected slots:
        void addModule(QString item, CPObject *pPC);
};


#endif // CC40_H
