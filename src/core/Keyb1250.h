#ifndef _KEYB1250_H
#define _KEYB1250_H
#include "Keyb.h"

BYTE scandef_pc1250[] = {
//+0		+1			+2			+3			+4			+5			+6			+7
'-',		K_CLR,		'*',		'/',		K_DA,		'e',		'd',		'c',
'+',		'9',		'3',		'6',		K_SHT,		'w',		's',		'x',
'.',		'8',		'2',		'5',		K_DEF,		'q',		'a',		'z',
NUL,		NUL,		NUL,		NUL,		NUL,		NUL,		NUL,		NUL,
NUL,		NUL,		NUL,		NUL,		NUL,		NUL,		NUL,		NUL,
NUL,		NUL,		NUL,		NUL,		NUL,		NUL,		NUL,		NUL,
NUL,		NUL,		NUL,		NUL,		NUL,		NUL,		NUL,		NUL,
NUL,		NUL,		NUL,		NUL,		NUL,		NUL,		NUL,		NUL,

NUL,		'7',		'1',		'4',		K_UA,		'r',		'f',		'v',
NUL,		NUL,		'=',		'p',		K_LA,		't',		'g',		'b',
NUL,		NUL,		NUL,		'o',		K_RA,		'y',		'h',		'n',
NUL,		NUL,		NUL,		NUL,		NUL,		'u',		'j',		'm',
NUL,		NUL,		NUL,		NUL,		NUL,		'i',		'k',		' ',
NUL,		NUL,		NUL,		NUL,		NUL,		NUL,		'l',		K_RET,
NUL,		NUL,		NUL,		NUL,		NUL,		NUL,		NUL,		'0',
};


//BYTE scandef_pc1253[] = {
////+0		+1			+2			+3			+4			+5			+6			+7
//'0'*,		K_CLR,		'*',		'/',		K_DA,		'RM'*,		'd',		'c',
//'+',		'9',		'3',		'6',		K_SHT,		'0'*,		's',		'x',
//'.',		'8',		'2',		'5',		K_DEF,		'q',		'a',		'z',
//NUL,		NUL,		NUL,		NUL,		NUL,		NUL,		NUL,		NUL,
//NUL,		NUL,		NUL,		NUL,		NUL,		NUL,		NUL,		NUL,
//NUL,		NUL,		NUL,		NUL,		NUL,		NUL,		NUL,		NUL,
//NUL,		NUL,		NUL,		NUL,		NUL,		NUL,		NUL,		NUL,
//NUL,		NUL,		NUL,		NUL,		NUL,		NUL,		NUL,		NUL,

//NUL,		'7',		'1',		'4',		'CL'*,		'CE'*,		'='*,		'SQR'*,
//NUL,		NUL,		'=',		'p',		'/'*,		'*'*,		'+'*,		'-'*,
//NUL,		NUL,		NUL,		'o',		'9'*,		'6'*,		'%'*,		'3'*,
//NUL,		NUL,		NUL,		NUL,		NUL,		'5'*,		'.'*,		'2'*,
//NUL,		NUL,		NUL,		NUL,		NUL,		'4'*,		'0'*,		'1'*,
//NUL,		NUL,		NUL,		NUL,		NUL,		NUL,		'l',		K_RET,
//NUL,		NUL,		NUL,		NUL,		NUL,		NUL,		NUL,		'0',
//};

BYTE scandef_pc1253[] = {
//+0		+1			+2			+3			+4			+5			+6			+7
NUL,		NUL,		NUL,		NUL,		NUL,		NUL,		NUL,		NUL,
NUL,		NUL,		NUL,		NUL,		NUL,		NUL,		NUL,		NUL,
NUL,		NUL,		NUL,		NUL,		NUL,		NUL,		NUL,		NUL,
NUL,		NUL,		NUL,		NUL,		NUL,		NUL,		NUL,		NUL,
NUL,		NUL,		NUL,		NUL,		NUL,		NUL,		NUL,		NUL,
NUL,		NUL,		NUL,		NUL,		NUL,		NUL,		NUL,		NUL,
NUL,		NUL,		NUL,		NUL,		NUL,		NUL,		NUL,		NUL,
NUL,		NUL,		NUL,		NUL,		NUL,		NUL,		NUL,		NUL,

NUL,		'j',		'i',		'g',		K_CLR,		K_CE,		'=',		K_SQR,
NUL,		NUL,		'h',		'f',		'/',		'*',		'+',		'-',
NUL,		NUL,		NUL,		'e',		'9',		'6',		'%',		'3',
NUL,		NUL,		NUL,		NUL,		'd',		'5',		'.',		'2',
NUL,		NUL,		NUL,		NUL,		NUL,		'4',		'0',		'1',
NUL,		NUL,		NUL,		NUL,		NUL,		NUL,		'a',		'b',
NUL,		NUL,		NUL,		NUL,		NUL,		NUL,		NUL,		'c',
};



