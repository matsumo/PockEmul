// *********************************************************************
//    Copyright (c) 2001 Zharkoi Oleg
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
// *********************************************************************

// *********************************************************************
// Chp41Trace.cpp : implementation file
// *********************************************************************
#if 1
#include "hp41.h"
#include "hp41Cpu.h"

// Trace
#define Opcodes 227
#define Globals 792
#define NameSize 14
struct Opc_type           //Instructions names
  {
  char name[9];
  };
struct Glob_type          // Global routines names
  {
  char name[NameSize];	  //name
  word addr;              //address
  };
struct TEF_type
  {
  char name[4];
  };
struct TEF_type TEFs[4][8]=
  {
    {"PT", "X",  "WPT","W",  "PQ", "XS","M","S" },
    {"PT", "X",  "WPT","ALL","PQ", "XS","M","S" },
    {"@R", "S&X","R<", "ALL","P-Q","XS","M","MS"},
    {"@PT","S&X","PT<","ALL","PQ", "XS","M","MS"}
  };

struct Opc_type Opcode[3][Opcodes]=
{
  {
    "",
    "ST=0",
    "ST=1",
    "ST=1?",
    "LC",
    "?PT=",
    "",
    "PT=",
    "",
    "SELPF",
    "REGN=C",
    "FLG=1?",
    "",
    "",
    "C=REGN",
    "RCR",
    "NOP",
    "WMLDL",
    "#080",
    "#0C0",
    "ENROM1",
    "ENROM3",
    "ENROM2",
    "ENROM4",
    "#018",
    "G=C",
    "C=G",
    "CGEX",
    "#118",
    "M=C",
    "C=M",
    "CMEX",
    "#218",
    "F=SB",
    "SB=F",
    "FEXSB",
    "#318",
    "ST=C",
    "C=ST",
    "CSTEX",
    "SPOPND",
    "POWOFF",
    "SELP",
    "SELQ",
    "?P=Q",
    "?LLD",
    "CLRABC",
    "GOTOC",
    "C=KEYS",
    "SETHEX",
    "SETDEC",
    "DISOFF",
    "DISTOG",
    "RTNC",
    "RTNNC",
    "RTN",
    "SRLDA",
    "SRLDB",
    "SRLDC",
    "SRLDAB",
    "SRLABC",
    "SLLDAB",
    "SLLABC",
    "SRSDA",
    "SRSDB",
    "SRSDC",
    "SLSDA",
    "SLSDB",
    "SRSDAB",
    "SLSDAB",
    "SRSABC",
    "SLSABC",
    "?F3=1",
    "?F4=1",
    "?F5=1",
    "?F10=1",
    "?F8=1",
    "?F6=1",
    "?F11=1",
    "#1EC",
    "?F2=1",
    "?F9=1",
    "?F7=1",
    "?F13=1",
    "?F1=1",
    "?F12=1",
    "?F0=1",
    "#3EC",
    "ROMBLK",
    "N=C",
    "C=N",
    "CNEX",
    "LDI",
    "STK=C",
    "C=STK",
    "WPTOG",
    "GOKEYS",
    "DADD=C",
    "#2B0",
    "DATA=C",
    "CXISA",
    "C=CORA",
    "C=C&A",
    "PFAD=C",
    "FLLDA",
    "FLLDB",
    "FLLDC",
    "FLLDAB",
    "FLLABC",
    "READEN",
    "FLSDC",
    "FRSDA",
    "FRSDB",
    "FRSDC",
    "FLSDA",
    "FLSDB",
    "FRSDAB",
    "FLSDAB",
    "FRSABC",
    "FLSABC",
    "WRTEN",
    "HPIL=C",
    "CLRST",
    "RSTKB",
    "CHKKB",
    "DECPT",
    "INCPT",
    "C=DATA",
    "GSUBNC",
    "GSUBC",
    "GOLNC",
    "GOLC",
    "GSB41C",
    "GOL41C",
    "WRTIME",
    "WDTIME",
    "WRALM",
    "WRSTS",
    "WRSCR",
    "WSINT",
    "#1A8",
    "STPINT",
    "DSWKUP",
    "ENWKUP",
    "DSALM",
    "ENALM",
    "STOPC",
    "STARTC",
    "PT=B",
    "PT=A",
    "RDTIME",
    "RCTIME",
    "RDALM",
    "RDSTS",
    "RDSCR",
    "RDINT",
    "ENWRIT",
    "STWRIT",
    "ENREAD",
    "STREAD",
    "#128",
    "CRDWPF",
    "#1A8",
    "CRDOHF",
    "#228",
    "CRDINF",
    "#2A8",
    "TSTBUF",
    "TRPCRD",
    "TCLCRD",
    "#3A8",
    "CRDFLG",
    "A=0",
    "B=0",
    "C=0",
    "ABEX",
    "B=A",
    "ACEX",
    "C=B",
    "BCEX",
    "A=C",
    "A=A+B",
    "A=A+C",
    "A=A+1",
    "A=A-B",
    "A=A-1",
    "A=A-C",
    "C=C+C",
    "C=A+C",
    "C=C+1",
    "C=A-C",
    "C=C-1",
    "C=-C",
    "C=-C-1",
    "?B#0",
    "?C#0",
    "?A<C",
    "?A<B",
    "?A#0",
    "?A#C",
    "ASR",
    "BSR",
    "CSR",
    "ASL",
    "GONC",
    "GOC",
    "DEFP4K",
    "DEFR4K",
    "DEFR8K",
    "U4KDEF",
    "U8KDEF",
    "A=B",
    "B=C",
    "C=A",
    "LC3",
    "CON",
    "FCNS",
    "XROM",
    "undef",
    "undef",
    "undef",
    "BUSY?",
    "ERROR?",
    "POWON?",
    "PRINT",
    "STATUS",
    "RTNCPU"
  },
  {
    "",
    "CF",
    "SF",
    "?FS",
    "LC",
    "?PT=",
    "",
    "PT=",
    "",
    "PERTCT",
    "REG=C",
    "?PF",
    "",
    "",
    "C=REG",
    "RCR",
    "NOP",
    "WMLDL",
    "#080",
    "#0C0",
    "ENBANK1",
    "ENBANK3",
    "ENBANK2",
    "ENBANK4",
    "#018",
    "G=C",
    "C=G",
    "C<>G",
    "#118",
    "M=C",
    "C=M",
    "C<>M",
    "#218",
    "F=ST",
    "ST=F",
    "ST<>F",
    "#318",
    "ST=C",
    "C=ST",
    "C<>ST",
    "CLRRTN",
    "POWOFF",
    "PT=P",
    "PT=Q",
    "?P=Q",
    "?BAT",
    "ABC=0",
    "GTOC",
    "C=KEY",
    "SETHEX",
    "SETDEC",
    "DISOFF",
    "DISTOG",
    "CRTN",
    "NCRTN",
    "RTN",
    "WRA12L",
    "WRB12L",
    "WRC12L",
    "WRAB6L",
    "WRABC4L",
    "WRAB6R",
    "WRABC4R",
    "WRA1L",
    "WRB1L",
    "WRC1L",
    "WRA1R",
    "WRB1R",
    "WRAB1L",
    "WRAB1R",
    "WRABC1L",
    "WRABC1R",
    "?PF 3",
    "?PF 4",
    "?EDAV",
    "?ORAV",
    "?FRAV",
    "?IFCR",
    "?TFAIL",
    "#1EC",
    "?WNDB",
    "?FRNS",
    "?SRQR",
    "?SERV",
    "?CRDR",
    "?ALM",
    "?PBSY",
    "#3EC",
    "ROMBLK",
    "N=C",
    "C=N",
    "C<>N",
    "LDI",
    "STK=C",
    "C=STK",
    "WPTOG",
    "GTOKEY",
    "RAMSLCT",
    "#2B0",
    "WDATA",
    "RDROM",
    "C=CORA",
    "C=CANDA",
    "PERSLCT",
    "RDA12L",
    "RDB12L",
    "RDC12L",
    "RDAB6L",
    "RDABC4L",
    "READAN",
    "RDC1L",
    "RDA1R",
    "RDB1R",
    "RDC1R",
    "RDA1L",
    "RDB1L",
    "RDAB1R",
    "RDAB1L",
    "RDABC1R",
    "RDABC1L",
    "WRITAN",
    "HPIL=C",
    "ST=0",
    "CLRKEY",
    "?KEY",
    "-PT",
    "+PT",
    "RDATA",
    "NCXQ",
    "CXQ",
    "NCGO",
    "CGO",
    "NCXQREL",
    "NCGOREL",
    "WTIME",
    "WTIME-",
    "WALM",
    "WSTS",
    "WSCR",
    "WINTST",
    "#1A8",
    "STPINT",
    "WKUPOFF",
    "WKUPON",
    "ALMOFF",
    "ALMON",
    "STOPC",
    "STARTC",
    "TIMER=B",
    "TIMER=A",
    "RTIME",
    "RTIMEST",
    "RALM",
    "RSTS",
    "RSCR",
    "RINT",
    "ENDWRIT",
    "STWRIT",
    "ENDREAD",
    "STREAD",
    "#128",
    "CRDWPF",
    "#1A8",
    "CRDOHF",
    "#228",
    "CRDINF",
    "#2A8",
    "TSTBUF",
    "SETCTF",
    "TCLCTF",
    "#3A8",
    "CRDEXF",
    "A=0",
    "B=0",
    "C=0",
    "A<>B",
    "B=A",
    "A<>C",
    "C=B",
    "B<>C",
    "A=C",
    "A=A+B",
    "A=A+C",
    "A=A+1",
    "A=A-B",
    "A=A-1",
    "A=A-C",
    "C=C+C",
    "C=A+C",
    "C=C+1",
    "C=A-C",
    "C=C-1",
    "C=-C",
    "C=-C-1",
    "?B#0",
    "?C#0",
    "?A<C",
    "?A<B",
    "?A#0",
    "?A#C",
    "ASR",
    "BSR",
    "CSR",
    "ASL",
    "JNC",
    "JC",
    "DEFP4K",
    "DEFR4K",
    "DEFR8K",
    "U4KDEF",
    "U8KDEF",
    "A=B",
    "B=C",
    "C=A",
    "LC3",
    "CON",
    "FCNS",
    "XROM",
    "undef",
    "undef",
    "undef",
    "BUSY?",
    "ERROR?",
    "POWON?",
    "PRINT",
    "STATUS",
    "RTNCPU"
  },
  {
    "",
    "CLRF",
    "SETF",
    "?FSET",
    "LD@R",
    "?R=",
    "",
    "R=",
    "",
    "SELPF",
    "WRIT",
    "?FI=",
    "",
    "",
    "READ",
    "RCR",
    "NOP",
    "WROM",
    "#080",
    "#0C0",
    "ENROM1",
    "ENROM3",
    "ENROM2",
    "ENROM4",
    "#018",
    "G=C",
    "C=G",
    "C<>G",
    "#118",
    "M=C",
    "C=M",
    "C<>M",
    "#218",
    "T=ST",
    "ST=T",
    "ST<>T",
    "#318",
    "ST=C",
    "C=ST",
    "C<>ST",
    "XQ>GO",
    "POWOFF",
    "SLCTP",
    "SLCTQ",
    "?P=Q",
    "?LOWBAT",
    "A=B=C=0",
    "GOTOADR",
    "C=KEY",
    "SETHEX",
    "SETDEC",
    "DSPOFF",
    "DSPTOG",
    "?CRTN",
    "?NCRTN",
    "RTN",
    "SRLDA",
    "SRLDB",
    "SRLDC",
    "SRLDAB",
    "SRLABC",
    "SLLDAB",
    "SLLABC",
    "SRSDA",
    "SRSDB",
    "SRSDC",
    "SLSDA",
    "SLSDB",
    "SRSDAB",
    "SLSDAB",
    "SRSABC",
    "SLSABC",
    "?FI= 3",
    "?FI= 4",
    "?FI= 5",
    "?FI= 10",
    "?FI= 8",
    "?FI= 6",
    "?FI= 11",
    "#1EC",
    "?FI= 2",
    "?FI= 9",
    "?FI= 7",
    "?FI= 13",
    "?FI= 1",
    "?FI= 12",
    "?FI= 0",
    "#3EC",
    "ROMBLK",
    "N=C",
    "C=N",
    "C<>N",
    "LDIS&X",
    "PUSHADR",
    "POPADR",
    "WPTOG",
    "GTOKEY",
    "RAMSLCT",
    "#2B0",
    "WRITDATA",
    "FETCHS&X",
    "C=CORA",
    "C=CANDA",
    "PRPHSLCT",
    "FLLDA",
    "FLLDB",
    "FLLDC",
    "FLLDAB",
    "FLLABC",
    "READEN",
    "FLSDC",
    "FRSDA",
    "FRSDB",
    "FRSDC",
    "FLSDA",
    "FLSDB",
    "FRSDAB",
    "FLSDAB",
    "FRSABC",
    "FLSABC",
    "WRTEN",
    "HPIL=C",
    "ST=0",
    "CLRKEY",
    "?KEY",
    "R=R-1",
    "R=R+1",
    "READDATA",
    "?NCXQ",
    "?CXQ",
    "?NCGO",
    "?CGO",
    "?NCXQREL",
    "?NCGOREL",
    "WRTIME",
    "WDTIME",
    "WRALM",
    "WRSTS",
    "WRSCR",
    "WSINT",
    "#1A8",
    "STPINT",
    "DSWKUP",
    "ENWKUP",
    "DSALM",
    "ENALM",
    "STOPC",
    "STARTC",
    "PT=B",
    "PT=A",
    "RDTIME",
    "RCTIME",
    "RDALM",
    "RDSTS",
    "RDSCR",
    "RDINT",
    "ENWRIT",
    "STWRIT",
    "ENREAD",
    "STREAD",
    "#128",
    "CRDWPF",
    "#1A8",
    "CRDOHF",
    "#228",
    "CRDINF",
    "#2A8",
    "TSTBUF",
    "TRPCRD",
    "TCLCRD",
    "#3A8",
    "CRDFLG",
    "A=0",
    "B=0",
    "C=0",
    "A<>B",
    "B=A",
    "A<>C",
    "C=B",
    "B<>C",
    "A=C",
    "A=A+B",
    "A=A+C",
    "A=A+1",
    "A=A-B",
    "A=A-1",
    "A=A-C",
    "C=C+C",
    "C=C+A",
    "C=C+1",
    "C=A-C",
    "C=C-1",
    "C=0-C",
    "C=-C-1",
    "?B#0",
    "?C#0",
    "?A<C",
    "?A<B",
    "?A#0",
    "?A#C",
    "RSHFA",
    "RSHFB",
    "RSHFC",
    "LSHFA",
    "JNC",
    "JC",
    "DEFP4K",
    "DEFR4K",
    "DEFR8K",
    "U4KDEF",
    "U8KDEF",
    "A=B",
    "B=C",
    "C=A",
    "LD@R3",
    "CON",
    "FCNS",
    "XROM",
    "undef",
    "undef",
    "undef",
    "BUSY?",
    "ERROR?",
    "POWON?",
    "PRINT",
    "STATUS",
    "RTNCPU"
  }
};

