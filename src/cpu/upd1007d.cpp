#include "upd1007.h"
#include "upd1007d.h"

#include "Debug.h"
#include "pcxxxx.h"


#define EXT_ROM		(pc > 0x0c00)
#define INC_POS		pos++;//pos += (type+1)
#define POS			pos //(pos + type)
#define BIT(x,n) (((x)>>(n))&1)


//const char *const Cdebug_upd1007::reg_5b[4] =  {"sx", "sy", "sz", "sz"};
//const char *const Cdebug_upd1007::reg_8b[8] =  {"pe", "pd", "ib", "ua", "ia", "ie", "tm", "tm"};
//const char *const Cdebug_upd1007::reg_16b[8] = {"ix", "iy", "iz", "us", "ss", "ky", "ky", "ky"};
//const char *const Cdebug_upd1007::jp_cond[8] = {"z", "nc", "lz", "uz", "nz", "c", "nlz"};





const upd1007_dasm Cdebug_upd1007::mnem[512] = {
    /* bit 7 of the second byte cleared */
        {	"adb",	XREG	},	/* code $00 */
        {	"sbb",	XREG	},	/* code $01 */
        {	"ad",	XREG	},	/* code $02 */
        {	"sb",	XREG	},	/* code $03 */
        {	"an",	XREG	},	/* code $04 */
        {	"na",	XREG	},	/* code $05 */
        {	"or",	XREG	},	/* code $06 */
        {	"xr",	XREG	},	/* code $07 */
        {	"adb",	YREG	},	/* code $08 */
        {	"sbb",	YREG	},	/* code $09 */
        {	"ad",	YREG	},	/* code $0A */
        {	"sb",	YREG	},	/* code $0B */
        {	"an",	YREG	},	/* code $0C */
        {	"na",	YREG	},	/* code $0D */
        {	"or",	YREG	},	/* code $0E */
        {	"xr",	YREG	},	/* code $0F */
        {	"adbm",	XARY	},	/* code $10 */
        {	"sbbm",	XARY	},	/* code $11 */
        {	"adm",	XARY	},	/* code $12 */
        {	"sbm",	XARY	},	/* code $13 */
        {	"anm",	XARY	},	/* code $14 */
        {	"nam",	XARY	},	/* code $15 */
        {	"orm",	XARY	},	/* code $16 */
        {	"xrm",	XARY	},	/* code $17 */
        {	"adbm",	YARY	},	/* code $18 */
        {	"sbbm",	YARY	},	/* code $19 */
        {	"adm",	YARY	},	/* code $1A */
        {	"sbm",	YARY	},	/* code $1B */
        {	"anm",	YARY	},	/* code $1C */
        {	"nam",	YARY	},	/* code $1D */
        {	"orm",	YARY	},	/* code $1E */
        {	"xrm",	YARY	},	/* code $1F */
        {	"tadb",	XREG	},	/* code $20 */
        {	"tsbb",	XREG	},	/* code $21 */
        {	"tad",	XREG	},	/* code $22 */
        {	"tsb",	XREG	},	/* code $23 */
        {	"tan",	XREG	},	/* code $24 */
        {	"tna",	XREG	},	/* code $25 */
        {	"tor",	XREG	},	/* code $26 */
        {	"txr",	XREG	},	/* code $27 */
        {	"tadb",	YREG	},	/* code $28 */
        {	"tsbb",	YREG	},	/* code $29 */
        {	"tad",	YREG	},	/* code $2A */
        {	"tsb",	YREG	},	/* code $2B */
        {	"tan",	YREG	},	/* code $2C */
        {	"tna",	YREG	},	/* code $2D */
        {	"tor",	YREG	},	/* code $2E */
        {	"txr",	YREG	},	/* code $2F */
        {	"tadbm",	XARY	},	/* code $30 */
        {	"tsbbm",	XARY	},	/* code $31 */
        {	"tadm",	XARY	},	/* code $32 */
        {	"tsbm",	XARY	},	/* code $33 */
        {	"tanm",	XARY	},	/* code $34 */
        {	"tnam",	XARY	},	/* code $35 */
        {	"torm",	XARY	},	/* code $36 */
        {	"txrm",	XARY	},	/* code $37 */
        {	"tadbm",	YARY	},	/* code $38 */
        {	"tsbbm",	YARY	},	/* code $39 */
        {	"tadm",	YARY	},	/* code $3A */
        {	"tsbm",	YARY	},	/* code $3B */
        {	"tanm",	YARY	},	/* code $3C */
        {	"tnam",	YARY	},	/* code $3D */
        {	"torm",	YARY	},	/* code $3E */
        {	"txrm",	YARY	},	/* code $3F */
        {	"ldw",	WRIM16	},	/* code $40 */
        {	"ldw",	WRIM16	},	/* code $41 */
        {	"ldw",	WRIM16	},	/* code $42 */
        {	"ldw",	WRIM16	},	/* code $43 */
        {	"ldw",	WRIM16	},	/* code $44 */
        {	"ldw",	WRIM16	},	/* code $45 */
        {	"ldw",	WRIM16	},	/* code $46 */
        {	"ldw",	WRIM16	},	/* code $47 */
        {	"adw",	IRIM8	},	/* code $48 */
        {	"adw",	IRIM8	},	/* code $49 */
        {	"adw",	IRIM8	},	/* code $4A */
        {	"adw",	IRIM8	},	/* code $4B */
        {	"sbw",	IRIM8	},	/* code $4C */
        {	"sbw",	IRIM8	},	/* code $4D */
        {	"sbw",	IRIM8	},	/* code $4E */
        {	"sbw",	IRIM8	},	/* code $4F */
        {	"ijmp",	CCINDIR	},	/* code $50 */
        {	"ijmp",	CCINDIR	},	/* code $51 */
        {	"ijmp",	CCINDIR	},	/* code $52 */
        {	"ijmp",	CCINDIR	},	/* code $53 */
        {	"ijmp",	CCINDIR	},	/* code $54 */
        {	"ijmp",	CCINDIR	},	/* code $55 */
        {	"ijmp",	CCINDIR	},	/* code $56 */
        {	"ijmp",	CCINDIR	},	/* code $57 */
        {	"rtn",	NONE	},	/* code $58 */
        {	"ijmp",	CCINDIR	},	/* code $59 */
        {	"ijmp",	CCINDIR	},	/* code $5A */
        {	"ijmp",	CCINDIR	},	/* code $5B */
        {	"ijmp",	CCINDIR	},	/* code $5C */
        {	"ijmp",	CCINDIR	},	/* code $5D */
        {	"ijmp",	CCINDIR	},	/* code $5E */
        {	"ijmp",	CCINDIR	},	/* code $5F */
        {	"cal",	CCIM16	},	/* code $60 */
        {	"cal",	CCIM16	},	/* code $61 */
        {	"cal",	CCIM16	},	/* code $62 */
        {	"cal",	CCIM16	},	/* code $63 */
        {	"cal",	CCIM16	},	/* code $64 */
        {	"cal",	CCIM16	},	/* code $65 */
        {	"cal",	CCIM16	},	/* code $66 */
        {	"cal",	CCIM16	},	/* code $67 */
        {	"cani",	NONE	},	/* code $68 */
        {	"cal",	CCIM16	},	/* code $69 */
        {	"cal",	CCIM16	},	/* code $6A */
        {	"cal",	CCIM16	},	/* code $6B */
        {	"cal",	CCIM16	},	/* code $6C */
        {	"cal",	CCIM16	},	/* code $6D */
        {	"cal",	CCIM16	},	/* code $6E */
        {	"cal",	CCIM16	},	/* code $6F */
        {	"jmp",	CCIM16	},	/* code $70 */
        {	"jmp",	CCIM16	},	/* code $71 */
        {	"jmp",	CCIM16	},	/* code $72 */
        {	"jmp",	CCIM16	},	/* code $73 */
        {	"jmp",	CCIM16	},	/* code $74 */
        {	"jmp",	CCIM16	},	/* code $75 */
        {	"jmp",	CCIM16	},	/* code $76 */
        {	"jmp",	CCIM16	},	/* code $77 */
        {	"rti",	NONE	},	/* code $78 */
        {	"jmp",	CCIM16	},	/* code $79 */
        {	"jmp",	CCIM16	},	/* code $7A */
        {	"jmp",	CCIM16	},	/* code $7B */
        {	"jmp",	CCIM16	},	/* code $7C */
        {	"jmp",	CCIM16	},	/* code $7D */
        {	"jmp",	CCIM16	},	/* code $7E */
        {	"jmp",	CCIM16	},	/* code $7F */
        {	"st",	MEMOREG	},	/* code $80 */
        {	"st",	MEMOREG	},	/* code $81 */
        {	"st",	MEMOREG	},	/* code $82 */
        {	"st",	MEMOREG	},	/* code $83 */
        {	"st",	MEMOREG	},	/* code $84 */
        {	"st",	MEMOREG	},	/* code $85 */
        {	"st",	MEMOREG	},	/* code $86 */
        {	"st",	MEMOREG	},	/* code $87 */
        {	"st",	MEMOREG	},	/* code $88 */
        {	"st",	MEMOREG	},	/* code $89 */
        {	"st",	MEMOREG	},	/* code $8A */
        {	"st",	MEMOREG	},	/* code $8B */
        {	"st",	MEMOREG	},	/* code $8C */
        {	"st",	MEMOREG	},	/* code $8D */
        {	"st",	MEMOREG	},	/* code $8E */
        {	"st",	MEMOREG	},	/* code $8F */
        {	"stm",	MEMOARY	},	/* code $90 */
        {	"stm",	MEMOARY	},	/* code $91 */
        {	"stm",	MEMOARY	},	/* code $92 */
        {	"stm",	MEMOARY	},	/* code $93 */
        {	"stm",	MEMOARY	},	/* code $94 */
        {	"stm",	MEMOARY	},	/* code $95 */
        {	"stm",	MEMOARY	},	/* code $96 */
        {	"stm",	MEMOARY	},	/* code $97 */
        {	"stm",	MEMOARY	},	/* code $98 */
        {	"stm",	MEMOARY	},	/* code $99 */
        {	"stm",	MEMOARY	},	/* code $9A */
        {	"stm",	MEMOARY	},	/* code $9B */
        {	"stm",	MEMOARY	},	/* code $9C */
        {	"stm",	MEMOARY	},	/* code $9D */
        {	"stm",	MEMOARY	},	/* code $9E */
        {	"stm",	MEMOARY	},	/* code $9F */
        {	"ld",	REGMEMO	},	/* code $A0 */
        {	"ld",	REGMEMO	},	/* code $A1 */
        {	"ld",	REGMEMO	},	/* code $A2 */
        {	"ld",	REGMEMO	},	/* code $A3 */
        {	"ld",	REGMEMO	},	/* code $A4 */
        {	"ld",	REGMEMO	},	/* code $A5 */
        {	"ld",	REGMEMO	},	/* code $A6 */
        {	"ld",	REGMEMO	},	/* code $A7 */
        {	"ld",	REGMEMO	},	/* code $A8 */
        {	"ld",	REGMEMO	},	/* code $A9 */
        {	"ld",	REGMEMO	},	/* code $AA */
        {	"ld",	REGMEMO	},	/* code $AB */
        {	"ld",	REGMEMO	},	/* code $AC */
        {	"ld",	REGMEMO	},	/* code $AD */
        {	"ld",	REGMEMO	},	/* code $AE */
        {	"ld",	REGMEMO	},	/* code $AF */
        {	"ldm",	ARYMEMO	},	/* code $B0 */
        {	"ldm",	ARYMEMO	},	/* code $B1 */
        {	"ldm",	ARYMEMO	},	/* code $B2 */
        {	"ldm",	ARYMEMO	},	/* code $B3 */
        {	"ldm",	ARYMEMO	},	/* code $B4 */
        {	"ldm",	ARYMEMO	},	/* code $B5 */
        {	"ldm",	ARYMEMO	},	/* code $B6 */
        {	"ldm",	ARYMEMO	},	/* code $B7 */
        {	"ldm",	ARYMEMO	},	/* code $B8 */
        {	"ldm",	ARYMEMO	},	/* code $B9 */
        {	"ldm",	ARYMEMO	},	/* code $BA */
        {	"ldm",	ARYMEMO	},	/* code $BB */
        {	"ldm",	ARYMEMO	},	/* code $BC */
        {	"ldm",	ARYMEMO	},	/* code $BD */
        {	"ldm",	ARYMEMO	},	/* code $BE */
        {	"ldm",	ARYMEMO	},	/* code $BF */
        {	"pst",	STIM8	},	/* code $C0 */
        {	"pst",	STIM8	},	/* code $C1 */
        {	"pst",	STIM8	},	/* code $C2 */
        {	"pst",	STIM8	},	/* code $C3 */
        {	"pst",	STREG	},	/* code $C4 */
        {	"pst",	STREG	},	/* code $C5 */
        {	"pst",	STREG	},	/* code $C6 */
        {	"pst",	STREG	},	/* code $C7 */
        {	"st",	SMEMOIM8	},	/* code $C8 */
        {	"st",	SMEMOIM8	},	/* code $C9 */
        {	"st",	SMEMOIM8	},	/* code $CA */
        {	"st",	SMEMOIM8	},	/* code $CB */
        {	"pst",	STREG	},	/* code $CC */
        {	"pst",	STREG	},	/* code $CD */
        {	"pst",	STREG	},	/* code $CE */
        {	"pst",	STREG	},	/* code $CF */
        {	"st",	MEMOIM8	},	/* code $D0 */
        {	"st",	MEMOIM8	},	/* code $D1 */
        {	"st",	MEMOIM8	},	/* code $D2 */
        {	"st",	MEMOIM8	},	/* code $D3 */
        {	"gst",	REGST	},	/* code $D4 */
        {	"gst",	REGST	},	/* code $D5 */
        {	"gst",	REGST	},	/* code $D6 */
        {	"gst",	REGST	},	/* code $D7 */
        {	"st",	SMEMOIM8	},	/* code $D8 */
        {	"st",	SMEMOIM8	},	/* code $D9 */
        {	"st",	SMEMOIM8	},	/* code $DA */
        {	"st",	SMEMOIM8	},	/* code $DB */
        {	"gst",	REGST	},	/* code $DC */
        {	"gst",	REGST	},	/* code $DD */
        {	"gst",	REGST	},	/* code $DE */
        {	"gst",	REGST	},	/* code $DF */
        {	"cmp",	REG	},	/* code $E0 */
        {	"cmpm",	ARY	},	/* code $E1 */
        {	"bup",	NONE	},	/* code $E2 */
        {	"byum",	ARY	},	/* code $E3 */
        {	"cmpb",	REG	},	/* code $E4 */
        {	"dium",	ARY	},	/* code $E5 */
        {	"rou",	REG	},	/* code $E6 */
        {	"roum",	ARY	},	/* code $E7 */
        {	"cmp",	REG	},	/* code $E8 */
        {	"cmpm",	ARY	},	/* code $E9 */
        {	"[EA00]",	YREG	},	/* code $EA */
        {	"byum",	ARY	},	/* code $EB */
        {	"cmpb",	REG	},	/* code $EC */
        {	"dium",	ARY	},	/* code $ED */
        {	"rou",	REG	},	/* code $EE */
        {	"roum",	ARY	},	/* code $EF */
        {	"inv",	REG	},	/* code $F0 */
        {	"invm",	ARY	},	/* code $F1 */
        {	"bdn",	NONE	},	/* code $F2 */
        {	"bydm",	ARY	},	/* code $F3 */
        {	"cmpbm",	ARYMTBM	},	/* code $F4 */
        {	"didm",	ARY	},	/* code $F5 */
        {	"rod",	REG	},	/* code $F6 */
        {	"rodm",	ARY	},	/* code $F7 */
        {	"inv",	REG	},	/* code $F8 */
        {	"invm",	ARY	},	/* code $F9 */
        {	"nop",	NONE	},	/* code $FA */
        {	"bydm",	ARY	},	/* code $FB */
        {	"cmpbm",	ARYMTBM	},	/* code $FC */
        {	"didm",	ARY	},	/* code $FD */
        {	"rod",	REG	},	/* code $FE */
        {	"rodm",	ARY	},	/* code $FF */
    /* bit 7 of the second byte set */
        {	"adb",	REGIM8	},	/* code $00 */
        {	"sbb",	REGIM8	},	/* code $01 */
        {	"ad",	REGIM8	},	/* code $02 */
        {	"sb",	REGIM8	},	/* code $03 */
        {	"an",	REGIM8	},	/* code $04 */
        {	"na",	REGIM8	},	/* code $05 */
        {	"or",	REGIM8	},	/* code $06 */
        {	"xr",	REGIM8	},	/* code $07 */
        {	"adb",	REGIM8	},	/* code $08 */
        {	"sbb",	REGIM8	},	/* code $09 */
        {	"ad",	REGIM8	},	/* code $0A */
        {	"sb",	REGIM8	},	/* code $0B */
        {	"an",	REGIM8	},	/* code $0C */
        {	"na",	REGIM8	},	/* code $0D */
        {	"or",	REGIM8	},	/* code $0E */
        {	"xr",	REGIM8	},	/* code $0F */
        {	"adbm",	ARYIM6	},	/* code $10 */
        {	"sbbm",	ARYIM6	},	/* code $11 */
        {	"adm",	ARYIM6	},	/* code $12 */
        {	"sbm",	ARYIM6	},	/* code $13 */
        {	"swp",	XREG	},	/* code $14 */
        {	"swpm",	XARY	},	/* code $15 */
        {	"xcls",	XREG	},	/* code $16 */
        {	"xclsm",	XARY	},	/* code $17 */
        {	"adbm",	ARYIM6	},	/* code $18 */
        {	"sbbm",	ARYIM6	},	/* code $19 */
        {	"adm",	ARYIM6	},	/* code $1A */
        {	"sbm",	ARYIM6	},	/* code $1B */
        {	"xc",	XREG	},	/* code $1C */
        {	"xcm",	XARY	},	/* code $1D */
        {	"xchs",	XREG	},	/* code $1E */
        {	"xchsm",	XARY	},	/* code $1F */
        {	"tadb",	REGIM8	},	/* code $20 */
        {	"tsbb",	REGIM8	},	/* code $21 */
        {	"tad",	REGIM8	},	/* code $22 */
        {	"tsb",	REGIM8	},	/* code $23 */
        {	"tan",	REGIM8	},	/* code $24 */
        {	"tna",	REGIM8	},	/* code $25 */
        {	"tor",	REGIM8	},	/* code $26 */
        {	"txr",	REGIM8	},	/* code $27 */
        {	"tadb",	REGIM8	},	/* code $28 */
        {	"tsbb",	REGIM8	},	/* code $29 */
        {	"tad",	REGIM8	},	/* code $2A */
        {	"tsb",	REGIM8	},	/* code $2B */
        {	"tan",	REGIM8	},	/* code $2C */
        {	"tna",	REGIM8	},	/* code $2D */
        {	"tor",	REGIM8	},	/* code $2E */
        {	"txr",	REGIM8	},	/* code $2F */
        {	"tadbm",	ARYIM6	},	/* code $30 */
        {	"tsbbm",	ARYIM6	},	/* code $31 */
        {	"tadm",	ARYIM6	},	/* code $32 */
        {	"tsbm",	ARYIM6	},	/* code $33 */
        {	"sup",	REG	},	/* code $34 */
        {	"[3580]",	DATA1	},	/* code $35 */
        {	"[3680]",	DATA1	},	/* code $36 */
        {	"[3780]",	DATA1	},	/* code $37 */
        {	"tadbm",	ARYIM6	},	/* code $38 */
        {	"tsbbm",	ARYIM6	},	/* code $39 */
        {	"tadm",	ARYIM6	},	/* code $3A */
        {	"tsbm",	ARYIM6	},	/* code $3B */
        {	"sdn",	REG	},	/* code $3C */
        {	"[3D80]",	DATA1	},	/* code $3D */
        {	"[3E80]",	DATA1	},	/* code $3E */
        {	"[3F80]",	DATA1	},	/* code $3F */
        {	"ldw",	WRIM16	},	/* code $40 */
        {	"ldw",	WRIM16	},	/* code $41 */
        {	"ldw",	WRIM16	},	/* code $42 */
        {	"ldw",	WRIM16	},	/* code $43 */
        {	"ldw",	WRIM16	},	/* code $44 */
        {	"ldw",	WRIM16	},	/* code $45 */
        {	"ldw",	WRIM16	},	/* code $46 */
        {	"ldw",	WRIM16	},	/* code $47 */
        {	"adw",	IRIM8	},	/* code $48 */
        {	"adw",	IRIM8	},	/* code $49 */
        {	"adw",	IRIM8	},	/* code $4A */
        {	"adw",	IRIM8	},	/* code $4B */
        {	"sbw",	IRIM8	},	/* code $4C */
        {	"sbw",	IRIM8	},	/* code $4D */
        {	"sbw",	IRIM8	},	/* code $4E */
        {	"sbw",	IRIM8	},	/* code $4F */
        {	"off",	DROP1	},	/* code $50 */
        {	"gst",	REGKI	},	/* code $51 */
        {	"bit",	REGIM8	},	/* code $52 */
        {	"bit",	XREG	},	/* code $53 */
        {	"ld",	XREG	},	/* code $54 */
        {	"ldm",	XARY	},	/* code $55 */
        {	"ld",	REGIM8	},	/* code $56 */
        {	"ldm",	ARYIM6	},	/* code $57 */
        {	"rtn",	NONE	},	/* code $58 */
        {	"gst",	REGKI	},	/* code $59 */
        {	"bit",	REGIM8	},	/* code $5A */
        {	"bit",	YREG	},	/* code $5B */
        {	"ld",	YREG	},	/* code $5C */
        {	"ldm",	YARY	},	/* code $5D */
        {	"ld",	REGIM8	},	/* code $5E */
        {	"ldm",	ARYIM6	},	/* code $5F */
        {	"cal",	CCIM16	},	/* code $60 */
        {	"cal",	CCIM16	},	/* code $61 */
        {	"cal",	CCIM16	},	/* code $62 */
        {	"cal",	CCIM16	},	/* code $63 */
        {	"cal",	CCIM16	},	/* code $64 */
        {	"cal",	CCIM16	},	/* code $65 */
        {	"cal",	CCIM16	},	/* code $66 */
        {	"cal",	CCIM16	},	/* code $67 */
        {	"cani",	NONE	},	/* code $68 */
        {	"cal",	CCIM16	},	/* code $69 */
        {	"cal",	CCIM16	},	/* code $6A */
        {	"cal",	CCIM16	},	/* code $6B */
        {	"cal",	CCIM16	},	/* code $6C */
        {	"cal",	CCIM16	},	/* code $6D */
        {	"cal",	CCIM16	},	/* code $6E */
        {	"cal",	CCIM16	},	/* code $6F */
        {	"jmp",	CCIM16	},	/* code $70 */
        {	"jmp",	CCIM16	},	/* code $71 */
        {	"jmp",	CCIM16	},	/* code $72 */
        {	"jmp",	CCIM16	},	/* code $73 */
        {	"jmp",	CCIM16	},	/* code $74 */
        {	"jmp",	CCIM16	},	/* code $75 */
        {	"jmp",	CCIM16	},	/* code $76 */
        {	"jmp",	CCIM16	},	/* code $77 */
        {	"rti",	NONE	},	/* code $78 */
        {	"jmp",	CCIM16	},	/* code $79 */
        {	"jmp",	CCIM16	},	/* code $7A */
        {	"jmp",	CCIM16	},	/* code $7B */
        {	"jmp",	CCIM16	},	/* code $7C */
        {	"jmp",	CCIM16	},	/* code $7D */
        {	"jmp",	CCIM16	},	/* code $7E */
        {	"jmp",	CCIM16	},	/* code $7F */
        {	"st",	OFFSREG	},	/* code $80 */
        {	"st",	OFFSREG	},	/* code $81 */
        {	"st",	OFFSREG	},	/* code $82 */
        {	"st",	OFFSREG	},	/* code $83 */
        {	"st",	OFFSREG	},	/* code $84 */
        {	"st",	OFFSREG	},	/* code $85 */
        {	"st",	OFFSREG	},	/* code $86 */
        {	"st",	OFFSREG	},	/* code $87 */
        {	"st",	OFFSREG	},	/* code $88 */
        {	"st",	OFFSREG	},	/* code $89 */
        {	"st",	OFFSREG	},	/* code $8A */
        {	"st",	OFFSREG	},	/* code $8B */
        {	"st",	OFFSREG	},	/* code $8C */
        {	"st",	OFFSREG	},	/* code $8D */
        {	"st",	OFFSREG	},	/* code $8E */
        {	"st",	OFFSREG	},	/* code $8F */
        {	"stm",	OFFSARY	},	/* code $90 */
        {	"stm",	OFFSARY	},	/* code $91 */
        {	"stm",	OFFSARY	},	/* code $92 */
        {	"stm",	OFFSARY	},	/* code $93 */
        {	"stm",	OFFSARY	},	/* code $94 */
        {	"stm",	OFFSARY	},	/* code $95 */
        {	"stm",	OFFSARY	},	/* code $96 */
        {	"stm",	OFFSARY	},	/* code $97 */
        {	"stm",	OFFSARY	},	/* code $98 */
        {	"stm",	OFFSARY	},	/* code $99 */
        {	"stm",	OFFSARY	},	/* code $9A */
        {	"stm",	OFFSARY	},	/* code $9B */
        {	"stm",	OFFSARY	},	/* code $9C */
        {	"stm",	OFFSARY	},	/* code $9D */
        {	"stm",	OFFSARY	},	/* code $9E */
        {	"stm",	OFFSARY	},	/* code $9F */
        {	"ld",	REGOFFS	},	/* code $A0 */
        {	"ld",	REGOFFS	},	/* code $A1 */
        {	"ld",	REGOFFS	},	/* code $A2 */
        {	"ld",	REGOFFS	},	/* code $A3 */
        {	"ld",	REGOFFS	},	/* code $A4 */
        {	"ld",	REGOFFS	},	/* code $A5 */
        {	"ld",	REGOFFS	},	/* code $A6 */
        {	"ld",	REGOFFS	},	/* code $A7 */
        {	"ld",	REGOFFS	},	/* code $A8 */
        {	"ld",	REGOFFS	},	/* code $A9 */
        {	"ld",	REGOFFS	},	/* code $AA */
        {	"ld",	REGOFFS	},	/* code $AB */
        {	"ld",	REGOFFS	},	/* code $AC */
        {	"ld",	REGOFFS	},	/* code $AD */
        {	"ld",	REGOFFS	},	/* code $AE */
        {	"ld",	REGOFFS	},	/* code $AF */
        {	"ldm",	ARYOFFS	},	/* code $B0 */
        {	"ldm",	ARYOFFS	},	/* code $B1 */
        {	"ldm",	ARYOFFS	},	/* code $B2 */
        {	"ldm",	ARYOFFS	},	/* code $B3 */
        {	"ldm",	ARYOFFS	},	/* code $B4 */
        {	"ldm",	ARYOFFS	},	/* code $B5 */
        {	"ldm",	ARYOFFS	},	/* code $B6 */
        {	"ldm",	ARYOFFS	},	/* code $B7 */
        {	"ldm",	ARYOFFS	},	/* code $B8 */
        {	"ldm",	ARYOFFS	},	/* code $B9 */
        {	"ldm",	ARYOFFS	},	/* code $BA */
        {	"ldm",	ARYOFFS	},	/* code $BB */
        {	"ldm",	ARYOFFS	},	/* code $BC */
        {	"ldm",	ARYOFFS	},	/* code $BD */
        {	"ldm",	ARYOFFS	},	/* code $BE */
        {	"ldm",	ARYOFFS	},	/* code $BF */
        {	"pst",	STIM8	},	/* code $C0 */
        {	"pst",	STIM8	},	/* code $C1 */
        {	"pst",	STIM8	},	/* code $C2 */
        {	"pst",	STIM8	},	/* code $C3 */
        {	"ldle",	REGIM8	},	/* code $C4 */
        {	"ldlo",	REGIM8	},	/* code $C5 */
        {	"stle",	REGIM8	},	/* code $C6 */
        {	"stlo",	REGIM8	},	/* code $C7 */
        {	"st",	SMEMOIM8	},	/* code $C8 */
        {	"st",	SMEMOIM8	},	/* code $C9 */
        {	"st",	SMEMOIM8	},	/* code $CA */
        {	"st",	SMEMOIM8	},	/* code $CB */
        {	"ldle",	REGIM8	},	/* code $CC */
        {	"ldlo",	REGIM8	},	/* code $CD */
        {	"stle",	REGIM8	},	/* code $CE */
        {	"stlo",	REGIM8	},	/* code $CF */
        {	"st",	MEMOIM8	},	/* code $D0 */
        {	"st",	MEMOIM8	},	/* code $D1 */
        {	"st",	MEMOIM8	},	/* code $D2 */
        {	"st",	MEMOIM8	},	/* code $D3 */
        {	"ldlem",	ARYIM6	},	/* code $D4 */
        {	"ldlom",	ARYIM6	},	/* code $D5 */
        {	"stlem",	ARYIM6	},	/* code $D6 */
        {	"stlom",	ARYIM6	},	/* code $D7 */
        {	"st",	SMEMOIM8	},	/* code $D8 */
        {	"st",	SMEMOIM8	},	/* code $D9 */
        {	"st",	SMEMOIM8	},	/* code $DA */
        {	"st",	SMEMOIM8	},	/* code $DB */
        {	"ldlem",	ARYIM6	},	/* code $DC */
        {	"ldlom",	ARYIM6	},	/* code $DD */
        {	"stlem",	ARYIM6	},	/* code $DE */
        {	"stlom",	ARYIM6	},	/* code $DF */
        {	"swp",	REG	},	/* code $E0 */
        {	"[E180]",	DATA1	},	/* code $E1 */
        {	"bup",	NONE	},	/* code $E2 */
        {	"byu",	REG	},	/* code $E3 */
        {	"biu",	REG	},	/* code $E4 */
        {	"diu",	REG	},	/* code $E5 */
        {	"mtb",	REG	},	/* code $E6 */
        {	"trp",	DROP1	},	/* code $E7 */
        {	"swp",	REG	},	/* code $E8 */
        {	"[E980]",	DATA1	},	/* code $E9 */
        {	"[EA80]",	YREG	},	/* code $EA */
        {	"byu",	REG	},	/* code $EB */
        {	"biu",	REG	},	/* code $EC */
        {	"diu",	REG	},	/* code $ED */
        {	"mtb",	REG	},	/* code $EE */
        {	"trp",	DROP1	},	/* code $EF */
        {	"bnusm",	ARY	},	/* code $F0 */
        {	"gst",	REGIF	},	/* code $F1 */
        {	"bdn",	NONE	},	/* code $F2 */
        {	"byd",	REG	},	/* code $F3 */
        {	"bium",	ARY	},	/* code $F4 */
        {	"did",	REG	},	/* code $F5 */
        {	"mtbm",	ARYMTBM	},	/* code $F6 */
        {	"trp",	DROP1	},	/* code $F7 */
        {	"bnusm",	ARY	},	/* code $F8 */
        {	"gst",	REGIF	},	/* code $F9 */
        {	"nop",	NONE	},	/* code $FA */
        {	"byd",	REG	},	/* code $FB */
        {	"bium",	ARY	},	/* code $FC */
        {	"did",	REG	},	/* code $FD */
        {	"mtbm",	ARYMTBM	},	/* code $FE */
        {	"trp",	DROP1	}	/* code $FF */
      };

