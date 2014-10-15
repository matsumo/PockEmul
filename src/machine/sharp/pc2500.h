#ifndef PC2500_H
#define PC2500_H

#include "init.h"
#include "pc1350.h"

class Cce515p;

// SPEED 73mm/s  -> 73*5 = 365 step/sec


class Cpc2500:public Cpc1350{						//PC1350 emulator main class
Q_OBJECT
public:
    virtual BYTE	Get_PortA(void);
//    BYTE	Get_PortB(void);
//    void	Set_PortB(BYTE data);
    virtual void	Set_PortF(BYTE);
    virtual BYTE    Get_PC(UINT32 adr);
    virtual WORD    Get_16rPC(UINT32 adr);

    virtual bool	Chk_Adr(UINT32 *d,UINT32 data);
    virtual bool	Chk_Adr_R(UINT32 *d, UINT32 *data);
//    virtual void	initExtension(void);
    virtual bool	Set_Connector(void);
    virtual bool	Get_Connector(void);
    virtual bool	run(void);				// emulator main
    virtual bool	init(void);
    virtual void resizeEvent ( QResizeEvent * );
    virtual bool UpdateFinalImage(void);

    Cce515p  *pce515p;
    Cconnector	*pTAPECONNECTOR;	qint64 pTAPECONNECTOR_value;

    Cpc2500(CPObject *parent = 0);
    virtual ~Cpc2500();
protected slots:
    void contextMenuEvent (QContextMenuEvent * e);

private:
    bool printMode;
    bool capslock;
};

#endif // PC2500_H
