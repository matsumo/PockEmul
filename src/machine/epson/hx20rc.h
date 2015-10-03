#ifndef HX20RC_H
#define HX20RC_H


#include "pobject.h"
#include "modelids.h"

class QQuickWidget;

class Chx20RC:public CPObject{
    Q_OBJECT
public:
    Chx20RC(CPObject *parent=0);
    ~Chx20RC();

    virtual bool init(void);				//initialize
    virtual bool exit(void);				//end
    virtual	bool run(void);					//check access
    virtual bool    SaveSession_File(QXmlStreamWriter *xmlOut);
    virtual bool	LoadSession_File(QXmlStreamReader *xmlIn);

    Cconnector	*pCONNECTOR;		qint64 pCONNECTOR_value;
protected slots:
    void contextMenuEvent ( QContextMenuEvent * );
    void mouseDoubleClickEvent(QMouseEvent *);
    void ShowEM(void);
    void HideEM(void);
private slots:
    Q_INVOKABLE void closeQuick();
private:
    bool ShiftRegisterOutput;
    bool ClearCounter,prevClearCounter;
    bool PowerOn,prevPowerOn;
    bool ShiftRegisterClear,prevShiftRegisterClear;
    bool Clock,prevClock;
    bool ShiftLoad,prevShiftLoad;

    quint64 Counter;
    quint8 ShiftRegister;

    QQuickWidget *EMView;
};

#endif // HX20RC

