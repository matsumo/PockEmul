/*
	Skelton for retropc emulator

	Origin : MESS UPD7810 Core
	Author : Takeda.Toshiya
	Date   : 2006.08.21 -

	[ uPD7801 ]
*/

#include "upd7801.h"
#include "pcxxxx.h"
#include "Inter.h"
#include "Debug.h"
#include "ui/cregsz80widget.h"

#define PRESCALER	16

#define VA	regs[0].w.l
#define BC	regs[1].w.l
#define DE 	regs[2].w.l
#define HL	regs[3].w.l

#define _V	regs[0].b.h
#define _A	regs[0].b.l
#define _B	regs[1].b.h
#define _C	regs[1].b.l
#define _D	regs[2].b.h
#define _E	regs[2].b.l
#define _H	regs[3].b.h
#define _L	regs[3].b.l

#define F_CY	0x01
#define F_L0	0x04
#define F_L1	0x08
#define F_HC	0x10
#define F_SK	0x20
#define F_Z	0x40

#define INTF0	0x01
#define INTFT	0x02
#define INTF1	0x04
#define INTF2	0x08
#define INTFS	0x10

static const quint8 irq_bits[5] = {
	INTF0, INTFT, INTF1, INTF2, INTFS
};

static const quint16 irq_addr[5] = {
	0x0004, 0x0008, 0x0010, 0x0020, 0x0040
};

typedef struct {
	int oplen;	// bytes of opecode
	int clock;	// clock
} op_t;

static const op_t op[256] = {
	{1, 4}, {1, 6}, {1, 7}, {1, 7}, {3,10}, {3,16}, {1, 4}, {2, 7}, {1,11}, {1, 4}, {1, 4}, {1, 4}, {1, 4}, {1, 4}, {1, 4}, {1, 4},
	{1, 4}, {1, 4}, {1, 7}, {1, 7}, {3,10}, {3,16}, {2, 7}, {2, 7}, {1,11}, {1, 4}, {1, 4}, {1, 4}, {1, 4}, {1, 4}, {1, 4}, {1, 4},
	{2,13}, {1,19}, {1, 7}, {1, 7}, {3,10}, {3,13}, {2, 7}, {2, 7}, {2,10}, {1, 7}, {1, 7}, {1, 7}, {1, 7}, {1, 7}, {1, 7}, {1, 7},
	{2,13}, {1,13}, {1, 7}, {1, 7}, {3,10}, {3,13}, {2, 7}, {2, 7}, {2,10}, {1, 7}, {1, 7}, {1, 7}, {1, 7}, {1, 7}, {1, 7}, {1, 7},
	{1, 4}, {1, 4}, {1, 4}, {1, 4}, {3,16}, {3,13}, {2, 7}, {2, 7}, {0, 0}, {2,10}, {2,10}, {2,10}, {0, 0}, {0, 0}, {2,17}, {2,17},
	{1, 4}, {1, 4}, {1, 4}, {1, 4}, {3,10}, {3,13}, {2, 7}, {2, 7}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10},
	{0, 0}, {1, 4}, {1,15}, {1,13}, {0, 0}, {3,13}, {2, 7}, {2, 7}, {2, 7}, {2, 7}, {2, 7}, {2, 7}, {2, 7}, {2, 7}, {2, 7}, {2, 7},
	{0, 0}, {3,13}, {1,19}, {1, 4}, {0, 0}, {3,13}, {2, 7}, {2, 7}, {2,16}, {2,16}, {2,16}, {2,16}, {2,16}, {2,16}, {2,16}, {2,16},
	{1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19},
	{1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19},
	{1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19},
	{1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19},
	{1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13},
	{1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13},
	{1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13},
	{1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13}

};
static const op_t op48[256] = {
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2,17}, {2,15},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2,17}, {2,15},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2,11}, {2,11}, {2,17}, {2,15},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2,17}, {2,17}, {2, 8}, {2, 8}, {2,11}, {2, 8}, {2,17}, {2,15},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}
};
static const op_t op4c[256] = {
	{2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10},
	{2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10},
	{2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10},
	{2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10},
	{2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10},
	{2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10},
	{2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10},
	{2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10},
	{2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10},
	{2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10},
	{2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10},
	{2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10},
	{2,10}, {2,10}, {2,10}, {2,10}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2,10}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}
};
static const op_t op4d[256] = {
	{2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10},
	{2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10},
	{2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10},
	{2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10},
	{2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10},
	{2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10},
	{2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10},
	{2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10},
	{2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10},
	{2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10},
	{2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10},
	{2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10},
	{2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}
};
static const op_t op60[256] = {
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}
};
static const op_t op64[256] = {
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11},
	{3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11},
	{3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11},
	{3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11},
	{3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11},
	{3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11},
	{3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11},
	{3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {3,17}, {3,17}, {3,17}, {3,17}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{3,17}, {3,17}, {3,17}, {3,17}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {3,17}, {3,17}, {3,17}, {3,17}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{3,17}, {3,17}, {3,17}, {3,17}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {3,14}, {3,14}, {3,14}, {3,14}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{3,17}, {3,17}, {3,17}, {3,17}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {3,14}, {3,14}, {3,14}, {3,14}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{3,17}, {3,17}, {3,17}, {3,17}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {3,14}, {3,14}, {3,14}, {3,14}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{3,17}, {3,17}, {3,17}, {3,17}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {3,14}, {3,14}, {3,14}, {3,14}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{3,17}, {3,17}, {3,17}, {3,17}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {3,14}, {3,14}, {3,14}, {3,14}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{3,17}, {3,17}, {3,17}, {3,17}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {3,14}, {3,14}, {3,14}, {3,14}, {2, 8}, {2, 8}, {2, 8}, {2, 8}
};
static const op_t op70[256] = {
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {4,20}, {4,20},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {4,20}, {4,20},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {4,20}, {4,20},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {4,20}, {4,20},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {4,17}, {4,17}, {4,17}, {4,17}, {4,17}, {4,17}, {4,17}, {4,17},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {4,17}, {4,17}, {4,17}, {4,17}, {4,17}, {4,17}, {4,17}, {4,17},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11},
	{2, 8}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11}, {2, 8}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11},
	{2, 8}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11}, {2, 8}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11},
	{2, 8}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11}, {2, 8}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11},
	{2, 8}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11}, {2, 8}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11},
	{2, 8}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11}, {2, 8}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11},
	{2, 8}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11}, {2, 8}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11},
	{2, 8}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11}, {2, 8}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11}
};
static const op_t op74[256] = {
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {3,14}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{3,14}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {3,14}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{3,14}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {3,14}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{3,14}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {3,14}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{3,14}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {3,14}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{3,14}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {3,14}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{3,14}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {3,14}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{3,14}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {3,14}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}
};

// flag control

#define ZHC_ADD(a, b, c) { \
	if(a) { \
		PSW &= ~F_Z; \
	} \
	else { \
		PSW |= F_Z; \
	} \
	if(a == b) { \
		PSW = (PSW & ~F_CY) | (c); \
	} \
	else if(a < b) { \
		PSW |= F_CY; \
	} \
	else { \
		PSW &= ~F_CY; \
	} \
	if((a & 15) < (b & 15)) { \
		PSW |= F_HC; \
	} \
	else { \
		PSW &= ~F_HC; \
	} \
}
#define ZHC_SUB(a, b, c) { \
	if(a) { \
		PSW &= ~F_Z; \
	} \
	else { \
		PSW |= F_Z; \
	} \
	if(a == b) { \
		PSW = (PSW & ~F_CY) | (c); \
	} \
	else if(a > b) { \
		PSW |= F_CY; \
	} \
	else { \
		PSW &= ~F_CY; \
	} \
	if((a & 15) > (b & 15)) { \
		PSW |= F_HC; \
	} \
	else { \
		PSW &= ~F_HC; \
	} \
}
#define SET_Z(n) { \
	if(n) { \
		PSW &= ~F_Z; \
	} \
	else { \
		PSW |= F_Z; \
	} \
}
#define SKIP_CY { \
	if(PSW & F_CY) { \
		PSW |= F_SK; \
	} \
}
#define SKIP_NC { \
	if(!(PSW & F_CY)) { \
		PSW |= F_SK; \
	} \
}
#define SKIP_Z { \
	if(PSW & F_Z) { \
		PSW |= F_SK; \
	} \
}
#define SKIP_NZ { \
	if(!(PSW & F_Z)) { \
		PSW |= F_SK; \
	} \
}

// memory



inline quint8 Cupd7801::RM8(quint16 addr)
{
#ifdef UPD7801_MEMORY_WAIT
	int wait;
    quint8 val = d_mem->read_data8w(addr, &wait);
	period += wait;
	return val;
#else

    return pPC->Get_8(addr);
#endif
}

inline void Cupd7801::WM8(quint16 addr, quint8 val)
{
#ifdef UPD7801_MEMORY_WAIT
	int wait;
	d_mem->write_data8w(addr, val, &wait);
	period += wait;
#else
    pPC->Set_8(addr,val);
#endif
}

inline quint16 Cupd7801::RM16(quint16 addr)
{
#ifdef UPD7801_MEMORY_WAIT
	int wait;
    quint16 val = d_mem->read_data16w(addr, &wait);
	period += wait;
	return val;
#else
    return pPC->Get_16(addr);
    return pPC->Get_16(addr);
#endif
}

inline void Cupd7801::WM16(quint16 addr, quint16 val)
{
#ifdef UPD7801_MEMORY_WAIT
	int wait;
	d_mem->write_data16w(addr, val, &wait);
	period += wait;
#else
    pPC->Set_16(addr,val);
#endif
}

inline quint8 Cupd7801::FETCH8()
{
#ifdef UPD7801_MEMORY_WAIT
	int wait;
    quint8 val = d_mem->read_data8w(PC++, &wait);
	period += wait;
	return val;
#else
    return pPC->Get_PC(PC++);
#endif
}

inline quint16 Cupd7801::FETCH16()
{
#ifdef UPD7801_MEMORY_WAIT
	int wait;
    quint16 val = d_mem->read_data16w(PC, &wait);
	period += wait;
#else
    quint16 val = pPC->Get_16(PC);
#endif
	PC += 2;
	return val;
}

inline quint16 Cupd7801::FETCHWA()
{
#ifdef UPD7801_MEMORY_WAIT
	int wait;
    quint16 val = (_V << 8) | d_mem->read_data8w(PC++, &wait);
	period += wait;
	return val;
#else
    return (_V << 8) | pPC->Get_8(PC++);
#endif
}

inline quint8 Cupd7801::POP8()
{
#ifdef UPD7801_MEMORY_WAIT
	int wait;
    quint8 val = d_mem->read_data8w(SP++, &wait);
	period += wait;
	return val;
#else
    return pPC->Get_8(SP++);
#endif
}

inline void Cupd7801::PUSH8(quint8 val)
{
#ifdef UPD7801_MEMORY_WAIT
	int wait;
	d_mem->write_data8w(--SP, val, &wait);
	period += wait;
#else
    pPC->Set_8(--SP, val);
#endif
}

inline quint16 Cupd7801::POP16()
{
#ifdef UPD7801_MEMORY_WAIT
	int wait;
    quint16 val = d_mem->read_data16w(SP, &wait);
	period += wait;
#else
    quint16 val = pPC->Get_16(SP);
#endif
	SP += 2;
	return val;
}

inline void Cupd7801::PUSH16(quint16 val)
{
	SP -= 2;
#ifdef UPD7801_MEMORY_WAIT
	int wait;
	d_mem->write_data16w(SP, val, &wait);
	period += wait;
#else
    pPC->Set_16(SP, val);
#endif
}

// io

inline quint8 Cupd7801::IN8(int port)
{
	if(port == P_C) {
        return (pPC->in(P_C) & 0x87) | (PORTC & 0x78);
	}

    return pPC->in(port);
}

inline void Cupd7801::OUT8(int port, quint8 val)
{
	if(port == P_C) {
		PORTC = val;
    }
    pPC->out(port,val);
}

// IOM : 0x20 = I/O, 0 = MEMORY
inline void Cupd7801::UPDATE_PORTC(quint8 IOM)
{
    pPC->out(P_C, (PORTC & MC) | ((SAK | TO | IOM) & ~MC));
}