struct Glob_type GlobalName[Globals]=
{
  "[ADATE]"  ,0X5B6E,
  "[ALMCAT]" ,0X5806,
  "[ALMNOW]" ,0X5F38,
  "[ATIME]"  ,0X5B67,
  "[ATIME24]",0X5B5F,
  "[CLK12]"  ,0X52D7,
  "[CLK24]"  ,0X52E2,
  "[CLKT]"   ,0X52EA,
  "[CLKTD]"  ,0X52F3,
  "[CLOCK]"  ,0X5634,
  "[CORRECT]",0X5407,
  "[DATE]"   ,0X5100,
  "[DATE+]"  ,0X52C1,
  "[DDAYS]"  ,0X52C8,
  "[DMY]"    ,0X5179,
  "[DOW]"    ,0X5089,
  "[MDY]"    ,0X516E,
  "[RCLAF]"  ,0X5122,
  "[RCLSW]"  ,0X5117,
  "[RUNSW]"  ,0X5135,
  "[SETAF]"  ,0X5B01,
  "[SETDATE]",0X5423,
  "[SETIME]" ,0X540F,
  "[SETSW]"  ,0X5B44,
  "[STOPSW]" ,0X5CE5,
  "[SW]"     ,0X5560,
  "[T+X]"    ,0X59FB,
  "[TIME]"   ,0X50EE,
  "[XYZALM]" ,0X5941,
  "[CLALMA]" ,0X5069,
  "[CLALMX]" ,0X52D0,
  "[CLRALMS]",0X507A,
  "[RCLALM]" ,0X5071,
  "[SWPT]"   ,0X5061,
  "[SEL_T_A]",0X50E2,
  "[TSTMAP]" ,0X14A1,
  "[XCAT]"   ,0X0B80,
  "[ASCLCD]" ,0X2C5D,
  "[ASCLCA]" ,0X2C5E,
  "[ADRFCH]" ,0X0004,
  "[GOTINT]" ,0X02F8,
  "[INTINT]" ,0X02FB,
  "[R_P]"    ,0X11C0,
  "[XCLSIG]" ,0X14B0,
  "[XDEG]"   ,0X171C,
  "[OFF]"    ,0X11C8,
  "[XGRAD]"  ,0X1726,
  "[DEROVF]" ,0X08EB,
  "[XMSGPR]" ,0X056D,
  "[XPACK]"  ,0X2000,
  "[FSTIN]"  ,0X14C2,
  "[LNC10_]" ,0X1AAD,
  "[LNC10]"  ,0X1AAE,
  "[GTAINC]" ,0X0304,
  "[NAMEA]"  ,0X0ED9,
  "[PR3RT]"  ,0X0EDD,
  "[RG9LCD]" ,0X08EF,
  "[BRT160]" ,0X1DA8,
  "[RTJLBL]" ,0X14C9,
  "[ONE_BY_X]",0X11D6,
  "[XRDN]"   ,0X14BD,
  "[BRT290]" ,0X1DAC,
  "[NFRST_PLUS]",0X0BEE,
  "[XROMNF]" ,0X2F6C,
  "[NAME20]" ,0X0EE6,
  "[GENLNK]" ,0X239A,
  "[SEPXY]"  ,0X14D2,
  "[LNC20]"  ,0X1ABD,
  "[CHK_NO_S1]",0X14D4,
  "[NAME21]" ,0X0EE9,
  "[P_R]"    ,0X11DC,
  "[XSIZE]",0X1795,
  "[CHK_NO_S]",0X14D8,
  "[CHK_NO_S2]",0X14D9,
  "[NAME33]",0X0EEF,
  "[OUTLCD]",0X2C80,
  "[PACK]",0X11E7,
  "[PAKEND]",0X20AC,
  "[XNNROW]",0X0026,
  "[CLRSB2]",0X0C00,
  "[MASK]",0X2C88,
  "[CLRSB3]",0X0C02,
  "[ERRAD]",0X14E2,
  "[PCTCH]",0X11EC,
  "[PARSE]",0X0C05,
  "[XROLLUP]",0X14E5,
  "[TBITMA]",0X2F7F,
  "[TBITMP]",0X2F81,
  "[XSTYON]",0X1411,
  "[INSSUB]",0X23B2,
  "[RDNSUB]",0X14E9,
  "[XX_EQ_Y]",0X1614,
  "[XX_LE_0]",0X160D,
  "[R_SUB]",0X14ED,
  "[XX_NE_0]",0X1611,
  "[X_EQ_0]",0X130E,
  "[X_EQ_Y]",0X1314,
  "[AFORMT]",0X0628,
  "[PSE]",0X11FC,
  "[NAME37]",0X0F09,
  "[X_GT_Y]",0X1320,
  "[X_LT_0]",0X12E8,
  "[X_TO_2]",0X106B,
  "[GTO_5]",0X29AA,
  "[X_XCHNG]",0X124C,
  "[X_XCHNG_Y]",0X12FC,
  "[LOAD3]",0X14FA,
  "[Y_MINUS_X]",0X1421,
  "[Y_TO_X]",0X102A,
  "[GTBYT]",0X29B0,
  "[PROMPT]",0X1209,
  "[GTAI40]",0X0341,
  "[GSUBS1]",0X23C9,
  "[GTBYTO]",0X29B2,
  "[BIGBRC]",0X004F,
  "[R_D]",0X120E,
  "[NXBYT3]",0X29B7,
  "[SETSST]",0X17F9,
  "[NXBYTA]",0X29B9,
  "[ASRCH]",0X26C5,
  "[GTBYTA]",0X29BB,
  "[GOSUB0]",0X23D2,
  "[STOP]",0X1215,
  "[ADDONE]",0X1800,
  "[GOL0]",0X23D0,
  "[BRT140]",0X1DEC,
  "[RUN_STOP]",0X1218,
  "[SRBMAP]",0X2FA5,
  "[CALDSP]",0X29C3,
  "[GOL1]",0X23D9,
  "[AD2_10]",0X1807,
  "[PARS05]",0X0C34,
  "[AD1_10]",0X1809,
  "[LN1_PLUS_X]",0X1220,
  "[DECAD]",0X29C7,
  "[AD2_13]",0X180C,
  "[DECADA]",0X29CA,
  "[GOL2]",0X23E2,
  "[GOSUB1]",0X23DB,
  "[GOSUB2]",0X23E4,
  "[INCAD]",0X29CF,
  "[LASTX]",0X1228,
  "[INCADP]",0X29D1,
  "[GTFEN1]",0X20EB,
  "[INCAD2]",0X29D3,
  "[GTFEND]",0X20E8,
  "[GOL3]",0X23EB,
  "[INCADA]",0X29D6,
  "[GOSUB3]",0X23ED,
  "[NAM40]",0X0F34,
  "[ENCP00]",0X0952,
  "[XROW1]",0X0074,
  "[PAKSPC]",0X20F2,
  "[TXTLB1]",0X2FC6,
  "[PGMAON]",0X0956,
  "[RCL]",0X122E,
  "[RFDS55]",0X0949,
  "[XVIEW]",0X036F,
  "[SUBONE]",0X1802,
  "[CHS]",0X123A,
  "[BRT200]",0X1E0F,
  "[GSB000]",0X23FA,
  "[INBYTP]",0X29E5,
  "[INBYT]",0X29E6,
  "[SCROLL]",0X2CDC,
  "[ROMH05]",0X066C,
  "[SCROL0]",0X2CDE,
  "[INBYT1]",0X29EA,
  "[GSB256]",0X23FA,
  "[GSB512]",0X23FA,
  "[MSGDLY]",0X037C,
  "[GSB768]",0X23FA,
  "[STMSGF]",0X037E,
  "[INBYT0]",0X29E3,
  "[INBYTC]",0X29E4,
  "[NOREG9]",0X095E,
  "[PKIOAS]",0X2114,
  "[INSLIN]",0X29F4,
  "[RSTSEQ]",0X0384,
  "[INLIN2]",0X29F6,
  "[CPGMHD]",0X067B,
  "[XRTN]",0X2703,
  "[PI]",0X1242,
  "[RDN]",0X1252,
  "[CLLCDE]",0X2CF0,
  "[CPGM10]",0X067F,
  "[PR10RT]",0X0372,
  "[RSTKB]",0X0098,
  "[RND]",0X1257,
  "[ROMH35]",0X0678,
  "[CLRLCD]",0X2CF6,
  "[DATOFF]",0X0390,
  "[RSTMSC]",0X0392,
  "[OUTROM]",0X2FEE,
  "[ROMHED]",0X066A,
  "[KEYOP]",0X068A,
  "[RST05]",0X009B,
  "[ERROF]",0X00A2,
  "[ROLLUP]",0X1260,
  "[RSTMS0]",0X038E,
  "[MP2_10]",0X184D,
  "[TRC30]",0X1E38,
  "[MP1_10]",0X184F,
  "[LN10]",0X1B45,
  "[KYOPCK]",0X0693,
  "[MP2_13]",0X1852,
  "[NBYTAB]",0X2D06,
  "[NXTBYT]",0X2D07,
  "[NBYTA0]",0X2D04,
  "[NFRNC]",0X00A5,
  "[SIGMA_PLUS]",0X126D,
  "[NXBYTO]",0X2D0B,
  "[INSHRT]",0X2A17,
  "[STOPS]",0X03A7,
  "[APPEND]",0X2D0E,
  "[NAM44_]",0X0F7D,
  "[NM44_5]",0X0F7E,
  "[RAK06]",0X0C7F,
  "[PSESTP]",0X03AC,
  "[SEARC1]",0X2434,
  "[ALPDEF]",0X03AE,
  "[APNDNW]",0X2D14,
  "[ERRIGN]",0X00BB,
  "[PARSDE]",0X0C90,
  "[MPY150]",0X1865,
  "[COS]",0X127C,
  "[PARS56]",0X0C93,
  "[RSTMS1]",0X0390,
  "[END2]",0X03B6,
  "[NFRSIG]",0X00C2,
  "[SNR10]",0X243F,
  "[NFRENT]",0X00C4,
  "[SHF10]",0X186D,
  "[BAKDE]",0X09A5,
  "[NFRKB]",0X00C7,
  "[NRM10]",0X1870,
  "[END3]",0X03BE,
  "[NRM12]",0X1872,
  "[NFRKB1]",0X00C6,
  "[NFRX]",0X00CC,
  "[NRM11]",0X1871,
  "[CLRREG]",0X2155,
  "[RSTSQ]",0X0385,
  "[RTN30]",0X272F,
  "[DATENT]",0X2D2C,
  "[ROW12]",0X2743,
  "[RTN]",0X125C,
  "[CLCTMG]",0X03C9,
  "[MOVREG]",0X215C,
  "[XLN1_PLUS_X]",0X1B73,
  "[PUTREG]",0X215E,
  "[SCI]",0X1265,
  "[SEARCH]",0X2433,
  "[NFRXY]",0X00DA,
  "[NAME4A]",0X0FA4,
  "[NRM13]",0X1884,
  "[SF]",0X1269,
  "[XCOPY]",0X2165,
  "[ALCL00]",0X06C9,
  "[QUTCAT]",0X03D5,
  "[SHF40]",0X186C,
  "[SIGMA_MINUS]",0X1271,
  "[NAME4D]",0X0FAC,
  "[DROPST]",0X00E4,
  "[STAYON]",0X12A3,
  "[ONE_BY_X13]",0X188E,
  "[INEX]",0X2A4A,
  "[ONE_BY_X10]",0X188B,
  "[SIGREG]",0X1277,
  "[FILLXL]",0X00EA,
  "[ERRRAM]",0X2172,
  "[XDSE]",0X159F,
  "[PACH4]",0X03E2,
  "[NFRPR]",0X00EE,
  "[SIN]",0X1288,
  "[DV2_10]",0X1898,
  "[DAT106]",0X2D4C,
  "[DV1_10]",0X189A,
  "[NFRC]",0X00F1,
  "[NFRPU]",0X00F0,
  "[DV2_13]",0X189D,
  "[ROW0]",0X2766,
  "[NFRFST]",0X00F7,
  "[PACH10]",0X03EC,
  "[PARS75]",0X0CCD,
  "[SIZE]",0X1292,
  "[STO_MINUS]",0X12B9,
  "[SNR12]",0X2441,
  "[DIV110]",0X18A5,
  "[DSPLN_]",0X0FC7,
  "[IN3B]",0X2A65,
  "[PACH11]",0X03F5,
  "[DIV15]",0X18A9,
  "[ERRPR]",0X2184,
  "[STO_DIVIDE]",0X12C1,
  "[BAKAPH]",0X09E3,
  "[SNROM]",0X2400,
  "[NFRNIO]",0X0106,
  "[DIV120]",0X18AF,
  "[RUNING]",0X0108,
  "[PACH12]",0X03FC,
  "[SQRT]",0X1298,
  "[SST]",0X129E,
  "[POWER_OF_10]",0X12CA,
  "[INSTR]",0X2A73,
  "[STBT30]",0X2FE0,
  "[STBT31]",0X2FE5,
  "[GOLNGH]",0X0FD9,
  "[GOLONG]",0X0FDA,
  "[TONE]",0X12D0,
  "[RAK60]",0X06FA,
  "[GOSUBH]",0X0FDD,
  "[GOSUB]",0X0FDE,
  "[SQR10]",0X18BE,
  "[INTXC]",0X2A7D,
  "[VIEW]",0X12D6,
  "[PAR111]",0X0CED,
  "[SQR13]",0X18C1,
  "[ERR0]",0X18C3,
  "[DAT231]",0X2D77,
  "[RUNNK]",0X011D,
  "[X_NE_0]",0X12DC,
  "[STOPSB]",0X03A9,
  "[RAK70]",0X070A,
  "[PAR112]",0X0CF5,
  "[GT3DBT]",0X0FEB,
  "[STO_MULT]",0X12A8,
  "[X_NE_Y]",0X12E2,
  "[LINNUM]",0X2A8B,
  "[STO_PLUS]",0X12B0,
  "[TAN]",0X1282,
  "[TEXT]",0X2CAF,
  "[TONSTF]",0X0054,
  "[LINNM1]",0X2A90,
  "[BKROM2]",0X2A91,
  "[TOREC]",0X1E75,
  "[LINN1A]",0X2A93,
  "[TRCS10]",0X1E57,
  "[TRG100]",0X1E78,
  "[TXTLBL]",0X2FC7,
  "[X_LE_0]",0X12EF,
  "[XASN]",0X276A,
  "[XAVIEW]",0X0364,
  "[XCUTB1]",0X0091,
  "[XCUTEB]",0X0090,
  "[LN560]",0X1BD3,
  "[XEND]",0X2728,
  "[X_LE_Y]",0X12F6,
  "[DAT260]",0X2D94,
  "[XGA00]",0X248D,
  "[STOST0]",0X013B,
  "[XX_LT_Y]",0X15EF,
  "[DAT280]",0X2D98,
  "[ABTSEQ]",0X0D12,
  "[TRG240]",0X1ED1,
  "[DAT300]",0X2D9B,
  "[XISG]",0X15A0,
  "[ABTS10]",0X0D16,
  "[XPRMPT]",0X03A0,
  "[XFT100]",0X18EC,
  "[XGI57]",0X24C1,
  "[XROM]",0X2FAF,
  "[DAT320]",0X2DA2,
  "[XSIGN]",0X0FF4,
  "[XX_GT_0]",0X15F1,
  "[X_LT_Y]",0X1308,
  "[XGI]",0X24C7,
  "[PMUL]",0X1BE9,
  "[TRGSET]",0X21D4,
  "[PARA06]",0X0D22,
  "[XX_GT_Y]",0X15F8,
  "[XRND]",0X0A2F,
  "[ASN15]",0X27C2,
  "[XX_LE_Y]",0X1601,
  "[XX_EQ_0]",0X1606,
  "[XX_LT_0]",0X15FA,
  "[PATCH1]",0X21DC,
  "[ROUND]",0X0A35,
  "[XX_LE_0A]",0X1609,
  "[XY_TO_X]",0X1B11,
  "[X_BY_Y13]",0X1893,
  "[PATCH2]",0X21E1,
  "[ASN20]",0X27CC,
  "[X_GT_0]",0X131A,
  "[NOPRT]",0X015B,
  "[XGI07]",0X24DA,
  "[OFSHFT]",0X0749,
  "[DROWSY]",0X0160,
  "[DRSY05]",0X0161,
  "[SAVRTN]",0X27D3,
  "[GTSRCH]",0X24DF,
  "[SAVR10]",0X27D5,
  "[XGOIND]",0X1323,
  "[NOSKP]",0X1619,
  "[DOSRC1]",0X24E3,
  "[DOSRCH]",0X24E4,
  "[HMS_PLUS]",0X1032,
  "[XEQ]",0X1328,
  "[OFFSHF]",0X0750,
  "[PATCH5]",0X21F3,
  "[DEC]",0X132B,
  "[SAVRC]",0X27DF,
  "[RSTANN]",0X0759,
  "[PARA60]",0X0D35,
  "[ANN_14]",0X075B,
  "[ANNOUT]",0X075C,
  "[DRSY25]",0X0173,
  "[IORUN]",0X27E4,
  "[PARA75]",0X0D49,
  "[XX_NE_Y]",0X1629,
  "[OCT]",0X1330,
  "[STSCR_]",0X1920,
  "[SIGN]",0X1337,
  "[STSCR]",0X1922,
  "[MSGAD]",0X1C18,
  "[HMS_MINUS]",0X1045,
  "[PARA61]",0X0D37,
  "[AON]",0X133C,
  "[DOSKP]",0X1631,
  "[LSWKUP]",0X0180,
  "[PLUS]",0X104A,
  "[EXSCR]",0X192A,
  "[RMCK15]",0X27F4,
  "[WKUP10]",0X0184,
  "[MSGDE]",0X1C22,
  "[MOD]",0X104F,
  "[AOFF]",0X1345,
  "[PATCH3]",0X21EE,
  "[XGTO]",0X2505,
  "[RCSCR_]",0X1932,
  "[MINUS]",0X1054,
  "[BSTE2]",0X2AF2,
  "[DF150]",0X0482,
  "[PATCH6]",0X1C06,
  "[MODE]",0X134D,
  "[DF160]",0X0485,
  "[MODE1]",0X134F,
  "[ROW940]",0X0487,
  "[INTFRC]",0X193B,
  "[DRSY51]",0X0194,
  "[DRSY50]",0X0190,
  "[XGNN10]",0X2512,
  "[NXLDEL]",0X2AFD,
  "[PCT]",0X1061,
  "[PARSEB]",0X0D6D,
  "[XCF]",0X164D,
  "[MSGNE]",0X1C38,
  "[FORMAT]",0X0A7B,
  "[MSGML]",0X1C2D,
  "[FLGANN]",0X1651,
  "[MSGNL]",0X1C3C,
  "[MULTIPLY]",0X105C,
  "[NXLSST]",0X2AF7,
  "[SINFRA]",0X194A,
  "[PATCH9]",0X1C03,
  "[PUTPCL]",0X2AF3,
  "[RCSCR]",0X1934,
  "[DIVIDE]",0X106F,
  "[PR14RT]",0X1365,
  "[MSGPR]",0X1C43,
  "[RMCK05]",0X27EC,
  "[DAT400]",0X2E05,
  "[RMCK10]",0X27F3,
  "[ROMCHK]",0X27E6,
  "[ABS]",0X1076,
  "[NXLIN]",0X2B14,
  "[INBCHS]",0X2E0A,
  "[DSWKUP]",0X01AD,
  "[INBYTJ]",0X2E0C,
  "[MSGOF]",0X1C4F,
  "[XR_S]",0X079D,
  "[ACOS]",0X107D,
  "[DAT500]",0X2E10,
  "[SUMCK2]",0X1669,
  "[LDSST0]",0X0797,
  "[DEROW]",0X04AD,
  "[MSGWR]",0X1C56,
  "[WKUP25]",0X01BA,
  "[CHKADR]",0X166E,
  "[AGTO]",0X1085,
  "[DERW00]",0X04B2,
  "[ERRDE]",0X282D,
  "[MOD10]",0X195C,
  "[GTLNKA]",0X2247,
  "[NXL1B]",0X2B23,
  "[MSGTA]",0X1C5F,
  "[ARCL]",0X108C,
  "[NXLINA]",0X2B1F,
  "[CAT3]",0X1383,
  "[MSGYES]",0X1C62,
  "[GTLINK]",0X224E,
  "[MSGNO]",0X1C64,
  "[ASHF]",0X1092,
  "[MSGRAM]",0X1C67,
  "[P6RTN]",0X1670,
  "[PARB40]",0X0D99,
  "[MSGROM]",0X1C6A,
  "[MSG]",0X1C6B,
  "[ASIN]",0X1098,
  "[MSGA]",0X1C6C,
  "[ROW933]",0X0467,
  "[SERR]",0X24E8,
  "[CHKAD4]",0X1686,
  "[MSGE]",0X1C71,
  "[ASN]",0X109E,
  "[SHIFT]",0X1348,
  "[SINFR]",0X1947,
  "[MSGX]",0X1C75,
  "[DTOR]",0X1981,
  "[RUN]",0X07C2,
  "[ASTO]",0X10A4,
  "[SKPLIN]",0X2AF9,
  "[SKP]",0X162E,
  "[IND]",0X0DB2,
  "[SUMCHK]",0X1667,
  "[TEN_TO_X]",0X1BF8,
  "[ATAN]",0X10AA,
  "[TRG430]",0X1F5B,
  "[MSG105]",0X1C80,
  "[FIX57]",0X0AC3,
  "[NXLCHN]",0X2B49,
  "[RTOD]",0X198C,
  "[ROLBAK]",0X2E42,
  "[TOOCT]",0X1F79,
  "[AVIEW]",0X10B2,
  "[NWGOOS]",0X07D4,
  "[SIGMA]",0X1C88,
  "[AXEQ]",0X10B5,
  "[LD90]",0X1995,
  "[MSG110]",0X1C86,
  "[UPLINK]",0X2235,
  "[IND21]",0X0DC4,
  "[OPROMT]",0X2E4C,
  "[BEEP]",0X10BB,
  "[DF200]",0X04E7,
  "[RW0110]",0X04E9,
  "[WKUP70]",0X01F5,
  "[PI_BY_2]",0X199A,
  "[INTARG]",0X07E1,
  "[STFLGS]",0X16A7,
  "[BST]",0X10C2,
  "[NXLIN3]",0X2B5F,
  "[TRC10]",0X19A1,
  "[AJ3]",0X0DD0,
  "[NXL3B2]",0X2B63,
  "[NOTFIX]",0X0ADD,
  "[CAT]",0X10C8,
  "[AJ2]",0X0DD4,
  "[TXRW10]",0X04F6,
  "[RW0141]",0X04F1,
  "[CF]",0X10CC,
  "[STOLCC]",0X2E5B,
  "[CLRPGM]",0X228C,
  "[INLIN]",0X2876,
  "[APHST_]",0X2E62,
  "[CLA]",0X10D1,
  "[MEMCHK]",0X0205,
  "[XTOHRS]",0X19B2,
  "[MESSL]",0X07EF,
  "[ENLCD]",0X07F6,
  "[XSCI]",0X16C0,
  "[MIDDIG]",0X0DE0,
  "[STORFC]",0X07E8,
  "[TXTROM]",0X04F5,
  "[NXLTX]",0X2B77,
  "[STO]",0X10DA,
  "[TXTROW]",0X04F2,
  "[TXTSTR]",0X04F6,
  "[WKUP21]",0X01A7,
  "[GTRMAD]",0X0800,
  "[CLDSP]",0X10E0,
  "[WKUP80]",0X01FF,
  "[XARCL]",0X1696,
  "[GCPKC]",0X2B80,
  "[XBST]",0X2250,
  "[XCUTE]",0X015B,
  "[XEQC01]",0X24EA,
  "[CLP]",0X10E7,
  "[STK]",0X0DF3,
  "[XBEEP]",0X16D1,
  "[DELNNN]",0X22A8,
  "[CHKRPC]",0X0222,
  "[GCPKC0]",0X2B89,
  "[CLREG]",0X10ED,
  "[ROW120]",0X0519,
  "[STK00]",0X0DFA,
  "[TODEC]",0X1FB3,
  "[TONE7X]",0X16DB,
  "[XDELET]",0X22AF,
  "[CLSIG]",0X10F3,
  "[STATCK]",0X1CC8,
  "[STK04]",0X0E00,
  "[TONEB]",0X16DD,
  "[XFS]",0X1645,
  "[XGNN12]",0X2514,
  "[CLST]",0X10F9,
  "[ROW11]",0X25AD,
  "[COLDST]",0X0232,
  "[XGNN40]",0X255D,
  "[XRS45]",0X07BE,
  "[XSF]",0X164A,
  "[SETQ_P]",0X0B15,
  "[XSGREG]",0X1659,
  "[CLX]",0X1101,
  "[XCLX1]",0X1102,
  "[XSST]",0X2260,
  "[XTONE]",0X16DE,
  "[XXEQ]",0X252F,
  "[CHSA]",0X1CDA,
  "[LDD_P_]",0X0B1D,
  "[CHSA1]",0X1CDC,
  "[COPY]",0X1109,
  "[HMSDV]",0X19E5,
  "[FCNTBL]",0X1400,
  "[ADD1]",0X1CE0,
  "[HMSMP]",0X19E7,
  "[D_R]",0X110E,
  "[ADD2]",0X1CE3,
  "[DSPCRG]",0X0B26,
  "[STBT10]",0X2EA3,
  "[GTACOD]",0X1FDB,
  "[LDDP10]",0X0B1E,
  "[DEG]",0X1114,
  "[DGENS8]",0X0836,
  "[DIGENT]",0X0837,
  "[GETXY]",0X1CEB,
  "[GCP112]",0X2BB5,
  "[GETY]",0X1CED,
  "[GETXSQ]",0X1CEE,
  "[GETX]",0X1CEF,
  "[AVAIL]",0X28C4,
  "[GETN]",0X1CEA,
  "[TGSHF1]",0X1FE7,
  "[AVAILA]",0X28C7,
  "[DSPCA]",0X0B35,
  "[GCPK05]",0X2BBE,
  "[GCPK04]",0X2BBC,
  "[GETYSQ]",0X1CEC,
  "[DEL]",0X1124,
  "[FDIGIT]",0X0E2F,
  "[GETLIN]",0X1419,
  "[DELETE]",0X1127,
  "[GRAD]",0X111A,
  "[LNSUB]",0X19F9,
  "[APND_]",0X1FF3,
  "[EXP10]",0X1A0A,
  "[APND10]",0X1FF5,
  "[DFKBCK]",0X0559,
  "[EXP13]",0X1A0D,
  "[DSE]",0X112D,
  "[DECMPL]",0X2EC2,
  "[APNDDG]",0X1FFA,
  "[END]",0X1132,
  "[XBAR_]",0X1D07,
  "[OVFL10]",0X1429,
  "[DFRST9]",0X0561,
  "[BSTEP]",0X28DE,
  "[DFILLF]",0X0563,
  "[XRAD]",0X1722,
  "[PACKE]",0X2002,
  "[NEXT1]",0X0E45,
  "[DCPL00]",0X2EC3,
  "[SD]",0X1D10,
  "[CAT2]",0X0B53,
  "[ENTER]",0X113E,
  "[DFRST8]",0X0562,
  "[DEGDO]",0X172A,
  "[ERR120]",0X22FF,
  "[EXP400]",0X1A21,
  "[BSTEPA]",0X28EB,
  "[ENG]",0X1135,
  "[NEXT]",0X0E50,
  "[FNDEND]",0X1730,
  "[E_TO_X]",0X1147,
  "[DELLIN]",0X2306,
  "[CLR]",0X1733,
  "[ERR110]",0X22FB,
  "[ERROR]",0X22F5,
  "[ERRSUB]",0X22E8,
  "[ADVNCE]",0X114D,
  "[FDIG20]",0X0E3D,
  "[INCGT2]",0X0286,
  "[LNSUB_MINUS]",0X19F8,
  "[NEXT2]",0X0E48,
  "[NEXT3]",0X0E4B,
  "[NROOM3]",0X28C2,
  "[FACT]",0X1154,
  "[PACKN]",0X2000,
  "[PCKDUR]",0X16FC,
  "[PR15RT]",0X22DF,
  "[RAD]",0X111F,
  "[SGTO19]",0X25C9,
  "[FC]",0X115A,
  "[DF060]",0X0587,
  "[PTLINK]",0X231A,
  "[PTLNKA]",0X231B,
  "[XASHF]",0X1748,
  "[LEFTJ]",0X2BF7,
  "[NULT_]",0X0E65,
  "[SAROM]",0X260D,
  "[SSTBST]",0X22DD,
  "[BSTE]",0X290B,
  "[E_TO_X_MIN1]",0X1163,
  "[PTBYTA]",0X2323,
  "[PTLNKB]",0X2321,
  "[TOGSHF]",0X1FE5,
  "[TONE7]",0X1716,
  "[XBAR]",0X1CFE,
  "[PTBYTP]",0X2328,
  "[DEEXP]",0X088C,
  "[FC_C]",0X116B,
  "[EXP710]",0X1A4C,
  "[PUTPCD]",0X232C,
  "[ROW10]",0X02A6,
  "[FIXEND]",0X2918,
  "[EXP720]",0X1A50,
  "[XASTO]",0X175C,
  "[ARGOUT]",0X2C10,
  "[FIX]",0X1171,
  "[MEMLFT]",0X05A1,
  "[NULT_3]",0X0E7C,
  "[GTCNTR]",0X0B8D,
  "[BLINK1]",0X0899,
  "[DCPLRT]",0X2F0B,
  "[BLINK]",0X0899,
  "[DCRT10]",0X2F0D,
  "[FRAC]",0X117C,
  "[FLINKP]",0X2925,
  "[INT]",0X1177,
  "[FLINKA]",0X2927,
  "[FLINK]",0X2928,
  "[FLINKM]",0X2929,
  "[EXP500]",0X1A61,
  "[FS]",0X1182,
  "[NULT_5]",0X0E8F,
  "[ERRTA]",0X2F17,
  "[NLT000]",0X0E91,
  "[CNTLOP]",0X0B9D,
  "[FS_C]",0X1188,
  "[INPTDG]",0X08A0,
  "[P10RTN]",0X02AC,
  "[BLANK]",0X05B7,
  "[DERUN]",0X08AD,
  "[CHRLCD]",0X05B9,
  "[AOUT15]",0X2C2B,
  "[CHKFUL]",0X05BA,
  "[FIND_NO_1]",0X1775,
  "[DIGST_]",0X08B2,
  "[GTOL]",0X118C,
  "[HMS_H]",0X1193,
  "[GTO]",0X1191,
  "[NLT020]",0X0EA0,
  "[ALLOK]",0X02CD,
  "[BRTS10]",0X1D6B,
  "[PAK200]",0X2055,
  "[H_HMS]",0X1199,
  "[PTBYTM]",0X2921,
  "[PROMFC]",0X05C7,
  "[PUTPCA]",0X2339,
  "[PUTPCF]",0X2331,
  "[ISG]",0X119E,
  "[NLT040]",0X0EAA,
  "[PROMF1]",0X05CB,
  "[R_SCAT]",0X0BB7,
  "[PUTPCX]",0X232F,
  "[PUTPC]",0X2337,
  "[BSTCAT]",0X0BBA,
  "[LBL]",0X11A4,
  "[LN]",0X11A6,
  "[PROMF2]",0X05D3,
  "[GETPC]",0X2950,
  "[ERRNE]",0X02E0,
  "[GETPCA]",0X2952,
  "[LNAP]",0X1A8A,
  "[BCDBIN]",0X02E3,
  "[CAT1]",0X0BC3,
  "[BRT100]",0X1D80,
  "[LOG]",0X11AC,
  "[REGLFT]",0X059A,
  "[GTONN]",0X2959,
  "[STDEV]",0X11B2,
  "[RSTST]",0X08A7,
  "[SARO21]",0X2640,
  "[SARO22]",0X2641,
  "[SIZSUB]",0X1797,
  "[SKPDEL]",0X2349,
  "[XECROM]",0X2F4A,
  "[MEAN]",0X11B9,
  "[SSTCAT]",0X0BB4,
  "[NULTST]",0X0EC6,
  "[GENNUM]",0X05E8,
  "[TOPOL]",0X1D49,
};


