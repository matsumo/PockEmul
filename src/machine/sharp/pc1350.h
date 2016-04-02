#ifndef _PC1350_H
#define _PC1350_H

#include <stdlib.h>
#include <stdio.h>



#include "pcxxxx.h"



extern BYTE scandef_pc1350[];

class Cbus;

class Cpc13XX:public CpcXXXX{						//PC1350 emulator main class
Q_OBJECT
        Q_PROPERTY(int backdoorS1angle READ backdoorS1Angle WRITE setbackdoorS1Angle)
public:
    virtual const char*	GetClassName(){ return("Cpc13XX");}

	virtual void	Set_PortA(BYTE data);
	virtual BYTE	Get_PortA(void);
	virtual bool	init(void);
    virtual void	initExtension(void){}
    virtual UINT8   in(UINT8,QString sender=QString() ){ Q_UNUSED(sender); return(1);}
    virtual UINT8   out(UINT8 ,UINT8 ,QString sender=QString()){ Q_UNUSED(sender); return(1);}
    virtual bool    CheckUpdateExtension(CExtension *ext);
    virtual int     mapKey(QKeyEvent *event);


    Cconnector	*pS1CONNECTOR;		qint64 pS1CONNECTOR_value;

    Cbus *busS1;

    bool backdoorS1Open;
    bool backdoorFlipping;

    Cpc13XX(CPObject *parent = 0);

    virtual ~Cpc13XX();
    virtual void manageCardVisibility();
    virtual void PostFlip();
    void animateBackDoorS1(bool _open);
    virtual void ComputeKey(KEYEVENT ke = KEY_PRESSED,int scancode=0,QMouseEvent *event=0);
    virtual bool UpdateFinalImage();
    virtual bool InitDisplay(void);

public slots:
    void endbackdoorAnimation();
    void linkObject(QString item, CPObject *pPC);
protected:

    QImage *backDoorImage;
    QImage BackImageBackup;

    int currentSlot;
    BYTE cnt;
    void setbackdoorS1Angle(int value);
    int backdoorS1Angle() const { return m_backdoorS1Angle; }
    int m_backdoorS1Angle;

};

class Cpc1350:public Cpc13XX{						//PC1350 emulator main class
Q_OBJECT
public:
    virtual BYTE	Get_PortA(void);
    virtual BYTE	Get_PortB(void);
    virtual void	Set_PortB(BYTE data);
    virtual void	Set_PortF(BYTE data);
		
	virtual bool	Chk_Adr(UINT32 *d,UINT32 data);
    virtual bool	Chk_Adr_R(UINT32 *d, UINT32 *data);
	virtual void	initExtension(void);
    virtual bool	Set_Connector(Cbus *_bus = 0);
    virtual bool	Get_Connector(Cbus *_bus = 0);
    virtual bool	run(void);				// emulator main

    virtual bool	LoadSession_File(QXmlStreamReader *);
    virtual void TurnON(void);

    Cpc1350(CPObject *parent = 0);

    virtual ~Cpc1350()
	{								//[constructor]
	}

};
#endif
