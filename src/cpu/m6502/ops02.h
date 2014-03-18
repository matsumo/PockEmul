#ifndef OPS02_H
#define OPS02_H


// vectors
#define NMI_VEC	0xfffa
#define RST_VEC	0xfffc
#define IRQ_VEC	0xfffe

// flags
#define F_C	0x01
#define F_Z	0x02
#define F_I	0x04
#define F_D	0x08
#define F_B	0x10
#define F_T	0x20
#define F_V	0x40
#define F_N	0x80

// some shortcuts for improved readability
#define A	a
#define X	x
#define Y	y
#define P	p
#define S	sp.b.l
#define SPD	sp.d

#define SET_NZ(n) \
    if((n) == 0) \
        P = (P & ~F_N) | F_Z; \
    else \
        P = (P & ~(F_N | F_Z)) | ((n) & F_N)

#define SET_Z(n) \
    if((n) == 0) \
        P |= F_Z; \
    else \
        P &= ~F_Z

#define EAL ea.b.l
#define EAH ea.b.h
#define EAW ea.w.l
#define EAD ea.d

#define ZPL zp.b.l
#define ZPH zp.b.h
#define ZPW zp.w.l
#define ZPD zp.d

#define PCL pc.b.l
#define PCH pc.b.h
#define PCW pc.w.l
#define PCD pc.d

// virtual machine interface

#define RDMEM_ID(addr) read_data8(addr)
#define WRMEM_ID(addr, data) write_data8(addr, data)

#define RDOP() read_data8(PCW++)
#define PEEKOP() read_data8(PCW)
#define RDOPARG() read_data8(PCW++)

#define RDMEM(addr) read_data8(addr)
#define WRMEM(addr, data) write_data8(addr, data)

#define CYCLES(c) pPC->pTIMER->state += (c)

// branch relative

#define BRA(cond) { \
    int8 tmp2 = RDOPARG(); \
    if(cond) { \
        RDMEM(PCW); \
        EAW = PCW + (int8)tmp2; \
        if(EAH != PCH) { \
            /*RDMEM((PCH << 8) | EAL) ;*/ \
            CYCLES(1); \
        } \
        PCD = EAD; \
        CYCLES(1); \
    } \
}

// Helper macros to build the effective address

#define EA_ZPG \
    ZPL = RDOPARG(); \
    EAD = ZPD

#define EA_ZPX \
    ZPL = RDOPARG(); \
    RDMEM(ZPD); \
    ZPL = X + ZPL; \
    EAD = ZPD

#define EA_ZPY \
    ZPL = RDOPARG(); \
    RDMEM(ZPD); \
    ZPL = Y + ZPL; \
    EAD = ZPD

#define EA_ABS \
    EAL = RDOPARG(); \
    EAH = RDOPARG()

#define EA_ABX_P \
    EA_ABS; \
    if(EAL + X > 0xff) { \
        /*RDMEM((EAH << 8) | ((EAL + X) & 0xff));*/ \
        CYCLES(1); \
    } \
    EAW += X;

#define EA_ABX_NP \
    EA_ABS; \
    /*RDMEM((EAH << 8) | ((EAL + X) & 0xff));*/ \
    EAW += X

#define EA_ABY_P \
    EA_ABS; \
    if(EAL + Y > 0xff) { \
        /*RDMEM((EAH << 8) | ((EAL + Y) & 0xff));*/ \
        CYCLES(1); \
    } \
    EAW += Y;

#define EA_ABY_NP \
    EA_ABS; \
    /*RDMEM((EAH << 8) | ((EAL + Y) & 0xff));*/ \
    EAW += Y

#define EA_IDX \
    ZPL = RDOPARG(); \
    RDMEM(ZPD); \
    ZPL = ZPL + X; \
    EAL = RDMEM(ZPD); \
    ZPL++; \
    EAH = RDMEM(ZPD)

#define EA_IDY_P \
    ZPL = RDOPARG(); \
    EAL = RDMEM(ZPD); \
    ZPL++; \
    EAH = RDMEM(ZPD); \
    if(EAL + Y > 0xff) { \
        /*RDMEM((EAH << 8) | ((EAL + Y) & 0xff));*/ \
        CYCLES(1); \
    } \
    EAW += Y;