/***********************/
/* searches a table that has been transformed */
/***********************/
void Chp41::FindGlobalName(uint addr, char *name)
{
  name[0]=0;
  for(int i=0; i<Globals; i++)
  {
    if(GlobalName[i].addr == addr)
      {
      strcpy(name,GlobalName[i].name);
      break;
      }
  }
}

/***********************/
uint Chp41::FindGlobalAddr(char *name)
{
  char name2[50];
  sprintf(name2,"[%s]",name);
  for(int i=0; i<Globals; i++)
  {
    if(0==strcmp(GlobalName[i].name,name2))
      return(GlobalName[i].addr);
  }
  return(0);
}

/***********************/
uint Chp41::GetInstSet()
{
  return(InstSetIndex);
}

/***********************/
void Chp41::ChangeInstSet(uint newset)
{
  if(InstSetIndex>2)
  {
    InstSetIndex=1;
    TEFIndex=newset;
  }
  else
    TEFIndex=InstSetIndex=newset;
}

/***********************/
char* Chp41::GetOpcodeName(uint opc)
{
  return Opcode[InstSetIndex][opc].name;
}

/***********************/
char* Chp41::GetTEFName(uint tef)
{
  return TEFs[TEFIndex][tef].name;
}

/***********************/
void Chp41::PrintRegisters(void)
{
  int i;

  fprintf(hLogFile, "\n  A=");
  for(i=13; i>=0; i--)
    fprintf(hLogFile, "%1X", hp41cpu->r->A_REG[i] & 0xF);

  fprintf(hLogFile, " B=");
  for(i=13; i>=0; i--)
    fprintf(hLogFile, "%1X", hp41cpu->r->B_REG[i] & 0xF);

  fprintf(hLogFile, " C=");
  for(i=13; i>=0; i--)
    fprintf(hLogFile, "%1X", hp41cpu->r->C_REG[i] & 0xF);

  fprintf(hLogFile, " Stack=");
  fprintf(hLogFile, "%04X ", hp41cpu->r->RET_STK0);
  fprintf(hLogFile, "%04X ", hp41cpu->r->RET_STK1);
  fprintf(hLogFile, "%04X ", hp41cpu->r->RET_STK2);
  fprintf(hLogFile, "%04X\n", hp41cpu->r->RET_STK3);

  fprintf(hLogFile, "  M=");
  for(i=13; i>=0; i--)
    fprintf(hLogFile, "%1X", hp41cpu->r->M_REG[i] & 0xF);

  fprintf(hLogFile, " N=");
  for(i=13; i>=0; i--)
    fprintf(hLogFile, "%1X", hp41cpu->r->N_REG[i] & 0xF);

  fprintf(hLogFile, " Cr=");
  fprintf(hLogFile, "%1X", hp41cpu->r->CARRY & 0xF);

  fprintf(hLogFile, " %cP=",(hp41cpu->PT_REG==&hp41cpu->r->P_REG)?'>':' ');
  fprintf(hLogFile, "%1X", hp41cpu->r->P_REG & 0xF);

  fprintf(hLogFile, " %cQ=",(hp41cpu->PT_REG==&hp41cpu->r->Q_REG)?'>':' ');
  fprintf(hLogFile, "%1X", hp41cpu->r->Q_REG & 0xF);

  fprintf(hLogFile, " G=");
  fprintf(hLogFile, "%02X", hp41cpu->r->G_REG & 0xFF);

  fprintf(hLogFile, " F0=");
  fprintf(hLogFile, "%02X", hp41cpu->r->F_REG & 0xFF);

  fprintf(hLogFile, " ST=");
  for(i=5; i>=0; i--)
    fprintf(hLogFile, "%1X", (hp41cpu->r->XST_REG & (1 << i)) >> i);
  fprintf(hLogFile, " ");
  for(i=7; i>=0; i--)
    fprintf(hLogFile, "%1X", (hp41cpu->r->ST_REG & (1 << i)) >> i);
  fprintf(hLogFile, "\n");

  fprintf(hLogFile, "  CLK_A=");
  for(i=13; i>=0; i--)
    fprintf(hLogFile, "%1X", CLK_A[i] & 0xF);

  fprintf(hLogFile, " ALM_A=");
  for(i=13; i>=0; i--)
    fprintf(hLogFile, "%1X", ALM_A[i] & 0xF);

  fprintf(hLogFile, " SCR_A=");
  for(i=13; i>=0; i--)
    fprintf(hLogFile, "%1X", SCR_A[i] & 0xF);
  fprintf(hLogFile, "\n");

  fprintf(hLogFile, "  CLK_B=");
  for(i=13; i>=0; i--)
    fprintf(hLogFile, "%1X", CLK_B[i] & 0xF);

  fprintf(hLogFile, " ALM_B=");
  for(i=13; i>=0; i--)
    fprintf(hLogFile, "%1X", ALM_B[i] & 0xF);

  fprintf(hLogFile, " SCR_B=");
  for(i=13; i>=0; i--)
    fprintf(hLogFile, "%1X", SCR_B[i] & 0xF);
  fprintf(hLogFile, "\n");

  fprintf(hLogFile, "  INTV_TV=");
  for(i=4; i>=0; i--)
    fprintf(hLogFile, "%1X", INTV_TV[i] & 0xF);

  fprintf(hLogFile, "  INTV_CNT=");
  for(i=4; i>=0; i--)
    fprintf(hLogFile, "%1X", INTV_CNT[i] & 0xF);

  fprintf(hLogFile, " TMR_S=");
  for(i=3; i>=0; i--)
    fprintf(hLogFile, "%1X", TMR_S[i] & 0xF);

  fprintf(hLogFile, " ACC_F=");
  for(i=3; i>=0; i--)
    fprintf(hLogFile, "%1X", ACC_F[i] & 0xF);

  fprintf(hLogFile, " Timer=");
  if (TimerSelA)
    fprintf(hLogFile, "A");
  else
    fprintf(hLogFile, "B");

  fprintf(hLogFile, " FI=");
  for(i=13; i>=0; i--)
    fprintf(hLogFile, "%1X", (hp41cpu->r->FI_REG & (1 << i)) >> i);
  fprintf(hLogFile, "\n");

  fprintf(hLogFile, "  RAM Addr=");
  fprintf(hLogFile, "%03X", hp41cpu->ram_selected);

  fprintf(hLogFile, "  Perph Addr=");
  fprintf(hLogFile, "%02X", hp41cpu->perph_selected);

  if (hp41cpu->r->BASE==16)
    fprintf(hLogFile, "  Base=16");
  else
    fprintf(hLogFile, "  Base=10");
  //RB++ 090821 KEY and DisplayOn
  fprintf(hLogFile, "  KEY DWN=%04X %02X", hp41cpu->r->KEYDOWN, hp41cpu->r->KEY_REG);
  fprintf(hLogFile, "  DSP=%X", DisplayOn);
  //RB-- 090821 KEY and DisplayOn
  fprintf(hLogFile, "\n");
}

