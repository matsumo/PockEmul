/*
 * this code is based on TI-5x emulator
 * (c) 2014 Hynek Sladky
 * http://hsl.wz.cz/ti_59.htm
 */

#ifndef TI59_H
#define TI59_H


class CPObject;
class Ctmc0501;
class Cconnector;

#include "pcxxxx.h"
#include "modelids.h"


class Cti59 : public CpcXXXX
{
    Q_OBJECT

public:
    Cti59(CPObject *parent=0,Models mod=TI59);
    virtual ~Cti59();

    virtual bool	Chk_Adr(UINT32 *d,UINT32 data);
    virtual bool	Chk_Adr_R(UINT32 *d, UINT32 *data);
    virtual UINT8 in(UINT8 address);
    virtual UINT8 out(UINT8 address,UINT8 value);

    virtual bool	Set_Connector(Cbus *_bus = 0);
    virtual bool	Get_Connector(Cbus *_bus = 0);

    virtual void ComputeKey(KEYEVENT ke = KEY_PRESSED,int scancode=0,QMouseEvent *event=0);


    void wheelEvent(QWheelEvent *event);

    quint16 kstrobe;

    bool init();
    virtual bool run();
    virtual void Reset();

    void TurnON();
    void TurnOFF();
    bool SaveConfig(QXmlStreamWriter *xmlOut);
    bool LoadConfig(QXmlStreamReader *xmlIn);

    virtual bool UpdateFinalImage(void);

    UINT8 getKey();

    Ctmc0501 *ti59cpu;

    Cconnector	*pPRINTERCONNECTOR;	qint64 pPRINTERCONNECTOR_value;

    QString Display();
    QString displayString;
    quint8 sendToPrinter;
    void generateCard();
private:
    Models currentModel;

    unsigned char dA[16], dB[16];
    char disp_filter;

    QAction * changeCardAction;

    bool slotChanged;
    int currentSlot;
    QString currentModule;
    int moduleNbCards;
    bool drawCard;
    QImage *currentCard, *renderedCard;
    int cardIndex;
    int currentPrgm;

protected slots:
    void contextMenuEvent ( QContextMenuEvent * );
    void changeCard(void);
    void addModule(QString item, CPObject *pPC);
};

#endif // TI59_H