// opecode

#define ACI(r) { \
    quint8 tmp = r + FETCH8() + (PSW & F_CY); \
	ZHC_ADD(tmp, r, (PSW & F_CY)); \
	r = tmp; \
}
#define ACI_IO(p) { \
    quint8 old = IN8(p); \
    quint8 tmp = old + FETCH8() + (PSW & F_CY); \
	ZHC_ADD(tmp, old, (PSW & F_CY)); \
	OUT8(p, tmp); \
}
#define ADC(r, n) { \
    quint8 tmp = r + n + (PSW & F_CY); \
	ZHC_ADD(tmp, r, (PSW & F_CY)); \
	r = tmp; \
}
#define ADCW() { \
    quint8 tmp = _A + RM8(FETCHWA()) + (PSW & F_CY); \
	ZHC_ADD(tmp, _A, (PSW & F_CY)); \
	_A = tmp; \
}
#define ADCX(r) { \
    quint8 tmp = _A + RM8(r) + (PSW & F_CY); \
	ZHC_ADD(tmp, _A, (PSW & F_CY)); \
	_A = tmp; \
}
#define ADD(r, n) { \
    quint8 tmp = r + n; \
	ZHC_ADD(tmp, r, 0); \
	r = tmp; \
}
#define ADDNC(r, n) { \
    quint8 tmp = r + n; \
	ZHC_ADD(tmp, r, 0); \
	r = tmp; \
	SKIP_NC; \
}
#define ADDNCW() { \
    quint8 tmp = _A + RM8(FETCHWA()); \
	ZHC_ADD(tmp, _A, 0); \
	_A = tmp; \
	SKIP_NC; \
}
#define ADDNCX(r) { \
    quint8 tmp = _A + RM8(r); \
	ZHC_ADD(tmp, _A, 0); \
	_A = tmp; \
	SKIP_NC; \
}
#define ADDW() { \
    quint8 tmp = _A + RM8(FETCHWA()); \
	ZHC_ADD(tmp, _A, 0); \
	_A = tmp; \
}
#define ADDX(r) { \
    quint8 tmp = _A + RM8(r); \
	ZHC_ADD(tmp, _A, 0); \
	_A = tmp; \
}
#define ADI(r) { \
    quint8 tmp = r + FETCH8(); \
	ZHC_ADD(tmp, r, 0); \
	r = tmp; \
}
#define ADI_IO(p) { \
    quint8 old = IN8(p); \
    quint8 tmp = old + FETCH8(); \
	ZHC_ADD(tmp, old, 0); \
	OUT8(p, tmp); \
}
#define ADINC(r) { \
    quint8 tmp = r + FETCH8(); \
	ZHC_ADD(tmp, r, 0); \
	r = tmp; \
	SKIP_NC; \
}
#define ADINC_IO(p) { \
    quint8 old = IN8(p); \
    quint8 tmp = old + FETCH8(); \
	ZHC_ADD(tmp, old, 0); \
	OUT8(p, tmp); \
	SKIP_NC; \
}
#define ANA(r, n) { \
	r &= n; \
	SET_Z(r); \
}
#define ANAW() { \
	_A &= RM8(FETCHWA()); \
	SET_Z(_A); \
}
#define ANAX(r) { \
	_A &= RM8(r); \
	SET_Z(_A); \
}
#define ANI(r) { \
	r &= FETCH8(); \
	SET_Z(r); \
}
#define ANI_IO(p) { \
    quint8 tmp = IN8(p) & FETCH8(); \
	OUT8(p, tmp); \
	SET_Z(tmp); \
}
#define ANIW() { \
    quint16 dst = FETCHWA(); \
    quint8 tmp = RM8(dst) & FETCH8(); \
	WM8(dst, tmp); \
	SET_Z(tmp); \
}
#define BIT(b) { \
	if(RM8(FETCHWA()) & (1 << b)) { \
		PSW |= F_SK; \
	} \
}
#define BLOCK() { \
	WM8(DE++, RM8(HL++)); \
	if(_C--) { \
		PSW &= ~F_CY; \
		PC--; \
	} \
	else { \
		PSW |= F_CY; \
	} \
}
#define CALF(o) { \
    quint16 dst = 0x800 + ((o & 7) << 8) + FETCH8(); \
	PUSH16(PC); \
	PC = dst; \
}
#define CALL() { \
    quint16 dst = FETCH16(); \
	PUSH16(PC); \
	PC = dst; \
}
#define CALT(o) { \
    quint16 dst = RM16(0x80 + ((o & 0x3f) << 1)); \
	PUSH16(PC); \
	PC = dst; \
}
#define DAA() { \
    quint8 lo = _A & 0xf, hi = _A >> 4, diff = 0; \
	if(lo <= 9 && !(PSW & F_HC)) { \
		diff = (hi >= 10 || (PSW & F_CY)) ? 0x60 : 0x00; \
	} \
	else if(lo >= 10 && !(PSW & F_HC)) { \
		diff = (hi >= 9 || (PSW & F_CY)) ? 0x66 : 0x06; \
	} \
	else if(lo <= 2 && (PSW & F_HC)) { \
		diff = (hi >= 10 || (PSW & F_CY)) ? 0x66 : 0x06; \
	} \
	_A += diff; \
	if(_A) { \
		PSW &= ~F_Z; \
	} \
	else { \
		PSW |= F_Z; \
	} \
	if((PSW & F_CY) || (lo <= 9 ? hi >= 10 : hi >= 9)) { \
		PSW |= F_CY; \
	} \
	else { \
		PSW &= ~F_CY; \
	} \
	if(lo >= 10) { \
		PSW |= F_HC; \
	} \
	else { \
		PSW &= ~F_HC; \
	} \
}
#define DCR(r) { \
    quint8 carry = PSW & F_CY; \
    quint8 tmp = r - 1; \
	ZHC_SUB(tmp, r, 0); \
	r = tmp; \
	SKIP_CY; \
	PSW = (PSW & ~F_CY) | carry; \
}
#define DCRW() { \
    quint8 carry = PSW & F_CY; \
    quint16 dst = FETCHWA(); \
    quint8 old = RM8(dst); \
    quint8 tmp = old - 1; \
	ZHC_SUB(tmp, old, 0); \
	WM8(dst, tmp); \
	SKIP_CY; \
	PSW = (PSW & ~F_CY) | carry; \
}
#define EQA(r, n) { \
    quint8 tmp = r - n; \
	ZHC_SUB(tmp, r, 0); \
	SKIP_Z; \
}
#define EQAW() { \
    quint8 tmp = _A - RM8(FETCHWA()); \
	ZHC_SUB(tmp, _A, 0); \
	SKIP_Z; \
}
#define EQAX(r) { \
    quint8 tmp = _A - RM8(r); \
	ZHC_SUB(tmp, _A, 0); \
	SKIP_Z; \
}
#define EQI(r) { \
    quint8 tmp = r - FETCH8(); \
	ZHC_SUB(tmp, r, 0); \
	SKIP_Z; \
}
#define EQI_IO(p) { \
    quint8 old = IN8(p); \
    quint8 tmp = old - FETCH8(); \
	ZHC_SUB(tmp, old, 0); \
	SKIP_Z; \
}
#define EQIW() { \
    quint8 old = RM8(FETCHWA()); \
    quint8 tmp = old - FETCH8(); \
	ZHC_SUB(tmp, old, 0); \
	SKIP_Z; \
}
#define EX() { \
    quint16 tmp; \
	tmp = VA; VA = altVA; altVA = tmp; \
}
#define EXX() { \
    quint16 tmp; \
	tmp = BC; BC = altBC; altBC = tmp; \
	tmp = DE; DE = altDE; altDE = tmp; \
	tmp = HL; HL = altHL; altHL = tmp; \
}
#define GTA(r, n) { \
    quint8 tmp = r - n - 1; \
	ZHC_SUB(tmp, r, 1); \
	SKIP_NC; \
}
#define GTAW() { \
    quint8 tmp = _A - RM8(FETCHWA()) - 1; \
	ZHC_SUB(tmp, _A, 1); \
	SKIP_NC; \
}
#define GTAX(r) { \
    quint8 tmp = _A - RM8(r) - 1; \
	ZHC_SUB(tmp, _A, 1); \
	SKIP_NC; \
}
#define GTI(r) { \
    quint8 tmp = r - FETCH8() - 1; \
	ZHC_SUB(tmp, r, 1); \
	SKIP_NC; \
}
#define GTI_IO(p) { \
    quint8 old = IN8(p); \
    quint8 tmp = old - FETCH8() - 1; \
	ZHC_SUB(tmp, old, 1); \
	SKIP_NC; \
}
#define GTIW() { \
    quint8 old = RM8(FETCHWA()); \
    quint8 tmp = old - FETCH8() - 1; \
	ZHC_SUB(tmp, old, 1); \
	SKIP_NC; \
}
#define INR(r) { \
    quint8 carry = PSW & F_CY; \
    quint8 tmp = r + 1; \
	ZHC_ADD(tmp, r, 0); \
	r = tmp; \
	SKIP_CY; \
	PSW = (PSW & ~F_CY) | carry; \
}
#define INRW() { \
    quint8 carry = PSW & F_CY; \
    quint16 dst = FETCHWA(); \
    quint8 old = RM8(dst); \
    quint8 tmp = old + 1; \
	ZHC_ADD(tmp, old, 0); \
	WM8(dst, tmp); \
	SKIP_CY; \
	PSW = (PSW & ~F_CY) | carry; \
}
#define JRE(o) { \
    quint8 tmp = FETCH8(); \
	if(o & 1) { \
		PC -= 256 - tmp; \
	} \
	else { \
		PC += tmp; \
	} \
}
#define LTA(r, n) { \
    quint8 tmp = r - n; \
	ZHC_SUB(tmp, r, 0); \
	SKIP_CY; \
}
#define LTAW() { \
    quint8 tmp = _A - RM8(FETCHWA()); \
	ZHC_SUB(tmp, _A, 0); \
	SKIP_CY; \
}
#define LTAX(r) { \
    quint8 tmp = _A - RM8(r); \
	ZHC_SUB(tmp, _A, 0); \
	SKIP_CY; \
}
#define LTI(r) { \
    quint8 tmp = r - FETCH8(); \
	ZHC_SUB(tmp, r, 0); \
	SKIP_CY; \
}
#define LTI_IO(p) { \
    quint8 old = IN8(p); \
    quint8 tmp = old - FETCH8(); \
	ZHC_SUB(tmp, old, 0); \
	SKIP_CY; \
}
#define LTIW() { \
    quint8 old = RM8(FETCHWA()); \
    quint8 tmp = old - FETCH8(); \
	ZHC_SUB(tmp, old, 0); \
	SKIP_CY; \
}
#define MVIW() { \
    quint16 dst = FETCHWA(); \
	WM8(dst, FETCH8()); \
}
#define NEA(r, n) { \
    quint8 tmp = r - n; \
	ZHC_SUB(tmp, r, 0); \
	SKIP_NZ; \
}
#define NEAW() { \
    quint8 tmp = _A - RM8(FETCHWA()); \
	ZHC_SUB(tmp, _A, 0); \
	SKIP_NZ; \
}
#define NEAX(r) { \
    quint8 tmp = _A - RM8(r); \
	ZHC_SUB(tmp, _A, 0); \
	SKIP_NZ; \
}
#define NEI(r) { \
    quint8 tmp = r - FETCH8(); \
	ZHC_SUB(tmp, r, 0); \
	SKIP_NZ; \
}
#define NEI_IO(p) { \
    quint8 old = IN8(p); \
    quint8 tmp = old - FETCH8(); \
	ZHC_SUB(tmp, old, 0); \
	SKIP_NZ; \
}
#define NEIW() { \
    quint8 old = RM8(FETCHWA()); \
    quint8 tmp = old - FETCH8(); \
	ZHC_SUB(tmp, old, 0); \
	SKIP_NZ; \
}
#define OFFA(r, n) { \
	if(r & n) { \
		PSW &= ~F_Z; \
	} \
	else { \
		PSW |= F_Z | F_SK; \
	} \
}
#define OFFAW() { \
	if(_A & RM8(FETCHWA())) { \
		PSW &= ~F_Z; \
	} \
	else { \
		PSW |= F_Z | F_SK; \
	} \
}
#define OFFAX(r) { \
	if(_A & RM8(r)) { \
		PSW &= ~F_Z; \
	} \
	else { \
		PSW |= F_Z | F_SK; \
	} \
}
#define OFFI(r) { \
	if(r & FETCH8()) { \
		PSW &= ~F_Z; \
	} \
	else { \
		PSW |= F_Z | F_SK; \
	} \
}
#define OFFI_IO(p) { \
	if(IN8(p) & FETCH8()) { \
		PSW &= ~F_Z; \
	} \
	else { \
		PSW |= F_Z | F_SK; \
	} \
}
#define OFFIW() { \
    quint8 tmp = RM8(FETCHWA()); \
	if(tmp & FETCH8()) { \
		PSW &= ~F_Z; \
	} \
	else { \
		PSW |= F_Z | F_SK; \
	} \
}
#define ONA(r, n) { \
	if(r & n) { \
		PSW = (PSW & ~F_Z) | F_SK; \
	} \
	else { \
		PSW |= F_Z; \
	} \
}
#define ONAW() { \
	if(_A & RM8(FETCHWA())) { \
		PSW = (PSW & ~F_Z) | F_SK; \
	} \
	else { \
		PSW |= F_Z; \
	} \
}
#define ONAX(r) { \
	if(_A & RM8(r)) { \
		PSW = (PSW & ~F_Z) | F_SK; \
	} \
	else { \
		PSW |= F_Z; \
	} \
}
#define ONI(r) { \
	if(r & FETCH8()) { \
		PSW = (PSW & ~F_Z) | F_SK; \
	} \
	else { \
		PSW |= F_Z; \
	} \
}
#define ONI_IO(p) { \
	if(IN8(p) & FETCH8()) { \
		PSW = (PSW & ~F_Z) | F_SK; \
	} \
	else { \
		PSW |= F_Z; \
	} \
}
#define ONIW() { \
    quint8 tmp = RM8(FETCHWA()); \
	if(tmp & FETCH8()) { \
		PSW = (PSW & ~F_Z) | F_SK; \
	} \
	else { \
		PSW |= F_Z; \
	} \
}
#define ORA(r, n) { \
	r |= n; \
	SET_Z(r); \
}
#define ORAW() { \
	_A |= RM8(FETCHWA()); \
	SET_Z(_A); \
}
#define ORAX(r) { \
	_A |= RM8(r); \
	SET_Z(_A); \
}
#define ORI(r) { \
	r |= FETCH8(); \
	SET_Z(r); \
}
#define ORI_IO(p) { \
    quint8 tmp = IN8(p) | FETCH8(); \
	OUT8(p, tmp); \
	SET_Z(tmp); \
}
#define ORIW() { \
    quint16 dst = FETCHWA(); \
    quint8 tmp = RM8(dst) | FETCH8(); \
	WM8(dst, tmp); \
	SET_Z(tmp); \
}
#define PEN() { \
}
#define PER() { \
}
#define PEX() { \
}
#define RLD() { \
    quint8 old = RM8(HL); \
    quint8 tmp = (old << 4) | (_A & 0x0f); \
	_A = (_A & 0xf0) | (old >> 4); \
	WM8(HL, tmp); \
}
#define RLL(r) { \
    quint8 carry = PSW & F_CY; \
	PSW = (PSW & ~F_CY) | ((r >> 7) & F_CY); \
	r = (r << 1) | carry; \
}
#define RLR(r) { \
    quint8 carry = (PSW & F_CY) << 7; \
	PSW = (PSW & ~F_CY) | (r & F_CY); \
	r = (r >> 1) | carry; \
}
#define RRD() { \
    quint8 old = RM8(HL); \
    quint8 tmp = (_A << 4) | (old >> 4); \
	_A = (_A & 0xf0) | (old & 0x0f); \
	WM8(HL, tmp); \
}
#define SBB(r, n) { \
    quint8 tmp = r - n - (PSW & F_CY); \
	ZHC_SUB(tmp, r, (PSW & F_CY)); \
	r = tmp; \
}
#define SBBW() { \
    quint8 tmp = _A - RM8(FETCHWA()) - (PSW & F_CY); \
	ZHC_SUB(tmp, _A, (PSW & F_CY)); \
	_A = tmp; \
}
#define SBBX(r) { \
    quint8 tmp = _A - RM8(r) - (PSW & F_CY); \
	ZHC_SUB(tmp, _A, (PSW & F_CY)); \
	_A = tmp; \
}
#define SBI(r) { \
    quint8 tmp = r - FETCH8() - (PSW & F_CY); \
	ZHC_SUB(tmp, r, (PSW & F_CY)); \
	r = tmp; \
}
#define SBI_IO(p) { \
    quint8 old = IN8(p); \
    quint8 tmp = old - FETCH8() - (PSW & F_CY); \
	ZHC_SUB(tmp, old, (PSW & F_CY)); \
	OUT8(p, tmp); \
}
#define SIO() { \
	scount = 32 + 4; \
}
#define SK(f) { \
	if(PSW & f) { \
		PSW |= F_SK; \
	} \
}
#define SKIT(f) { \
	if(IRR & f) { \
		PSW |= F_SK; \
	} \
	IRR &= ~f; \
}
#define SKN(f) { \
	if(!(PSW & f)) { \
		PSW |= F_SK; \
	} \
}
#define SKNIT(f) { \
	if(!(IRR & f)) { \
		PSW |= F_SK; \
	} \
	IRR &= ~f; \
}
#define SLL(r) { \
	PSW = (PSW & ~F_CY) | ((r >> 7) & F_CY); \
	r <<= 1; \
}
#define SLR(r) { \
	PSW = (PSW & ~F_CY) | (r & F_CY); \
	r >>= 1; \
}
#define STM() { \
	tcount = (((TM0 | (TM1 << 8)) & 0xfff) + 1) * PRESCALER; \
}
#define SUB(r, n) { \
    quint8 tmp = r - n; \
	ZHC_SUB(tmp, r, 0); \
	r = tmp; \
}
#define SUBNB(r, n) { \
    quint8 tmp = r - n; \
	ZHC_SUB(tmp, r, 0); \
	r = tmp; \
	SKIP_NC; \
}
#define SUBNBW() { \
    quint8 tmp = _A - RM8(FETCHWA()); \
	ZHC_SUB(tmp, _A, 0); \
	_A = tmp; \
	SKIP_NC; \
}
#define SUBNBX(r) { \
    quint8 tmp = _A - RM8(r); \
	ZHC_SUB(tmp, _A, 0); \
	_A = tmp; \
	SKIP_NC; \
}
#define SUBW() { \
    quint8 tmp = _A - RM8(FETCHWA()); \
	ZHC_SUB(tmp, _A, 0); \
	_A = tmp; \
}
#define SUBX(r) { \
    quint8 tmp = _A - RM8(r); \
	ZHC_SUB(tmp, _A, 0); \
	_A = tmp; \
}
#define SUI(r) { \
    quint8 tmp = r - FETCH8(); \
	ZHC_SUB(tmp, r, 0); \
	r = tmp; \
}
#define SUI_IO(p) { \
    quint8 old = IN8(p); \
    quint8 tmp = old - FETCH8(); \
	ZHC_SUB(tmp, old, 0); \
	OUT8(p, tmp); \
}
#define SUINB(r) { \
    quint8 tmp = r - FETCH8(); \
	ZHC_SUB(tmp, r, 0); \
	r = tmp; \
	SKIP_NC; \
}
#define SUINB_IO(p) { \
    quint8 old = IN8(p); \
    quint8 tmp = old - FETCH8(); \
	ZHC_SUB(tmp, old, 0); \
	OUT8(p, tmp); \
	SKIP_NC; \
}
#define XRA(r, n) { \
	r ^= n; \
	SET_Z(r); \
}
#define XRAW() { \
	_A ^= RM8(FETCHWA()); \
	SET_Z(_A); \
}
#define XRAX(r) { \
	_A ^= RM8(r); \
	SET_Z(_A); \
}
#define XRI(r) { \
	r ^= FETCH8(); \
	SET_Z(r); \
}
#define XRI_IO(p) { \
    quint8 tmp = IN8(p) ^ FETCH8(); \
	OUT8(p, tmp); \
	SET_Z(tmp); \
}