/***********************/
void Chp41::StartTrace(void)
{
  hLogFile=fopen("hp41.log","wt");
  if (hLogFile == NULL)
  {
    LOG("Unable to create trace.log file.");
    return;
  }
  fTrace=true;
}

/***********************/
void Chp41::StopTrace(void)
{
  fTrace=false;
  fclose(hLogFile);
}

/***********************/
void Chp41::SwitchTrace(void)
{
  if (fTrace)
    StopTrace();
  else
    StartTrace();
}

/***********************/
flag Chp41::GetTrace(void)
{
  return(fTrace);
}

/***********************/
// logs a string and closes the file to be sure we catch it
/***********************/
void SafeLog(char *psz)
  {
  FILE *hLogFile;
  hLogFile=fopen("trace.log","at");
  fprintf(hLogFile, "%s\n",psz);
  fflush(hLogFile);
  fclose(hLogFile);
  }


/****************************/
/* sends trace output to file */
/****************************/
void Chp41::TraceOut()
  {
  PC_TRACE=hp41cpu->r->PC_REG;
  PrintRegisters();
  fprintf(hLogFile, "%04X  ",PC_TRACE);
  trace();
  fprintf(hLogFile,szTraceOut);
  fprintf(hLogFile, "\n");
  fflush(hLogFile);
  }


