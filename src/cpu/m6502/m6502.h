#ifndef M6502_H
#define M6502_H

#include "cpu.h"

#define SIG_CPU_OVERFLOW	0
#define SIG_CPU_IRQ	101
#define SIG_CPU_FIRQ	102
#define SIG_CPU_NMI	103
#define SIG_CPU_BUSREQ	104
#define SIG_CPU_DEBUG	201

typedef union {
#ifdef POCKEMUL_BIG_ENDIAN
    struct {
        quint8 h3, h2, h, l;
    } b;
    struct {
        qint8 h3, h2, h, l;
    } sb;
    struct {
        quint16 h, l;
    } w;
    struct {
        qint16 h, l;
    } sw;
#else
    struct {
        quint8 l, h, h2, h3;
    } b;
    struct {
        quint16 l, h;
    } w;
    struct {
        qint8 l, h, h2, h3;
    } sb;
    struct {
        qint16 l, h;
    } sw;
#endif
    quint32 d;
    qint32 sd;
}	PAIR;

class Cm6502:public CCPU{
Q_OBJECT
public:
    virtual const char*	GetClassName(){ return("Cm6502");}

    virtual	bool	init(void);						//initialize
    virtual	bool	exit(void);						//end
    virtual void	step(void);						//step SC61860

    virtual	void	Load_Internal(QXmlStreamReader *){}
    virtual	void	save_internal(QXmlStreamWriter *){}

//    virtual	DWORD	get_mem(DWORD adr,int size);		//get memory
//    virtual	void	set_mem(DWORD adr,int size,DWORD data);	//set memory

    virtual	bool	Get_Xin(void) { return true;}
    virtual	void	Set_Xin(bool) {}
    virtual	bool	Get_Xout(void) { return true;}
    virtual	void	Set_Xout(bool) {}

    virtual	UINT32	get_PC(void);					//get Program Counter
    virtual void    set_PC(UINT32);
    virtual void	Regs_Info(UINT8);


    virtual void	Reset(void);

    Cm6502(CPObject *parent);
    virtual ~Cm6502(){}

    int run(int clock);
    void write_signal(int id, quint32 data, quint32 mask);
public:
    PAIR pc, sp, zp, ea;
    quint16 prev_pc;
    quint8 a, x, y, p;
    bool pending_irq, after_cli;
    bool nmi_state, irq_state, so_state;
    int icount;
    bool busreq;

    void run_one_opecode();
    void OP(quint8 code);
    void update_irq();
    BYTE read_data8( UINT32 address);
    void write_data8( UINT32 address, BYTE value);

};
#if 0
/*
    Skelton for retropc emulator

    Origin : MAME
    Author : Takeda.Toshiya
    Date   : 2010.08.10-

    [ M6502 ]
*/


#define SIG_M6502_OVERFLOW	0

class Cm6502 : public CCPU
{
private:
    DEVICE *d_mem, *d_pic;

    pair pc, sp, zp, ea;
    uint16 prev_pc;
    uint8 a, x, y, p;
    bool pending_irq, after_cli;
    bool nmi_state, irq_state, so_state;
    int icount;
    bool busreq;

    void run_one_opecode();
    void OP(uint8 code);
    void update_irq();

public:
    Cm6502(VM* parent_vm, EMU* parent_emu) : CCPU(parent_vm, parent_emu) {
        busreq = false;
    }
    virtual ~Cm6502() {}

    // common functions
    void initialize();
    void reset();
    int run(int clock);
    void write_signal(int id, uint32 data, uint32 mask);
    void set_intr_line(bool line, bool pending, uint32 bit) {
        write_signal(SIG_CPU_IRQ, line ? 1 : 0, 1);
    }
    uint32 get_pc() {
        return prev_pc;
    }

    // unique function
    void set_context_mem(DEVICE* device) {
        d_mem = device;
    }
    void set_context_intr(DEVICE* device) {
        d_pic = device;
    }
};


#endif

#endif // M6502_H