#define EA_IDY_NP \
    ZPL = RDOPARG(); \
    EAL = RDMEM(ZPD); \
    ZPL++; \
    EAH = RDMEM(ZPD); \
    /*RDMEM((EAH << 8) | ((EAL + Y) & 0xff));*/ \
    EAW += Y

#define EA_ZPI \
    ZPL = RDOPARG(); \
    EAL = RDMEM(ZPD); \
    ZPL++; \
    EAH = RDMEM(ZPD)

#define EA_IND \
    EA_ABS; \
    tmp = RDMEM(EAD); \
    EAL++; \
    EAH = RDMEM(EAD); \
    EAL = tmp

// read a value into tmp

#define RD_IMM		tmp = RDOPARG()
#define RD_DUM		RDMEM(PCW)
#define RD_ACC		tmp = A
#define RD_ZPG		EA_ZPG; tmp = RDMEM(EAD)
#define RD_ZPX		EA_ZPX; tmp = RDMEM(EAD)
#define RD_ZPY		EA_ZPY; tmp = RDMEM(EAD)
#define RD_ABS		EA_ABS; tmp = RDMEM(EAD)
#define RD_ABX_P	EA_ABX_P; tmp = RDMEM(EAD)
#define RD_ABX_NP	EA_ABX_NP; tmp = RDMEM(EAD)
#define RD_ABY_P	EA_ABY_P; tmp = RDMEM(EAD)
#define RD_ABY_NP	EA_ABY_NP; tmp = RDMEM(EAD)
#define RD_IDX		EA_IDX; tmp = RDMEM_ID(EAD)
#define RD_IDY_P	EA_IDY_P; tmp = RDMEM_ID(EAD)
#define RD_IDY_NP	EA_IDY_NP; tmp = RDMEM_ID(EAD)
#define RD_ZPI		EA_ZPI; tmp = RDMEM(EAD)

// write a value from tmp

#define WR_ZPG		EA_ZPG; WRMEM(EAD, tmp)
#define WR_ZPX		EA_ZPX; WRMEM(EAD, tmp)
#define WR_ZPY		EA_ZPY; WRMEM(EAD, tmp)
#define WR_ABS		EA_ABS; WRMEM(EAD, tmp)
#define WR_ABX_NP	EA_ABX_NP; WRMEM(EAD, tmp)
#define WR_ABY_NP	EA_ABY_NP; WRMEM(EAD, tmp)
#define WR_IDX		EA_IDX; WRMEM_ID(EAD, tmp)
#define WR_IDY_NP	EA_IDY_NP; WRMEM_ID(EAD, tmp)
#define WR_ZPI		EA_ZPI; WRMEM(EAD, tmp)

// dummy read from the last EA

#define RD_EA	RDMEM(EAD)

// write back a value from tmp to the last EA

#define WB_ACC	A = (uint8)tmp;
#define WB_EA	WRMEM(EAD, tmp)

// opcodes

#define PUSH(Rg) WRMEM(SPD, Rg); S--
#define PULL(Rg) S++; Rg = RDMEM(SPD)

#ifdef HAS_N2A03
#define ADC \
    { \
        int c = (P & F_C); \
        int sum = A + tmp + c; \
        P &= ~(F_V | F_C); \
        if(~(A ^ tmp) & (A ^ sum) & F_N) { \
            P |= F_V; \
        } \
        if(sum & 0xff00) { \
            P |= F_C; \
        } \
        A = (uint8)sum; \
    } \
    SET_NZ(A)
