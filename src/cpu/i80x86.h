#ifndef I80X86_H
#define I80X86_H

#if 1
/*
    Intel 80186/80188 emulator header
*/

#include "cpu.h"

#ifdef POCKEMUL_BIG_ENDIAN
#		define I86_BIGENDIAN	1
#else
#		define I86_LITTLEENDIAN	1
#endif


#define I86_RUN	0
#define I86_HALT	1

#if 1
typedef unsigned char	uint8;
typedef signed char	int8;
typedef unsigned short	uint16;
typedef signed short	int16;
typedef unsigned long	uint32;
typedef signed long	int32;
#else
typedef quint8	uint8;
typedef qint8	int8;
typedef quint16	uint16;
typedef qint16	int16;
typedef quint32	uint32;
typedef qint32	int32;

#endif

typedef struct {
    uint8 *m;
    uint16 ax;
    uint16 cx;
    uint16 bx;
    uint16 dx;
    uint16 sp;
    uint16 bp;
    uint16 si;
    uint16 di;
    uint16 cs;
    uint16 ds;
    uint16 es;
    uint16 ss;
    uint16 ip;
    uint16 f;
    uint16 hlt;
    uint16 *prefix;
} I86regs16;

typedef struct {
    uint8 *m;
#if defined(I86_BIGENDIAN)
    uint8 ah, al;
    uint8 ch, cl;
    uint8 bh, bl;
    uint8 dh, dl;
#else
    uint8 al, ah;
    uint8 cl, ch;
    uint8 bl, bh;
    uint8 dl, dh;
#endif
} I86regs8;

typedef struct {
    I86regs16 pad;
    void *tag;
    int total_states;
    int states;
    int trace;
} I86info;

typedef union {
    uint8 *m;
    I86regs16 r16;
    I86regs8 r8;
    I86info i;
} I86stat;

class Ci80x86 : public CCPU
{
public:
    Ci80x86(CPObject *parent);
    virtual ~Ci80x86();

    virtual	bool	init(void);						//initialize
    virtual	bool	exit(void);						//end
    virtual void	step(void);						//step SC61860
    virtual void	Reset(void);

    virtual	void	Load_Internal(QXmlStreamReader *);
    virtual	void	save_internal(QXmlStreamWriter *);

    virtual bool Get_Xin(){ return true;}
    virtual void Set_Xin(bool){}
    virtual bool Get_Xout(){ return true;}
    virtual void Set_Xout(bool){}
    virtual UINT32 get_PC();
    virtual void Regs_Info(UINT8 Type);

    uint8 i86read8(const I86stat *, uint16, uint16);
    void i86write8(I86stat *, uint16, uint16, uint8);
    uint16 i86read16(const I86stat *, uint16, uint16);
    void i86write16(I86stat *, uint16, uint16, uint16);
    uint8 i86inp8(const I86stat *, uint16);
    uint16 i86inp16(const I86stat *, uint16);
    void i86out8(I86stat *, uint16, uint8);
    void i86out16(I86stat *, uint16, uint16);
    void i86reset(I86stat *);
    void i86trace(const I86stat *);

    int i86disasm(char *, const I86stat *, uint16, uint16);
    char *i86regs(char *, const I86stat *);

    int i86nmi(I86stat *);
    int i86int(I86stat *, int);
    int i86exec(I86stat *);

    static const int op_length[];
    static const int regrm_length[];
    static const int op_states[];
    static const int op_mem_states[];
    static const int math_states[];
    static const int math_mem_states[];
    static const int shift_states[];
    static const int shift_mem_states[];
    static const int shift1_states[];
    static const int shift1_mem_states[];
    static const int math16s_states[];
    static const int math16s_mem_states[];
    static const int grp1_8_states[];
    static const int grp1_8_mem_states[];
    static const int grp1_16_mem_states[];
    static const int grp1_16_states[];
    static const int grp2_8_mem_states[];
    static const int grp2_8_states[];
    static const int grp2_16_states[];
    static const int grp2_16_mem_states[];
    static const uint16 parity[];

    uint8 getreg8(const I86stat *i86, uint8 reg);
    void setreg8(I86stat *i86, uint8 reg, uint8 x);
    uint16 getreg16(const I86stat *i86, uint8 reg);
    void setreg16(I86stat *i86, uint8 reg, uint16 x);
    uint16 getsreg(const I86stat *i86, uint8 reg);
    void setsreg(I86stat *i86, uint8 reg, uint16 x);
    void getsegoff(const I86stat *i86, uint8 rm, uint16 *seg, uint16 *off);
    uint8 getrm8(const I86stat *i86, uint8 rm);
    void setrm8(I86stat *i86, uint8 rm, uint8 x);
    uint16 getrm16(const I86stat *i86, uint8 rm);
    void setrm16(I86stat *i86, uint8 rm, uint16 x);


    I86stat i86stat;

};



#endif

/*
    Copyright 2009 maruhiro
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

#endif // I80X86_H