TransMap KeyMap1250[]={
{1,	"  -         ",	'-',	369,	182,	1},		// OK
{2,	"  (         ",	'(',	999,	108,	6},
{3,	"  )         ",	')',	999,	108,	6},
{4,	"  *         ",	'*',	369,	153,	1},		// OK
{5,	"  ,         ",	',',	999,	220,	6},
{6,	"  .         ",	'.',	296,	182,	1},		// OK
{7,	"  /         ",	'/',	369,	124,	1},		// OK
{8,	"  :         ",	':',	999,	144,	6},
{9,	"  ;         ",	';',	999,	182,	6},
{11,	"  +         ",	'+',	333,	182,	1},		// OK
{12,	" CLS        ",	K_CLR,	369,	95,		1},		// OK
{13,	" DEF        ",	K_DEF,	16,		96,		7},		// OK
{14,	" Down Arrow ",	K_DA,	86,		96,		7},		// OK
{15,	" ENTER      ",	K_RET,	210,	183,	6},		// OK
{16,	" ENTER      ",	K_RET,	222,	183,	6},		// OK
{17,	" ENTER      ",	K_RET,	234,	183,	6},		// OK
{18,	" Equal      ",	'=',	234,	154,	6},		// OK
{19,	" Left Arrow ",	K_LA ,	156,	96,		7},		// OK
{20,	" MODE       ",	K_MOD,	377,	26,		8},		// OK
{21,	" On/Brk     ",	K_BRK,	226,	96,		7},		// OK
{22,	" On/Off     ",	K_OF ,	999,	24,		6},
{23,	" Rigth Arrow",	K_RA ,	191,	96,		7},		// OK
{24,	" SHIFT1     ",	K_SHT,	51,		96,		7},		// OK
{25,	" SHIFT2     ",	K_SHT,	999,	160,	6},	
{26,	" SML        ",	K_SML,	999,	183,	6},
{27,	" Space      ",	' ',	186,	183,	6},		// OK
{28,	" Up Arrow   ",	K_UA ,	121,	96,		7},		// OK
{29,	"0           ",	'0',	259,	182,	1},		// OK
{30,	"1           ",	'1',	259,	153,	1},		// OK
{31,	"2           ",	'2',	296,	153,	1},		// OK
{32,	"3           ",	'3',	333,	153,	1},		// OK
{33,	"4           ",	'4',	259,	124,	1},		// OK
{34,	"5           ",	'5',	296,	124,	1},		// OK
{35,	"6           ",	'6',	333,	124,	1},		// OK
{36,	"7           ",	'7',	259,	95,		1},		// OK
{37,	"8           ",	'8',	296,	95,		1},		// OK
{38,	"9           ",	'9',	333,	95,		1},		// OK
{39,	"A           ",	'a',	15,		154,	6},		// OK
{40,	"B           ",	'b',	112,	183,	6},		// OK
{41,	"C           ",	'c',	64,		183,	6},		// OK
{42,	"D           ",	'd',	64,		154,	6},		// OK
{43,	"E           ",	'e',	64,		125,	6},		// OK
{44,	"F           ",	'f',	88,		154,	6},		// OK
{45,	"G           ",	'g',	112,	154,	6},		// OK
{46,	"H           ",	'h',	137,	154,	6},		// OK
{47,	"I           ",	'i',	186,	125,	6},		// OK
{48,	"J           ",	'j',	161,	154,	6},		// OK
{49,	"K           ",	'k',	186,	154,	6},		// OK
{50,	"L           ",	'l',	210,	154,	6},		// OK
{51,	"M           ",	'm',	161,	183,	6},		// OK
{52,	"N           ",	'n',	137,	183,	6},		// OK
{53,	"O           ",	'o',	210,	125,	6},		// OK
{54,	"P           ",	'p',	234,	125,	6},		// OK
{55,	"Q           ",	'q',	15,		125,	6},		// OK
{56,	"R           ",	'r',	88,		125,	6},		// OK
{57,	"S           ",	's',	39,		154,	6},		// OK
{58,	"T           ",	't',	112,	125,	6},		// OK
{59,	"U           ",	'u',	161,	125,	6},		// OK
{60,	"V           ",	'v',	88,		183,	6},		// OK
{61,	"W           ",	'w',	39,		125,	6},		// OK
{62,	"X           ",	'x',	39,		183,	6},		// OK
{63,	"Y           ",	'y',	137,	125,	6},		// OK
{64,	"Z           ",	'z',	15,		183,	6}		// OK
};
int KeyMap1250Lenght=63;
#endif