#else
#define ADC \
    if(P & F_D) { \
        int c = (P & F_C); \
        int lo = (A & 0x0f) + (tmp & 0x0f) + c; \
        int hi = (A & 0xf0) + (tmp & 0xf0); \
        P &= ~(F_V | F_C | F_N | F_Z); \
        if(!((lo + hi) & 0xff)) { \
            P |= F_Z; \
        } \
        if(lo > 0x09) { \
            hi += 0x10; \
            lo += 0x06; \
        } \
        if(hi & 0x80) { \
            P |= F_N; \
        } \
        if(~(A ^ tmp) & (A ^ hi) & F_N) { \
            P |= F_V; \
        } \
        if(hi > 0x90) { \
            hi += 0x60; \
        } \
        if(hi & 0xff00) { \
            P |= F_C; \
        } \
        A = (lo & 0x0f) + (hi & 0xf0); \
    } \
    else { \
        int c = (P & F_C); \
        int sum = A + tmp + c; \
        P &= ~(F_V | F_C); \
        if(~(A ^ tmp) & (A ^ sum) & F_N) { \
            P |= F_V; \
        } \
        if(sum & 0xff00) { \
            P |= F_C; \
        } \
        A = (uint8)sum; \
        SET_NZ(A); \
    }
#endif

#define AND \
    A = (uint8)(A & tmp); \
    SET_NZ(A)

#define ASL \
    P = (P & ~F_C) | ((tmp >> 7) & F_C); \
    tmp = (uint8)(tmp << 1); \
    SET_NZ(tmp)

#define BCC BRA(!(P & F_C))
#define BCS BRA(P & F_C)
#define BEQ BRA(P & F_Z)

#define BIT \
    P &= ~(F_N | F_V | F_Z); \
    P |= tmp & (F_N | F_V); \
    if((tmp & A) == 0) \
        P |= F_Z

#define BMI BRA(P & F_N)
#define BNE BRA(!(P & F_Z))
#define BPL BRA(!(P & F_N))

#define BRK \
    RDOPARG(); \
    PUSH(PCH); \
    PUSH(PCL); \
    PUSH(P | F_B); \
    P = (P | F_I); \
    PCL = RDMEM(IRQ_VEC); \
    PCH = RDMEM(IRQ_VEC + 1)

#define BVC BRA(!(P & F_V))
#define BVS BRA(P & F_V)

#define CLC P &= ~F_C
#define CLD P &= ~F_D
#define CLI \
    if(irq_state && (P & F_I)) { \
        if(PEEKOP() != 0x40) { \
            after_cli = true; \
        } \
    } \
    P &= ~F_I
#define CLV P &= ~F_V

#define CMP \
    P &= ~F_C; \
    if(A >= tmp) { \
        P |= F_C; \
    } \
    SET_NZ((uint8)(A - tmp))
#define CPX \
    P &= ~F_C; \
    if(X >= tmp) { \
        P |= F_C; \
    } \
    SET_NZ((uint8)(X - tmp))
#define CPY \
    P &= ~F_C; \
    if(Y >= tmp) { \
        P |= F_C; \
    } \
    SET_NZ((uint8)(Y - tmp))

#define DEC \
    tmp = (uint8)(tmp - 1); \
    SET_NZ(tmp)
#define DEX \
    X = (uint8)(X - 1); \
    SET_NZ(X)
#define DEY \
    Y = (uint8)(Y - 1); \
    SET_NZ(Y)

#define EOR \
    A = (uint8)(A ^ tmp); \
    SET_NZ(A)

#define INC \
    tmp = (uint8)(tmp + 1); \
    SET_NZ(tmp)
#define INX \
    X = (uint8)(X + 1); \
    SET_NZ(X)
#define INY \
    Y = (uint8)(Y + 1); \
    SET_NZ(Y)

#define JMP PCD = EAD
#define JSR \
    EAL = RDOPARG(); \
    RDMEM(SPD); \
    PUSH(PCH); \
    PUSH(PCL); \
    EAH = RDOPARG(); \
    CallSubLevel++; \
    PCD = EAD

#define LDA \
    A = (uint8)tmp; \
    SET_NZ(A)
#define LDX \
    X = (uint8)tmp; \
    SET_NZ(X)
#define LDY \
    Y = (uint8)tmp; \
    SET_NZ(Y)

#define LSR \
    P = (P & ~F_C) | (tmp & F_C); \
    tmp = (uint8)tmp >> 1; \
    SET_NZ(tmp)

#define NOP

#define ORA \
    A = (uint8)(A | tmp); \
    SET_NZ(A)

#define PHA PUSH(A)
#define PHP PUSH(P)

