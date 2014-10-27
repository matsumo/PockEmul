#ifndef E500_H
#define E500_H

#include "pcxxxx.h"
#include "modelids.h"

class CHD61102;
//class CRP5C01;

class Ce500:public CpcXXXX{						//E500 emulator main class
Q_OBJECT
public:
    const char*	GetClassName(){ return("Ce500");}

    virtual void	TurnON(void);
    virtual void	TurnOFF(void);


    virtual bool	Set_Connector(void);
    virtual bool	Get_Connector(void);
    virtual BYTE	Get_PortA(void);
    virtual BYTE	Get_PortB(void);



//	bool	Mem_Mirror(DWORD *d);
    virtual bool	Chk_Adr(UINT32 *d,UINT32 data);
    virtual bool	Chk_Adr_R(UINT32 *d, UINT32 *data);

    virtual bool	LoadConfig(QXmlStreamReader *xmlIn);	// Load PC Configuration
    virtual bool	SaveConfig(QXmlStreamWriter *xmlOut);	// Save PC Configuration

    virtual UINT8 in(UINT8 address);
    virtual UINT8 out(UINT8 address,UINT8 value);

    Ce500(CPObject *parent = 0,Models mod=E500);

    virtual ~Ce500();

    bool init();

    CHD61102        *pHD61102_1;
    CHD61102        *pHD61102_2;

//    CRP5C01         *pRP5C01;

    BYTE disp(qint8 cmd, UINT32 data);
    BYTE getKey();
    bool run();

    void computeSound();
    virtual void MemMirror(UINT32 *d);
    virtual void initExtension();
    virtual void BuildContextMenu(QMenu * menu);
protected slots:
    void internalRam();

protected:
//	BYTE previous_key;

private:
    QAction *internalRamAction;
    bool Xin,Xout;
    quint64 start2khz,start4khz;
    qint64 tmp_state;
    Models model;

    int internalRamKb;


};


#endif // E500_H