int Cupd7801::run(int clock)
{
	// run cpu
	if(clock == -1) {
		// run only one opcode
		count = 0;
		run_one_opecode();
		return -count;
	}
	else {
		// run cpu while given clocks
		count += clock;
		int first_count = count;
		
		while(count > 0) {
			run_one_opecode();
		}
		return first_count - count;
	}
}

void Cupd7801::run_one_opecode()
{
	if(wait) {
		period = 1;
	}
	else {
		// interrupt is enabled after next opecode of ei
		if(IFF & 2) {
			IFF--;
		}
		
		// run 1 opecode
		period = 0;
		prevPC = PC;
		OP();
	}
	count -= period;
	
	// update serial count
	if(scount && (scount -= period) <= 0) {
		scount = 0;
		IRR |= INTFS;
		OUT8(P_SO, SR);
		SR = IN8(P_SI);
		if(SAK) {
			SAK = 0;
			UPDATE_PORTC(0);
		}
	}
	// update timer
	if(tcount && (tcount -= period) <= 0) {
		tcount += (((TM0 | (TM1 << 8)) & 0xfff) + 1) * PRESCALER;
		IRR |= INTFT;
		if(TO) {
			TO = 0;
			UPDATE_PORTC(0);
		}
	}
	
	// check interrupt
	if(IFF == 1 && !SIRQ) {
		for(int i = 0; i < 5; i++) {
            quint8 bit = irq_bits[i];
			if((IRR & bit) && !(MK & bit)) {
				if(HALT) {
					HALT = 0;
					PC++;
				}
				PUSH8(PSW);
				PUSH16(PC);
				
				PC = irq_addr[i];
				PSW &= ~(F_SK | F_L0 | F_L1);
				IFF = 0;
				IRR &= ~bit;
				break;
			}
		}
	}
}

void Cupd7801::write_signal(int id, quint32 data, quint32 mask)
{
	if(id == SIG_UPD7801_INTF0) {
		if(data & mask) {
			IRR |= INTF0;
		}
		else {
			IRR &= ~INTF0;
		}
	}
	else if(id == SIG_UPD7801_INTF1) {
		if(data & mask) {
			IRR |= INTF1;
		}
		else {
			IRR &= ~INTF1;
		}
	}
	else if(id == SIG_UPD7801_INTF2) {
        static quint32 prev = 0;
		if((data & mask) && (MK & 0x20)) {
			IRR |= INTF2;
		}
		else if(!(data & mask) && !(MK & 0x20)) {
			IRR |= INTF2;
		}
	}
	else if(id == SIG_UPD7801_WAIT) {
		wait = ((data & mask) != 0);
	}
}