#define PLA \
    RDMEM(SPD); \
    PULL(A); \
    SET_NZ(A)
#define PLP \
    RDMEM(SPD); \
    if(P & F_I) { \
        PULL(P); \
        if(irq_state && !(P & F_I)) { \
            after_cli = true; \
        } \
    } \
    else { \
        PULL(P); \
    } \
    P |= (F_T | F_B);

#define ROL \
    tmp = (tmp << 1) | (P & F_C); \
    P = (P & ~F_C) | ((tmp >> 8) & F_C); \
    tmp = (uint8)tmp; \
    SET_NZ(tmp)
#define ROR \
    tmp |= (P & F_C) << 8; \
    P = (P & ~F_C) | (tmp & F_C); \
    tmp = (uint8)(tmp >> 1); \
    SET_NZ(tmp)

#define RTI \
    RDOPARG(); \
    RDMEM(SPD); \
    PULL(P); \
    PULL(PCL); \
    PULL(PCH); \
    CallSubLevel--; \
    P |= F_T | F_B; \
    if(irq_state && !(P & F_I)) { \
        after_cli = true; \
    }
#define RTS \
    RDOPARG(); \
    RDMEM(SPD); \
    PULL(PCL); \
    PULL(PCH); \
    RDMEM(PCW); \
    PCW++; \
    CallSubLevel--;

#ifdef HAS_N2A03
#define SBC \
    { \
        int c = (P & F_C) ^ F_C; \
        int sum = A - tmp - c; \
        P &= ~(F_V | F_C); \
        if((A ^ tmp) & (A ^ sum) & F_N) { \
            P |= F_V; \
        } \
        if((sum & 0xff00) == 0) { \
            P |= F_C; \
        } \
        A = (uint8)sum; \
    } \
    SET_NZ(A)
#else
#define SBC \
    if(P & F_D) { \
        int c = (P & F_C) ^ F_C; \
        int sum = A - tmp - c; \
        int lo = (A & 0x0f) - (tmp & 0x0f) - c; \
        int hi = (A & 0xf0) - (tmp & 0xf0); \
        if(lo & 0x10) { \
            lo -= 6; \
            hi--; \
        } \
        P &= ~(F_V | F_C | F_Z | F_N); \
        if((A ^ tmp) & (A ^ sum) & F_N) { \
            P |= F_V; \
        } \
        if(hi & 0x0100) { \
            hi -= 0x60; \
        } \
        if((sum & 0xff00) == 0) { \
            P |= F_C; \
        } \
        if(!((A - tmp - c) & 0xff)) { \
            P |= F_Z; \
        } \
        if((A - tmp - c) & 0x80) { \
            P |= F_N; \
        } \
        A = (lo & 0x0f) | (hi & 0xf0); \
    } \
    else { \
        int c = (P & F_C) ^ F_C; \
        int sum = A - tmp - c; \
        P &= ~(F_V | F_C); \
        if((A ^ tmp) & (A ^ sum) & F_N) { \
            P |= F_V; \
        } \
        if((sum & 0xff00) == 0) { \
            P |= F_C; \
        } \
        A = (uint8)sum; \
        SET_NZ(A); \
    }
#endif

#define SEC P |= F_C
#define SED P |= F_D
#define SEI P |= F_I

#define STA tmp = A
#define STX tmp = X
#define STY tmp = Y

#define TAX \
    X = A; \
    SET_NZ(X)
#define TAY \
    Y = A; \
    SET_NZ(Y)
#define TSX \
    X = S; \
    SET_NZ(X)
#define TXA \
    A = X; \
    SET_NZ(A)
#define TXS S = X
#define TYA \
    A = Y; \
    SET_NZ(A)

#define ANC \
    P &= ~F_C; \
    A = (uint8)(A & tmp); \
    if(A & 0x80) { \
        P |= F_C; \
    } \
    SET_NZ(A)

#define ASR \
    tmp &= A; \
    LSR

#define AST \
    S &= tmp; \
    A = X = S; \
    SET_NZ(A)

