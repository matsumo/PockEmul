#ifndef _PC1360_H_
#define _PC1360_H_

#include <stdlib.h>
#include <stdio.h>

#include "pc1350.h"
#include "Keyb.h"
#include "Connect.h"




class Cpc1360:public Cpc13XX{						//PC1360 emulator main class
Q_OBJECT
    Q_PROPERTY(int backdoorS1angle READ backdoorS1Angle WRITE setbackdoorS1Angle)
    Q_PROPERTY(int backdoorS2angle READ backdoorS2Angle WRITE setbackdoorS2Angle)
public:
    const char*	GetClassName(){ return("Cpc1360");}

    virtual BYTE	Get_PortA(void);
    virtual BYTE	Get_PortB(void);
//	virtual void	Set_PortB(BYTE data);
    virtual void	Set_PortF(BYTE data);
    virtual void	Set_PortC(BYTE data);
	virtual bool	Chk_Adr(UINT32 *d,UINT32 data);
    virtual bool	Chk_Adr_R(UINT32 *d, UINT32 *data);

    virtual bool	Set_Connector(Cbus *_bus = 0);
    virtual bool	Get_Connector(Cbus *_bus = 0);

    virtual bool	init(void);
    virtual bool	run(void);

    virtual void PreFlip(Direction dir,View targetView);

    virtual void TurnON(void);

    Cconnector	*pS2CONNECTOR;		qint64 pS2CONNECTOR_value;
    Cbus *busS2;

    Cpc1360(CPObject *parent = 0);
    virtual ~Cpc1360();

    bool backdoorS2Open;


    virtual void ComputeKey(KEYEVENT ke = KEY_PRESSED,int scancode=0,QMouseEvent *event=0);
    virtual bool UpdateFinalImage();
    virtual bool InitDisplay(void);
    virtual void manageCardVisibility();
public slots:
    void linkObject(QString item, CPObject *pPC);
    void endbackdoorAnimation(void);
private:


    void setbackdoorS2Angle(int value);
    int backdoorS2Angle() const { return m_backdoorS2Angle; }

    int m_backdoorS2Angle;


    void animateBackDoorS1(bool _open);
    void animateBackDoorS2(bool _open);
    void manageCardVisibilityS1();
    void manageCardVisibilityS2();
};
#endif