const QString Cdebug_upd1007::ir_tab[4] = { "ix", "iy", "iz", "sp" };

const QString Cdebug_upd1007::wr_tab[8] = {
  "ix", "iy", "iz", "v3", "v2", "v1", "v0", "sp" };

const QString Cdebug_upd1007::cc_tab[16] = {
  "", "k,", "lz,",  "uz,",  "nz,", "v,",  "h,",  "c,",
  "*,", "nk,", "nlz,", "nuz,", "z,",  "nv,", "nh,", "nc," };

const QString Cdebug_upd1007::st_tab[8] = {
  "ko", "if", "as", "ie",		/* immediate value */
  "ko", "f", "as", "ie" };	/* register */

Cdebug_upd1007::Cdebug_upd1007(CPObject *parent)	: Cdebug(parent)
{

}

QString Cdebug_upd1007::IntToStr(BYTE x) {
    return QString("%1").arg(x);
}

QString Cdebug_upd1007::IntToHex(BYTE x,BYTE size) {
    return QString("%1").arg(x,size,16,QChar('0'));
}

QString Cdebug_upd1007::CaHexB(BYTE x) {
    return QString("&H%1").arg(x,2,16,QChar('0'));
}


BYTE Cdebug_upd1007::Reg1 (BYTE x) {
    return ((x >> 1) & 0x38) | (x & 0x07);
}