#ifdef HAS_N2A03
#define ARR \
    { \
        tmp &= A; \
        ROR; \
        P &=~(F_V| F_C); \
        if(tmp & 0x40) { \
            P |= F_C; \
        } \
        if((tmp & 0x60) == 0x20 || (tmp & 0x60) == 0x40) { \
            P |= F_V; \
        } \
    }
#else
#define ARR \
    if(P & F_D) { \
        tmp &= A; \
        int t = tmp; \
        int hi = tmp & 0xf0; \
        int lo = tmp & 0x0f; \
        if(P & F_C) { \
            tmp = (tmp >> 1) | 0x80; \
            P |= F_N; \
        } \
        else { \
            tmp >>= 1; \
            P &= ~F_N; \
        } \
        if(tmp) { \
            P &= ~F_Z; \
        } \
        else { \
            P |= F_Z; \
        } \
        if((t ^ tmp) & 0x40) { \
            P |= F_V; \
        } \
        else { \
            P &= ~F_V; \
        } \
        if(lo + (lo & 0x01) > 0x05) { \
            tmp = (tmp & 0xf0) | ((tmp + 6) & 0x0f); \
        } \
        if(hi + (hi & 0x10) > 0x50) { \
            P |= F_C; \
            tmp = (tmp+0x60) & 0xff; \
        } \
        else { \
            P &= ~F_C; \
        } \
    } \
    else { \
        tmp &= A; \
        ROR; \
        P &=~(F_V| F_C); \
        if(tmp & 0x40) { \
            P |= F_C; \
        } \
        if((tmp & 0x60) == 0x20 || (tmp & 0x60) == 0x40) { \
            P |= F_V; \
        } \
    }
#endif

#define ASX \
    P &= ~F_C; \
    X &= A; \
    if(X >= tmp) { \
        P |= F_C; \
    } \
    X = (uint8)(X - tmp); \
    SET_NZ(X)

#define AXA \
    A = (uint8)((A | 0xee) & X & tmp); \
    SET_NZ(A)

#define DCP \
    tmp = (uint8)(tmp - 1); \
    P &= ~F_C; \
    if(A >= tmp) { \
        P |= F_C; \
    } \
    SET_NZ((uint8)(A - tmp))

#define DOP RDOPARG()

#define ISB \
    tmp = (uint8)(tmp + 1); \
    SBC

#define LAX \
    A = X = (uint8)tmp; \
    SET_NZ(A)

#ifdef HAS_N2A03
#define OAL \
    A = X = (uint8)((A | 0xff) & tmp); \
    SET_NZ(A)
#else
#define OAL \
    A = X = (uint8)((A | 0xee) & tmp); \
    SET_NZ(A)
#endif

#define RLA \
    tmp = (tmp << 1) | (P & F_C); \
    P = (P & ~F_C) | ((tmp >> 8) & F_C); \
    tmp = (uint8)tmp; \
    A &= tmp; \
    SET_NZ(A)
#define RRA \
    tmp |= (P & F_C) << 8; \
    P = (P & ~F_C) | (tmp & F_C); \
    tmp = (uint8)(tmp >> 1); \
    ADC

#define SAX tmp = A & X

#define SLO \
    P = (P & ~F_C) | ((tmp >> 7) & F_C); \
    tmp = (uint8)(tmp << 1); \
    A |= tmp; \
    SET_NZ(A)

#define SRE \
    P = (P & ~F_C) | (tmp & F_C); \
    tmp = (uint8)tmp >> 1; \
    A ^= tmp; \
    SET_NZ(A)

#define SAH tmp = A & X & (EAH + 1)

#define SSH \
    S = A & X; \
    tmp = S & (EAH + 1)

#ifdef HAS_N2A03
#define SXH \
    if(Y && Y > EAL) { \
        EAH |= (Y << 1); \
    } \
    tmp = X & (EAH + 1)
#define SYH \
    if(X && X > EAL) { \
        EAH |= (X << 1); \
    } \
    tmp = Y & (EAH + 1)
#else
#define SXH tmp = X & (EAH + 1)
#define SYH tmp = Y & (EAH + 1)
#endif

#define TOP PCW += 2
#define KIL PCW--

#endif // OPS02_H