void Cupd7801::OP()
{
    quint8 ope = FETCH8();
	
	if((PSW & F_SK) && ope != 0x72) {
		// skip this mnemonic
		switch(ope) {
		case 0x48: PSW &= ~(F_SK | F_L0 | F_L1); ope = FETCH8(); PC += op48[ope].oplen - 2; period += op48[ope].clock; break;
		case 0x4c: PSW &= ~(F_SK | F_L0 | F_L1); ope = FETCH8(); PC += op4c[ope].oplen - 2; period += op4c[ope].clock; break;
		case 0x4d: PSW &= ~(F_SK | F_L0 | F_L1); ope = FETCH8(); PC += op4d[ope].oplen - 2; period += op4d[ope].clock; break;
		case 0x60: PSW &= ~(F_SK | F_L0 | F_L1); ope = FETCH8(); PC += op60[ope].oplen - 2; period += op60[ope].clock; break;
		case 0x64: PSW &= ~(F_SK | F_L0 | F_L1); ope = FETCH8(); PC += op64[ope].oplen - 2; period += op64[ope].clock; break;
		case 0x70: PSW &= ~(F_SK | F_L0 | F_L1); ope = FETCH8(); PC += op70[ope].oplen - 2; period += op70[ope].clock; break;
		case 0x74: PSW &= ~(F_SK | F_L0 | F_L1); ope = FETCH8(); PC += op74[ope].oplen - 2; period += op74[ope].clock; break;
		case 0x34: PSW &= ~(F_SK        | F_L1);                 PC += op[ope].oplen - 1;   period += op[ope].clock;   break;
		case 0x69: PSW &= ~(F_SK | F_L0       );                 PC += op[ope].oplen - 1;   period += op[ope].clock;   break;
		case 0x6f: PSW &= ~(F_SK        | F_L1);                 PC += op[ope].oplen - 1;   period += op[ope].clock;   break;
		default:   PSW &= ~(F_SK | F_L0 | F_L1);                 PC += op[ope].oplen - 1;   period += op[ope].clock;   break;
		}
		return;
	}
	period += op[ope].clock;
	
	switch(ope) {
	case 0x00:	// nop
		break;
	case 0x01:	// hlt
		HALT = 1; PC--; break;
	case 0x02:	// inx sp
		SP++; break;
	case 0x03:	// dcx sp
		SP--; break;
	case 0x04:	// lxi sp,word
		SP = FETCH16(); break;
	case 0x05:	// aniw wa,byte
		ANIW(); break;
	case 0x06:
		break;
	case 0x07:	// ani a,byte
		ANI(_A); break;
	case 0x08:	// ret
		PC = POP16(); break;
	case 0x09:	// sio
		SIO(); break;
	case 0x0a:	// mov a,b
		_A = _B; break;
	case 0x0b:	// mov a,c
		_A = _C; break;
	case 0x0c:	// mov a,d
		_A = _D; break;
	case 0x0d:	// mov a,e
		_A = _E; break;
	case 0x0e:	// mov a,h
		_A = _H; break;
	case 0x0f:	// mov a,l
		_A = _L; break;
	case 0x10:	// ex
		EX(); break;
	case 0x11:	// exx
		EXX(); break;
	case 0x12:	// inx b
		BC++; break;
	case 0x13:	// dcx b
		BC--; break;
	case 0x14:	// lxi b,word
		BC = FETCH16(); break;
	case 0x15:	// oriw wa,byte
		ORIW(); break;
	case 0x16:	// xri a,byte
		XRI(_A); break;
	case 0x17:	// ori a,byte
		ORI(_A); break;
	case 0x18:	// rets
		PC = POP16(); PSW |= F_SK; break;
	case 0x19:	// stm
		if(!TO) {
			TO = 0x10; UPDATE_PORTC(0);
		}
		STM(); break;
	case 0x1a:	// mov b, a
		_B = _A; break;
	case 0x1b:	// mov c, a
		_C = _A; break;
	case 0x1c:	// mov d, a
		_D = _A; break;
	case 0x1d:	// mov e, a
		_E = _A; break;
	case 0x1e:	// mov h, a
		_H = _A; break;
	case 0x1f:	// mov l, a
		_L = _A; break;
	case 0x20:	// inrw wa
		INRW(); break;
	case 0x21:	// table
		BC = RM16(PC + _A + 1); break;
	case 0x22:	// inx d
		DE++; break;
	case 0x23:	// dcx d
		DE--; break;
	case 0x24:	// lxi d,word
		DE = FETCH16(); break;
	case 0x25:	// gtiw wa,byte
		GTIW(); break;
	case 0x26:	// adinc a,byte
		ADINC(_A); break;
	case 0x27:	// gti a,byte
		GTI(_A); break;
	case 0x28:	// ldaw wa
		_A = RM8(FETCHWA()); break;
	case 0x29:	// ldax b
		_A = RM8(BC); break;
	case 0x2a:	// ldax d
		_A = RM8(DE); break;
	case 0x2b:	// ldax h
		_A = RM8(HL); break;
	case 0x2c:	// ldax d+
		_A = RM8(DE++);; break;
	case 0x2d:	// ldax h+
		_A = RM8(HL++); break;
	case 0x2e:	// ldax d-
		_A = RM8(DE--); break;
	case 0x2f:	// ldax h-
		_A = RM8(HL--); break;
	case 0x30:	// dcrw wa
		DCRW(); break;
	case 0x31:	// block
		BLOCK(); break;
	case 0x32:	// inx h
		HL++; break;
	case 0x33:	// dcx h
		HL--; break;
	case 0x34:	// lxi h,word
		if(PSW & F_L0) {
			PC += 2;
		}
		else {
			HL = FETCH16();
		}
		PSW = (PSW & ~F_L1) | F_L0; return;
	case 0x35:	// ltiw wa,byte
		LTIW(); break;
	case 0x36:	// suinb a,byte
		SUINB(_A); break;
	case 0x37:	// lti a,byte
		LTI(_A); break;
	case 0x38:	// staw wa
		WM8(FETCHWA(), _A); break;
	case 0x39:	// stax b
		WM8(BC, _A); break;
	case 0x3a:	// stax d
		WM8(DE, _A); break;
	case 0x3b:	// stax h
		WM8(HL, _A); break;
	case 0x3c:	// stax d+
		WM8(DE++, _A); break;
	case 0x3d:	// stax h+
		WM8(HL++, _A); break;
	case 0x3e:	// stax d-
		WM8(DE--, _A); break;
	case 0x3f:	// stax h-
		WM8(HL--, _A); break;
	case 0x40:
		break;
	case 0x41:	// inr a
		INR(_A); break;
	case 0x42:	// inr b
		INR(_B); break;
	case 0x43:	// inr c
		INR(_C); break;
	case 0x44:	// call word
		CALL(); break;
	case 0x45:	// oniw wa,byte
		ONIW(); break;
	case 0x46:	// adi a,byte
		ADI(_A); break;
	case 0x47:	// oni a,byte
		ONI(_A); break;
	case 0x48:	// 48 xx
		OP48(); break;
	case 0x49:	// mvix b,byte
		WM8(BC, FETCH8()); break;
	case 0x4a:	// mvix d,byte
		WM8(DE, FETCH8()); break;
	case 0x4b:	// mvix h,byte
		WM8(HL, FETCH8()); break;
	case 0x4c:	// 4c xx
		OP4C(); break;
	case 0x4d:	// 4d xx
		OP4D(); break;
	case 0x4e:	// jre
	case 0x4f:	// jre
		JRE(ope); break;
	case 0x50:
		break;
	case 0x51:	// dcr a
		DCR(_A); break;
	case 0x52:	// dcr b
		DCR(_B); break;
	case 0x53:	// dcr c
		DCR(_C); break;
	case 0x54:	// jmp word
		PC = FETCH16(); break;
	case 0x55:	// offiw wa,byte
		OFFIW(); break;
	case 0x56:	// aci a,byte
		ACI(_A); break;
	case 0x57:	// offi a,byte
		OFFI(_A); break;
	case 0x58:	// bit 0,wa
		BIT(0); break;
	case 0x59:	// bit 1,wa
		BIT(1); break;
	case 0x5a:	// bit 2,wa
		BIT(2); break;
	case 0x5b:	// bit 3,wa
		BIT(3); break;
	case 0x5c:	// bit 4,wa
		BIT(4); break;
	case 0x5d:	// bit 5,wa
		BIT(5); break;
	case 0x5e:	// bit 6,wa
		BIT(6); break;
	case 0x5f:	// bit 7,wa
		BIT(7); break;
	case 0x60:	// 60 xx
		OP60(); break;
	case 0x61:	// daa
		DAA(); break;
	case 0x62:	// reti
		PC = POP16(); PSW = POP8(); SIRQ = 0; return;
	case 0x63:	// calb
		PUSH16(PC); PC = BC; break;
	case 0x64:	// 64 xx
		OP64(); break;
	case 0x65:	// neiw wa,byte
		NEIW(); break;
	case 0x66:	// sui a,byte
		SUI(_A); break;
	case 0x67:	// nei a,byte
		NEI(_A); break;
	case 0x68:	// mvi v,byte
		_V = FETCH8(); 
		break;
	case 0x69:	// mvi a,byte
		if(PSW & F_L1) {
			PC++;
		}
		else {
			_A = FETCH8();
		}
		PSW = (PSW & ~F_L0) | F_L1; return;
	case 0x6a:	// mvi b,byte
		_B = FETCH8(); break;
	case 0x6b:	// mvi c,byte
		_C = FETCH8(); break;
	case 0x6c:	// mvi d,byte
		_D = FETCH8(); break;
	case 0x6d:	// mvi e,byte
		_E = FETCH8(); break;
	case 0x6e:	// mvi h,byte
		_H = FETCH8(); break;
	case 0x6f:	// mvi l,byte
		if(PSW & F_L0) {
			PC++;
		}
		else {
			_L = FETCH8();
		}
		PSW = (PSW & ~F_L1) | F_L0; return;
	case 0x70:	// 70 xx
		OP70(); break;
	case 0x71:	// mviw wa,byte
		MVIW(); break;
	case 0x72:	// softi
		PUSH8(PSW); PUSH16(PC); PSW &= ~F_SK; SIRQ = 1; PC = 0x0060; break;
	case 0x73:	// jb
		PC = BC; break;
	case 0x74:	// 74xx
		OP74(); break;
	case 0x75:	// eqiw wa,byte
		EQIW(); break;
	case 0x76:	// sbi a,byte
		SBI(_A); break;
	case 0x77:	// eqi a,byte
		EQI(_A); break;
	case 0x78: case 0x79: case 0x7a: case 0x7b: case 0x7c: case 0x7d: case 0x7e: case 0x7f:	// calf
		CALF(ope); break;
	case 0x80: case 0x81: case 0x82: case 0x83: case 0x84: case 0x85: case 0x86: case 0x87:
	case 0x88: case 0x89: case 0x8a: case 0x8b: case 0x8c: case 0x8d: case 0x8e: case 0x8f:
	case 0x90: case 0x91: case 0x92: case 0x93: case 0x94: case 0x95: case 0x96: case 0x97:
	case 0x98: case 0x99: case 0x9a: case 0x9b: case 0x9c: case 0x9d: case 0x9e: case 0x9f:
	case 0xa0: case 0xa1: case 0xa2: case 0xa3: case 0xa4: case 0xa5: case 0xa6: case 0xa7:
	case 0xa8: case 0xa9: case 0xaa: case 0xab: case 0xac: case 0xad: case 0xae: case 0xaf:
	case 0xb0: case 0xb1: case 0xb2: case 0xb3: case 0xb4: case 0xb5: case 0xb6: case 0xb7:
	case 0xb8: case 0xb9: case 0xba: case 0xbb: case 0xbc: case 0xbd: case 0xbe: case 0xbf:	// calt
		CALT(ope); break;
	case 0xc0: case 0xc1: case 0xc2: case 0xc3: case 0xc4: case 0xc5: case 0xc6: case 0xc7:
	case 0xc8: case 0xc9: case 0xca: case 0xcb: case 0xcc: case 0xcd: case 0xce: case 0xcf:
	case 0xd0: case 0xd1: case 0xd2: case 0xd3: case 0xd4: case 0xd5: case 0xd6: case 0xd7:
	case 0xd8: case 0xd9: case 0xda: case 0xdb: case 0xdc: case 0xdd: case 0xde: case 0xdf:	// jr
		PC += ope & 0x1f; break;
	case 0xe0: case 0xe1: case 0xe2: case 0xe3: case 0xe4: case 0xe5: case 0xe6: case 0xe7:
	case 0xe8: case 0xe9: case 0xea: case 0xeb: case 0xec: case 0xed: case 0xee: case 0xef:
	case 0xf0: case 0xf1: case 0xf2: case 0xf3: case 0xf4: case 0xf5: case 0xf6: case 0xf7:
	case 0xf8: case 0xf9: case 0xfa: case 0xfb: case 0xfc: case 0xfd: case 0xfe: case 0xff:	// jr
		PC -= 0x20 - (ope & 0x1f); break;
	default:
//		__assume(0);
        break;
	}
	PSW &= ~(F_L0 | F_L1);
}

