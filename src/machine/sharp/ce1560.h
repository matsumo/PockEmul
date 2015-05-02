#ifndef CE1560_H
#define CE1560_H


#include "Log.h"
#include "lh5810.h"
#include "cprinter.h"
#include "Connect.h"
#include "Inter.h"
#include "Keyb.h"
#include "cmotor.h"

#include "s6b0108.h"

class CbusPc1500;

class Cce1560:public CpcXXXX {
    Q_OBJECT
    Q_PROPERTY(int screenangle READ screenAngle WRITE setScreenAngle)
    Q_PROPERTY(qreal zoom READ zoom WRITE setZoom)

public:

    virtual bool	Chk_Adr(UINT32 *,UINT32 ) { return true; }
    virtual bool	Chk_Adr_R(UINT32 *, UINT32 *) { return true; }
    virtual UINT8 in(UINT8 ) { return 0; }
    virtual UINT8 out(UINT8 ,UINT8 ) { return 0; }
    virtual bool		run(void);
    virtual bool		init(void);
    virtual bool		exit(void);
    virtual bool	LoadConfig(QXmlStreamReader *xmlIn);	// Load PC Configuration
    virtual bool	SaveConfig(QXmlStreamWriter *xmlOut);	// Save PC Configuration
    virtual void ComputeKey(KEYEVENT ke = KEY_PRESSED,int scancode=0);
    virtual bool UpdateFinalImage();

    virtual bool	InitDisplay(void);

    Cconnector	*pCONNECTOR;	qint64 pCONNECTOR_value;
    Cconnector	*pEXTCONNECTOR;	qint64 pEXTCONNECTOR_value;
    Cconnector	*pTAPECONNECTOR;qint64 pTAPECONNECTOR_value;
    CbusPc1500    *bus;

    CS6B0108 *ps6b0108[4];

    Cce1560(CPObject *parent = 0);
    virtual ~Cce1560();

public slots:
    void endscreenAnimation(void);
    void updateAnimation();

private:
    void setZoom(qreal value){ m_zoom = value; }
    qreal zoom() const { return m_zoom; }
    qreal m_zoom;
    void setScreenAngle(int value) { m_screenAngle = value; }
    int screenAngle() const { return m_screenAngle; }
    int m_screenAngle;

    void animateScreen();
    bool screenOpen;
    bool screenFlipping;
    QImage coverImage;

    bool inhibitSwitch;
    int  ramDiskPg;
    int  firmwarePg;
    int  rom8000Pg;

#endif
};