/****************************/
/* generates a single line of trace code */
/****************************/
void Chp41::trace()
  {
  szTraceLabel[0]='\0';
  FindGlobalName(PC_TRACE, szTraceLabel);
  uint g_len=strlen(szTraceLabel);
  if (g_len<8)
    {
    szTraceLabel[g_len]='\t';
    szTraceLabel[g_len+1]='\0';
    }
  // TraceTyte1=GetNextTyte()
  word page=(PC_TRACE&0xf000)>>12;
  ModulePage *pPage=PageMatrix[page][active_bank[page]-1];
  if (pPage==NULL)
    TraceTyte1=0;
  else
    TraceTyte1=pPage->Image[PC_TRACE&0xfff];
  if (hp41cpu->perph_in_control)
    {
    }
  else              /* execute mcode */
    {
    switch(TraceTyte1&3)
      {
      case 0 :     /* misc */
        {
        trace_class0();
        break;
        }
      case 1 :    /* long jumps */
        {
        trace_class1();
        break;
        }
      case 2 :   /* TEF */
        {
        trace_class2();
        break;
        }
      case 3 :   /* short jumps */
        {
        trace_class3();
        break;
        }
      }
    }
  }


/****************************/
void  Chp41::trace_class0()
  {
  hp41cpu->Modifier=(TraceTyte1&0x03c0)>>6;
  switch ((TraceTyte1&0x003c)>>2)
    {
    case 0:
      {
      trace_subclass0();      /*  reads Modifier and writes to inst_str */
      break;
      }
    case 1:
      {
      trace_subclass1();
      break;
      }
    case 2:
      {
      trace_subclass2();
      break;
      }
    case 3:
      {
      trace_subclass3();
      break;
      }
    case 4:
      {
      trace_subclass4();
      break;
      }
    case 5:
      {
      trace_subclass5();
      break;
      }
    case 6:
      {
      trace_subclass6();
      break;
      }
    case 7:
      {
      trace_subclass7();
      break;
      }
    case 8:
      {
      trace_subclass8();
      break;
      }
    case 9:
      {
      trace_subclass9();
      break;
      }
    case 10:
      {
      trace_subclassA();
      break;
      }
    case 11:
      {
      trace_subclassB();
      break;
      }
    case 12:
      {
      trace_subclassC();
      break;
      }
    case 13:
      {
      trace_subclassD();
      break;
      }
    case 14:
      {
      trace_subclassE();
      break;
      }
    case 15:
      {
      trace_subclassF();
      break;
      }
    }
  }


