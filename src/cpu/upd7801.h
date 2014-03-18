/*
	Skelton for retropc emulator

	Origin : MESS UPD7810 Core
	Author : Takeda.Toshiya
	Date   : 2006.08.21 -

	[ uPD7801 ]
*/

#ifndef _UPD7801_H_
#define _UPD7801_H_

#include "cpu.h"




#define SIG_UPD7801_INTF0	0
#define SIG_UPD7801_INTF1	1
#define SIG_UPD7801_INTF2	2
#define SIG_UPD7801_WAIT	3

// virtual i/o port address
#define P_A	0
#define P_B	1
#define P_C	2
#define P_SI	3
#define P_SO	4

class Cupd7801 : public CCPU
{
private:

	
	/* ---------------------------------------------------------------------------
	registers
	--------------------------------------------------------------------------- */
	
	int count, period, scount, tcount;
	bool wait;
	
    DPAIR regs[4];
    quint16 SP, PC, prevPC, altVA, altBC, altDE, altHL;
    quint8 PSW, IRR, IFF, SIRQ, HALT, MK, MB, MC, TM0, TM1, SR;
	// for port c
    quint8 SAK, TO, PORTC;
	
    virtual	bool	Get_Xin(void) {}
    virtual	void	Set_Xin(bool) {}
    virtual	bool	Get_Xout(void) {}
    virtual	void	Set_Xout(bool) {}

    virtual	DWORD	get_PC(void);				//get Program Counter
    virtual void	Regs_Info(UINT8);

	/* ---------------------------------------------------------------------------
	virtual machine interface
	--------------------------------------------------------------------------- */
	
	// memory
    inline quint8 RM8(quint16 addr);
    inline void WM8(quint16 addr, quint8 val);
    inline quint16 RM16(quint16 addr);
    inline void WM16(quint16 addr, quint16 val);
    inline quint8 FETCH8();
    inline quint16 FETCH16();
    inline quint16 FETCHWA();
    inline quint8 POP8();
    inline void PUSH8(quint8 val);
    inline quint16 POP16();
    inline void PUSH16(quint16 val);
	
	// i/o
    inline quint8 IN8(int port);
    inline void OUT8(int port, quint8 val);
    inline void UPDATE_PORTC(quint8 IOM);
	
	/* ---------------------------------------------------------------------------
	opecode
	--------------------------------------------------------------------------- */
	
	void run_one_opecode();
	void OP();
	void OP48();
	void OP4C();
	void OP4D();
	void OP60();
	void OP64();
	void OP70();
	void OP74();
	
public:
    Cupd7801(CPObject *parent);
    ~Cupd7801();

    bool	init(void);						//initialize
    bool	exit(void);						//end
    void	step(void);						//step SC61860
    void	Reset(void);

    void	Load_Internal(QXmlStreamReader *);
    void	save_internal(QXmlStreamWriter *);
//	void	save_internal(void);

    DWORD	get_mem(DWORD adr,int size);		//get memory
    void	set_mem(DWORD adr,int size,DWORD data);	//set memory
	// common function
	void reset();
	int run(int clock);
    void write_signal(int id, quint32 data, quint32 mask);
    quint32 get_pc() {
		return prevPC;
	}
	

};

#endif
