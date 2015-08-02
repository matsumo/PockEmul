#ifndef TMC0501_H
#define TMC0501_H


/*
 * this code is based on TI-5x emulator
 * (c) 2014 Hynek Sladky
 * http://hsl.wz.cz/ti_59.htm
 */


#include "cpu.h"
#include "modelids.h"

#define MAXROM 0x07FF

typedef UINT8 TRegister[16];

// ====================================
// CPU state variables
// ====================================
typedef struct {
  // registers
  unsigned char A[16], B[16], C[16], D[16], E[16];
  // keyboard map
#define	KN_BIT	0
#define	KO_BIT	1
#define	KP_BIT	2
#define	KQ_BIT	3
#define	KR_BIT	4
#define	KS_BIT	5
#define	KT_BIT	6
  unsigned char key[16];
  // bit registers
  unsigned short KR, SR, fA, fB;
  // EXT signal (used for data exchange)
  unsigned short EXT;
  // PREG temporary register (used for direct PC control)
  unsigned short PREG;
  // ALU output signal (used for data exchange)
  unsigned char Sout[16];
  // various CPU flags
#define	FLG_IDLE	0x0001
#define	FLG_HOLD	0x0002
#define	FLG_JUMP	0x0004
#define	FLG_RCL_SHIFT	4
#define	FLG_RECALL	0x0010
#define	FLG_STORE	0x0020
#define	FLG_RAM_OP	0x0040
#define	FLG_RAM_READ	0x0080
#define	FLG_RAM_WRITE	0x0100
#define	FLG_EXT_VALID	0x0200
#define	FLG_IO_VALID	0x0400
#define	FLG_COND	0x0800 // must be here for easy jump processing...
#define	FLG_DISP	0x1000
#define	FLG_DISP_C	0x4000 // must be here for easy BUSY processing...
#define	FLG_BUSY	0x8000
  unsigned short flags;
  // R5 ALU register
  unsigned char R5;
  // cycle digit counter
  unsigned char digit;
  // CPU cycle counter (used to simulate real CPU frequency)
  unsigned cycle;
  // SCOM's & firstROM's address counter = program counter
  unsigned short addr;
  // SCOM register addressing
  unsigned char REG_ADDR;
  // RAM register addressing
  unsigned char RAM_ADDR, RAM_OP;
  // Library module addressing
  unsigned short LIB;
  // printer support
  char PRN_BUF[20];
  unsigned char PRN_PTR;
  unsigned PRN_BUSY;
  // card support
#define	CRD_LEN	(4+30*8+2)
  unsigned char CRD_PTR;
  unsigned char CRD_BUF[CRD_LEN]; // 246 bytes
  unsigned char CRD_FLAGS;
#define	CRD_READ	0x01
#define	CRD_WRITE	0x02
  // SCOM registers
  unsigned char SCOM[16][16];

  // RAM registers
  unsigned char RAM[120][16];
} TMC0501regs;

// mask definitions
typedef struct {
  unsigned char start, end, cpos, cval;
} mask_type;
static const mask_type mask_info[16] = {
  {-1, 0,  0,   0},
  {0, 15,  0,   0}, // ALL
  {0,  0,  0,   0}, // DPT
  {0,  0,  0,   1}, // DPT 1
  {0,  0,  0, 0xC}, // DPT C
  {3,  3,  3,   1}, // LLSD 1
  {1,  2,  1,   0}, // EXP
  {1,  2,  1,   1}, // EXP 1
  {-1, 0,  0,   0},
  {3, 15,  3,   0}, // MANT
  {-1, 0,  0,   0},
  {3, 15,  3,   5}, // MLSD 5
  {1, 15,  1,   0}, // MAEX
  {1, 15,  3,   1}, // MLSD 1
  {1, 15, 15,   1}, // MMSD 1
  {1, 15,  1,   1}  // MAEX 1
};

class Ctmc0501:public CCPU {

public:


    Ctmc0501(CPObject *parent,Models mod=TI59);
    virtual ~Ctmc0501();

    virtual	bool	init(void);						//initialize
    virtual	bool	exit(void);						//end
    virtual void	step(void);
    virtual void	Reset(void);

    virtual	void	Load_Internal(QXmlStreamReader *xmlIn);
    virtual	void	save_internal(QXmlStreamWriter *xmlOut);

    virtual	bool	Get_Xin(void) {return true;}
    virtual	void	Set_Xin(bool) {}
    virtual	bool	Get_Xout(void) {return true;}
    virtual	void	Set_Xout(bool) {}

    virtual	UINT32	get_PC(void) { return r->addr;}					//get Program Counter
    virtual void	Regs_Info(UINT8);


    QString IntToHex(int val, int nb);

    TMC0501regs *r;
private:

//    Tracer: TextFile;

    void BranchOP();
    void CallOP();
    void FlagOP();
    void Error(QString msg);
    void MaskOP();
    void MiscOP();
    void Emulate();
    int execute(unsigned short opcode);
    void Xch(unsigned char *src1, unsigned char *src2, const mask_type *mask);
    void Alu(unsigned char *dst, unsigned char *srcX, unsigned char *srcY, const mask_type *mask, unsigned char flags);

    Models currentModel;
};


#endif // TMC0501