BYTE Cdebug_upd1007::Reg2 (BYTE y){
    return ((y >> 5) & 0x07) | ((y << 3) & 0x38);
}


BYTE Cdebug_upd1007::Rl1 (BYTE x, BYTE y) {
    return ((x >> 1) & 0x38) | ((y >> 5) & 0x07);
}


BYTE Cdebug_upd1007::Rf2 (BYTE x, BYTE y){
    return (x & 0x07) | ((y << 3) & 0x38);
}


BYTE Cdebug_upd1007::Im6 (BYTE x, BYTE y) {
    return (y & 0x1F) | ((! x << 2) & 0x20);
}


/* returns the mnemonic */
QString Cdebug_upd1007::Mnemonic (UINT16 kod)
{
    return mnem[kod].str;
}


/* returns the arguments */
QString Cdebug_upd1007::Arguments (UINT16 kod) {
#if 1
    BYTE x,y,z;

QString sign, name;
QString Result;
sign =  (kod & 0x04) ? "-" : "+";
z = (! kod << 3) & 0x40;

switch (mnem[kod].kind) {

  case NONE:
    Result = "";

  case DROP1:
    {
      pupd1007->FetchByte();
      Result = "";
    }

  case CCIM16:
    {
      x = pupd1007->FetchByte();
      y = pupd1007->FetchByte();
      Result = cc_tab[kod & 0x0F] + CaHexB(x) + IntToHex(y,2);
    }

  case WRIM16:
    {
      x = pupd1007->FetchByte();
      y = pupd1007->FetchByte();
      Result = wr_tab[kod & 0x07] + "," + CaHexB(x) + IntToHex(y,2);
    }

  case IRIM8:
    Result = ir_tab[kod & 0x03] + "," + CaHexB(pupd1007->FetchByte());

  case SMEMOIM8:
    {
      if ((kod & 0x10) == 0) sign = "+"; else sign = "-";
      Result = sign + "(" + ir_tab[kod & 0x03] + ")," + CaHexB(pupd1007->FetchByte());
    }

  case MEMOIM8:
    Result = "(" + ir_tab[kod & 0x03] + ")," + CaHexB(pupd1007->FetchByte());

  case XREG:
    {
      x = pupd1007->FetchByte();
      y = pupd1007->FetchByte();
      name = (y & 0x10) ? "i" : "r";
      Result = name + IntToStr(Reg2(y) | 0x40) + ",r" + IntToStr(Reg1(x));
    }

  case YREG:
    {
      x = pupd1007->FetchByte();
      y = pupd1007->FetchByte();
      if ((y & 0x10) != 0)  name = "i"; else name = "r";
      Result = "r" + IntToStr(Reg1(x)) + "," +
      name + IntToStr(Reg2(y) | 0x40);
    }

  case REGIM8:
    {
      x = pupd1007->FetchByte();
      y = pupd1007->FetchByte();
      Result = "r" + IntToStr(Reg1(x) | z) + "," + CaHexB(y);
    }

  case XARY:
    {
      x = pupd1007->FetchByte();
      y = pupd1007->FetchByte();
      if ((y & 0x10) != 0)  name = "i"; else name = "r";
      Result= name + IntToStr(Rf2(x,y) | 0x40) + ".." +
              name + IntToStr(Reg2(y) | 0x40) +
              ",r" + IntToStr(Reg1(x)) + "..r" + IntToStr(Rl1(x,y));
    }

  case YARY:
    {
      x = pupd1007->FetchByte();
      y = pupd1007->FetchByte();
      if ((y & 0x10) != 0)  name = "i"; else name = "r";
      Result= "r" + IntToStr(Reg1(x)) + "..r" + IntToStr(Rl1(x,y)) +
              "," + name + IntToStr(Rf2(x,y) | 0x40) +
              ".." + name + IntToStr(Reg2(y) | 0x40);
    }

  case ARYIM6:
    {
      x = pupd1007->FetchByte();
      y = pupd1007->FetchByte();
      Result= "r" + IntToStr(Reg1(x) | z) +
              "..r" + IntToStr(Rl1(x,y) | z) + "," + CaHexB(Im6(x,y));
    }

  case MEMOREG:
    Result = sign + "(" + ir_tab[kod & 0x03] + "),r" +
      IntToStr(Reg1(pupd1007->FetchByte()) | z);

  case REG:
    Result = "r" + IntToStr(Reg1(pupd1007->FetchByte()) | z);

  case OFFSREG:
    {
      x = pupd1007->FetchByte();
      y = pupd1007->FetchByte();
      if ((y & 0x10) != 0) name = "i"; else name = "r";
      Result = "(" + ir_tab[kod & 0x03] + sign;
      if ((kod & 0x08) == 0)
        Result = Result + "r" + IntToStr(Reg1(x)) + ")," +
      name + IntToStr(Reg2(y) | 0x40);
      else
        Result = Result + CaHexB(y) + "),r" + IntToStr(Reg1(x));
    }

  case REGMEMO:
    {
      x = pupd1007->FetchByte();
      Result = "r" + IntToStr(Reg1(x) | z) +
      ",(" + ir_tab[kod & 0x03] + ")" + sign;
    }

  case REGOFFS:
    {
      x = pupd1007->FetchByte();
      y = pupd1007->FetchByte();
      if ((y & 0x10) != 0) name = "i"; else name = "r";
      if ((kod & 0x08) == 0)
        Result = name + IntToStr(Reg2(y) | 0x40) + ",(" +
      ir_tab[kod & 0x03] + sign + "r" + IntToStr(Reg1(x)) + ")";
      else
        Result = "r" + IntToStr(Reg1(x)) + ",(" +
      ir_tab[kod & 0x03] + sign + CaHexB(y) + ")";
    }

  case MEMOARY:
    {
      x = pupd1007->FetchByte();
      y = pupd1007->FetchByte();
      Result = sign + "(" + ir_tab[kod & 0x03] + "),r" +
      IntToStr(Reg1(x) | z) + "..r" + IntToStr(Rl1(x,y) | z);
    }

  case OFFSARY:
    {
      x = pupd1007->FetchByte();
      y = pupd1007->FetchByte();
//{ indirect is illegal }
      if ((y & 0x10) != 0) name = "?"; else name = "r";
      Result = "(" + ir_tab[kod & 0x03] + sign;
      if ((kod & 0x08) == 0)
        Result = Result + "r" + IntToStr(Reg1(x)) + ")," +
                  name + IntToStr(Rf2(x,y) | 0x40) + ".." +
                  name + IntToStr(Reg2(y) | 0x40);
      else
        Result = Result + CaHexB(Im6(x,y)) + "),r" +
                  IntToStr(Reg1(x)) + "..r" + IntToStr(Rl1(x,y));
    }

  case ARY:
    {
      x = pupd1007->FetchByte();
      y = pupd1007->FetchByte();
      if ((y & 0x10) != 0)  name = "i"; else name = "r";
      if ((kod & 0x08) == 0)
        Result = name + IntToStr(Rf2(x,y) | 0x40) + ".." +
                name + IntToStr(Reg2(y) | 0x40);
      else
        Result = "r" + IntToStr(Reg1(x)) + "..r" + IntToStr(Rl1(x,y));
    }

  case ARYMTBM:
    {
      x = pupd1007->FetchByte();
      y = pupd1007->FetchByte();
      Result = "r" + IntToStr(Reg1(x) | z) +
      "..r" + IntToStr(Rl1(x,y) | z);
    }

  case ARYMEMO:
    {
      x = pupd1007->FetchByte();
      y = pupd1007->FetchByte();
      Result = "r" + IntToStr(Reg1(x) | z) +
      "..r" + IntToStr(Rl1(x,y) | z) +
      ",(" + ir_tab[kod & 0x03] + ")" + sign;
    }

  case ARYOFFS:
    {
      x = pupd1007->FetchByte();
      y = pupd1007->FetchByte();
//{ indirect is illegal }
      if ((y & 0x10) != 0)  name = "?"; else name = "r";
      if ((kod & 0x08) == 0)
        Result = name + IntToStr(Rf2(x,y) | 0x40) + ".." +
                  name + IntToStr(Reg2(y) | 0x40) + ",(" + ir_tab[kod & 0x03] +
                  sign + "r" + IntToStr(Reg1(x)) + ")";
      else
        Result = "r" + IntToStr(Reg1(x)) + "..r" + IntToStr(Rl1(x,y)) +
      ",(" + ir_tab[kod & 0x03] + sign + CaHexB(Im6(x,y)) + ")";
    }

  case STREG:
    Result = st_tab[kod & 0x07] + ",r" + IntToStr(Reg1(pupd1007->FetchByte()) | z);

  case STIM8:
    Result = st_tab[kod & 0x07] + "," + CaHexB(pupd1007->FetchByte());

  case REGST:
    Result = "r" + IntToStr(Reg1(pupd1007->FetchByte()) | z) + "," +
      st_tab[kod & 0x07];

  case REGIF:
    Result = "r" + IntToStr(Reg1(pupd1007->FetchByte()) | z) + ",if";

  case REGKI:
    Result = "r" + IntToStr(Reg1(pupd1007->FetchByte()) | z) + ",ki";

  case CCINDIR:
    {
      Result = cc_tab[kod & 0x0F];
      x = pupd1007->FetchByte();
      y = pupd1007->FetchByte();
      if ((y & 0x10) != 0) name = "i"; else name = "r";
      if ((Reg1(x) == Reg2(y)) & ((y & 0x1F) == 0x07) & (name == "r"))
        Result = Result + wr_tab[x & 0x07];
      else
        Result = Result + name + IntToStr(Reg2(y) | 0x40) +
      ",r" + IntToStr(Reg1(x));
    }

  case DATA1:
    Result = "[" + IntToHex(pupd1007->FetchByte(),2) + "]";

default:
  Result = "";

}
#endif

return Result;
}


UINT32 Cdebug_upd1007::DisAsm_1(UINT32 adr)
{
    pupd1007 = (CUPD1007 *)(pPC->pCPU);
    DasmAdr = adr;

    const upd1007_dasm *inst;
    UINT32 dasmflags = 0;
    UINT8 op, op1;

    int pos =adr;//(adr>0x0C00);
    if (adr<0x0C00) pos<<=1;

    char *buffer = (char *)malloc(200);
    char *startbuffer = buffer;

    buffer[0] = '\0';

    pupd1007->info.savepc = pupd1007->info.pc;
    pupd1007->info.pc = adr;

    sprintf(Buffer,"%s",QString(IntToHex(pupd1007->info.pc, 4) + ":").toLatin1().data());
    UINT16 index = pupd1007->Fetchopcode();
    sprintf(Buffer,"%s%s",Buffer,Mnemonic (index).toLatin1().data());
    sprintf(Buffer,"%s%s",Buffer,Arguments (index).toLatin1().data());

    NextDasmAdr = pupd1007->info.pc;
    pupd1007->info.pc = pupd1007->info.savepc;
    return NextDasmAdr;


    debugged = true;

    return(NextDasmAdr);

}



