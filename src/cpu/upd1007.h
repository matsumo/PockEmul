#ifndef UPD1007_H
#define UPD1007_H

/*
This CPU core is based on documentations works done by:
- Piotr Piatek ( http://www.pisi.com.pl/piotr433/fx8000ee.htm )
*/

#include "cpu.h"

typedef quint32 UINT32;


//**************************************************************************
//  DEFINITIONS
//**************************************************************************

#define IX  0
#define IY  1
#define IZ  2
#define V3  3
#define V2  4
#define V1  5
#define V0  6
#define SP  7




typedef void   (*Proc3) (UINT8 x, UINT8 y);
typedef void   (*Proc4) (UINT8 x);
typedef void   (*Proc5) (UINT8 *x, UINT8 ye);
typedef UINT8  (*Func1)();
typedef UINT8  (*Func2)(UINT8 x);
typedef UINT8  (*Func3)(UINT8 x, UINT8 y);
typedef UINT16 (*Func4)(UINT8 x, UINT8 y);
typedef UINT16 (*Func5)(UINT8 *x, UINT8 y);

// device config
struct upd1007_config
{
    BYTE mr[0x80];	//{ main (general purpose) register file }
    BYTE koreg;//: byte;
    BYTE kireg;//: byte;
    BYTE iereg;//: byte;
    BYTE ifreg;//: byte;	{ 'if' is a reserved word in Pascal }
    BYTE asreg;//: byte;	{ 'as' is a reserved word in Pascal }

    quint8 regbank; //	{ $00 when RB0, $40 when RB1 }
    int regstep;	// { determines the order of processed registers
//			  in a register array,
//			  +1 if the register index is incremented (default),
//			  -1 if the register index is decremented }

    BYTE flag;//: byte;
    UINT16 pc;

        UINT16 savepc;	//	{ address of the executed instruction }
        int irqcnt[3]; //	{ interrupt request counters
                      // implementing the interrupt latency:
                      // value < 0 - counting in progress
                      // value = 0 - idle
                      // value > 0 - waiting to be serviced }

        BYTE opcode[3];
        int opindex; //: integer;		{ index to the opcode table }
        int cycles; //: integer;		{ counter of the clock pulses }
        int acycles; //: integer;		{ clock pulse counter accumulator }
//        procptr: pointer;		{ pointer to a procedure that should be
//                      executed before a machine code instruction,
//                      usually to complete an I/O register write
//                      cycle }
        int OscFreq; //: integer;		{ CPU clock frequency in kHz }
        bool CpuStop; //: boolean;		{ True stops the CPU, used in the debug mode }
        int CpuDelay; //: integer;		{ delay after hiding the Debug Window,
                      // prevents the program from crashing when the
                      //Debug Window was made visible too early }
        bool CpuSleep; // : boolean;		{ True if the CPU in the power-off state, can
                      // be waken up by an INT2 interrupt or AC key }
        int CpuSteps; //: integer;		{ ignored when < 0 }
        int BreakPoint; //: integer;	{ ignored when < 0 }
        int SelfTest; //: integer;		{ for how many LCD refresh cycles (forever if
                      // negative) the self-test jumper should be
                      //connected after the reset }
};


#define ROM0SIZE 0x1800
#define ROM1SIZE 0x8000
#define RAMSTART 0xC000
#define RAMSIZE	 0x2000
#define RAMEND	 RAMSTART+RAMSIZE
#define NONE_REG 0xff     /* index of a non-existing general purpose register */

class CUPD1007 : public CCPU
{
public:

    BYTE dummysrc; //	{ free adress space }
    BYTE dummydst;	//	{ free address space }


    CUPD1007(CPObject *parent, QString rom0fn);
    virtual ~CUPD1007();

    upd1007_config info;

    virtual	bool	init(void);						//initialize
    virtual	bool	exit(void);						//end
    virtual void	step(void);						//step SC61860
    virtual void	Reset(void);


    virtual	void	Load_Internal(QXmlStreamReader *);
    virtual	void	save_internal(QXmlStreamWriter *);
            void	save(void);

    virtual	bool	Get_Xin(void){ return true;}
    virtual	void	Set_Xin(bool){}
    virtual	bool	Get_Xout(void){ return true;}
    virtual	void	Set_Xout(bool){}

    virtual	UINT32	get_PC(void);					//get Program Counter
    virtual void	Regs_Info(UINT8);

            UINT8 Get_PD(void);
            UINT8 Get_PE(void);


            BYTE rom0[ROM0SIZE][3];
            BYTE rom1[ROM1SIZE];
            BYTE ram[RAMSIZE];

            UINT16 ea;  /* temporary pointer used in the indexed addressing mode */
            const static BYTE cc[8];
            /* bits of the IE register */
            const static BYTE INT_enable[3];
        /* bits of the IF register */
            const static BYTE INT_serv[3];
            const static BYTE INT_input[3];

