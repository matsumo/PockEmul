#ifndef RLH1000_H
#define RLH1000_H

#include "pcxxxx.h"
#include "buspanasonic.h"

class Cm6502;


class Crlh1000 : public CpcXXXX
{
    Q_OBJECT
    Q_PROPERTY(int backdoorangle READ backdoorAngle WRITE setbackdoorAngle)
public:
    Crlh1000(CPObject *parent = 0);
    virtual ~Crlh1000();

    virtual bool        Chk_Adr(UINT32 *d,UINT32 data);
    virtual bool        Chk_Adr_R(UINT32 *d, UINT32 *data);
    virtual UINT8 in(UINT8 address);
    virtual UINT8 out(UINT8 address,UINT8 value);

    virtual void ComputeKey(KEYEVENT ke = KEY_PRESSED,int scancode=0);
    virtual bool Set_Connector(void);
    virtual bool Get_Connector(void);
    virtual bool UpdateFinalImage();

    virtual bool init();
    virtual bool run();
    virtual void Reset();

    void TurnON();
    void TurnOFF();
    bool SaveConfig(QXmlStreamWriter *xmlOut);
    bool LoadConfig(QXmlStreamReader *xmlIn);

    Cm6502 *m6502;
    quint8 lineFE[0x100];
    quint8 lineFD[0x100];
    quint8 lineFF[0x100];
    quint8 strobe32;
    quint8 extrinsicRAM;
    quint8 extrinsicROM;
    UINT8 getKey(quint8 row);
    UINT8 ReadBusMem(BUS_FUNC f, UINT32 adr, quint8 dest);

public slots:
    void endbackdoorAnimation(void);
    void addModule(QString item, CPObject *pPC);

private:
    CbusPanasonic *bus;
    quint8 latchByte;
    quint8 timercnt1,timercnt2,timercnt3;
    int currentSlot;
    quint32 currentAdr;
    bool slotChanged;
    bool   backdoorOpen;
    QImage *backDoorImage;

    void setbackdoorAngle(int value);
    int backdoorAngle() const { return m_backdoorAngle; }
    int m_backdoorAngle;


    void animateBackDoor();
    bool backdoorFlipping;
    int  backdoorKeyIndex;
    int  capsuleKeyIndex;
};

#endif // RLH1000_H
