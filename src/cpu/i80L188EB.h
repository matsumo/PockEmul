#ifndef I80L188EB_H
#define I80L188EB_H

#include "i80x86.h"

class C8253PIT;
class Ci80uart;

class Ci80L188EB : public Ci80x86
{

public:
    Ci80L188EB(CPObject *parent);
    virtual ~Ci80L188EB();

    virtual	bool	init(void);						//initialize
    virtual	bool	exit(void);						//end
    virtual void	step(void);						//step SC61860
    virtual void	Reset(void);



    C8253PIT *p8253;

    Ci80uart *pserial0;
    Ci80uart *pserial1;

    quint16 eoi;

};

#define MASK_TXE 0x08
#define MASK_TI 0x20
#define MASK_RI 0x40

class Ci80uart {

public:
    Ci80uart(Ci80L188EB *parent);

    quint16 SBRK()  { return (SxCON&0x0100); }
    quint16 TB8()   { return (SxCON&0x0080); }
    quint16 CEN()   { return (SxCON&0x0040); }
    quint16 REN()   { return (SxCON&0x0020); }
    quint16 EVN()   { return (SxCON&0x0010); }
    quint16 PEN()   { return (SxCON&0x0008); }
    quint16 M02()   { return (SxCON&0x0007); }

    quint16 DBRK1() { return (SxSTS&0x0200); }
    quint16 DBRK0() { return (SxSTS&0x0100); }
    quint16 RB8()   { return (SxSTS&0x0080); }
    quint16 RI()    { return (SxSTS&0x0040); }
    quint16 TI()    { return (SxSTS&0x0020); }
    quint16 FE()    { return (SxSTS&0x0010); }
    quint16 TXE()   { return (SxSTS&0x0008); }
    quint16 OE()    { return (SxSTS&0x0004); }
    quint16 CTS()   { return (SxSTS&0x0002); }


    void  Reset(void);

    void step(quint64 states);

    void write_SxCON(quint16 value);
    quint8 get_SxRBUF() { return SxRBUF;}
    void set_SxTBUFL(quint8 data);
    void set_SxTBUFH(quint8 data);
    void set_BxCMP(quint16 data) { BxCMP = data;}

    quint8 get_RXD(void) {return RXD;}
    void set_RXD(quint8 data) { RXD = data; }
    bool get_TXD(void) {return TXD;}
    void set_TXD(quint8 data) { TXD = data; }

    Ci80L188EB *i80l188ebcpu;


    quint8 mode;
    quint8 samplingCNT;
    bool TParity;

    quint8 SxRBUF;


    quint8 SxTBUF;
    bool SxTBUFPending;

    quint16 BxCNT;
    quint16 BxCMP;
    quint16 SxCON;
    quint16 SxSTS;

    bool TXD,RXD;
    bool risingTXD;
    quint8 receivedBit;
    quint16 RShiftReg;
    quint16 TShiftReg;
    bool TShiftRegPending;
    quint8 transmitedBit;

    void set_CTS(bool value) {
        if (value)
            SxSTS |= 0x0002;
        else SxSTS &= ~(0x002);
    }




};


#endif // I80L188EB_H