            void addState(int x);
            UINT16 Fetchopcode();
            BYTE FetchByte();
            BYTE Reg1(BYTE x);
            BYTE Reg2(BYTE x);
            BYTE Reg3(BYTE x, BYTE y);
            BYTE AsLimit(BYTE y);
            BYTE Rl1(BYTE x, BYTE y);
            BYTE Im6(BYTE x, BYTE y);
            BYTE Ireg();
            void NextReg(BYTE *x);
            UINT16 Wreg(BYTE x, BYTE y);
            UINT16 PostIncw(BYTE x, BYTE y);
            UINT16 PostDecw(BYTE x, BYTE y);
            UINT16 PreIncw(BYTE x, BYTE y);
            UINT16 PreDecw(BYTE x, BYTE y);
            void PlusOffset(BYTE x);
            void MinusOffset(BYTE x);
            void UnReg(void *op2);
            void RotReg(void *op2);
            void UnAry(void *op2);
            void RotAry(void *op2);
            void Mtbm(void *op2);
            void Xreg(void *op2);
            void Yreg(void *op2);
            void TXreg(void *op2);
            void TYreg(void *op2);
            void ExchReg(void *op2);
            void RegIm8(void *op2);
            void TRegIm8(void *op2);
            void Xary(void *op2);
            void Yary(void *op2);
            void TXary(void *op2);
            void TYary(void *op2);
            void ExchAry(void *op2);
            void AryIm6(void *op2);
            void TAryIm6(void *op2);
            void Ldw(void *op2);
            void AdwSbw(void *op2);
            void Cond(void *op2);
            void NotCond(void *op2);
            void KeyCond(void *op2);
            void NotKeyCond(void *op2);
            void Jump(BYTE x1, BYTE x2);
            void Call(BYTE x1, BYTE x2);
            void Trap(void *op2);
            void Ijmp(BYTE x1, BYTE x2);
            void Rtn(void *op2);
            void Cani(void *op2);
            void Rti(void *op2);
            void Nop(void *op2);
            void BlockCopy(void *op2);
            void BlockSearch(void *op2);
            void StMemoReg(void *op2);
            void StMemoIm8(void *op2);
            void StmMemoAry(void *op2);
            BYTE OpSwp1(BYTE *x, BYTE y);
            BYTE OpBnus(BYTE *x, BYTE y);
            BYTE OpLd(BYTE x, BYTE y);
            BYTE *DstPtr(UINT16 address);
            void StImOffsReg(void *op2);
            void StRegOffsReg(void *op2);
            void StmImOffsAry(void *op2);
            void StmRegOffsAry(void *op2);
            void LdRegMemo(void *op2);
            void LdmAryMemo(void *op2);
            void LdRegImOffs(void *op2);
            void LdRegRegOffs(void *op2);
            void LdmAryImOffs(void *op2);
            void LdmAryRegOffs(void *op2);
            void PstIm8(void *op2);
            void PstReg(void *op2);
            void Gst(void *op2);
            void Off(void *op2);
            void ZeroBits(BYTE x);
            BYTE OpAd(BYTE x, BYTE y);
            BYTE OpSb(BYTE x, BYTE y);
            BYTE OpAdb(BYTE x, BYTE y);
            BYTE OpSbb(BYTE x, BYTE y);
            BYTE OpAn(BYTE x, BYTE y);
            BYTE OpBit(BYTE x, BYTE y);
            BYTE OpXr(BYTE x, BYTE y);
            BYTE OpNa(BYTE x, BYTE y);
            BYTE OpOr(BYTE x, BYTE y);
            void DoPorts();
            BYTE OpRod(BYTE *x, BYTE y);
            BYTE OpRou(BYTE *x, BYTE y);
            BYTE OpMtb(BYTE *x, BYTE y);
            BYTE OpInv(BYTE *x, BYTE y);
            BYTE OpCmp(BYTE *x, BYTE y);
            BYTE OpCmpb(BYTE *x, BYTE y);
            BYTE OpDiu(BYTE *x, BYTE y);
            BYTE OpDid(BYTE *x, BYTE y);
            BYTE OpByu(BYTE *x, BYTE y);
            BYTE OpByd(BYTE *x, BYTE y);
            void OpXc(BYTE *x, BYTE *y);
            void OpXcls(BYTE *x, BYTE *y);
            void OpXchs(BYTE *x, BYTE *y);
            void OpSwp2(BYTE *x, BYTE *y);
            void OpKo(BYTE x);
            void OpIf(BYTE x);
            void OpAs(BYTE x);
            void OpIe(BYTE x);
            void OpFl(BYTE x);
            void Ldle(void *op2);
            void Ldlo(void *op2);
            void Stle(void *op2);
            void Stlo(void *op2);
            void Ldlem(void *op2);
            void Ldlom(void *op2);
            void Stlem(void *op2);
            void Stlom(void *op2);
            BYTE Get_kireg();
            BYTE Get_koreg();
            BYTE Get_asreg();
            BYTE Get_flag();
            BYTE Get_iereg();
            BYTE Get_ifreg();
            void execute_one(UINT8 op);

protected:

};




#endif // UPD1007_H