void Cupd7801::OP48()
{
    quint8 ope = FETCH8();
	period += op48[ope].clock;
	
	switch(ope) {
	case 0x00:	// skit intf0
		SKIT(INTF0); break;
	case 0x01:	// skit intft
		SKIT(INTFT); break;
	case 0x02:	// skit intf1
		SKIT(INTF1); break;
	case 0x03:	// skit intf2
		SKIT(INTF2); break;
	case 0x04:	// skit intfs
		SKIT(INTFS); break;
	case 0x0a:	// sk cy
		SK(F_CY); break;
	case 0x0c:	// sk z
		SK(F_Z); break;
	case 0x0e:	// push v
		PUSH16(VA); break;
	case 0x0f:	// pop v
		VA = POP16(); break;
	case 0x10:	// sknit intf0
		SKNIT(INTF0); break;
	case 0x11:	// sknit intft
		SKNIT(INTFT); break;
	case 0x12:	// sknit intf1
		SKNIT(INTF1); break;
	case 0x13:	// sknit intf2
		SKNIT(INTF2); break;
	case 0x14:	// sknit intfs
		SKNIT(INTFS); break;
	case 0x1a:	// skn cy
		SKN(F_CY); break;
	case 0x1c:	// skn z
		SKN(F_Z); break;
	case 0x1e:	// push b
		PUSH16(BC); break;
	case 0x1f:	// pop b
		BC = POP16(); break;
	case 0x20:	// ei
		IFF = 3; break;
	case 0x24:	// di
		IFF = 0; break;
	case 0x2a:	// clc
		PSW &= ~F_CY; break;
	case 0x2b:	// stc
		PSW |= F_CY; break;
	case 0x2c:	// pen
		PEN(); break;
	case 0x2d:	// pex
		PEX(); break;
	case 0x2e:	// push d
		PUSH16(DE); break;
	case 0x2f:	// pop d
		DE = POP16(); break;
	case 0x30:	// rll a
		RLL(_A); break;
	case 0x31:	// rlr a
		RLR(_A); break;
	case 0x32:	// rll c
		RLL(_C); break;
	case 0x33:	// rlr c
		RLR(_C); break;
	case 0x34:	// sll a
		SLL(_A); break;
	case 0x35:	// slr a
		SLR(_A); break;
	case 0x36:	// sll c
		SLL(_C); break;
	case 0x37:	// slr c
		SLR(_C); break;
	case 0x38:	// rld
		RLD(); break;
	case 0x39:	// rrd
		RRD(); break;
	case 0x3c:	// per
		PER(); break;
	case 0x3e:	// push h
		PUSH16(HL); break;
	case 0x3f:	// pop h
		HL = POP16(); break;
	default:
//		emu->out_debug(_T("PC=%4x\tCPU\tUNKNOWN OP : 48 %2x\n"), prevPC, ope);
        break;
	}
}

void Cupd7801::OP4C()
{
    quint8 ope = FETCH8();
	period += op4c[ope].clock;
	
	switch(ope) {
	case 0xc0:	// mov a,pa
		_A = IN8(P_A); break;
	case 0xc1:	// mov a,pb
		_A = IN8(P_B); break;
	case 0xc2:	// mov a,pc
		_A = IN8(P_C); break;
	case 0xc3:	// mov a,mk
		_A = MK; break;
	case 0xc4:	// mov a,mb ?
		_A = MB; break;
	case 0xc5:	// mov a,mc ?
		_A = MC; break;
	case 0xc6:	// mov a,tm0 ?
		_A = TM0; break;
	case 0xc7:	// mov a,tm1 ?
		_A = TM1; break;
	case 0xc8:	// mov a,s
		if(!SAK) {
			SAK = 8; UPDATE_PORTC(0);
		}
		_A = SR; break;
	default:
		if(ope < 0xc0) {
			// in byte
			UPDATE_PORTC(0x20);
			_A = RM8((_B << 8) | ope);
			UPDATE_PORTC(0);
		}
		else {
//			emu->out_debug(_T("PC=%4x\tCPU\tUNKNOWN OP : 4c %2x\n"), prevPC, ope);
            break;
		}
	}
}

void Cupd7801::OP4D()
{
    quint8 ope = FETCH8();
	period += op4d[ope].clock;
	
	switch(ope) {
	case 0xc0:	// mov pa,a
		OUT8(P_A, _A); break;
	case 0xc1:	// mov pb,a
		OUT8(P_B, _A); break;
	case 0xc2:	// mov pc,a
		OUT8(P_C, _A); break;
	case 0xc3:	// mov mk,a
		MK = _A; break;
	case 0xc4:	// mov mb,a
		MB = _A; break;
	case 0xc5:	// mov mc,a
		if(MC != _A) {
			MC = _A; UPDATE_PORTC(0); break;
		}
		MC = _A; break;
	case 0xc6:	// mov tm0,a
		TM0 = _A; break;
	case 0xc7:	// mov tm1,a
		TM1 = _A; break;
	case 0xc8:	// mov s,a
		if(!SAK) {
			SAK = 8; UPDATE_PORTC(0);
		}
		SR = _A; break;
	default:
		if(ope < 0xc0) {
			// out byte
			UPDATE_PORTC(0x20);
			WM8((_B << 8) | ope, _A);
			UPDATE_PORTC(0);
		}
		else {
//			emu->out_debug(_T("PC=%4x\tCPU\tUNKNOWN OP : 4d %2x\n"), prevPC, ope);
            break;
		}
	}
}