/****************************/
void  Chp41::trace_subclass0()
  {
  switch(hp41cpu->Modifier)
    {
    case 0:  /* NOP */
    case 1:  /* WMLDL */
    case 4:  /* ENBANK1 */
    case 6:  /* ENBANK2  */
    case 5:  /* ENBANK3  */
    case 7:  /* ENBANK4  */
    case 2:  /* NOT USED */
    case 3:
      {
      sprintf(szTraceOut, "%03X\t%s\t%s", TraceTyte1, szTraceLabel, GetOpcodeName(16+hp41cpu->Modifier));
      break;
      }
    default:  /* HPIL=C 0-7 */
      {
      sprintf(szTraceOut, "%03X\t%s\t%s\t%X", TraceTyte1, szTraceLabel, GetOpcodeName(121), hp41cpu->Modifier-8);
      break;
      }
    }
  }


/****************************/
void  Chp41::trace_subclass1()
  {
  if (hp41cpu->Modifier==7)                 /* not used */
    {
    sprintf(szTraceOut, "%03X", TraceTyte1);
    }
  else if (hp41cpu->Modifier==15)            /* ST=0 */
    {
    sprintf(szTraceOut, "%03X\t%s\t%s", TraceTyte1, szTraceLabel, GetOpcodeName(122));
    }
  else                             /* CF 0-13 */
    {
    sprintf(szTraceOut, "%03X\t%s\t%s\t%d", TraceTyte1, szTraceLabel, GetOpcodeName(1), TypeA[hp41cpu->Modifier]);
    }
  }


