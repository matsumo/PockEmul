#ifndef UPD1007_H
#define UPD1007_H

/*
This CPU core is based on documentations works done by:
- Piotr Piatek ( http://www.pisi.com.pl/piotr433/fx8000ee.htm )
*/

#include "cpu.h"

class CpcXXXX;
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

#define INT_LATENCY 7



// device config
struct upd1007_config
{
    CpcXXXX *pPC;
    UINT16 ea;  /* temporary pointer used in the indexed addressing mode */
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

typedef void   (*Proc2) (upd1007_config *info,void *op);
typedef void   (*Proc3) (upd1007_config *info,UINT8 x, UINT8 y);
typedef void   (*Proc4) (upd1007_config *info,UINT8 x);
typedef void   (*Proc5) (upd1007_config *info,UINT8 *x, UINT8 *y);
typedef UINT8  (*Func1)(upd1007_config *info);
typedef UINT8  (*Func2)(upd1007_config *info,UINT8 x);
typedef UINT8  (*Func3)(upd1007_config *info,UINT8 x, UINT8 y);
typedef UINT16 (*Func4)(upd1007_config *info,UINT8 x, UINT8 y);
typedef UINT16 (*Func5)(upd1007_config *info,UINT8 *x, UINT8 y);

#define ROM0SIZE 0x1800
#define ROM1SIZE 0x8000
#define RAMSTART 0xC000
#define RAMSIZE	 0x2000
#define RAMEND	 RAMSTART+RAMSIZE
#define NONE_REG 0xff     /* index of a non-existing general purpose register */

class CUPD1007 : public CCPU
{
public:

    CUPD1007(CPObject *parent, QString rom0fn);
    virtual ~CUPD1007();

    upd1007_config reginfo;

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

            BYTE rom0[ROM0SIZE][3];

            const static BYTE cc[8];
            /* bits of the IE register */
            const static BYTE INT_enable[3];
        /* bits of the IF register */
            const static BYTE INT_serv[3];
            const static BYTE INT_input[3];