void Cupd7801::OP60()
{
    quint8 ope = FETCH8();
	period += op60[ope].clock;
	
	switch(ope) {
	case 0x08:	// ana v,a
		ANA(_V, _A); break;
	case 0x09:	// ana a,a
		ANA(_A, _A); break;
	case 0x0a:	// ana b,a
		ANA(_B, _A); break;
	case 0x0b:	// ana c,a
		ANA(_C, _A); break;
	case 0x0c:	// ana d,a
		ANA(_D, _A); break;
	case 0x0d:	// ana e,a
		ANA(_E, _A); break;
	case 0x0e:	// ana h,a
		ANA(_H, _A); break;
	case 0x0f:	// ana l,a
		ANA(_L, _A); break;
	case 0x10:	// xra v,a
		XRA(_V, _A); break;
	case 0x11:	// xra a,a
		XRA(_A, _A); break;
	case 0x12:	// xra b,a
		XRA(_B, _A); break;
	case 0x13:	// xra c,a
		XRA(_C, _A); break;
	case 0x14:	// xra d,a
		XRA(_D, _A); break;
	case 0x15:	// xra e,a
		XRA(_E, _A); break;
	case 0x16:	// xra h,a
		XRA(_H, _A); break;
	case 0x17:	// xra l,a
		XRA(_L, _A); break;
	case 0x18:	// ora v,a
		ORA(_V, _A); break;
	case 0x19:	// ora a,a
		ORA(_A, _A); break;
	case 0x1a:	// ora b,a
		ORA(_B, _A); break;
	case 0x1b:	// ora c,a
		ORA(_C, _A); break;
	case 0x1c:	// ora d,a
		ORA(_D, _A); break;
	case 0x1d:	// ora e,a
		ORA(_E, _A); break;
	case 0x1e:	// ora h,a
		ORA(_H, _A); break;
	case 0x1f:	// ora l,a
		ORA(_L, _A); break;
	case 0x20:	// addnc v,a
		ADDNC(_V, _A); break;
	case 0x21:	// addnc a,a
		ADDNC(_A, _A); break;
	case 0x22:	// addnc b,a
		ADDNC(_B, _A); break;
	case 0x23:	// addnc c,a
		ADDNC(_C, _A); break;
	case 0x24:	// addnc d,a
		ADDNC(_D, _A); break;
	case 0x25:	// addnc e,a
		ADDNC(_E, _A); break;
	case 0x26:	// addnc h,a
		ADDNC(_H, _A); break;
	case 0x27:	// addnc l,a
		ADDNC(_L, _A); break;
	case 0x28:	// gta v,a
		GTA(_V, _A); break;
	case 0x29:	// gta a,a
		GTA(_A, _A); break;
	case 0x2a:	// gta b,a
		GTA(_B, _A); break;
	case 0x2b:	// gta c,a
		GTA(_C, _A); break;
	case 0x2c:	// gta d,a
		GTA(_D, _A); break;
	case 0x2d:	// gta e,a
		GTA(_E, _A); break;
	case 0x2e:	// gta h,a
		GTA(_H, _A); break;
	case 0x2f:	// gta l,a
		GTA(_L, _A); break;
	case 0x30:	// subnb v,a
		SUBNB(_V, _A); break;
	case 0x31:	// subnb a,a
		SUBNB(_A, _A); break;
	case 0x32:	// subnb b,a
		SUBNB(_B, _A); break;
	case 0x33:	// subnb c,a
		SUBNB(_C, _A); break;
	case 0x34:	// subnb d,a
		SUBNB(_D, _A); break;
	case 0x35:	// subnb e,a
		SUBNB(_E, _A); break;
	case 0x36:	// subnb h,a
		SUBNB(_H, _A); break;
	case 0x37:	// subnb l,a
		SUBNB(_L, _A); break;
	case 0x38:	// lta v,a
		LTA(_V, _A); break;
	case 0x39:	// lta a,a
		LTA(_A, _A); break;
	case 0x3a:	// lta b,a
		LTA(_B, _A); break;
	case 0x3b:	// lta c,a
		LTA(_C, _A); break;
	case 0x3c:	// lta d,a
		LTA(_D, _A); break;
	case 0x3d:	// lta e,a
		LTA(_E, _A); break;
	case 0x3e:	// lta h,a
		LTA(_H, _A); break;
	case 0x3f:	// lta l,a
		LTA(_L, _A); break;
	case 0x40:	// add v,a
		ADD(_V, _A); break;
	case 0x41:	// add a,a
		ADD(_A, _A); break;
	case 0x42:	// add b,a
		ADD(_B, _A); break;
	case 0x43:	// add c,a
		ADD(_C, _A); break;
	case 0x44:	// add d,a
		ADD(_D, _A); break;
	case 0x45:	// add e,a
		ADD(_E, _A); break;
	case 0x46:	// add h,a
		ADD(_H, _A); break;
	case 0x47:	// add l,a
		ADD(_L, _A); break;
	case 0x50:	// adc v,a
		ADC(_V, _A); break;
	case 0x51:	// adc a,a
		ADC(_A, _A); break;
	case 0x52:	// adc b,a
		ADC(_B, _A); break;
	case 0x53:	// adc c,a
		ADC(_C, _A); break;
	case 0x54:	// adc d,a
		ADC(_D, _A); break;
	case 0x55:	// adc e,a
		ADC(_E, _A); break;
	case 0x56:	// adc h,a
		ADC(_H, _A); break;
	case 0x57:	// adc l,a
		ADC(_L, _A); break;
	case 0x60:	// sub v,a
		SUB(_V, _A); break;
	case 0x61:	// sub a,a
		SUB(_A, _A); break;
	case 0x62:	// sub b,a
		SUB(_B, _A); break;
	case 0x63:	// sub c,a
		SUB(_C, _A); break;
	case 0x64:	// sub d,a
		SUB(_D, _A); break;
	case 0x65:	// sub e,a
		SUB(_E, _A); break;
	case 0x66:	// sub h,a
		SUB(_H, _A); break;
	case 0x67:	// sub l,a
		SUB(_L, _A); break;
	case 0x68:	// nea v,a
		NEA(_V, _A); break;
	case 0x69:	// nea a,a
		NEA(_A, _A); break;
	case 0x6a:	// nea b,a
		NEA(_B, _A); break;
	case 0x6b:	// nea c,a
		NEA(_C, _A); break;
	case 0x6c:	// nea d,a
		NEA(_D, _A); break;
	case 0x6d:	// nea e,a
		NEA(_E, _A); break;
	case 0x6e:	// nea h,a
		NEA(_H, _A); break;
	case 0x6f:	// nea l,a
		NEA(_L, _A); break;
	case 0x70:	// sbb v,a
		SBB(_V, _A); break;
	case 0x71:	// sbb a,a
		SBB(_A, _A); break;
	case 0x72:	// sbb b,a
		SBB(_B, _A); break;
	case 0x73:	// sbb c,a
		SBB(_C, _A); break;
	case 0x74:	// sbb d,a
		SBB(_D, _A); break;
	case 0x75:	// sbb e,a
		SBB(_E, _A); break;
	case 0x76:	// sbb h,a
		SBB(_H, _A); break;
	case 0x77:	// sbb l,a
		SBB(_L, _A); break;
	case 0x78:	// eqa v,a
		EQA(_V, _A); break;
	case 0x79:	// eqa a,a
		EQA(_A, _A); break;
	case 0x7a:	// eqa b,a
		EQA(_B, _A); break;
	case 0x7b:	// eqa c,a
		EQA(_C, _A); break;
	case 0x7c:	// eqa d,a
		EQA(_D, _A); break;
	case 0x7d:	// eqa e,a
		EQA(_E, _A); break;
	case 0x7e:	// eqa h,a
		EQA(_H, _A); break;
	case 0x7f:	// eqa l,a
		EQA(_L, _A); break;
	case 0x88:	// ana a,v
		ANA(_A, _V); break;
	case 0x89:	// ana a,a
		ANA(_A, _A); break;
	case 0x8a:	// ana a,b
		ANA(_A, _B); break;
	case 0x8b:	// ana a,c
		ANA(_A, _C); break;
	case 0x8c:	// ana a,d
		ANA(_A, _D); break;
	case 0x8d:	// ana a,e
		ANA(_A, _E); break;
	case 0x8e:	// ana a,h
		ANA(_A, _H); break;
	case 0x8f:	// ana a,l
		ANA(_A, _L); break;
	case 0x90:	// xra a,v
		XRA(_A, _V); break;
	case 0x91:	// xra a,a
		XRA(_A, _A); break;
	case 0x92:	// xra a,b
		XRA(_A, _B); break;
	case 0x93:	// xra a,c
		XRA(_A, _C); break;
	case 0x94:	// xra a,d
		XRA(_A, _D); break;
	case 0x95:	// xra a,e
		XRA(_A, _E); break;
	case 0x96:	// xra a,h
		XRA(_A, _H); break;
	case 0x97:	// xra a,l
		XRA(_A, _L); break;
	case 0x98:	// ora a,v
		ORA(_A, _V); break;
	case 0x99:	// ora a,a
		ORA(_A, _A); break;
	case 0x9a:	// ora a,b
		ORA(_A, _B); break;
	case 0x9b:	// ora a,c
		ORA(_A, _C); break;
	case 0x9c:	// ora a,d
		ORA(_A, _D); break;
	case 0x9d:	// ora a,e
		ORA(_A, _E); break;
	case 0x9e:	// ora a,h
		ORA(_A, _H); break;
	case 0x9f:	// ora a,l
		ORA(_A, _L); break;
	case 0xa0:	// addnc a,v
		ADDNC(_A, _V); break;
	case 0xa1:	// addnc a,a
		ADDNC(_A, _A); break;
	case 0xa2:	// addnc a,b
		ADDNC(_A, _B); break;
	case 0xa3:	// addnc a,c
		ADDNC(_A, _C); break;
	case 0xa4:	// addnc a,d
		ADDNC(_A, _D); break;
	case 0xa5:	// addnc a,e
		ADDNC(_A, _E); break;
	case 0xa6:	// addnc a,h
		ADDNC(_A, _H); break;
	case 0xa7:	// addnc a,l
		ADDNC(_A, _L); break;
	case 0xa8:	// gta a,v
		GTA(_A, _V); break;
	case 0xa9:	// gta a,a
		GTA(_A, _A); break;
	case 0xaa:	// gta a,b
		GTA(_A, _B); break;
	case 0xab:	// gta a,c
		GTA(_A, _C); break;
	case 0xac:	// gta a,d
		GTA(_A, _D); break;
	case 0xad:	// gta a,e
		GTA(_A, _E); break;
	case 0xae:	// gta a,h
		GTA(_A, _H); break;
	case 0xaf:	// gta a,l
		GTA(_A, _L); break;
	case 0xb0:	// subnb a,v
		SUBNB(_A, _V); break;
	case 0xb1:	// subnb a,a
		SUBNB(_A, _A); break;
	case 0xb2:	// subnb a,b
		SUBNB(_A, _B); break;
	case 0xb3:	// subnb a,c
		SUBNB(_A, _C); break;
	case 0xb4:	// subnb a,d
		SUBNB(_A, _D); break;
	case 0xb5:	// subnb a,e
		SUBNB(_A, _E); break;
	case 0xb6:	// subnb a,h
		SUBNB(_A, _H); break;
	case 0xb7:	// subnb a,l
		SUBNB(_A, _L); break;
	case 0xb8:	// lta a,v
		LTA(_A, _V); break;
	case 0xb9:	// lta a,a
		LTA(_A, _A); break;
	case 0xba:	// lta a,b
		LTA(_A, _B); break;
	case 0xbb:	// lta a,c
		LTA(_A, _C); break;
	case 0xbc:	// lta a,d
		LTA(_A, _D); break;
	case 0xbd:	// lta a,e
		LTA(_A, _E); break;
	case 0xbe:	// lta a,h
		LTA(_A, _H); break;
	case 0xbf:	// lta a,l
		LTA(_A, _L); break;
	case 0xc0:	// add a,v
		ADD(_A, _V); break;
	case 0xc1:	// add a,a
		ADD(_A, _A); break;
	case 0xc2:	// add a,b
		ADD(_A, _B); break;
	case 0xc3:	// add a,c
		ADD(_A, _C); break;
	case 0xc4:	// add a,d
		ADD(_A, _D); break;
	case 0xc5:	// add a,e
		ADD(_A, _E); break;
	case 0xc6:	// add a,h
		ADD(_A, _H); break;
	case 0xc7:	// add a,l
		ADD(_A, _L); break;
	case 0xc8:	// ona a,v
		ONA(_A, _V); break;
	case 0xc9:	// ona a,a
		ONA(_A, _A); break;
	case 0xca:	// ona a,b
		ONA(_A, _B); break;
	case 0xcb:	// ona a,c
		ONA(_A, _C); break;
	case 0xcc:	// ona a,d
		ONA(_A, _D); break;
	case 0xcd:	// ona a,e
		ONA(_A, _E); break;
	case 0xce:	// ona a,h
		ONA(_A, _H); break;
	case 0xcf:	// ona a,l
		ONA(_A, _L); break;
	case 0xd0:	// adc a,v
		ADC(_A, _V); break;
	case 0xd1:	// adc a,a
		ADC(_A, _A); break;
	case 0xd2:	// adc a,b
		ADC(_A, _B); break;
	case 0xd3:	// adc a,c
		ADC(_A, _C); break;
	case 0xd4:	// adc a,d
		ADC(_A, _D); break;
	case 0xd5:	// adc a,e
		ADC(_A, _E); break;
	case 0xd6:	// adc a,h
		ADC(_A, _H); break;
	case 0xd7:	// adc a,l
		ADC(_A, _L); break;
	case 0xd8:	// offa a,v
		OFFA(_A, _V); break;
	case 0xd9:	// offa a,a
		OFFA(_A, _A); break;
	case 0xda:	// offa a,b
		OFFA(_A, _B); break;
	case 0xdb:	// offa a,c
		OFFA(_A, _C); break;
	case 0xdc:	// offa a,d
		OFFA(_A, _D); break;
	case 0xdd:	// offa a,e
		OFFA(_A, _E); break;
	case 0xde:	// offa a,h
		OFFA(_A, _H); break;
	case 0xdf:	// offa a,l
		OFFA(_A, _L); break;
	case 0xe0:	// sub a,v
		SUB(_A, _V); break;
	case 0xe1:	// sub a,a
		SUB(_A, _A); break;
	case 0xe2:	// sub a,b
		SUB(_A, _B); break;
	case 0xe3:	// sub a,c
		SUB(_A, _C); break;
	case 0xe4:	// sub a,d
		SUB(_A, _D); break;
	case 0xe5:	// sub a,e
		SUB(_A, _E); break;
	case 0xe6:	// sub a,h
		SUB(_A, _H); break;
	case 0xe7:	// sub a,l
		SUB(_A, _L); break;
	case 0xe8:	// nea a,v
		NEA(_A, _V); break;
	case 0xe9:	// nea a,a
		NEA(_A, _A); break;
	case 0xea:	// nea a,b
		NEA(_A, _B); break;
	case 0xeb:	// nea a,c
		NEA(_A, _C); break;
	case 0xec:	// nea a,d
		NEA(_A, _D); break;
	case 0xed:	// nea a,e
		NEA(_A, _E); break;
	case 0xee:	// nea a,h
		NEA(_A, _H); break;
	case 0xef:	// nea a,l
		NEA(_A, _L); break;
	case 0xf0:	// sbb a,v
		SBB(_A, _V); break;
	case 0xf1:	// sbb a,a
		SBB(_A, _A); break;
	case 0xf2:	// sbb a,b
		SBB(_A, _B); break;
	case 0xf3:	// sbb a,c
		SBB(_A, _C); break;
	case 0xf4:	// sbb a,d
		SBB(_A, _D); break;
	case 0xf5:	// sbb a,e
		SBB(_A, _E); break;
	case 0xf6:	// sbb a,h
		SBB(_A, _H); break;
	case 0xf7:	// sbb a,l
		SBB(_A, _L); break;
	case 0xf8:	// eqa a,v
		EQA(_A, _V); break;
	case 0xf9:	// eqa a,a
		EQA(_A, _A); break;
	case 0xfa:	// eqa a,b
		EQA(_A, _B); break;
	case 0xfb:	// eqa a,c
		EQA(_A, _C); break;
	case 0xfc:	// eqa a,d
		EQA(_A, _D); break;
	case 0xfd:	// eqa a,e
		EQA(_A, _E); break;
	case 0xfe:	// eqa a,h
		EQA(_A, _H); break;
	case 0xff:	// eqa a,l
		EQA(_A, _L); break;
	default:
        break;
//		emu->out_debug(_T("PC=%4x\tCPU\tUNKNOWN OP : 60 %2x\n"), prevPC, ope);
	}
}

