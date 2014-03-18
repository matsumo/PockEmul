/*
    Zilog Z80 Emulator Header
*/

#if !defined(Z80_H)
#define Z80_H

#include "cpu.h"

#include <stdio.h>
//#if defined(Z80_USE_SDL)
//#	include "SDL_byteorder.h"
//#	if SDL_BYTEORDER == SDL_LIL_ENDIAN
//#		define Z80_LITTLEENDIAN	1
//#	else
//#		define Z80_BIGENDIAN	1
//#	endif
//#endif



#ifdef POCKEMUL_BIG_ENDIAN
#   define Z80_BIGENDIAN	1
#else
#   define Z80_LITTLEENDIAN	1
#endif



/* z80exec()の戻り値 */
#define Z80_RUN	0	/* 動作中 */
#define Z80_HALT	1	/* HALT中 */
#define Z80_UNDERFLOW	2	/* スタックアンダーフロー */

#define Z80_STATES(z)	((z)->i.total_states - (z)->i.states)
#define Z80_RESET_STATES(z)	((z)->i.total_states = (z)->i.states)

typedef unsigned char	uint8;
typedef signed   char	int8;
typedef unsigned short	uint16;
typedef signed   short	int16;
typedef unsigned long	uint32;
typedef signed   long	int32;

//#pragma pack(1)

/* 8bitsレジスタ */
typedef struct {
    uint8 *m;
#if defined(Z80_LITTLEENDIAN)
    uint8 f, a;
    uint8 c, b;
    uint8 e, d;
    uint8 l, h;
    uint8 ixl, ixh;
    uint8 iyl, iyh;
    uint8 i;
    uint8 pad1;
    uint8 f_d, a_d;
#elif defined(Z80_BIGENDIAN)
    uint8 a, f;
    uint8 b, c;
    uint8 d, e;
    uint8 h, l;
    uint8 ixh, ixl;
    uint8 iyh, iyl;
    uint8 pad2;
    uint8 i;
    uint8 a_d, f_d;
#endif
    uint8 pad3, pad4;
    uint8 pad5, pad6;
    uint8 pad7, pad8;
    uint8 pad9, pad10;
    uint8 pad11, pad12;
    uint8 iff;	/* IFF1, IFF2 */
    uint8 im;
    uint8 halt;	/* HALT*/
    uint8 pad13;
    uint8 pad14, pad15;
} Z80regs;

typedef struct {
    uint8 *m;
    uint16 af;
    uint16 bc;
    uint16 de;
    uint16 hl;
    uint16 ix;
    uint16 iy;
    uint16 pad2;
    uint16 af_d;
    uint16 bc_d;
    uint16 de_d;
    uint16 hl_d;
    uint16 sp;
    uint16 pc;
} Z80regs16;


typedef struct {
    Z80regs pad1;
    int states;
    uint16 stack_under;
    uint16 pad2;
    int total_states;
    int trace;
    int emulate_subroutine;
    void *tag;
} Z80info;

typedef union {
    uint8     *m;
    Z80regs   r;
    Z80regs16 r16;
    Z80info   i;
} Z80stat;


//#pragma pack()

/* z80.c */
class CZ80:public CCPU{

public:

    bool	init(void);						//initialize
    bool	exit(void);						//end
    virtual void	step(void);
    void	Reset(void);

    void	Load_Internal(QFile *file);
    void	save_internal(QFile *file);
    void	Load_Internal(QXmlStreamReader *);
    void	save_internal(QXmlStreamWriter *);

    UINT32	get_mem(UINT32 adr,int size);		//get memory
    void	set_mem(UINT32 adr,int size,UINT32 data);	//set memory

    bool	Get_Xin(void);
    void	Set_Xin(bool);
    bool	Get_Xout(void);
    void	Set_Xout(bool);

    UINT32	get_PC(void);					//get Program Counter
    void	Regs_Info(UINT8);

    void	TIMER_INC(void);
    bool	Is_Timer_Reached;

//    void	AddState(UINT8);

    UINT8	cpu_readop(UINT32);


    //LH5801REG lh5801;
    Z80stat z80;
//    long	ticks;

    const char*	GetClassName(){ return("CZ80");}




//    void	RTN(void);
//    void	Set_C(void);
//    void	UnSet_C(void);


public:
    void z80srand(uint32);
    int z80reset(Z80stat *);
    int z80nmi(Z80stat *);
    int z80int0chk(const Z80stat *);
    int z80int0(Z80stat *, uint8);
    int z80int1chk(const Z80stat *);
    int z80int1(Z80stat *);
    int z80int2chk(const Z80stat *);
    int z80int2(Z80stat *, uint8);
    int z80exec(Z80stat *);
    int z80retn(Z80stat *);

    /* z80disasm.c */
    void *z80disasm(char *, uint8 *);
    char *z80regs(char *, const Z80stat *);

    //#include "z80memory.h"
    uint8 z80read8(const Z80stat *z, uint16 address);
    void z80write8(const Z80stat *z, uint16 address, uint8 value);

    /*
        16bits READ/WRITE
    */
    uint16 z80read16(const Z80stat *z, uint16 address);
    void z80write16(const Z80stat *z, uint16 address, uint16 value);



    int z80inport(Z80stat *z, uint8 *x, uint8 address);
    int z80outport(Z80stat *z, uint8 address, uint8 x);
    //int z80subroutine(Z80stat *, uint16);
    void z80log(const Z80stat *);

    static uint32 rnd;
    static const int state_xx[];
    const static int state_cb_xx[];
    const static int state_dd_xx[] ;
    const static int state_dd_cb_xx[];
    const static int state_ed_xx[] ;
    const static uint16 len_xx[];
    const static uint16 len_cb_xx[];
    const static uint16 len_dd_xx[];
    const static uint16 len_dd_cb_xx[];
    const static uint16 len_ed_xx[];
    const static uint8 parity[];

    static void daa_result(uint8 *x, uint8 *c, uint8 a, uint8 f);

    CZ80(CPObject *parent);
    virtual ~CZ80();

    int z80nsc800intr(Z80stat *z, uint8 vector);
};
#endif

/*
    Copyright 2005 ~ 2009 maruhiro
    All rights reserved.

    Redistribution and use in source and binary forms,
    with or without modification, are permitted provided that
    the following conditions are met:

     1. Redistributions of source code must retain the above copyright notice,
        this list of conditions and the following disclaimer.

     2. Redistributions in binary form must reproduce the above copyright notice,
        this list of conditions and the following disclaimer in the documentation
        and/or other materials provided with the distribution.

    THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
    INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
    FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
    THE REGENTS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
    SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
    PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
    OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
    WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
    OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
    ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/* eof */