            static void addState(upd1007_config *info, int x);
            UINT16 Fetchopcode();
            static BYTE FetchByte(upd1007_config *info);
            static BYTE Reg1(BYTE x);
            static BYTE Reg2(upd1007_config *info, BYTE x);
            static BYTE Reg3(upd1007_config *info, BYTE x, BYTE y);
            static BYTE AsLimit(upd1007_config *info, BYTE y);
            static BYTE Rl1(BYTE x, BYTE y);
            static BYTE Im6(BYTE x, BYTE y);
            static BYTE Ireg(upd1007_config *info);
            static void NextReg(upd1007_config *info,BYTE *x);
            static UINT16 Wreg(upd1007_config *info, BYTE x, BYTE y);
            static UINT16 PostIncw(upd1007_config *info,BYTE x, BYTE y);
            static UINT16 PostDecw(upd1007_config *info,BYTE x, BYTE y);
            static UINT16 PreIncw(upd1007_config *info,BYTE x, BYTE y);
            static UINT16 PreDecw(upd1007_config *info, BYTE x, BYTE y);
            static void PlusOffset(upd1007_config *info,BYTE x);
            static void MinusOffset(upd1007_config *info,BYTE x);
            static void UnReg(upd1007_config *info,void *op2);
            static void RotReg(upd1007_config *info,void *op2);
            static void UnAry(upd1007_config *info,void *op2);
            static void RotAry(upd1007_config *info,void *op2);
            static void Mtbm(upd1007_config *info,void *op2);
            static void Xreg(upd1007_config *info,void *op2);
            static void Yreg(upd1007_config *info,void *op2);
            static void TXreg(upd1007_config *info,void *op2);
            static void TYreg(upd1007_config *info,void *op2);
            static void ExchReg(upd1007_config *info,void *op2);
            static void RegIm8(upd1007_config *info,void *op2);
            static void TRegIm8(upd1007_config *info,void *op2);
            static void Xary(upd1007_config *info,void *op2);
            static void Yary(upd1007_config *info,void *op2);
            static void TXary(upd1007_config *info,void *op2);
            static void TYary(upd1007_config *info,void *op2);
            static void ExchAry(upd1007_config *info,void *op2);
            static void AryIm6(upd1007_config *info,void *op2);
            static void TAryIm6(upd1007_config *info,void *op2);
            static void Ldw(upd1007_config *info,void *op2);
            static void AdwSbw(upd1007_config *info,void *op2);
            static void Cond(upd1007_config *info,void *op2);
            static void NotCond(upd1007_config *info,void *op2);
            static void KeyCond(upd1007_config *info,void *op2);
            static void NotKeyCond(upd1007_config *info,void *op2);
            static void Jump(upd1007_config *info, BYTE x1, BYTE x2);
            static void Call(upd1007_config *info,BYTE x1, BYTE x2);
            static void Trap(upd1007_config *info, void *op2);
            static void Ijmp(upd1007_config *info, BYTE x1, BYTE x2);
            static void Rtn(upd1007_config *info,void *op2);
            static void Cani(upd1007_config *info,void *op2);
            static void Rti(upd1007_config *info,void *op2);
            static void Nop(upd1007_config *info,void *op2);
            static void BlockCopy(upd1007_config *info,void *op2);
            static void BlockSearch(upd1007_config *info,void *op2);
            static void StMemoReg(upd1007_config *info,void *op2);
            static void StMemoIm8(upd1007_config *info,void *op2);
            static void StmMemoAry(upd1007_config *info,void *op2);
            static BYTE OpSwp1(upd1007_config *info, BYTE *x, BYTE y);
            static BYTE OpBnus(upd1007_config *info,BYTE *x, BYTE y);
            static BYTE OpLd(upd1007_config *info,BYTE x, BYTE y);
            static void StImOffsReg(upd1007_config *info,void *op2);
            static void StRegOffsReg(upd1007_config *info,void *op2);
            static void StmImOffsAry(upd1007_config *info,void *op2);
            static void StmRegOffsAry(upd1007_config *info,void *op2);
            static void LdRegMemo(upd1007_config *info,void *op2);
            static void LdmAryMemo(upd1007_config *info,void *op2);
            static void LdRegImOffs(upd1007_config *info,void *op2);
            static void LdRegRegOffs(upd1007_config *info,void *op2);
            static void LdmAryImOffs(upd1007_config *info,void *op2);
            static void LdmAryRegOffs(upd1007_config *info,void *op2);
            static void PstIm8(upd1007_config *info,void *op2);
            static void PstReg(upd1007_config *info,void *op2);
            static void Gst(upd1007_config *info,void *op2);
            static void Off(upd1007_config *info,void *op2);
            static void ZeroBits(upd1007_config *info,BYTE x);
            static BYTE OpAd(upd1007_config *info, BYTE x, BYTE y);
            static BYTE OpSb(upd1007_config *info,BYTE x, BYTE y);
            static BYTE OpAdb(upd1007_config *info,BYTE x, BYTE y);
            static BYTE OpSbb(upd1007_config *info,BYTE x, BYTE y);
            static BYTE OpAn(upd1007_config *info,BYTE x, BYTE y);
            static BYTE OpBit(upd1007_config *info,BYTE x, BYTE y);
            static BYTE OpXr(upd1007_config *info,BYTE x, BYTE y);
            static BYTE OpNa(upd1007_config *info,BYTE x, BYTE y);
            static BYTE OpOr(upd1007_config *info,BYTE x, BYTE y);
            static void DoPorts(upd1007_config *info);
            static BYTE OpRod(upd1007_config *info,BYTE *x, BYTE y);
            static BYTE OpRou(upd1007_config *info,BYTE *x, BYTE y);
            static BYTE OpMtb(upd1007_config *info,BYTE *x, BYTE y);
            static BYTE OpInv(upd1007_config *info,BYTE *x, BYTE y);
            static BYTE OpCmp(upd1007_config *info,BYTE *x, BYTE y);
            static BYTE OpCmpb(upd1007_config *info,BYTE *x, BYTE y);
            static BYTE OpDiu(upd1007_config *info,BYTE *x, BYTE y);
            static BYTE OpDid(upd1007_config *info,BYTE *x, BYTE y);
            static BYTE OpByu(upd1007_config *info,BYTE *x, BYTE y);
            static BYTE OpByd(upd1007_config *info,BYTE *x, BYTE y);
            static void OpXc(upd1007_config *info,BYTE *x, BYTE *y);
            static void OpXcls(upd1007_config *info,BYTE *x, BYTE *y);
            static void OpXchs(upd1007_config *info,BYTE *x, BYTE *y);
            static void OpSwp2(upd1007_config *info,BYTE *x, BYTE *y);
            static void OpKo(upd1007_config *info,BYTE x);
            static void OpIf(upd1007_config *info,BYTE x);
            static void OpAs(upd1007_config *info,BYTE x);
            static void OpIe(upd1007_config *info,BYTE x);
            static void OpFl(upd1007_config *info,BYTE x);
            static void Ldle(upd1007_config *info,void *op2);
            static void Ldlo(upd1007_config *info,void *op2);
            static void Stle(upd1007_config *info,void *op2);
            static void Stlo(upd1007_config *info,void *op2);
            static void Ldlem(upd1007_config *info,void *op2);
            static void Ldlom(upd1007_config *info,void *op2);
            static void Stlem(upd1007_config *info,void *op2);
            static void Stlom(upd1007_config *info,void *op2);
            static BYTE Get_kireg(upd1007_config *info);
            static BYTE Get_koreg(upd1007_config *info);
            static BYTE Get_asreg(upd1007_config *info);
            static BYTE Get_flag(upd1007_config *info);
            static BYTE Get_iereg(upd1007_config *info);
            static BYTE Get_ifreg(upd1007_config *info);

            void ExecInstr();
protected:

};




#endif // UPD1007_H