void Cupd7801::OP64()
{
    quint8 ope = FETCH8();
	period += op64[ope].clock;
	
	switch(ope) {
	case 0x08:	// ani v,byte
		ANI(_V); break;
	case 0x09:	// ani a,byte
		ANI(_A); break;
	case 0x0a:	// ani b,byte
		ANI(_B); break;
	case 0x0b:	// ani c,byte
		ANI(_C); break;
	case 0x0c:	// ani d,byte
		ANI(_D); break;
	case 0x0d:	// ani e,byte
		ANI(_E); break;
	case 0x0e:	// ani h,byte
		ANI(_H); break;
	case 0x0f:	// ani l,byte
		ANI(_L); break;
	case 0x10:	// xri v,byte
		XRI(_V); break;
	case 0x11:	// xri a,byte
		XRI(_A); break;
	case 0x12:	// xri b,byte
		XRI(_B); break;
	case 0x13:	// xri c,byte
		XRI(_C); break;
	case 0x14:	// xri d,byte
		XRI(_D); break;
	case 0x15:	// xri e,byte
		XRI(_E); break;
	case 0x16:	// xri h,byte
		XRI(_H); break;
	case 0x17:	// xri l,byte
		XRI(_L); break;
	case 0x18:	// ori v,byte
		ORI(_V); break;
	case 0x19:	// ori a,byte
		ORI(_A); break;
	case 0x1a:	// ori b,byte
		ORI(_B); break;
	case 0x1b:	// ori c,byte
		ORI(_C); break;
	case 0x1c:	// ori d,byte
		ORI(_D); break;
	case 0x1d:	// ori e,byte
		ORI(_E); break;
	case 0x1e:	// ori h,byte
		ORI(_H); break;
	case 0x1f:	// ori l,byte
		ORI(_L); break;
	case 0x20:	// adinc v,byte
		ADINC(_V); break;
	case 0x21:	// adinc a,byte
		ADINC(_A); break;
	case 0x22:	// adinc b,byte
		ADINC(_B); break;
	case 0x23:	// adinc c,byte
		ADINC(_C); break;
	case 0x24:	// adinc d,byte
		ADINC(_D); break;
	case 0x25:	// adinc e,byte
		ADINC(_E); break;
	case 0x26:	// adinc h,byte
		ADINC(_H); break;
	case 0x27:	// adinc l,byte
		ADINC(_L); break;
	case 0x28:	// gti v,byte
		GTI(_V); break;
	case 0x29:	// gti a,byte
		GTI(_A); break;
	case 0x2a:	// gti b,byte
		GTI(_B); break;
	case 0x2b:	// gti c,byte
		GTI(_C); break;
	case 0x2c:	// gti d,byte
		GTI(_D); break;
	case 0x2d:	// gti e,byte
		GTI(_E); break;
	case 0x2e:	// gti h,byte
		GTI(_H); break;
	case 0x2f:	// gti l,byte
		GTI(_L); break;
	case 0x30:	// suinb v,byte
		SUINB(_V); break;
	case 0x31:	// suinb a,byte
		SUINB(_A); break;
	case 0x32:	// suinb b,byte
		SUINB(_B); break;
	case 0x33:	// suinb c,byte
		SUINB(_C); break;
	case 0x34:	// suinb d,byte
		SUINB(_D); break;
	case 0x35:	// suinb e,byte
		SUINB(_E); break;
	case 0x36:	// suinb h,byte
		SUINB(_H); break;
	case 0x37:	// suinb l,byte
		SUINB(_L); break;
	case 0x38:	// lti v,byte
		LTI(_V); break;
	case 0x39:	// lti a,byte
		LTI(_A); break;
	case 0x3a:	// lti b,byte
		LTI(_B); break;
	case 0x3b:	// lti c,byte
		LTI(_C); break;
	case 0x3c:	// lti d,byte
		LTI(_D); break;
	case 0x3d:	// lti e,byte
		LTI(_E); break;
	case 0x3e:	// lti h,byte
		LTI(_H); break;
	case 0x3f:	// lti l,byte
		LTI(_L); break;
	case 0x40:	// adi v,byte
		ADI(_V); break;
	case 0x41:	// adi a,byte
		ADI(_A); break;
	case 0x42:	// adi b,byte
		ADI(_B); break;
	case 0x43:	// adi c,byte
		ADI(_C); break;
	case 0x44:	// adi d,byte
		ADI(_D); break;
	case 0x45:	// adi e,byte
		ADI(_E); break;
	case 0x46:	// adi h,byte
		ADI(_H); break;
	case 0x47:	// adi l,byte
		ADI(_L); break;
	case 0x48:	// oni v,byte
		ONI(_V); break;
	case 0x49:	// oni a,byte
		ONI(_A); break;
	case 0x4a:	// oni b,byte
		ONI(_B); break;
	case 0x4b:	// oni c,byte
		ONI(_C); break;
	case 0x4c:	// oni d,byte
		ONI(_D); break;
	case 0x4d:	// oni e,byte
		ONI(_E); break;
	case 0x4e:	// oni h,byte
		ONI(_H); break;
	case 0x4f:	// oni l,byte
		ONI(_L); break;
	case 0x50:	// aci v,byte
		ACI(_V); break;
	case 0x51:	// aci a,byte
		ACI(_A); break;
	case 0x52:	// aci b,byte
		ACI(_B); break;
	case 0x53:	// aci c,byte
		ACI(_C); break;
	case 0x54:	// aci d,byte
		ACI(_D); break;
	case 0x55:	// aci e,byte
		ACI(_E); break;
	case 0x56:	// aci h,byte
		ACI(_H); break;
	case 0x57:	// aci l,byte
		ACI(_L); break;
	case 0x58:	// offi v,byte
		OFFI(_V); break;
	case 0x59:	// offi a,byte
		OFFI(_A); break;
	case 0x5a:	// offi b,byte
		OFFI(_B); break;
	case 0x5b:	// offi c,byte
		OFFI(_C); break;
	case 0x5c:	// offi d,byte
		OFFI(_D); break;
	case 0x5d:	// offi e,byte
		OFFI(_E); break;
	case 0x5e:	// offi h,byte
		OFFI(_H); break;
	case 0x5f:	// offi l,byte
		OFFI(_L); break;
	case 0x60:	// sui v,byte
		SUI(_V); break;
	case 0x61:	// sui a,byte
		SUI(_A); break;
	case 0x62:	// sui b,byte
		SUI(_B); break;
	case 0x63:	// sui c,byte
		SUI(_C); break;
	case 0x64:	// sui d,byte
		SUI(_D); break;
	case 0x65:	// sui e,byte
		SUI(_E); break;
	case 0x66:	// sui h,byte
		SUI(_H); break;
	case 0x67:	// sui l,byte
		SUI(_L); break;
	case 0x68:	// nei v,byte
		NEI(_V); break;
	case 0x69:	// nei a,byte
		NEI(_A); break;
	case 0x6a:	// nei b,byte
		NEI(_B); break;
	case 0x6b:	// nei c,byte
		NEI(_C); break;
	case 0x6c:	// nei d,byte
		NEI(_D); break;
	case 0x6d:	// nei e,byte
		NEI(_E); break;
	case 0x6e:	// nei h,byte
		NEI(_H); break;
	case 0x6f:	// nei l,byte
		NEI(_L); break;
	case 0x70:	// sbi v,byte
		SBI(_V); break;
	case 0x71:	// sbi a,byte
		SBI(_A); break;
	case 0x72:	// sbi b,byte
		SBI(_B); break;
	case 0x73:	// sbi c,byte
		SBI(_C); break;
	case 0x74:	// sbi d,byte
		SBI(_D); break;
	case 0x75:	// sbi e,byte
		SBI(_E); break;
	case 0x76:	// sbi h,byte
		SBI(_H); break;
	case 0x77:	// sbi l,byte
		SBI(_L); break;
	case 0x78:	// eqi v,byte
		EQI(_V); break;
	case 0x79:	// eqi a,byte
		EQI(_A); break;
	case 0x7a:	// eqi b,byte
		EQI(_B); break;
	case 0x7b:	// eqi c,byte
		EQI(_C); break;
	case 0x7c:	// eqi d,byte
		EQI(_D); break;
	case 0x7d:	// eqi e,byte
		EQI(_E); break;
	case 0x7e:	// eqi h,byte
		EQI(_H); break;
	case 0x7f:	// eqi l,byte
		EQI(_L); break;
	case 0x88:	// ani pa,byte
		ANI_IO(P_A); break;
	case 0x89:	// ani pb,byte
		ANI_IO(P_B); break;
	case 0x8a:	// ani pc,byte
		ANI_IO(P_C); break;
	case 0x8b:	// ani mk,byte
		ANI(MK); break;
	case 0x90:	// xri pa,byte
		XRI_IO(P_A); break;
	case 0x91:	// xri pb,byte
		XRI_IO(P_B); break;
	case 0x92:	// xri pc,byte
		XRI_IO(P_C); break;
	case 0x93:	// xri mk,byte
		XRI(MK); break;
	case 0x98:	// ori pa,byte
		ORI_IO(P_A); break;
	case 0x99:	// ori pb,byte
		ORI_IO(P_B); break;
	case 0x9a:	// ori pc,byte
		ORI_IO(P_C); break;
	case 0x9b:	// ori mk,byte
		ORI(MK); break;
	case 0xa0:	// adinc pa,byte
		ADINC_IO(P_A); break;
	case 0xa1:	// adinc pb,byte
		ADINC_IO(P_B); break;
	case 0xa2:	// adinc pc,byte
		ADINC_IO(P_C); break;
	case 0xa3:	// adinc mk,byte
		ADINC(MK); break;
	case 0xa8:	// gti pa,byte
		GTI_IO(P_A); break;
	case 0xa9:	// gti pb,byte
		GTI_IO(P_B); break;
	case 0xaa:	// gti pc,byte
		GTI_IO(P_C); break;
	case 0xab:	// gti mk,byte
		GTI(MK); break;
	case 0xb0:	// suinb pa,byte
		SUINB_IO(P_A); break;
	case 0xb1:	// suinb pb,byte
		SUINB_IO(P_B); break;
	case 0xb2:	// suinb pc,byte
		SUINB_IO(P_C); break;
	case 0xb3:	// suinb mk,byte
		SUINB(MK); break;
	case 0xb8:	// lti pa,byte
		LTI_IO(P_A); break;
	case 0xb9:	// lti pb,byte
		LTI_IO(P_B); break;
	case 0xba:	// lti pc,byte
		LTI_IO(P_C); break;
	case 0xbb:	// lti mk,byte
		LTI(MK); break;
	case 0xc0:	// adi pa,byte
		ADI_IO(P_A); break;
	case 0xc1:	// adi pb,byte
		ADI_IO(P_B); break;
	case 0xc2:	// adi pc,byte
		ADI_IO(P_C); break;
	case 0xc3:	// adi mk,byte
		ADI(MK); break;
	case 0xc8:	// oni pa,byte
		ONI_IO(P_A); break;
	case 0xc9:	// oni pb,byte
		ONI_IO(P_B); break;
	case 0xca:	// oni pc,byte
		ONI_IO(P_C); break;
	case 0xcb:	// oni mk,byte
		ONI(MK); break;
	case 0xd0:	// aci pa,byte
		ACI_IO(P_A); break;
	case 0xd1:	// aci pb,byte
		ACI_IO(P_B); break;
	case 0xd2:	// aci pc,byte
		ACI_IO(P_C); break;
	case 0xd3:	// aci mk,byte
		ACI(MK); break;
	case 0xd8:	// offi pa,byte
		OFFI_IO(P_A); break;
	case 0xd9:	// offi pb,byte
		OFFI_IO(P_B); break;
	case 0xda:	// offi pc,byte
		OFFI_IO(P_C); break;
	case 0xdb:	// offi mk,byte
		OFFI(MK); break;
	case 0xe0:	// sui pa,byte
		SUI_IO(P_A); break;
	case 0xe1:	// sui pb,byte
		SUI_IO(P_B); break;
	case 0xe2:	// sui pc,byte
		SUI_IO(P_C); break;
	case 0xe3:	// sui mk,byte
		SUI(MK); break;
	case 0xe8:	// nei pa,byte
		NEI_IO(P_A); break;
	case 0xe9:	// nei pb,byte
		NEI_IO(P_B); break;
	case 0xea:	// nei pc,byte
		NEI_IO(P_C); break;
	case 0xeb:	// nei mk,byte
		NEI(MK); break;
	case 0xf0:	// sbi pa,byte
		SBI_IO(P_A); break;
	case 0xf1:	// sbi pb,byte
		SBI_IO(P_B); break;
	case 0xf2:	// sbi pc,byte
		SBI_IO(P_C); break;
	case 0xf3:	// sbi mk,byte
		SBI(MK); break;
	case 0xf8:	// eqi pa,byte
		EQI_IO(P_A); break;
	case 0xf9:	// eqi pb,byte
		EQI_IO(P_B); break;
	case 0xfa:	// eqi pc,byte
		EQI_IO(P_C); break;
	case 0xfb:	// eqi mk,byte
		EQI(MK); break;
	default:
//		emu->out_debug(_T("PC=%4x\tCPU\tUNKNOWN OP : 64 %2x\n"), prevPC, ope);
        break;
	}
}