/****************************/
void  Chp41::trace_subclass2()
  {
  if (hp41cpu->Modifier==7)                   /* not used */
    {
    sprintf(szTraceOut, "%03X", TraceTyte1);
    }
  else if (hp41cpu->Modifier==15)             /* CLRKEY */
    {
    sprintf(szTraceOut, "%03X\t%s\t%s", TraceTyte1, szTraceLabel, GetOpcodeName(123));
    }
  else                             /* SF 0-13 */
    {
    sprintf(szTraceOut, "%03X\t%s\t%s\t%d", TraceTyte1, szTraceLabel, GetOpcodeName(2), TypeA[hp41cpu->Modifier]);
    }
  }


/****************************/
void  Chp41::trace_subclass3()
  {
  if (hp41cpu->Modifier==7)                         /* not used */
    {
    sprintf(szTraceOut, "%03X\t%s", TraceTyte1, szTraceLabel);
    }
  else if (hp41cpu->Modifier==15)                   /* ?KEY */
    {
    sprintf(szTraceOut, "%03X\t%s\t%s", TraceTyte1, szTraceLabel, GetOpcodeName(124));
    }
  else                                     /* ?FS 0-13 */
    {
    sprintf(szTraceOut, "%03X\t%s\t%s\t%d", TraceTyte1, szTraceLabel, GetOpcodeName(3), TypeA[hp41cpu->Modifier]);
    }
  }


/****************************/
/* LC 0-15 (HP) OR LC 0-9,A-F (OTHERS) */
/****************************/
void  Chp41::trace_subclass4()
  {
  if(InstSetIndex)
    sprintf(szTraceOut, "%03X\t%s\t%s\t%X", TraceTyte1, szTraceLabel, GetOpcodeName(4), hp41cpu->Modifier);
  else
    sprintf(szTraceOut, "%03X\t%s\t%s\t%d", TraceTyte1, szTraceLabel, GetOpcodeName(4), hp41cpu->Modifier);
  }


/****************************/
void  Chp41::trace_subclass5()
  {
  if (hp41cpu->Modifier==7)
    {
    sprintf(szTraceOut, "%03X\t%s", TraceTyte1, szTraceLabel);
    }
  else if (hp41cpu->Modifier==15)                   /* -PT */
    {
    sprintf(szTraceOut, "%03X\t%s\t%s", TraceTyte1, szTraceLabel, GetOpcodeName(125));
    }
  else                                    /* ?PT 0-15 */
    {
    sprintf(szTraceOut, "%03X\t%s\t%s\t%d", TraceTyte1, szTraceLabel, GetOpcodeName(5), TypeA[hp41cpu->Modifier]);
    }
  }


/****************************/
void  Chp41::trace_subclass6()
  {
  sprintf(szTraceOut, "%03X\t%s\t%s", TraceTyte1, szTraceLabel, GetOpcodeName(24+hp41cpu->Modifier));
  }


/****************************/
void  Chp41::trace_subclass7()
  {
  if (hp41cpu->Modifier==7)                         /* LITERAL */
    {
    sprintf(szTraceOut, "%03X\t%s", TraceTyte1, szTraceLabel);
    }
  else if (hp41cpu->Modifier==15)                   /* +PT */
    {
    sprintf(szTraceOut, "%03X\t%s\t%s", TraceTyte1, szTraceLabel, GetOpcodeName(126));
    }
  else                                    /* PT 0-15 */
    {
    sprintf(szTraceOut, "%03X\t%s\t%s\t%d", TraceTyte1, szTraceLabel, GetOpcodeName(7), TypeA[hp41cpu->Modifier]);
    }
  }


/****************************/
/* MISC */
/****************************/
void  Chp41::trace_subclass8()
  {
  sprintf(szTraceOut, "%03X\t%s\t%s", TraceTyte1, szTraceLabel, GetOpcodeName(40+hp41cpu->Modifier));
  }


/****************************/
/* PERTCT 0-15 (HP) OR PERTCT 0-9,A-F (OTHERS) */
/****************************/
void  Chp41::trace_subclass9()
  {
  if(InstSetIndex)
    sprintf(szTraceOut, "%03X\t%s\t%s\t%X", TraceTyte1, szTraceLabel, GetOpcodeName(9), hp41cpu->Modifier);
  else
    sprintf(szTraceOut, "%03X\t%s\t%s\t%d", TraceTyte1, szTraceLabel, GetOpcodeName(9), hp41cpu->Modifier);
  }


/****************************/
void  Chp41::trace_subclassA()
  {
  if (hp41cpu->perph_selected==0)   /* ram */
    {
    sprintf(szTraceOut, "%03X\t%s\t%s\t%X", TraceTyte1, szTraceLabel, GetOpcodeName(10), hp41cpu->Modifier);
    return;
    }
  switch(hp41cpu->perph_selected)
    {
    case 0xfb:   /* timer write */
      {
      if((hp41cpu->ram_selected > 0x39) || (hp41cpu->ram_selected < 0x10))
        sprintf(szTraceOut, "%03X\t%s\t%s\t%X", TraceTyte1, szTraceLabel, GetOpcodeName(10),hp41cpu-> Modifier);
      else
        sprintf(szTraceOut, "%03X\t%s\t%s\tmodif=%d", TraceTyte1, szTraceLabel, GetOpcodeName(134+hp41cpu->Modifier), hp41cpu->Modifier);
      break;
      }
    case 0xfc:  /* card reader */
      {
      sprintf(szTraceOut, "%03X\t%s\t%s", TraceTyte1, szTraceLabel, GetOpcodeName(156+hp41cpu->Modifier));
      break;
      }
    case 0xfd: /* main display */
      {
      sprintf(szTraceOut, "%03X\t%s\t%s", TraceTyte1, szTraceLabel, GetOpcodeName(56+hp41cpu->Modifier));
      break;
      }
    case 0xfe:   /* wand */
      {
      sprintf(szTraceOut, "%03X\t%s\t%s", TraceTyte1, szTraceLabel, "wand");
      break;
      }
    case 0x10:  /* halfnut display */
      {
      sprintf(szTraceOut, "%03X\t%s\t%s", TraceTyte1, szTraceLabel, "halfnut write");
      break;
      }
    default:
      {
      sprintf(szTraceOut, "%03X\t%s", TraceTyte1, szTraceLabel);
      break;
      }
    }
  }


/****************************/
void  Chp41::trace_subclassB()
  {
  sprintf(szTraceOut, "%03X\t%s\t%s", TraceTyte1, szTraceLabel, GetOpcodeName(72+hp41cpu->Modifier));
  }


