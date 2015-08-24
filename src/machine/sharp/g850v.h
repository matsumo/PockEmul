#ifndef G850_H
#define G850_H

#include <QMenu>

#include "pcxxxx.h"



class CZ80;
class CSED1560;


class Cg850v:public CpcXXXX{

Q_OBJECT

public:
    const char*	GetClassName(){ return("Cg850");}

    bool	LoadConfig(QXmlStreamReader *xmlIn);
    bool	SaveConfig(QXmlStreamWriter *xmlOut);
    bool	InitDisplay(void);
    bool	CompleteDisplay(void);

    virtual bool	run(void);				// emulator main

    virtual bool Mem_Mirror(UINT32 *d);
    virtual void TurnON(void);
    virtual void TurnOFF();
    void    Reset(void);


    virtual bool Chk_Adr(UINT32 *d,UINT32 data);
    virtual bool Chk_Adr_R(UINT32 *d, UINT32 *data);
    virtual UINT8 in(UINT8 address,QString sender=QString());
    virtual UINT8 out(UINT8 address,UINT8 value,QString sender=QString());
    virtual bool Set_Connector(Cbus *_bus = 0);
    virtual bool Get_Connector(Cbus *_bus = 0);
    BYTE        getKey(void);

    virtual bool	init(void);				// initialize

    Cg850v(CPObject *parent = 0);
    virtual ~Cg850v();

    CSED1560 *pSED1560;

private:
    int exBank,romBank,ramBank;
    BYTE ks1,ks2;
    BYTE keyBreak,pio8Io,pin11If,pio3Out,pio8Out,io3Out;
    BYTE interruptType,interruptMask;

};





#endif // G850_H