void Cupd7801::OP70()
{
    quint8 ope = FETCH8();
	period += op70[ope].clock;
	
	switch(ope) {
	case 0x0e:	// sspd word
		WM16(FETCH16(), SP); break;
	case 0x0f:	// lspd word
		SP = RM16(FETCH16()); break;
	case 0x1e:	// sbcd word
		WM16(FETCH16(), BC); break;
	case 0x1f:	// lbcd word
		BC = RM16(FETCH16()); break;
	case 0x2e:	// sded word
		WM16(FETCH16(), DE); break;
	case 0x2f:	// lded word
		DE = RM16(FETCH16()); break;
	case 0x3e:	// shld word
		WM16(FETCH16(), HL); break;
	case 0x3f:	// lhld word
		HL = RM16(FETCH16()); break;
	case 0x68:	// mov v,word
		_V = RM8(FETCH16()); 
		break;
	case 0x69:	// mov a,word
		_A = RM8(FETCH16()); break;
	case 0x6a:	// mov b,word
		_B = RM8(FETCH16()); break;
	case 0x6b:	// mov c,word
		_C = RM8(FETCH16()); break;
	case 0x6c:	// mov d,word
		_D = RM8(FETCH16()); break;
	case 0x6d:	// mov e,word
		_E = RM8(FETCH16()); break;
	case 0x6e:	// mov h,word
		_H = RM8(FETCH16()); break;
	case 0x6f:	// mov l,word
		_L = RM8(FETCH16()); break;
	case 0x78:	// mov word,v
		WM8(FETCH16(), _V); break;
	case 0x79:	// mov word,a
		WM8(FETCH16(), _A); break;
	case 0x7a:	// mov word,b
		WM8(FETCH16(), _B); break;
	case 0x7b:	// mov word,c
		WM8(FETCH16(), _C); break;
	case 0x7c:	// mov word,d
		WM8(FETCH16(), _D); break;
	case 0x7d:	// mov word,e
		WM8(FETCH16(), _E); break;
	case 0x7e:	// mov word,h
		WM8(FETCH16(), _H); break;
	case 0x7f:	// mov word,l
		WM8(FETCH16(), _L); break;
	case 0x89:	// anax b
		ANAX(BC); break;
	case 0x8a:	// anax d
		ANAX(DE); break;
	case 0x8b:	// anax h
		ANAX(HL); break;
	case 0x8c:	// anax d+
		ANAX(DE++); break;
	case 0x8d:	// anax h+
		ANAX(HL++); break;
	case 0x8e:	// anax d-
		ANAX(DE--); break;
	case 0x8f:	// anax h-
		ANAX(HL--); break;
	case 0x91:	// xrax b
		XRAX(BC); break;
	case 0x92:	// xrax d
		XRAX(DE); break;
	case 0x93:	// xrax h
		XRAX(HL); break;
	case 0x94:	// xrax d+
		XRAX(DE++); break;
	case 0x95:	// xrax h+
		XRAX(HL++); break;
	case 0x96:	// xrax d-
		XRAX(DE--); break;
	case 0x97:	// xrax h-
		XRAX(HL--); break;
	case 0x99:	// orax b
		ORAX(BC); break;
	case 0x9a:	// orax d
		ORAX(DE); break;
	case 0x9b:	// orax h
		ORAX(HL); break;
	case 0x9c:	// orax d+
		ORAX(DE++); break;
	case 0x9d:	// orax h+
		ORAX(HL++); break;
	case 0x9e:	// orax d-
		ORAX(DE--); break;
	case 0x9f:	// orax h-
		ORAX(HL--); break;
	case 0xa1:	// addncx b
		ADDNCX(BC); break;
	case 0xa2:	// addncx d
		ADDNCX(DE); break;
	case 0xa3:	// addncx h
		ADDNCX(HL); break;
	case 0xa4:	// addncx d+
		ADDNCX(DE++); break;
	case 0xa5:	// addncx h+
		ADDNCX(HL++); break;
	case 0xa6:	// addncx d-
		ADDNCX(DE--); break;
	case 0xa7:	// addncx h-
		ADDNCX(HL--); break;
	case 0xa9:	// gtax b
		GTAX(BC); break;
	case 0xaa:	// gtax d
		GTAX(DE); break;
	case 0xab:	// gtax h
		GTAX(HL); break;
	case 0xac:	// gtax d+
		GTAX(DE++); break;
	case 0xad:	// gtax h+
		GTAX(HL++); break;
	case 0xae:	// gtax d-
		GTAX(DE--); break;
	case 0xaf:	// gtax h-
		GTAX(HL--); break;
	case 0xb1:	// subnbx b
		SUBNBX(BC); break;
	case 0xb2:	// subnbx d
		SUBNBX(DE); break;
	case 0xb3:	// subnbx h
		SUBNBX(HL); break;
	case 0xb4:	// subnbx d+
		SUBNBX(DE++); break;
	case 0xb5:	// subnbx h+
		SUBNBX(HL++); break;
	case 0xb6:	// subnbx d-
		SUBNBX(DE--); break;
	case 0xb7:	// subnbx h-
		SUBNBX(HL--); break;
	case 0xb9:	// ltax b
		LTAX(BC); break;
	case 0xba:	// ltax d
		LTAX(DE); break;
	case 0xbb:	// ltax h
		LTAX(HL); break;
	case 0xbc:	// ltax d+
		LTAX(DE++); break;
	case 0xbd:	// ltax h+
		LTAX(HL++); break;
	case 0xbe:	// ltax d-
		LTAX(DE--); break;
	case 0xbf:	// ltax h-
		LTAX(HL--); break;
	case 0xc1:	// addx b
		ADDX(BC); break;
	case 0xc2:	// addx d
		ADDX(DE); break;
	case 0xc3:	// addx h
		ADDX(HL); break;
	case 0xc4:	// addx d+
		ADDX(DE++); break;
	case 0xc5:	// addx h+
		ADDX(HL++); break;
	case 0xc6:	// addx d-
		ADDX(DE--); break;
	case 0xc7:	// addx h-
		ADDX(HL--); break;
	case 0xc9:	// onax b
		ONAX(BC); break;
	case 0xca:	// onax d
		ONAX(DE); break;
	case 0xcb:	// onax h
		ONAX(HL); break;
	case 0xcc:	// onax d+
		ONAX(DE++); break;
	case 0xcd:	// onax h+
		ONAX(HL++); break;
	case 0xce:	// onax d-
		ONAX(DE--); break;
	case 0xcf:	// onax h-
		ONAX(HL--); break;
	case 0xd1:	// adcx b
		ADCX(BC); break;
	case 0xd2:	// adcx d
		ADCX(DE); break;
	case 0xd3:	// adcx h
		ADCX(HL); break;
	case 0xd4:	// adcx d+
		ADCX(DE++); break;
	case 0xd5:	// adcx h+
		ADCX(HL++); break;
	case 0xd6:	// adcx d-
		ADCX(DE--); break;
	case 0xd7:	// adcx h-
		ADCX(HL--); break;
	case 0xd9:	// offax b
		OFFAX(BC); break;
	case 0xda:	// offax d
		OFFAX(DE); break;
	case 0xdb:	// offax h
		OFFAX(HL); break;
	case 0xdc:	// offax d+
		OFFAX(DE++); break;
	case 0xdd:	// offax h+
		OFFAX(HL++); break;
	case 0xde:	// offax d-
		OFFAX(DE--); break;
	case 0xdf:	// offax h-
		OFFAX(HL--); break;
	case 0xe1:	// subx b
		SUBX(BC); break;
	case 0xe2:	// subx d
		SUBX(DE); break;
	case 0xe3:	// subx h
		SUBX(HL); break;
	case 0xe4:	// subx d+
		SUBX(DE++); break;
	case 0xe5:	// subx h+
		SUBX(HL++); break;
	case 0xe6:	// subx d-
		SUBX(DE--); break;
	case 0xe7:	// subx h-
		SUBX(HL--); break;
	case 0xe9:	// neax b
		NEAX(BC); break;
	case 0xea:	// neax d
		NEAX(DE); break;
	case 0xeb:	// neax h
		NEAX(HL); break;
	case 0xec:	// neax d+
		NEAX(DE++); break;
	case 0xed:	// neax h+
		NEAX(HL++); break;
	case 0xee:	// neax d-
		NEAX(DE--); break;
	case 0xef:	// neax h-
		NEAX(HL--); break;
	case 0xf1:	// sbbx b
		SBBX(BC); break;
	case 0xf2:	// sbbx d
		SBBX(DE); break;
	case 0xf3:	// sbbx h
		SBBX(HL); break;
	case 0xf4:	// sbbx d+
		SBBX(DE++); break;
	case 0xf5:	// sbbx h+
		SBBX(HL++); break;
	case 0xf6:	// sbbx d-
		SBBX(DE--); break;
	case 0xf7:	// sbbx h-
		SBBX(HL--); break;
	case 0xf9:	// eqax b
		EQAX(BC); break;
	case 0xfa:	// eqax d
		EQAX(DE); break;
	case 0xfb:	// eqax h
		EQAX(HL); break;
	case 0xfc:	// eqax d+
		EQAX(DE++); break;
	case 0xfd:	// eqax h+
		EQAX(HL++); break;
	case 0xfe:	// eqax d-
		EQAX(DE--); break;
	case 0xff:	// eqax h-
		EQAX(HL--); break;
	default:
//		emu->out_debug(_T("PC=%4x\tCPU\tUNKNOWN OP : 70 %2x\n"), prevPC, ope);
        break;
	}
}

void Cupd7801::OP74()
{
    quint8 ope = FETCH8();
	period += op74[ope].clock;
	
	switch(ope) {
	case 0x88:	// anaw wa
		ANAW(); break;
	case 0x90:	// xraw wa
		XRAW(); break;
	case 0x98:	// oraw wa
		ORAW(); break;
	case 0xa0:	// addncw wa
		ADDNCW(); break;
	case 0xa8:	// gtaw wa
		GTAW(); break;
	case 0xb0:	// subnbw wa
		SUBNBW(); break;
	case 0xb8:	// ltaw wa
		LTAW(); break;
	case 0xc0:	// addw wa
		ADDW(); break;
	case 0xc8:	// onaw wa
		ONAW(); break;
	case 0xd0:	// adcw wa
		ADCW(); break;
	case 0xd8:	// offaw wa
		OFFAW(); break;
	case 0xe0:	// subw wa
		SUBW(); break;
	case 0xe8:	// neaw wa
		NEAW(); break;
	case 0xf0:	// sbbw wa
		SBBW(); break;
	case 0xf8:	// eqaw wa
		EQAW(); break;
	default:
//		emu->out_debug(_T("PC=%4x\tCPU\tUNKNOWN OP : 74 %2x\n"), prevPC, ope);
        break;
    }
}

Cupd7801::Cupd7801(CPObject *parent) : CCPU(parent)
{
    pDEBUG = new Cdebug_upd7810(parent);
    fn_status="upd7801.sta";
    fn_log="upd7801.log";


    //step_Previous_State = 0;

    regwidget = (CregCPU*) new Cregsz80Widget(0,this);
}

Cupd7801::~Cupd7801()
{
}

bool Cupd7801::init()
{
    Check_Log();
    pDEBUG->init();
    Reset();

    return true;
}

bool Cupd7801::exit()
{
    return true;
}

void Cupd7801::step()
{
    pPC->pTIMER->state += run(-1);
}

void Cupd7801::Reset()
{
    PC = SP = 0;

    VA = BC = DE = HL = altVA = altBC = altDE = altHL = 0;
    PSW = IRR = IFF = SIRQ = HALT = 0;
    _V = MB = MC = TM0 = TM1 = SR = 0xff;
    altVA = VA;
    MK = 0x1f;
    PORTC = TO = SAK = 0;
    count = 0;
    scount = tcount = 0;
    wait = false;
}



void Cupd7801::Load_Internal(QXmlStreamReader *)
{
}

void Cupd7801::save_internal(QXmlStreamWriter *)
{
}

DWORD Cupd7801::get_mem(DWORD adr, int size)
{
}

void Cupd7801::set_mem(DWORD adr, int size, DWORD data)
{
}

DWORD Cupd7801::get_PC()
{
    return PC;
}

void Cupd7801::Regs_Info(UINT8)
{
    sprintf(Regs_String,"EMPTY");

#if 1
    char buf[32];

    sprintf(
    Regs_String,
    "AF=%04x\nBC=%04x\nDE=%04x\nHL=%04x\nSP=%04x\nPC=%04x\n"
    "%c%c%c%c%c%c(%02x)",
//    imem[0x31],imem[0x32],imem[0x33],imem[0x35],
                VA,BC,DE,HL,SP,PC,
                (PSW & F_Z ? 'Z': '-'),
                (PSW & F_SK ? 'S': '-'),
                (PSW & F_HC ? 'H': '-'),
                (PSW & F_L1 ? '1': '-'),
                (PSW & F_L0 ? '0': '-'),
                (PSW & F_CY ? 'C': '-'),
                PSW
    );
//    char linebuf[100];
//    strcat(Regs_String," ,");
//    for (int i=0;i < 0x05;i++)
//    {
//        sprintf(linebuf,"%02x:[",i*0x10);
//        strcat(Regs_String,linebuf);
//        for (int j=0;j<0x10;j++)
//        {
//            sprintf(linebuf,"%02X ",imem[i*0x10 + j]);
//            strcat(Regs_String,linebuf);
//        }
//
//        strcat(Regs_String,"],");
//    }
#endif
}