/****************************/
void  Chp41::trace_subclassC()
  {
  switch(hp41cpu->Modifier)
    {
    case 1:  /* N=C */
      {
      sprintf(szTraceOut, "%03X\t%s\t%s", TraceTyte1, szTraceLabel, GetOpcodeName(88+hp41cpu->Modifier));
      break;
      }
    case 2:  /* C=N */
      {
      sprintf(szTraceOut, "%03X\t%s\t%s", TraceTyte1, szTraceLabel, GetOpcodeName(88+hp41cpu->Modifier));
      break;
      }
    case 3:  /* C<>N */
      {
      sprintf(szTraceOut, "%03X\t%s\t%s", TraceTyte1, szTraceLabel, GetOpcodeName(88+hp41cpu->Modifier));
      break;
      }
    case 4:  /* LDI */
      {
      // tyte2=GetNextTyte()
      word page=(PC_TRACE&0xf000)>>12;
      ModulePage *pPage=PageMatrix[page][active_bank[page]-1];
      word tyte2=pPage->Image[(PC_TRACE+1)&0xfff];
      sprintf(szTraceOut, "%03X%03X\t%s\t%s\t%03X", TraceTyte1, tyte2, szTraceLabel, GetOpcodeName(88+hp41cpu->Modifier), tyte2);
      break;
      }
    case 5:  /* STK=C */
      {
      sprintf(szTraceOut, "%03X\t%s\t%s", TraceTyte1, szTraceLabel, GetOpcodeName(88+hp41cpu->Modifier));
      break;
      }
    case 6:  /* C=STK */
      {
      sprintf(szTraceOut, "%03X\t%s\t%s", TraceTyte1, szTraceLabel, GetOpcodeName(88+hp41cpu->Modifier));
      break;
      }
    case 8:  /* GTOKEY */
      {
      sprintf(szTraceOut, "%03X\t%s\t%s", TraceTyte1, szTraceLabel, GetOpcodeName(88+hp41cpu->Modifier));
      break;
      }
    case 9:  /* RAMSLCT */
      {
      sprintf(szTraceOut, "%03X\t%s\t%s", TraceTyte1, szTraceLabel, GetOpcodeName(88+hp41cpu->Modifier));
      break;
      }
    case 11:  /* WDATA */
      {
      switch(hp41cpu->perph_selected)
        {
        case 0:   /* ram */
          {
          sprintf(szTraceOut, "%03X\t%s\t%s", TraceTyte1, szTraceLabel, GetOpcodeName(99));
          break;
          }
        case 0xfd:    /* main display */
          {
          sprintf(szTraceOut, "%03X\t%s\t%s", TraceTyte1, szTraceLabel, GetOpcodeName(120));
          break;
          }
        case 0xfb:    /* timer */
          {
          sprintf(szTraceOut, "%03X\t%s\t%s", TraceTyte1, szTraceLabel, GetOpcodeName(99));
          break;
          }
        default:
          {
          sprintf(szTraceOut, "%03X\t%s", TraceTyte1, szTraceLabel);
          break;
          }
        }
      break;
      }
    case 12:  /* RDROM */
      {
      sprintf(szTraceOut, "%03X\t%s\t%s", TraceTyte1, szTraceLabel, GetOpcodeName(88+hp41cpu->Modifier));
      break;
      }
    case 13:  /* C=CORA */
      {
      sprintf(szTraceOut, "%03X\t%s\t%s", TraceTyte1, szTraceLabel, GetOpcodeName(88+hp41cpu->Modifier));
      break;
      }
    case 14:  /* C=CANDA */
      {
      sprintf(szTraceOut, "%03X\t%s\t%s", TraceTyte1, szTraceLabel, GetOpcodeName(88+hp41cpu->Modifier));
      break;
      }
    case 15:  /* PERSLCT */
      {
      sprintf(szTraceOut, "%03X\t%s\t%s", TraceTyte1, szTraceLabel, GetOpcodeName(88+hp41cpu->Modifier));
      break;
      }
    case 0:  /* NOT USED  */
    case 7:
    case 10:
      {
      sprintf(szTraceOut, "%03X\t%s", TraceTyte1, szTraceLabel);
      break;
      }
    }
  }


/****************************/
void  Chp41::trace_subclassD()
  {
  sprintf(szTraceOut, "%03X\t%s", TraceTyte1, szTraceLabel);
  }


/****************************/
void  Chp41::trace_subclassE()
  {
  if (hp41cpu->perph_selected==0)   /* ram */
    {
    if (hp41cpu->Modifier==0)
      {
      sprintf(szTraceOut, "%03X\t%s\t%s", TraceTyte1, szTraceLabel, GetOpcodeName(127));
      }
    else
      {
      sprintf(szTraceOut, "%03X\t%s\t%s\t%d", TraceTyte1, szTraceLabel, GetOpcodeName(14), hp41cpu->Modifier);
      }
    return;
    }
  switch(hp41cpu->perph_selected)
    {
    case 0xfb:   /* timer */
      {
      if (hp41cpu->Modifier>5)
        {
        sprintf(szTraceOut, "%03X\t%s\t%s\t%d", TraceTyte1, szTraceLabel, GetOpcodeName(14), hp41cpu->Modifier);
        }
      else
        {
        sprintf(szTraceOut, "%03X\t%s\t%s\tmodif=%d", TraceTyte1, szTraceLabel, GetOpcodeName(150+hp41cpu->Modifier), hp41cpu->Modifier);
        }
      break;
      }
    case 0xfc:  /* card reader */
      {
      sprintf(szTraceOut, "%03X\t%s\t%s", TraceTyte1, szTraceLabel, "card_reader");
      break;
      }
    case 0xfd: /* main display */
      {
      sprintf(szTraceOut, "%03X\t%s\t%s", TraceTyte1, szTraceLabel, GetOpcodeName(104+hp41cpu->Modifier));
      break;
      }
    case 0xfe:   /* wand */
      {
      sprintf(szTraceOut, "%03X\t%s\t%s", TraceTyte1, szTraceLabel, "wand read");
      break;
      }
    case 0x10:  /* halfnut display */
      {
      sprintf(szTraceOut, "%03X\t%s\t%s", TraceTyte1, szTraceLabel, "halfnut read");
      break;
      }
    default:
      {
      sprintf(szTraceOut, "%03X\t%s\t%s", TraceTyte1, szTraceLabel, "unknown peripherial");
      break;
      }
    }
  }


/****************************/
void  Chp41::trace_subclassF()
  {
  if ((hp41cpu->Modifier==7) || (hp41cpu->Modifier==15))        /* LITERAL */
    {
    sprintf(szTraceOut, "%03X\t%s\t%s", TraceTyte1, szTraceLabel, "unknown");
    }
  else                                   /* RCR 0-13 */
    {
    sprintf(szTraceOut, "%03X\t%s\t%s\t%d", TraceTyte1, szTraceLabel, GetOpcodeName(15), TypeA[hp41cpu->Modifier]);
    }
  }

/****************************/
void  Chp41::trace_class1()
  {
  char lbl[14];
  int len;
  word jmp_add;

  // tyte2=GetNextTyte()
  word page=(PC_TRACE&0xf000)>>12;
  ModulePage *pPage=PageMatrix[page][active_bank[page]-1];
  word tyte2=pPage->Image[(PC_TRACE+1)&0xfff];

  jmp_add=((tyte2&0x03fc)<<6|(TraceTyte1&0x03fc)>>2);

  lbl[0]='\0';
  FindGlobalName(jmp_add, lbl);
  len=strlen(lbl);

  switch (tyte2&0x0003)
    {
    case 0:              /* NCXQ */
      {
      if(len)
        sprintf(szTraceOut, "%03X%03X\t%s\t%s\t%s %04X", TraceTyte1, tyte2, szTraceLabel, GetOpcodeName(128), lbl, jmp_add);
      else
        sprintf(szTraceOut, "%03X%03X\t%s\t%s\t%04X", TraceTyte1, tyte2, szTraceLabel, GetOpcodeName(128), jmp_add);
      break;
      }
    case 1:              /* CQX */
      {
      if(len)
        sprintf(szTraceOut, "%03X%03X\t%s\t%s\t%s %04X", TraceTyte1, tyte2, szTraceLabel, GetOpcodeName(129), lbl, jmp_add);
      else
        sprintf(szTraceOut, "%03X%03X\t%s\t%s\t%04X", TraceTyte1, tyte2, szTraceLabel, GetOpcodeName(129), jmp_add);
      break;
      }
    case 2:              /* NCGO */
      {
      if(len)
        sprintf(szTraceOut, "%03X%03X\t%s\t%s\t%s %04X", TraceTyte1, tyte2, szTraceLabel, GetOpcodeName(130), lbl, jmp_add);
      else
        sprintf(szTraceOut, "%03X%03X\t%s\t%s\t%04X", TraceTyte1, tyte2, szTraceLabel, GetOpcodeName(130), jmp_add);
      break;
      }
    case 3:              /* CGO */
      {
      if(len)
        sprintf(szTraceOut, "%03X%03X\t%s\t%s\t%s %04X", TraceTyte1, tyte2, szTraceLabel, GetOpcodeName(131), lbl, jmp_add);
      else
        sprintf(szTraceOut, "%03X%03X\t%s\t%s\t%04X", TraceTyte1, tyte2, szTraceLabel, GetOpcodeName(131), jmp_add);
      break;
      }
    }
  }

/****************************/
void  Chp41::trace_class2()
  {
  word subclass;

  hp41cpu->Modifier=(TraceTyte1&0x001c)>>2;
  subclass=(TraceTyte1&0x03e0)>>5;
  sprintf(szTraceOut, "%03X\t%s\t%s\t%s", TraceTyte1, szTraceLabel, GetOpcodeName(172+subclass), GetTEFName(hp41cpu->Modifier));
  }

/****************************/
void  Chp41::trace_class3()
  {
  word oncarry;
  char lbl[14];
  word len, jmp_add;
  short displacement;

  oncarry=(TraceTyte1&0x0004)>>2;
  displacement=((TraceTyte1&0x01f8)>>3);
  if (TraceTyte1&0x0200)
    displacement=-(64-displacement);
  jmp_add=PC_TRACE+displacement;
  lbl[0]='\0';
  FindGlobalName(jmp_add, lbl);
  len=strlen(lbl);
  if(len)
    sprintf(szTraceOut, "%03X\t%s\t%s\t%+d\t%s %04X", TraceTyte1, szTraceLabel, GetOpcodeName(204+oncarry), displacement, lbl, jmp_add);
  else
    sprintf(szTraceOut, "%03X\t%s\t%s\t%+d\t%04X", TraceTyte1, szTraceLabel, GetOpcodeName(204+oncarry), displacement, jmp_add);
  }

#endif
