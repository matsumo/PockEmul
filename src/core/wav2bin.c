/*
 *  wav2bin.c
 *
 *  last modified:
 *  2006/02/17 Marcus von Cube <marcus@mvcsys.de>
 *             Handling of data files handled (preliminary!)
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ERR_OK          0
#define ERR_NOK        -1

#define TYPE_NOK        0
#define TYPE_BAS        1
#define TYPE_IMG        2

#define IDENT_UNKNOWN   0x00
#define IDENT_OLD_BAS   0x20
#define IDENT_OLD_PAS   0x21
#define IDENT_OLD_BIN   0x26
#define IDENT_NEW_BAS   0x70
#define IDENT_NEW_PAS   0x71
#define IDENT_EXT_BAS   0x72
#define IDENT_EXT_PAS   0x73
#define IDENT_DATA      0x74
#define IDENT_NEW_BIN   0x76
#define IDENT_PC1211    0x80
#define IDENT_PC1500    0x0A
#define IDENT_PC15_BIN  0xA0
#define IDENT_PC15_BAS  0xA1

#define BAS_EXT_LINE_NB 0x1F
#define BAS_EXT_CODE    0xFE
#define BAS_1500_EOF    0xFF
#define BAS_OLD_EOF     0xF0
#define BAS_NEW_EOF     0xFFFF
#define DATA_VARIABLE   0xFE00
#define DATA_EOF        0x0F

#define TRANS_HIGH      0x8F
#define TRANS_LOW       0x70

#define BIT_0           8
#define BIT_1           16
#define BIT_MID         ((BIT_0 + BIT_1) / 2)

#define SYNC_NB_TRY     50

#define ORDER_STD       0
#define ORDER_INV       1

#define BASE_FREQ1      4000
#define BASE_FREQ2      2500

#define TOKEN_NB        3
static char* Token[] = { "T:", "G:", "D:" } ;

typedef unsigned char    u_char;    /*  8 bits */
typedef unsigned short   u_short;   /* 16 bits */
typedef unsigned long    u_long;    /* 32 bits */

typedef struct {
    FILE*   ptrFd ;
    u_long  ident ;
    u_long  freq ;
    u_long  wav_freq ;
    u_long  bitLen ;
    u_long  count ;
    u_long  total ;
    u_long  sum ;
    u_long  type ;
    u_long  graph ;
    u_long  debug ;
    int     synching ;
} FileInfo ;

typedef struct {
    u_short length ;
    u_char  dim1 ;
    u_char  dim2 ;
    u_char  itemLen ;
    u_char  type ;
} TypeInfo ;

static char* CodeOld[] = {
    "[00]","[01]","[02]","[03]","[04]","[05]","[06]","[07]",
    "[08]","[09]","[0A]","[0B]","[0C]","[0D]","[0E]","[0F]",
    ""," ","\"","?","!","#","%","ù",
    "$","„","˚",",",";",":","@","&",
    "[20]","[21]","[22]","[23]","[24]","[25]","[26]","[27]",
    "[28]","[29]","[2A]","[2B]","[2C]","[2D]","[2E]","[2F]",
    "(",")",">","<","=","+","-","*",
    "/","^","[3A]","[3B]","[3C]","[3D]","[3E]","[3F]",
    "0","1","2","3","4","5","6","7",
    "8","9",".","e","€","~","_","[4F]",
    "[50]","A","B","C","D","E","F","G",
    "H","I","J","K","L","M","N","O",
    "P","Q","R","S","T","U","V","W",
    "X","Y","Z","[6B]","[6C]","[6D]","[6E]","[6F]",
    "[70]","[71]","[72]","[73]","[74]","[75]","[76]","[77]",
    "[78]","[79]","[7A]","[7B]","[7C]","ASC ","VAL ","LEN ",
    "[80]","AND ",">=","<=","<>","OR ","NOT ","SQR ",
    "CHR$ ","COM$ ","INKEY$ ","STR$ ","LEFT$ ","RIGHT$ ","MID$ ","[8F]",
    "TO ","STEP ","THEN ","RANDOM ","[94]","WAIT ","ERROR ","[97]",
    "[98]","KEY ","[9A]","SETCOM ","[9C]","[9D]","ROM ","LPRINT ",
    "SIN ","COS ","TAN ","ASN ","ACS ","ATN ","EXP ","LN ",
    "LOG ","INT ","ABS ","SGN ","DEG ","DMS ","RND ","PEEK ",
    "RUN ","NEW ","MEM ","LIST ","CONT ","DEBUG ","CSAVE ","CLOAD ",
    "MERGE ","TRON ","TROFF ","PASS ","LLIST ","PI ","[BE]","[BF]",
    "GRAD ","PRINT ","INPUT ","RADIAN ","DEGREE ","CLEAR ","[C6]","[C7]",
    "[C8]","CALL ","DIM ","DATA ","ON ","OFF ","POKE ","READ ",
    "IF ","FOR ","LET ","REM ","END ","NEXT ","STOP ","GOTO ",
    "GOSUB ","CHAIN ","PAUSE ","BEEP ","AREAD ","USING ","RETURN ","RESTORE ",
    "[E0]","[E1]","[E2]","[E3]","[E4]","[E5]","[E6]","[E7]",
    "[E8]","[E9]","[EA]","[EB]","[EC]","[ED]","[EE]","[EF]",
    "[F0]","[F1]","[F2]","[F3]","[F4]","[F5]","[F6]","[F7]",
    "[F8]","[F9]","[FA]","[FB]","[FC]","[FD]","[FE]","[FF]",
};

static char* CodeNew[] = {
    "[00]","[01]","[02]","[03]","[04]","[05]","[06]","[07]",
    "[08]","[09]","[0A]","[0B]","[0C]","[0D]","[0E]","[0F]",
    "[10]","[11]","[12]","[13]","[14]","[15]","[16]","[17]",
    "[18]","[19]","[1A]","[1B]","[1C]","[1D]","[1E]","[1F]",
    " ","!","\"","#","$","%","&","'",
    "(",")","*","+",",","-",".","/",
    "0","1","2","3","4","5","6","7",
    "8","9",":",";","<","=",">","?",
    "@","A","B","C","D","E","F","G",
    "H","I","J","K","L","M","N","O",
    "P","Q","R","S","T","U","V","W",
    "X","Y","Z","[","\\","]","^","_",
    "`","a","b","c","d","e","f","g",
    "h","i","j","k","l","m","n","o",
    "p","q","r","s","t","u","v","w",
    "x","y","z","{","|","}","~","[7F]",
    "MDF ","REC ","POL ","ROT ","DECI ","HEX ","TEN ","RCP ",
    "SQU ","CUR ","HSN ","HCS ","HTN ","AHS ","AHC ","AHT ",
    "FACT ","LN ","LOG ","EXP ","SQR ","SIN ","COS ","TAN ",
    "INT ","ABS ","SGN ","DEG ","DMS ","ASN ","ACS ","ATN ",
    "RND ","AND ","OR ","NOT ","ASC ","VAL ","LEN ","PEEK ",
    "CHR$ ","STR$ ","MID$ ","LEFT$ ","RIGHT$ ","INKEY$ ","PI ","MEM ",
    "RUN ","NEW ","CONT ","PASS ","LIST ","LLIST ","CSAVE ","CLOAD ",
    "MERGE ","EQU# ","MEM# ","OPEN ","CLOSE ","SAVE ","LOAD ","CONSOLE ",
    "RANDOM ","DEGREE ","RADIAN ","GRAD ","BEEP ","WAIT ","GOTO ","TRON ",
    "TROFF ","CLEAR ","USING ","DIM ","CALL ","POKE ","CLS ","CURSOR ",
    "TO ","STEP ","THEN ","ON ","IF ","FOR ","LET ","REM ",
    "END ","NEXT ","STOP ","READ ","DATA ","PAUSE ","PRINT ","INPUT ",
    "GOSUB ","AREAD ","LPRINT ","RETURN ","RESTORE","CHAIN ","GCURSOR ","GPRINT ",
    "LINE ","POINT ","PSET ","PRESET ","BASIC ","TEXT ","OPEN$ ","[EF]",
    "[F0]","[F1]","[F2]","[F3]","[F4]","","","",
    "","€","","„","˚","[FD]","[FE]","[FF]",
};

static char* CodeExt[] = {
    "[00]","[01]","[02]","[03]","[04]","[05]","[06]","[07]",
    "[08]","[09]","[0A]","[0B]","[0C]","[0D]","[0E]","[0F]",
    "RUN ","NEW ","CONT ","PASS ","LIST ","LLIST ","CLOAD ","MERGE ",
    "LOAD ","RENUM ","[1A]","DELETE ","FILES ","INIT ","CONVERT ","[1F]",
    "CSAVE ","OPEN ","CLOSE ","SAVE ","CONSOLE ","RANDOM ","DEGREE ","RADIAN ",
    "GRAD ","BEEP ","WAIT ","GOTO ","TRON ","TROFF ","CLEAR ","USING ",
    "DIM ","CALL ","POKE ","GPRINT ","PSET ","PRESET ","BASIC ","TEXT ",
    "WIDTH ","[39]","ERASE ","LFILES ","KILL ","COPY ","NAME ","SET ",
    "LTEXT ","GRAPH ","LF ","CSIZE ","COLOR ","[45]","DEFDBL ","DEFSNG ",
    "FIELD ","PUT ","GET ","LSET ","RSET ","[4D]","[4E]","[4F]",
    "CLS ","CURSOR ","TO ","STEP ","THEN ","ON ","IF ","FOR ",
    "LET ","REM ","END ","NEXT ","STOP ","READ ","DATA ","PAUSE ",
    "PRINT ","INPUT ","GOSUB ","AREAD ","LPRINT ","RETURN ","RESTORE ","CHAIN ",
    "GCURSOR ","LINE ","LLINE ","RLINE ","GLCURSOR ","SORGN ","CROTATE ","CIRCLE ",
    "PAINT ","OUTPUT ","APPEND ","AS ","ARUN ","AUTOGOTO ","[76]","[77]",
    "ERROR ","[79]","[7A]","[7B]","[7C]","[7D]","[7E]","[7F]",
    "MDF ","REC ","POL ","ROT ","DECI ","HEX ","TEN ","RCP ",
    "SQU ","CUR ","HSN ","HCS ","HTN ","AHS ","AHC ","AHT ",
    "FACT ","LN ","LOG ","EXP ","SQR ","SIN ","COS ","TAN ",
    "INT ","ABS ","SGN ","DEG ","DMS ","ASN ","ACS ","ATN ",
    "RND ","AND ","OR ","NOT ","PEEK ","XOR ","[A6]","[A7]",
    "[A8]","[A9]","[AA]","[AB]","[AC]","POINT ","PI ","MEM ",
    "EOF ","DSKF ","LOF ","LOC ","[B4]","[B5]","NCR ","NPR",
    "[B8]","[B9]","[BA]","[BB]","[BC]","[BD]","[BE]","CUB ",
    "ERN ","ERL ","[C2]","[C3]","[C4]","[C5]","[C6]","[C7]",
    "[C8]","[C9]","[CA]","[CB]","[CC]","[CD]","[CE]","[CF]",
    "ASC ","VAL ","LEN ","KLEN ","[D4]","[D5]","[D6]","[D7]",
    "[D8]","[D9]","[DA]","[DB]","[DC]","[DD]","[DE]","[DF]",
    "AKCNV$ ","KACNV$ ","JIS$ ","[E3]","[E4]","[E5]","[E6]","[E7]",
    "OPEN$ ","INKEY$ ","MID$ ","LEFT$ ","RIGHT$ ","KMID$ ","KLEFT$ ","KRIGHT$ ",
    "CHR$ ","STR$ ","HEX$ ","[F3]","[F4]","[F5]","[F6]","[F7]",
    "[F8]","[F9]","[FA]","[FB]","[FC]","[FD]","[FE]","[FF]",
};

static char* CodePc1500_1[] = {
    "[00]","[01]","[02]","[03]","[04]","[05]","[06]","[07]",
    "[08]","[09]","[0A]","[0B]","[0C]","[0D]","[0E]","[0F]",
    "[10]","[11]","[12]","[13]","[14]","[15]","[16]","[17]",
    "[18]","[19]","[1A]","[1B]","[1C]","[1D]","[1E]","[1F]",
    " ","!","\"","#","$","%","&","",
    "(",")","*","+",",","-",".","/",
    "0","1","2","3","4","5","6","7",
    "8","9",":",";","<","=",">","?",
    "@","A","B","C","D","E","F","G",
    "H","I","J","K","L","M","N","O",
    "P","Q","R","S","T","U","V","W",
    "X","Y","Z","˚","ù","„","^","_",
    "`","a","b","c","d","e","f","g",
    "h","i","j","k","l","m","n","o",
    "p","q","r","s","t","u","v","w",
    "x","y","z","{","|","}","~","[7F]",
    "[80]","[81]","[82]","[83]","[84]","[85]","[86]","[87]",
    "[88]","[89]","[8A]","[8B]","[8C]","[8D]","[8E]","[8F]",
    "[90]","[91]","[92]","[93]","[94]","[95]","[96]","[97]",
    "[98]","[99]","[9A]","[9B]","[9C]","[9D]","[9E]","[9F]",
    "[A0]","[A1]","[A2]","[A3]","[A4]","[A5]","[A6]","[A7]",
    "[A8]","[A9]","[AA]","[AB]","[AC]","[AD]","[AE]","[AF]",
    "[B0]","[B1]","[B2]","[B3]","[B4]","[B5]","[B6]","[B7]",
    "[B8]","[B9]","[BA]","[BB]","[BC]","[BD]","[BE]","[BF]",
    "[C0]","[C1]","[C2]","[C3]","[C4]","[C5]","[C6]","[C7]",
    "[C8]","[C9]","[CA]","[CB]","[CC]","[CD]","[CE]","[CF]",
    "[D0]","[D1]","[D2]","[D3]","[D4]","[D5]","[D6]","[D7]",
    "[D8]","[D9]","[DA]","[DB]","[DC]","[DD]","[DE]","[DF]",
    "[E0]","[E1]","[E2]","[E3]","[E4]","[E5]","[E6]","[E7]",
    "[E8]","[E9]","[EA]","[EB]","[EC]","[ED]","[EE]","[EF]",
    "[F0]","[F1]","[F2]","[F3]","[F4]","[F5]","[F6]","[F7]",
    "[F8]","[F9]","[FA]","[FB]","[FC]","[FD]","[FE]","[FF]",
};

static char* CodePc1500_E6[] = {
    "[E600]","[E601]","[E602]","[E603]","[E604]","[E605]","[E606]","[E607]",
    "[E608]","[E609]","[E60A]","[E60B]","[E60C]","[E60D]","[E60E]","[E60F]",
    "[E610]","[E611]","[E612]","[E613]","[E614]","[E615]","[E616]","[E617]",
    "[E618]","[E619]","[E61A]","[E61B]","[E61C]","[E61D]","[E61E]","[E61F]",
    "[E620]","[E621]","[E622]","[E623]","[E624]","[E625]","[E626]","[E627]",
    "[E628]","[E629]","[E62A]","[E62B]","[E62C]","[E62D]","[E62E]","[E62F]",
    "[E630]","[E631]","[E632]","[E633]","[E634]","[E635]","[E636]","[E637]",
    "[E638]","[E639]","[E63A]","[E63B]","[E63C]","[E63D]","[E63E]","[E63F]",
    "[E640]","[E641]","[E642]","[E643]","[E644]","[E645]","[E646]","[E647]",
    "[E648]","[E649]","[E64A]","[E64B]","[E64C]","[E64D]","[E64E]","[E64F]",
    "[E650]","[E651]","[E652]","[E653]","[E654]","[E655]","[E656]","[E657]",
    "[E658]","[E659]","[E65A]","[E65B]","[E65C]","[E65D]","[E65E]","[E65F]",
    "[E660]","[E661]","[E662]","[E663]","[E664]","[E665]","[E666]","[E667]",
    "[E668]","[E669]","[E66A]","[E66B]","[E66C]","[E66D]","[E66E]","[E66F]",
    "[E670]","[E671]","[E672]","[E673]","[E674]","[E675]","[E676]","[E677]",
    "[E678]","[E679]","[E67A]","[E67B]","[E67C]","[E67D]","[E67E]","[E67F]",
    "CSIZE ","GRAPH ","GLCURSOR ","LCURSOR ","SORGN ","ROTATE ","TEXT ","[E687]",
    "[E688]","[E689]","[E68A]","[E68B]","[E68C]","[E68D]","[E68E]","[E68F]",
    "[E690]","[E691]","[E692]","[E693]","[E694]","[E695]","[E696]","[E697]",
    "[E698]","[E699]","[E69A]","[E69B]","[E69C]","[E69D]","[E69E]","[E69F]",
    "[E6A0]","[E6A1]","[E6A2]","[E6A3]","[E6A4]","[E6A5]","[E6A6]","[E6A7]",
    "[E6A8]","RMT ","[E6AA]","[E6AB]","[E6AC]","[E6AD]","[E6AE]","[E6AF]",
    "[E6B0]","[E6B1]","[E6B2]","[E6B3]","[E6B4]","[E6B5]","[E6B6]","[E6B7]",
    "[E6B8]","[E6B9]","[E6BA]","[E6BB]","[E6BC]","[E6BD]","[E6BE]","[E6BF]",
    "[E6C0]","[E6C1]","[E6C2]","[E6C3]","[E6C4]","[E6C5]","[E6C6]","[E6C7]",
    "[E6C8]","[E6C9]","[E6CA]","[E6CB]","[E6CC]","[E6CD]","[E6CE]","[E6CF]",
    "[E6D0]","[E6D1]","[E6D2]","[E6D3]","[E6D4]","[E6D5]","[E6D6]","[E6D7]",
    "[E6D8]","[E6D9]","[E6DA]","[E6DB]","[E6DC]","[E6DD]","[E6DE]","[E6DF]",
    "[E6E0]","[E6E1]","[E6E2]","[E6E3]","[E6E4]","[E6E5]","[E6E6]","[E6E7]",
    "[E6E8]","[E6E9]","[E6EA]","[E6EB]","[E6EC]","[E6ED]","[E6EE]","[E6EF]",
    "[E6F0]","[E6F1]","[E6F2]","[E6F3]","[E6F4]","[E6F5]","[E6F6]","[E6F7]",
    "[E6F8]","[E6F9]","[E6FA]","[E6FB]","[E6FC]","[E6FD]","[E6FE]","[E6FF]",
};

static char* CodePc1500_E8[] = {
    "[E800]","[E801]","[E802]","[E803]","[E804]","[E805]","[E806]","[E807]",
    "[E808]","[E809]","[E80A]","[E80B]","[E80C]","[E80D]","[E80E]","[E80F]",
    "[E810]","[E811]","[E812]","[E813]","[E814]","[E815]","[E816]","[E817]",
    "[E818]","[E819]","[E81A]","[E81B]","[E81C]","[E81D]","[E81E]","[E81F]",
    "[E820]","[E821]","[E822]","[E823]","[E824]","[E825]","[E826]","[E827]",
    "[E828]","[E829]","[E82A]","[E82B]","[E82C]","[E82D]","[E82E]","[E82F]",
    "[E830]","[E831]","[E832]","[E833]","[E834]","[E835]","[E836]","[E837]",
    "[E838]","[E839]","[E83A]","[E83B]","[E83C]","[E83D]","[E83E]","[E83F]",
    "[E840]","[E841]","[E842]","[E843]","[E844]","[E845]","[E846]","[E847]",
    "[E848]","[E849]","[E84A]","[E84B]","[E84C]","[E84D]","[E84E]","[E84F]",
    "[E850]","[E851]","[E852]","[E853]","[E854]","[E855]","[E856]","DEV$ ",
    "COM$ ","INSTAT ","RINKEY$ ","[E85B]","[E85C]","[E85D]","[E85E]","[E85F]",
    "[E860]","[E861]","[E862]","[E863]","[E864]","[E865]","[E866]","[E867]",
    "[E868]","[E869]","[E86A]","[E86B]","[E86C]","[E86D]","[E86E]","[E86F]",
    "[E870]","[E871]","[E872]","[E873]","[E874]","[E875]","[E876]","[E877]",
    "[E878]","[E879]","[E87A]","[E87B]","[E87C]","[E87D]","[E87E]","[E87F]",
    "OUTSTAT ","[E881]","SETCOM ","TERMINAL ","DTE ","TRANSMIT ","SETDEV ","[E887]",
    "[E888]","[E889]","[E88A]","[E88B]","[E88C]","[E88D]","[E88E]","[E88F]",
    "[E890]","[E891]","[E892]","[E893]","[E894]","[E895]","[E896]","[E897]",
    "[E898]","[E899]","[E89A]","[E89B]","[E89C]","[E89D]","[E89E]","[E89F]",
    "[E8A0]","[E8A1]","[E8A2]","[E8A3]","[E8A4]","[E8A5]","[E8A6]","[E8A7]",
    "[E8A8]","[E8A9]","[E8AA]","[E8AB]","[E8AC]","[E8AD]","[E8AE]","[E8AF]",
    "[E8B0]","[E8B1]","[E8B2]","[E8B3]","[E8B4]","[E8B5]","[E8B6]","[E8B7]",
    "[E8B8]","[E8B9]","[E8BA]","[E8BB]","[E8BC]","[E8BD]","[E8BE]","[E8BF]",
    "[E8C0]","[E8C1]","[E8C2]","[E8C3]","[E8C4]","[E8C5]","[E8C6]","[E8C7]",
    "[E8C8]","[E8C9]","[E8CA]","[E8CB]","[E8CC]","[E8CD]","[E8CE]","[E8CF]",
    "[E8D0]","[E8D1]","[E8D2]","[E8D3]","[E8D4]","[E8D5]","[E8D6]","[E8D7]",
    "[E8D8]","[E8D9]","[E8DA]","[E8DB]","[E8DC]","[E8DD]","[E8DE]","[E8DF]",
    "[E8E0]","[E8E1]","[E8E2]","[E8E3]","[E8E4]","[E8E5]","[E8E8]","[E8E7]",
    "[E8E8]","[E8E9]","[E8EA]","[E8EB]","[E8EC]","[E8ED]","[E8EE]","[E8EF]",
    "[E8F0]","[E8F1]","[E8F2]","[E8F3]","[E8F4]","[E8F5]","[E8F6]","[E8F7]",
    "[E8F8]","[E8F9]","[E8FA]","[E8FB]","[E8FC]","[E8FD]","[E8FE]","[E8FF]",
};

static char* CodePc1500_F0[] = {
    "[F000]","[F001]","[F002]","[F003]","[F004]","[F005]","[F006]","[F007]",
    "[F008]","[F009]","[F00A]","[F00B]","[F00C]","[F00D]","[F00E]","[F00F]",
    "[F010]","[F011]","[F012]","[F013]","[F014]","[F015]","[F016]","[F017]",
    "[F018]","[F019]","[F01A]","[F01B]","[F01C]","[F01D]","[F01E]","[F01F]",
    "[F020]","[F021]","[F022]","[F023]","[F024]","[F025]","[F026]","[F027]",
    "[F028]","[F029]","[F02A]","[F02B]","[F02C]","[F02D]","[F02E]","[F02F]",
    "[F030]","[F031]","[F032]","[F033]","[F034]","[F035]","[F036]","[F037]",
    "[F038]","[F039]","[F03A]","[F03B]","[F03C]","[F03D]","[F03E]","[F03F]",
    "[F040]","[F041]","[F042]","[F043]","[F044]","[F045]","[F046]","[F047]",
    "[F048]","[F049]","[F04A]","[F04B]","[F04C]","[F04D]","[F04E]","[F04F]",
    "VPCURSOR ","[F051]","ERN ","ERL ","HCURSOR ","HPCURSOR ","VCURSOR ","[F057]",
    "[F058]","[F059]","[F05A]","[F05B]","[F05C]","[F05D]","[F05E]","[F05F]",
    "[F060]","SPACE$ ","[F062]","[F063]","[F064]","[F065]","[F066]","[F067]",
    "[F068]","[F069]","[F06A]","[F06B]","[F06C]","[F06D]","[F06E]","[F06F]",
    "[F070]","HEX$ ","[F072]","[F073]","[F074]","[F075]","[F076]","[F077]",
    "[F078]","[F079]","[F07A]","[F07B]","[F07C]","[F07D]","[F07E]","[F07F]",
    "LOAD ","SAVE ","[F082]","[F083]","CURSOR ","USING ","[F086]","[F087]",
    "CLS ","CLOAD ","[F08A]","[F08B]","[F08C]","[F08D]","[F08E]","MERGE ",
    "LIST ","INPUT ","[F092]","GCURSOR ","[F094]","CSAVE ","[F096]","PRINT ",
    "[F098]","[F099]","[F09A]","[F09B]","[F09C]","[F09D]","[F09E]","GPRINT ",
    "[F0A0]","[F0A1]","[F0A2]","[F0A3]","[F0A4]","[F0A5]","[F0A6]","[F0A7]",
    "[F0A8]","RMT ","[F0AA]","[F0AB]","[F0AC]","[F0AD]","[F0AE]","[F0AF]",
    "FEED ","CONSOLE ","CHAIN ","[F0B3]","ZONE ","COLOR ","LF ","LINE ",
    "LLIST ","LPRINT ","RLINE ","TAB ","TEST ","[F0BD]","[F0BE]","[F0BF]",
    "[F0C0]","[F0C1]","[F0C2]","REPKEY ","[F0C4]","[F0C5]","[F0C6]","[F0C7]",
    "[F0C8]","[F0C9]","[F0CA]","[F0CB]","[F0CC]","[F0CD]","[F0CE]","[F0CF]",
    "[F0D0]","[F0D1]","[F0D2]","[F0D3]","[F0D4]","[F0D5]","[F0D6]","[F0D7]",
    "[F0D8]","[F0D9]","[F0DA]","[F0DB]","[F0DC]","[F0DD]","[F0DE]","MODE ",
    "[F0E0]","GCLS ","KEY ","[F0E3]","[F0E4]","[F0E5]","[F0E6]","SLEEP ",
    "[F0E8]","[F0E9]","[F0EA]","[F0EB]","[F0EC]","[F0ED]","[F0EE]","[F0EF]",
    "[F0F0]","[F0F1]","[F0F2]","[F0F3]","[F0F4]","[F0F5]","[F0F6]","[F0F7]",
    "[F0F8]","[F0F9]","[F0FA]","[F0FB]","[F0FC]","[F0FD]","[F0FE]","[F0FF]",
};

static char* CodePc1500_F1[] = {
    "[F100]","[F101]","[F102]","[F103]","[F104]","[F105]","[F106]","[F107]",
    "[F108]","[F109]","[F10A]","[F10B]","[F10C]","[F10D]","[F10E]","[F10F]",
    "[F110]","[F111]","[F112]","[F113]","[F114]","[F115]","[F116]","[F117]",
    "[F118]","[F119]","[F11A]","[F11B]","[F11C]","[F11D]","[F11E]","[F11F]",
    "[F120]","[F121]","[F122]","[F123]","[F124]","[F125]","[F126]","[F127]",
    "[F128]","[F129]","[F12A]","[F12B]","[F12C]","[F12D]","[F12E]","[F12F]",
    "[F130]","[F131]","[F132]","[F133]","[F134]","[F135]","[F136]","[F137]",
    "[F138]","[F139]","[F13A]","[F13B]","[F13C]","[F13D]","[F13E]","[F13F]",
    "[F140]","[F141]","[F142]","[F143]","[F144]","[F145]","[F146]","[F147]",
    "[F148]","[F149]","[F14A]","[F14B]","[F14C]","[F14D]","[F14E]","[F14F]",
    "AND ","OR ","[F152]","[F153]","[F154]","[F155]","[F156]","[F157]",
    "MEM ","[F159]","[F15A]","TIME ","INKEY$ ","PI ","[F15E]","[F15F]",
    "ASC ","STR$ ","VAL ","CHR$ ","LEN ","DEG ","DMS ","STATUS ",
    "POINT ","[F169]","[F16A]","SQR ","[F16C]","NOT ","PEEK# ","PEEK ",
    "ABS ","INT ","RIGHT$ ","ASN ","ACS ","ATN ","LN ","LOG ",
    "EXP ","SGN ","LEFT$ ","MID$ ","RND ","SIN ","COS ","TAN ",
    "AREAD ","ARUN ","BEEP ","CONT ","[F184]","[F185]","GRAD ","CLEAR ",
    "[F188]","[F189]","CALL ","DIM ","DEGREE ","DATA ","END","[F18F]",
    "[F190]","[F191]","GOTO ","[F193]","GOSUB ","[F195]","IF ","[F197]",
    "LET ","RETURN ","NEXT ","NEW ","ON ","OPN ","OFF ","[F19F]",
    "POKE# ","POKE ","PAUSE ","[F1A3]","RUN ","FOR ","READ ","RESTORE ",
    "RANDOM ","[F1A9]","RADIAN ","REM ","STOP ","STEP ","THEN ","TRON ",
    "TROFF ","TO ","[F1B2]","WAIT ","ERROR ","LOCK ","UNLOCK ","[F1B7]",
    "[F1B8]","[F1B9]","[F1BA]","[F1BB]","[F1BC]","[F1BD]","[F1BE]","[F1BF]",
    "[F1C0]","[F1C1]","[F1C2]","[F1C3]","[F1C4]","[F1C5]","[F1C6]","[F1C7]",
    "[F1C8]","[F1C9]","[F1CA]","[F1CB]","[F1CC]","[F1CD]","[F1CE]","[F1CF]",
    "[F1D0]","[F1D1]","[F1D2]","[F1D3]","[F1D4]","[F1D5]","[F1D6]","[F1D7]",
    "[F1D8]","[F1D9]","[F1DA]","[F1DB]","[F1DC]","[F1DD]","[F1DE]","[F1DF]",
    "[F1E0]","[F1E1]","[F1E2]","[F1E3]","[F1E4]","[F1E5]","[F1E6]","[F1E7]",
    "[F1E8]","[F1E9]","[F1EA]","[F1EB]","[F1EC]","[F1ED]","[F1EE]","[F1EF]",
    "[F1F0]","[F1F1]","[F1F2]","[F1F3]","[F1F4]","[F1F5]","[F1F6]","[F1F7]",
    "[F1F8]","[F1F9]","[F1FA]","[F1FB]","[F1FC]","[F1FD]","[F1FE]","[F1FF]",
};

int FindFreqAndSyncFromWav (FileInfo*  ptrFile);

void CvStringIToShort (char*    ptrStr,
                       u_short* ptrVal)
{
    /* Convert the String to a short value with msb first (INTEL) */
    *ptrVal =  (u_short) ptrStr[0] & 0xFF ;
    *ptrVal += ((u_short) ptrStr[1] & 0xFF) << 8 ;
}


void CvStringIToLong (char*   ptrStr,
                      u_long* ptrVal)
{
    u_long      tmp ;

    /* Convert the String to a long value with msb first (INTEL) */
    *ptrVal =  (u_short) ptrStr[0] & 0xFF ;
    *ptrVal += ((u_short) ptrStr[1] & 0xFF) << 8 ;
    *ptrVal += ((u_short) ptrStr[2] & 0xFF) << 16 ;
    *ptrVal += ((u_short) ptrStr[3] & 0xFF) << 24 ;
}


int WriteByteToFile (char   byte,
                     FILE** ptrFd)
{
    int         error ;

    error = ERR_OK ;
    if (*ptrFd != NULL) {
        error = fputc (byte, *ptrFd) ;
        if (error == EOF) {
            printf ("\nERROR : Can't write in the file\n") ;
            error = ERR_NOK ;
        }
        else
            error = ERR_OK ;
    }
    return (error);
}


int WriteStringToFile (char*  ptrStr,
                       FILE** ptrFd)
{
    int         error ;

    error = ERR_OK ;
    if (*ptrFd != NULL) {
        error = fputs (ptrStr, *ptrFd) ;
        if (error == EOF) {
            printf ("\nERROR : Can't write in the file\n") ;
            error = ERR_NOK ;
        }
        else
            error = ERR_OK ;
    }
    return (error);
}


int ReadStringFromFile (char*  ptrStr,
                        long   nb,
                        FILE** ptrFd)
{
    int         ii ;
    int         inVal ;
    int         error ;

    error = ERR_OK ;

    for ( ii = 0 ; ii < nb ; ii++  ) {
        inVal = fgetc (*ptrFd) ;
        if (inVal == EOF) {
            printf ("\nERROR : Can't read over End Of File\n") ;
            error = ERR_NOK ;
            break ;
        }
        *ptrStr++ = (char) inVal ;
    }
    *ptrStr = 0 ;
    return (error);
}


int ReadLongFromFile (u_long* ptrVal,
                      FILE**  ptrFd)
{
    char        str[10] ;
    int         error ;

    error = ReadStringFromFile (str, 4, ptrFd) ;
    CvStringIToLong (str, ptrVal) ;

    return (error);
}


int ReadShortFromFile (u_short* ptrVal,
                       FILE**   ptrFd)
{
    char        str[10] ;
    int         error ;

    error = ReadStringFromFile (str, 2, ptrFd) ;
    CvStringIToShort (str, ptrVal) ;

    return (error);
}


int ReadWavHead (u_long*   ptrFreq,
                 FileInfo* ptrFile)
{
    char        str[20] ;
    u_long      tmpL ;
    u_short     tmpS ;
    int         error ;

    do {
        error = ReadStringFromFile (str, 4, &ptrFile->ptrFd) ;
        if (error != ERR_OK) break ;
        if (strcmp (str, "RIFF") != 0) {
            printf ("\nERROR : Header isn't 'RIFF'\n") ;
            error = ERR_NOK ;
            break ;
        }

        error = ReadLongFromFile (&tmpL, &ptrFile->ptrFd) ;
        if (error != ERR_OK) break ;

        error = ReadStringFromFile (str, 8, &ptrFile->ptrFd) ;
        if (error != ERR_OK) break ;
        if (strcmp (str, "WAVEfmt ") != 0) {
            printf ("\nERROR : Header format isn't 'WAVEfmt '\n") ;
            error = ERR_NOK ;
            break ;
        }

        error = ReadLongFromFile (&tmpL, &ptrFile->ptrFd) ;
        if (error != ERR_OK) break ;

        error = ReadShortFromFile (&tmpS, &ptrFile->ptrFd) ;
        if (error != ERR_OK) break ;
        if (tmpS != 1) {
            printf ("\nERROR : Format isn't PCM\n") ;
            error = ERR_NOK ;
            break ;
        }

        error = ReadShortFromFile (&tmpS, &ptrFile->ptrFd) ;
        if (error != ERR_OK) break ;
        if (tmpS != 1) {
            printf ("\nERROR : Format isn't Mono\n") ;
            error = ERR_NOK ;
            break ;
        }

        error = ReadLongFromFile (ptrFreq, &ptrFile->ptrFd) ;
        if (error != ERR_OK) break ;
        error = ReadLongFromFile (&tmpL, &ptrFile->ptrFd) ;
        if (error != ERR_OK) break ;

        error = ReadShortFromFile (&tmpS, &ptrFile->ptrFd) ;
        if (error != ERR_OK) break ;
        if (tmpS != 1) {
            printf ("\nERROR : Format isn't 1 byte / sample\n") ;
            error = ERR_NOK ;
            break ;
        }

        error = ReadShortFromFile (&tmpS, &ptrFile->ptrFd) ;
        if (error != ERR_OK) break ;
        if (tmpS != 8) {
            printf ("\nERROR : Format isn't 8 bits / sample\n") ;
            error = ERR_NOK ;
            break ;
        }

        error = ReadStringFromFile (str, 4, &ptrFile->ptrFd) ;
        if (error != ERR_OK) break ;
        if (strcmp (str, "data") != 0) {
            error = fseek (ptrFile->ptrFd, -2, SEEK_CUR) ;
            if (error != ERR_OK) {
                printf ("\nERROR : Can't seek the file\n") ;
                break ;
            }

            error = ReadStringFromFile (str, 4, &ptrFile->ptrFd) ;
            if (error != ERR_OK) break ;
            if (strcmp (str, "data") != 0) {
                error = ReadStringFromFile (str, 8, &ptrFile->ptrFd) ;
                if (error != ERR_OK) break ;

                error = ReadStringFromFile (str, 4, &ptrFile->ptrFd) ;
                if (error != ERR_OK) break ;

                if (strcmp (str, "data") != 0) {
                    printf ("\nERROR : Data Header isn't 'data'\n") ;
                    error = ERR_NOK ;
                    break ;
                }
            }
        }

        error = ReadLongFromFile (&tmpL, &ptrFile->ptrFd) ;
        if (error != ERR_OK) break ;

    } while (0) ;
    return (error);
}


int ReadBitFromWav (u_long*   ptrTrans,
                    FileInfo* ptrFile)
{
    int         sign ;
    int         inVal ;
    int         ii ;
    int         error ;

    error = ERR_OK ;
    *ptrTrans = 0;
    sign = 1;
    for ( ii = 0 ; ii < ptrFile->bitLen ; ii++ ) {
        inVal = fgetc (ptrFile->ptrFd) ;
        if (inVal == EOF) {
            printf ("\nEnd of File\n") ;
            error = ERR_NOK ;
            break ;
        }
        if (sign > 0) {
            if (inVal > TRANS_HIGH) {
                continue ;
            }
            if (inVal < TRANS_LOW) {
                sign = -1;
                *ptrTrans = *ptrTrans + 1 ;
            }
        }
        else {
            if (inVal < TRANS_LOW) {
                continue ;
            }
            if (inVal > TRANS_HIGH) {
                sign = 1;
                *ptrTrans = *ptrTrans + 1 ;
            }
        }
    }

    if ( (ptrFile->debug & 0x0001) > 0 )
        printf(" %d", *ptrTrans);

    return (error);
}


int SyncBitFromWav (int       sign,
                    u_long*   ptrTrans,
                    u_long*   ptrLimit,
                    FileInfo* ptrFile)
{
    int         inVal ;
    int         ii ;
    int         tmp ;
    int         error ;

    if ( (ptrFile->debug & 0x0002) > 0 )
        printf(" %d -> ", *ptrTrans);

    ii    = 1 ;
    do {
        if ( (ii % 2) == 0 )
            tmp = - ptrFile->bitLen + ii ;
        else
            tmp = - ptrFile->bitLen - ii ;

        error = fseek (ptrFile->ptrFd, tmp, SEEK_CUR) ;
        if (error != ERR_OK) {
            printf ("\nERROR : Can't seek the file\n") ;
            break ;
        }
        error = ReadBitFromWav (ptrTrans, ptrFile);
        if (error != ERR_OK) break ;

        ii++;

    } while ( (ii < *ptrLimit) &&
              ( ( (sign > 0) && (*ptrTrans < BIT_1) ) ||
                ( (sign < 0) && (*ptrTrans > BIT_0) ) ) ) ;

    if ( (ptrFile->debug & 0x0002) > 0 ) {
        printf("%d bits -> ", tmp + ptrFile->bitLen);
        printf("%d\n", *ptrTrans);
    }

    if ( (*ptrTrans < BIT_1) &&
         (*ptrTrans > BIT_0) )
        *ptrLimit = 0 ;
    else
        *ptrLimit = ii ;

    return (error);
}


int ReadBitFieldFromWav (u_long    nbBits,
                         char*     ptrBits,
                         FileInfo* ptrFile)
{
    int     ii ;
    u_long  trans ;
    u_long  limit ;
    int     error ;

    if ( (ptrFile->debug & 0x0004) > 0 )
        printf (" Bits") ;

    for ( ii = 0 ; ii < nbBits ; ii++ ) {
        error = ReadBitFromWav (&trans, ptrFile);
        if (error != ERR_OK) break ;

        if (trans > BIT_MID) {
            if (trans < BIT_1) {
                limit = ptrFile->bitLen ;
                error = SyncBitFromWav (1, &trans, &limit, ptrFile);
                if (error != ERR_OK) break ;
                if (limit == 0) {
                    printf ("\nERROR : Synchro lost %d\n",trans) ;
                    error = ERR_NOK ;
                    break ;
                }
             }
        }
        else {
            if (trans > BIT_0 ) {
                limit = ptrFile->bitLen ;
                error = SyncBitFromWav (-1, &trans, &limit, ptrFile);
                if (error != ERR_OK) break ;
                if (limit == 0) {
                    printf ("\nERROR : Synchro lost %d\n",trans) ;
                    error = ERR_NOK ;
                    break ;
                }
            }
            else if ( trans < BIT_0 / 2 ) {
                printf ("\nERROR : Synchro lost %d\n",trans) ;
                error = ERR_NOK ;
                break ;
            }
        }
        if (error != ERR_OK) {
            return FindFreqAndSyncFromWav( ptrFile );
        }

        if (trans > BIT_MID)
            ptrBits[ii] = 1;     /* Bit a 1 */
        else
            ptrBits[ii] = 0;     /* Bit a 0 */

        if ( (ptrFile->debug & 0x0004) > 0 )
            printf (" %d", ptrBits[ii]) ;
    }
    if ( (ptrFile->debug & 0x0004) > 0 )
        printf ("\n") ;

    return (error);
}


int SkipBitsFromWav (u_long*   ptrNb,
                     FileInfo* ptrFile)
{
    int     ii ;
    int     max_ii ;
    char    bit ;
    int     error ;
    u_long  position ;

    position = ftell( ptrFile->ptrFd );
    do {
        max_ii = ptrFile->synching ? 0x7fff : 250;
        *ptrNb = 0 ;
        ii = 0 ;
        do {
            ii++ ;
            error = ReadBitFieldFromWav (1, &bit, ptrFile) ;
            if (error != ERR_OK) break ;

        } while (bit > 0 && ii < max_ii ) ;
        if (error != ERR_OK) break ;

        if ( bit > 0 ) {
            /* new synch detected */
            error = fseek (ptrFile->ptrFd, position, SEEK_SET) ;
            return FindFreqAndSyncFromWav( ptrFile );
        }

        ii-- ;
        error = fseek (ptrFile->ptrFd, - ptrFile->bitLen, SEEK_CUR) ;
        if (error != ERR_OK) {
            printf ("\nERROR : Can't seek the file\n") ;
            break ;
        }

        *ptrNb = ii ;
        if ( (ii > 0) && ((ptrFile->debug & 0x0008) > 0 ) )
            printf (" Skip %d\n", ii) ;

    } while (0) ;

    return (error);
}


int FindSyncFromWav (u_long*    ptrNb,
                     FileInfo*  ptrFile)
{
    int         ii ;
    u_long      trans ;
    u_long      limit ;
    int         error ;

    do {
        ptrFile->synching = 1;

        /* Calculate the number of WAV bytes per bit */
        ptrFile->bitLen = (ptrFile->wav_freq * 8) / ptrFile->freq ;

        *ptrNb = 0 ;
        ii = 0;
        do {
            ii++;
            do {
                error = ReadBitFromWav (&trans, ptrFile);
                if (error != ERR_OK) break ;

                if ( (ptrFile->debug & 0x0100) > 0 )
                    printf (" Search %d\n", trans) ;

            } while (trans < (BIT_MID / 2) ) ;
            if (error != ERR_OK) break ;

            limit = ptrFile->bitLen ;
            error = SyncBitFromWav (1, &trans, &limit, ptrFile);
            if (error != ERR_OK) break ;

            if ( (ptrFile->debug & 0x0100) > 0 )
                printf (" Search %d\n", trans) ;

        } while ( (trans < BIT_1) && (ii < SYNC_NB_TRY) ) ;
        if (ii >= SYNC_NB_TRY) break;

        /* Skip the Synchro */
        error = SkipBitsFromWav (ptrNb, ptrFile);
        if (error != ERR_OK) break ;

        ptrFile->synching = 0;

    } while (0) ;

    return (error);
}


int FindFreqAndSyncFromWav (FileInfo*  ptrFile)
{
    u_long      length ;
    int         error ;

    do {
        if (ptrFile->wav_freq >= (BASE_FREQ1 * 2)) {
            /* Search the Synchro for the 4000 Hz frequency */
            ptrFile->freq = BASE_FREQ1 ;
            error = FindSyncFromWav (&length, ptrFile) ;
            if (error != ERR_OK) break ;
        }
        else
            length = 0 ;

        if (length == 0) {
            /* Search the Synchro for the 2500 Hz frequency */
            ptrFile->freq = BASE_FREQ2 ;
            error = FindSyncFromWav (&length, ptrFile) ;
            if (error != ERR_OK) break ;
        }

        if (length > 0) {
            length = (length * 80 + ptrFile->freq / 2) / ptrFile->freq ;
            printf ("Synchro found : %ld.%ld seconds",
                    length / 10, length % 10);
            printf (" at %ld Hz, last count : %ld, total : %ld\n",
                    ptrFile->freq, ptrFile->count, ptrFile->total );
        }
        else {
            printf ("\nERROR : No Synchro found\n");
            error = ERR_NOK ;
            break ;
        }
        ptrFile->count = 0;

    } while (0) ;

    return (error);
}


int ReadQuaterFromWav (u_long*   ptrQuat,
                       FileInfo* ptrFile)
{
    u_long  code ;
    char    bit[5] ;
    int     error ;

    do {
        *ptrQuat = 0 ;
        error = ReadBitFieldFromWav (5, bit, ptrFile) ;
        if (error != ERR_OK) break ;

        code = (u_long) bit[4] ;
        code = (code << 1) + (u_long) bit[3] ;
        code = (code << 1) + (u_long) bit[2] ;
        code = (code << 1) + (u_long) bit[1] ;
        *ptrQuat = code ;

        if ( (ptrFile->debug & 0x0010) > 0 )
            printf(" %1X", code);

        error = SkipBitsFromWav (&code, ptrFile);
        if (error != ERR_OK) break ;

    } while (0) ;

    return (error);
}


int ReadByteFromWav (u_long    order,
                     u_long*   ptrByte,
                     FileInfo* ptrFile)
{
    u_long  lsq ;
    u_long  msq ;
    int     error ;

    do {
        *ptrByte = 0 ;
        if (order == ORDER_INV) {
            error = ReadQuaterFromWav (&lsq, ptrFile) ;
            if (error != ERR_OK) break ;
            error = ReadQuaterFromWav (&msq, ptrFile) ;
            if (error != ERR_OK) break ;
        }
        else {
            error = ReadQuaterFromWav (&msq, ptrFile) ;
            if (error != ERR_OK) break ;
            error = ReadQuaterFromWav (&lsq, ptrFile) ;
            if (error != ERR_OK) break ;
        }

        *ptrByte = (msq << 4) + lsq ;
        if ( (ptrFile->debug & 0x0020) > 0 )
            printf(" %02X", *ptrByte);

    } while (0) ;

    return (error);
}


int ReadByteSumFromWav (u_long    order,
                        u_long*   ptrByte,
                        FileInfo* ptrFile)
{
    u_long  sum ;
    int     error ;

    do {
        error = ReadByteFromWav (order, ptrByte, ptrFile);
        if (error != ERR_OK) break ;

        ptrFile->count = ptrFile->count + 1;
        ptrFile->total = ptrFile->total + 1;
        if ( (ptrFile->debug & 0x8000) != 0 )
            printf(" %d:", ptrFile->count);
        if ( (ptrFile->debug & 0x0040) > 0 )
            printf(" %02X", *ptrByte);

        if ( (ptrFile->debug & 0x0400) > 0 )
            printf("%d\n", *ptrByte);

        switch (ptrFile->ident) {
        case IDENT_PC15_BIN :
        case IDENT_PC15_BAS :
            if ( (ptrFile->count % 80) == 0) {
                error = ReadByteFromWav (order, &sum, ptrFile);
                if (error != ERR_OK) break ;
                error = ReadByteFromWav (order, &sum, ptrFile);
                if (error != ERR_OK) break ;
            }
            break ;

        case IDENT_PC1211 :
        case IDENT_OLD_BAS :
        case IDENT_OLD_PAS :
        case IDENT_OLD_BIN :
        case IDENT_DATA :
            if ( (ptrFile->count % 8) == 0) {
                error = ReadByteFromWav (order, &sum, ptrFile);
                if (error != ERR_OK) break ;
                if ( (ptrFile->debug & 0x0040) > 0 )
                    printf(" (%02X)", sum);
            }
            break ;

        case IDENT_NEW_BAS :
        case IDENT_NEW_PAS :
        case IDENT_EXT_BAS :
        case IDENT_EXT_PAS :
        case IDENT_NEW_BIN :
            if ( (ptrFile->count % 120) == 0) {
                error = ReadByteFromWav (order, &sum, ptrFile);
                if (error != ERR_OK) break ;
            }
            break ;

        default :
            printf ("\nERROR : Unknown Ident\n") ;
            break ;
        }

    } while (0) ;

    return (error);
}


void PrintSaveIdFromWav (u_long code)
{
    printf ("Wave format   : 0x%02X -> ", code) ;

    if (code == IDENT_PC1211)
        printf ("Basic, PC-1211\n") ;
    else if (code == IDENT_PC15_BAS)
        printf ("Basic, PC-1500\n") ;
    else if (code == IDENT_PC15_BIN)
        printf ("Binary, PC-1500\n") ;
    else if (code == IDENT_NEW_BAS)
        printf ("Basic, New\n") ;
    else if (code == IDENT_NEW_PAS)
        printf ("Basic with password, New\n") ;
    else if (code == IDENT_EXT_BAS)
        printf ("Basic, Extended\n") ;
    else if (code == IDENT_EXT_PAS)
        printf ("Basic with password, Extended\n") ;
    else if (code == IDENT_DATA)
        printf ("Data\n") ;
    else if (code == IDENT_NEW_BIN)
        printf ("Binary, New\n") ;
    else if (code == IDENT_OLD_BAS)
        printf ("Basic, Old\n") ;
    else if (code == IDENT_OLD_PAS)
        printf ("Basic with password, Old\n") ;
    else if (code == IDENT_OLD_BIN)
        printf ("Binary, Old\n") ;
    else
        printf ("Unknown\n") ;
}


int ReadSaveIdFromWav (FileInfo* ptrFile)
{
    u_long  code ;
    u_long  tmpL ;
    int     ii ;
    int     error ;

    do {
        ptrFile->ident = IDENT_UNKNOWN ;
        if (ptrFile->freq == BASE_FREQ2) {
            /* Ident for 2500 Hz, 4 bits */
            error = ReadQuaterFromWav (&code, ptrFile) ;
            if (error != ERR_OK) break ;

            if (code == IDENT_PC1500) {
                for ( ii = 0 ; ii < 8 ; ii++ ) {
                    error = ReadByteFromWav (ORDER_INV, &tmpL, ptrFile);
                    if (error != ERR_OK) break ;
                }
                if (error != ERR_OK) break ;

                error = ReadByteFromWav (ORDER_INV, &tmpL, ptrFile);
                if (error != ERR_OK) break ;

                code = ((code << 4) & 0xF0) + (tmpL & 0x0F) ;
            }
        }

        if (ptrFile->freq == BASE_FREQ1) {
            /* Ident for 4000 Hz, 8 bits */
            error = ReadByteFromWav (ORDER_STD, &code, ptrFile) ;
            if (error != ERR_OK) break ;
        }

        ptrFile->ident = code ;

    } while (0) ;

    return (error);
}


int ReadSaveNameOrPassFromWav (char*     ptrName,
                               FileInfo* ptrFile)
{
    int     ii ;
    u_long  byte ;
    u_long  tmpL ;
    char    tmpS[10] ;
    int     error ;

    do {
        ptrName[0] = 0 ;
        tmpS[0]    = 0 ;

        if ( (ptrFile->ident == IDENT_PC15_BIN) ||
             (ptrFile->ident == IDENT_PC15_BAS) ) {
            for ( ii = 0 ; ii < 16 ; ii++ ) {
                error = ReadByteFromWav (ORDER_INV, &byte, ptrFile);
                if (error != ERR_OK) break ;

                if (byte > 0)
                    strcat (ptrName, CodePc1500_1[byte]) ;
            }
            if (error != ERR_OK) break ;

            for ( ii = 0 ; ii < 9 ; ii++ ) {
                error = ReadByteFromWav (ORDER_INV, &byte, ptrFile);
                if (error != ERR_OK) break ;
            }
            if (error != ERR_OK) break ;
        }
        else {
            for ( ii = 0 ; ii < 9 ; ii++ ) {
                error = ReadByteFromWav (ORDER_INV, &byte, ptrFile);
                if (error != ERR_OK) break ;
                if ( (byte > 0) && (ii < 7) ) {
                    switch (ptrFile->ident) {
                    case IDENT_PC1211 :
                    case IDENT_OLD_BAS :
                    case IDENT_OLD_PAS :
                    case IDENT_OLD_BIN :
                        strcat (tmpS, CodeOld[byte]) ;
                        break ;

                    case IDENT_NEW_BAS :
                    case IDENT_NEW_PAS :
                    case IDENT_EXT_BAS :
                    case IDENT_EXT_PAS :
                    case IDENT_DATA :
                    case IDENT_NEW_BIN :
                        strcat (tmpS, CodeNew[byte]) ;
                        break ;

                    default :
                        printf ("\nERROR : Unknown Ident\n") ;
                        break ;
                    }
                }
            }
            if (error != ERR_OK) break ;

            tmpL = strlen (tmpS) ;
            for ( ii = 0 ; ii < tmpL ; ii++ )
                ptrName[ii] = tmpS[tmpL - ii - 1] ;
            ptrName[tmpL] = 0 ;
        }

    } while (0) ;

    return (error);
}


int ReadHeadFromBinWav (u_long*   ptrSize,
                        FileInfo* ptrFile)
{
    int     ii ;
    u_long  addr ;
    u_long  length ;
    u_long  tmpL[10] ;
    int     error ;

    do {
        for ( ii = 0 ; ii < 8 ; ii++ ) {
            error = ReadByteFromWav (ORDER_INV, &tmpL[ii], ptrFile);
            if (error != ERR_OK) break ;
        }
        if (error != ERR_OK) break ;

        if ( (ptrFile->ident == IDENT_PC15_BIN) ||
             (ptrFile->ident == IDENT_PC15_BAS) ) {
            addr = tmpL[0] ;
            addr = (addr << 8) + tmpL[1] ;
            length = tmpL[2] ;
            length = (length << 8) + tmpL[3] ;
        }
        else {
            addr   = tmpL[4] ;
            addr   = (addr << 8) + tmpL[5] ;
            length = tmpL[6] ;
            length = (length << 8) + tmpL[7] ;

            error = ReadByteFromWav (ORDER_INV, &tmpL[0], ptrFile);
            if (error != ERR_OK) break ;
        }

        if (ptrFile->ident != IDENT_PC15_BAS) {
            length = length + 1 ;
            printf ("Start Address : 0x%04X\n", addr);
            printf ("Buffer Length : %d bytes\n", length);
        }
        *ptrSize = length ;

    } while (0) ;

    return (error);
}


int ReadHeadFromDataWav (TypeInfo* ptrType,
                         FileInfo* ptrFile)
{
    int     ii ;
    u_long  tmpL[6] ;
    int     error ;

    do {
        for ( ii = 0 ; ii < 6 ; ii++ ) {
            error = ReadByteFromWav (ORDER_INV, &tmpL[ii], ptrFile);
            if (error != ERR_OK) break ;
        }
        if (error != ERR_OK) break ;

        ptrType->length = ( (u_short) tmpL[ 0 ] << 8 ) + tmpL[ 1 ] ;
        ptrType->dim1    = tmpL[ 2 ];
        ptrType->dim2    = tmpL[ 3 ];
        ptrType->itemLen = tmpL[ 4 ];
        ptrType->type    = tmpL[ 5 ];

    } while (0) ;

    return (error);
}


void ConvertByteToBas (char*     ptrText,
                       char*     ptrCode,
                       u_long    byte,
                       u_long    graph)
{
    u_long  tmp ;
    char    tmpC[10] ;

    if (graph > 0) {
        strcat (ptrText, ptrCode) ;
    }
    else {
        tmp = (u_long) *ptrCode ;
        if ( (tmp > 0x1F) && (tmp < 0x80) ) {
            strcat (ptrText, ptrCode) ;
        }
        else {
            sprintf(tmpC, "[%02X]", byte);
            strcat (ptrText, tmpC) ;
        }
    }
}


int ReadLineFromBasWav (u_long    order,
                        char*     ptrText,
                        u_long*   ptrLen,
                        FileInfo* ptrFile)
{
    int     ii ;
    u_long  byte ;
    u_long  byte2 ;
    u_long  length ;
    u_long  tmpL ;
    u_long  lineNb ;
    char    tmpC[10] ;
    int     error ;

    do {
        *ptrText = 0 ;
        length   = 0 ;

        error = ReadByteSumFromWav (order, &byte, ptrFile);
        if (error != ERR_OK) break ;

        if ( (ptrFile->ident == IDENT_PC1211)  ||
             (ptrFile->ident == IDENT_OLD_BAS) ||
             (ptrFile->ident == IDENT_OLD_PAS) ) {
            if (byte == BAS_OLD_EOF) {  /* End of OLD Basic File */
                error = ERR_NOK;
                break;
            }
        }

        if (ptrFile->ident == IDENT_PC15_BAS) {
            if (byte == BAS_1500_EOF) { /* End of PC-1500 Basic File */
                error = ERR_NOK;
                break;
            }
        }

        error = ReadByteSumFromWav (order, &byte2, ptrFile);
        if (error != ERR_OK) break ;

        /* calculate the line number */
        if ( (ptrFile->ident == IDENT_PC1211)  ||
             (ptrFile->ident == IDENT_OLD_BAS) ||
             (ptrFile->ident == IDENT_OLD_PAS) ) {
            tmpL = 100 * (byte & 0x0F) ;
            tmpL = tmpL + (10 * (byte2 >> 4)) + (byte2 & 0x0F) ;
        }
        else {
            tmpL = byte ;
            tmpL = (tmpL << 8) + byte2 ;
            if (tmpL == BAS_NEW_EOF) {  /* End of New, Ext Basic File */
                error = ERR_NOK;
                break;
            }
        }

        if (ptrFile->type == TYPE_IMG) {
            ptrText[length++] = (char) byte ;
            ptrText[length++] = (char) byte2 ;
        }
        else {
            sprintf(tmpC, "%ld ", tmpL);
            strcat (ptrText, tmpC) ;
        }

        /* Read the line */
        if ( (ptrFile->ident == IDENT_PC1211)  ||
             (ptrFile->ident == IDENT_OLD_BAS) ||
             (ptrFile->ident == IDENT_OLD_PAS) ) {
            do {
                error = ReadByteSumFromWav (order, &byte, ptrFile);
                if (error != ERR_OK) break ;

                if (ptrFile->type == TYPE_IMG) {
                    ptrText[length++] = (char) byte ;
                }
                else {
                    if (byte > 0) {
                        ConvertByteToBas (ptrText, CodeOld[byte], byte, ptrFile->graph) ;
                    }
                }

            } while (byte != 0) ;
        }
        else {
            /* Read the Line length */
            error = ReadByteSumFromWav (order, &tmpL, ptrFile);
            if (error != ERR_OK) break ;

            if (ptrFile->type == TYPE_IMG)
                ptrText[length++] = (char) tmpL ;

            for ( ii = 1 ; ii < tmpL ; ii ++) {
                error = ReadByteSumFromWav (order, &byte, ptrFile);
                if (error != ERR_OK) break ;

                if (ptrFile->type == TYPE_IMG) {
                    ptrText[length++] = (char) byte ;
                }
                else {
                    switch (ptrFile->ident) {
                    case IDENT_NEW_BAS :
                    case IDENT_NEW_PAS :
                        ConvertByteToBas (ptrText, CodeNew[byte], byte, ptrFile->graph) ;
                        break ;

                    case IDENT_EXT_BAS :
                    case IDENT_EXT_PAS :
                        if (byte == BAS_EXT_CODE) {
                            error = ReadByteSumFromWav (order, &byte, ptrFile);
                            if (error != ERR_OK) break ;

                            ii++ ;
                            strcat (ptrText, CodeExt[byte]) ;
                        }
                        else if (byte == BAS_EXT_LINE_NB) {
                            error = ReadByteSumFromWav (order, &byte, ptrFile);
                            if (error != ERR_OK) break ;
                            ii++ ;

                            error = ReadByteSumFromWav (order, &byte2, ptrFile);
                            if (error != ERR_OK) break ;
                            ii++ ;

                            lineNb = byte ;
                            lineNb = (lineNb << 8) + byte2 ;
                            sprintf(tmpC, "%ld ", lineNb);
                            strcat (ptrText, tmpC) ;
                        }
                        else {
                            ConvertByteToBas (ptrText, CodeNew[byte], byte, ptrFile->graph) ;
                        }
                        break ;

                    case IDENT_PC15_BAS :
                        if ( (byte != 0xE6) &&
                             (byte != 0xE7) &&
                             (byte != 0xE8) &&
                             (byte != 0xF0) &&
                             (byte != 0xF1) )
                            ConvertByteToBas (ptrText, CodePc1500_1[byte], byte, ptrFile->graph) ;
                        else {
                            ii++ ;
                            error = ReadByteSumFromWav (order, &byte2, ptrFile);
                            if (error != ERR_OK) break ;

                            if ( (byte == 0xE6) ||
                                 (byte == 0xE7) )
                                strcat (ptrText, CodePc1500_E6[byte2]) ;

                            if (byte == 0xE8)
                                strcat (ptrText, CodePc1500_E8[byte2]) ;

                            if (byte == 0xF0)
                                strcat (ptrText, CodePc1500_F0[byte2]) ;

                            if (byte == 0xF1)
                                strcat (ptrText, CodePc1500_F1[byte2]) ;
                        }
                        break ;

                    default :
                        printf ("\nERROR : Unknown Ident\n") ;
                        break ;
                    }
                }
            }
            /* Read the CR, not used here */
            error = ReadByteSumFromWav (order, &byte, ptrFile);
            if (error != ERR_OK) break ;

            if (ptrFile->type == TYPE_IMG)
                ptrText[length++] = (char) byte ;
        }

        *ptrLen  = length ;
        if (ptrFile->type != TYPE_IMG) {
            (void) strcat (ptrText, "\r\n") ;
            if ( (ptrFile->debug & 0x0080) > 0 )
                printf(" %s", ptrText);
        }

    } while (0) ;

    return (error);
}


void DebugReadEndFromBinWav (u_long    order,
                             FileInfo* ptrFile)
{
    int     ii ;
    u_long  byte ;
    int     error ;

    if ( (ptrFile->debug & 0x200) > 0) {
        printf ("\nEnd @%06lX ", ftell( ptrFile->ptrFd ) ) ;
        for ( ii = 0 ; ii < 0x10 ; ii ++) {
            /* Read a byte from the Wav file */
            error = ReadByteFromWav (order, &byte, ptrFile);
            if (error != ERR_OK) break ;

            if ( (ptrFile->debug & 0x70) == 0)
                printf(" %02X", byte);
        }
    }
}


int ConvertWavToBin (char*  ptrSrcFile,
                     char*  ptrDstFile,
                     u_long type,
                     u_long graph,
                     u_long debug)
{
    FileInfo    fileInfo ;
    TypeInfo    typeInfo ;
    FILE        *dstFd ;
    char        text[256] ;
    int         ii ;
    long        nbByte ;
    u_long      byte ;
    u_long      freq ;
    u_long      length ;
    u_long      order ;
    int         error ;

    do {
        fileInfo.ptrFd  = NULL ;
        fileInfo.ident  = IDENT_UNKNOWN ;
        fileInfo.freq   = 0 ;
        fileInfo.bitLen = 0 ;
        fileInfo.count  = 0 ;
        fileInfo.total  = 0 ;
        fileInfo.sum    = 0 ;
        fileInfo.type   = type ;
        fileInfo.graph  = graph ;
        fileInfo.debug  = debug ;
        fileInfo.synching = 0 ;
        dstFd       = NULL ;
        error       = ERR_OK ;

        /* Open the source file */
        fileInfo.ptrFd = fopen (ptrSrcFile, "rb") ;
        if (fileInfo.ptrFd == NULL) {
            printf ("\nERROR : Can't open the source file : %s\n",
                    ptrSrcFile) ;
            error = ERR_NOK ;
            break ;
        }

        /* Seek to the end of the source file */
        error = fseek (fileInfo.ptrFd, 0, SEEK_END) ;
        if (error != ERR_OK) {
            printf ("\nERROR : Can't seek the file\n") ;
            break ;
        }

        /* Get the length of the source file */
        nbByte = ftell (fileInfo.ptrFd) ;
        if (nbByte == ERR_NOK) {
            printf ("\nERROR : Can't ftell the file\n") ;
            error = ERR_NOK ;
            break ;
        }

        /* Seek to the begining of the source file */
        error = fseek (fileInfo.ptrFd, 0, SEEK_SET) ;
        if (error != ERR_OK) {
            printf ("\nERROR : Can't seek the file\n") ;
            break ;
        }

        if (nbByte <= 58) {
            printf ("\nERROR : Source file is not a valid WAV File\n") ;
            error = ERR_NOK ;
            break ;
        }

        /* Read the header of the source WAV file */
        error = ReadWavHead (&freq, &fileInfo) ;
        if (error != ERR_OK) break ;

        /* Check the lower frequency limit */
        if (freq < 5000) {
            printf ("\nERROR : Sampling frequency is lower than 5000 Hz : %d Hz\n", freq);
            error = ERR_NOK ;
            break ;
        }

        /* Check the higher frequency limit */
        if (freq > 22050) {
            printf ("\nERROR : Sampling frequency is greater than 22050 Hz : %d Hz\n", freq);
            error = ERR_NOK ;
            break ;
        }

        /* Search the Synchro and the Frequency of the save */
        fileInfo.wav_freq = freq;
        error = FindFreqAndSyncFromWav (&fileInfo) ;
        if (error != ERR_OK) break ;

        /* Read the Save Id */
        error = ReadSaveIdFromWav (&fileInfo);
        if (error != ERR_OK) break ;
        (void) PrintSaveIdFromWav (fileInfo.ident);

        /* Read the Save Name */
        error = ReadSaveNameOrPassFromWav (text, &fileInfo);
        if (error != ERR_OK) break ;
        printf ("Save name     : %s\n", text) ;

        /* Read the PassWord if necessary */
        if ( (fileInfo.ident == IDENT_OLD_PAS) ||
             (fileInfo.ident == IDENT_NEW_PAS) ||
             (fileInfo.ident == IDENT_EXT_PAS) ) {
            error = ReadSaveNameOrPassFromWav (text, &fileInfo);
            if (error != ERR_OK) break ;
            printf ("Password      : %s\n", text) ;
        }

        /* Compare the destination file to "NULL", if not open the file */
        if (strcmp (ptrDstFile, "NULL") != 0) {
            /* Open the destination file */
            dstFd = fopen (ptrDstFile, "wb") ;
            if (dstFd == NULL) {
                printf ("\nERROR : Can't open the destination file : %s\n",
                        ptrDstFile) ;
                error = ERR_NOK ;
                break ;
            }
            text[0] = 0 ;
        }
        else
            (void) strcpy (text, " (file = NULL)") ;

        /* Init the read counter */
        fileInfo.count  = 0 ;
        order = ORDER_STD ;

        switch (fileInfo.ident) {
        case IDENT_PC15_BIN :
            order = ORDER_INV ;

        case IDENT_OLD_BIN :
        case IDENT_NEW_BIN :
            /* Read the Header from Bin Wav file */
            error = ReadHeadFromBinWav (&length, &fileInfo);
            if (error != ERR_OK) break ;
            fileInfo.count  = 0 ;

            printf ("Output format : Binary%s\n", text) ;
            for ( ii = 0 ; ii < length ; ii ++) {
                /* Read a byte from the Wav file */
                error = ReadByteSumFromWav (order, &byte, &fileInfo);
                if (error != ERR_OK) break ;

                /* Write it in the destination file queue */
                error = WriteByteToFile (byte, &dstFd) ;
                if (error != ERR_OK) break ;
            }

            (void) DebugReadEndFromBinWav (order, &fileInfo);
            break ;

        case IDENT_DATA :
            while ( error == ERR_OK ) {
                /* Read the Header from Data Wav file */
                error = ReadHeadFromDataWav (&typeInfo, &fileInfo);
                if (error != ERR_OK) break ;
                fileInfo.count = 0 ;
                length = typeInfo.length;

                order = ORDER_INV ;

                /* write the header back to file */
                byte = ( typeInfo.length >> 8 ) & 0xff;
                error = WriteByteToFile (byte, &dstFd) ;
                if (error != ERR_OK) break ;

                byte = typeInfo.length & 0xff;
                error = WriteByteToFile (byte, &dstFd) ;
                if (error != ERR_OK) break ;

                byte = typeInfo.dim1;
                error = WriteByteToFile (byte, &dstFd) ;
                if (error != ERR_OK) break ;

                byte = typeInfo.dim2;
                error = WriteByteToFile (byte, &dstFd) ;
                if (error != ERR_OK) break ;

                byte = typeInfo.itemLen;
                error = WriteByteToFile (byte, &dstFd) ;
                if (error != ERR_OK) break ;

                byte = typeInfo.type;
                error = WriteByteToFile (byte, &dstFd) ;
                if (error != ERR_OK) break ;

                if ( length == DATA_VARIABLE ) {
                    /* Variable length block, read until end of data */
                    printf ("Variable length block\n") ;

                    length = 6;
                    do {
                        /* Read a byte from the Wav file */
                        error = ReadByteSumFromWav (order, &byte, &fileInfo);
                        if (error != ERR_OK) break ;

                        /* Write it in the destination file queue */
                        error = WriteByteToFile (byte, &dstFd) ;
                        if (error != ERR_OK) break ;

                        ++length;
                    }
                    while ( ( length - 7 ) % 8 != 0 || byte != DATA_EOF );
                    printf ("... length : %ld\n", length) ;
                }
                else {
                    length += 3;
                    printf ("Data block, length : %d\n",length) ;
                    for ( ii = 6 ; ii < length ; ++ii ) {
                        /* Read a byte from the Wav file */
                        error = ReadByteSumFromWav (order, &byte, &fileInfo);
                        if (error != ERR_OK) break ;

                        /* Write it in the destination file queue */
                        error = WriteByteToFile (byte, &dstFd) ;
                        if (error != ERR_OK) break ;
                    }
                }
            }
            (void) DebugReadEndFromBinWav (order, &fileInfo);
            break ;

        case IDENT_PC15_BAS :
            /* Read the Header from Basic Wav file */
            order = ORDER_INV ;
            error = ReadHeadFromBinWav (&length, &fileInfo);
            if (error != ERR_OK) break ;
            fileInfo.count  = 0 ;

        case IDENT_PC1211  :
        case IDENT_OLD_BAS :
        case IDENT_OLD_PAS :
        case IDENT_NEW_BAS :
        case IDENT_NEW_PAS :
        case IDENT_EXT_BAS :
        case IDENT_EXT_PAS :
            if (fileInfo.type == TYPE_IMG)
                printf ("Output format : Forced to Binary%s\n", text) ;
            else
                if (fileInfo.graph > 0)
                    printf ("Output format : ASCII, graphic%s\n", text) ;
                else
                    printf ("Output format : ASCII%s\n", text) ;

            do {
                /* Read the Basic line from the Wav file */
                error = ReadLineFromBasWav (order, text, &length, &fileInfo);
                if (error != ERR_OK) break ;

                if (fileInfo.type == TYPE_IMG) {
                    for ( ii = 0 ; ii < length ; ii ++) {
                        error = WriteByteToFile (text[ii], &dstFd) ;
                        if (error != ERR_OK) break ;
                    }
                }
                else {
                    /* Write it in the destination file */
                    error = WriteStringToFile (text, &dstFd) ;
                    if (error != ERR_OK) break ;
                }

            } while (1) ;

            (void) DebugReadEndFromBinWav (order, &fileInfo);
            break ;

        default :
            printf ("\nERROR : Unknown Ident\n") ;
            break ;
        }

    } while (0) ;

    /* Close the source file */
    if (fileInfo.ptrFd != NULL) {
        error = fclose (fileInfo.ptrFd) ;
        if (error != ERR_OK) {
            printf ("\nERROR : Can't close the source file\n") ;
        }
    }

    /* Close the destination file */
    if (dstFd != NULL) {
        error = fclose (dstFd) ;
        if (error != ERR_OK) {
            printf ("\nERROR : Can't close the destination file\n") ;
        }
    }

    return (error);
}


void PrintHelp (void)
{
    printf ("\n") ;
    printf ("Usage   : WAV2BIN srcfile dstfile [%stype] [%sgraph]\n",
             Token[0], Token[1]) ;
    printf ("\n") ;
    printf ("srcFile : Source file name (Wav file)\n") ;
    printf ("dstFile : Destination file name (Basic or Binary file)\n") ;
    printf ("type    : Destination file type for Basic source file :\n") ;
    printf ("           IMG -> Basic binary image, BAS -> Basic (default)\n") ;
    printf ("graph   : Special character conversion for BAS type :\n") ;
    printf ("           YES -> Convert, NO -> No conversion (default)\n") ;
    printf ("\n") ;
    printf ("Version : 1.40\n") ;
    printf ("Author  : Pocket -> www.pocketmuseum.com\n") ; /* Please do not remove */
}


void main (int argc, char* argv[])

{
    char    argS[8][100] = { "", "", "", "", "", "", "", "" } ;
    char    argD[8][100] = { "", "", "BAS", "NO", "0", "", "", "" } ;
    int     ii ;
    int     jj ;
    int     nbArg ;
    char    *ptrToken ;
    char    *ptrErr ;
    int     type ;
    int     graph ;
    u_long  debug ;

    do {

        /* Check if the number of arguments is correct */
        if ( (argc < 3) || (argc > 5) ) {
            PrintHelp() ;
            break ;
        }

        /* Extract the source file name */
        (void) strcpy (argD[0], argv[1]) ;

        /* Extract the destination file name */
        (void) strcpy (argS[1], argv[2]) ;
        (void) strupr (argS[1]) ;
        if (strcmp (argS[1], "NULL") == 0)
            (void) strcpy (argD[1], argS[1]) ;
        else
            (void) strcpy (argD[1], argv[2]) ;

        /* Extract the arguments */
        nbArg = argc - 3 ;
        for ( ii = 0 ; ii < nbArg ; ii++ ) {
            (void) strcpy (argS[ii], argv[ii + 3]) ;
            (void) strupr (argS[ii]) ;
        }

        for ( ii = 0 ; ii < nbArg ; ii++ ) {
            for ( jj = 0 ; jj < TOKEN_NB ; jj++ ) {
                ptrToken = strstr (argS[ii], Token[jj]) ;
                if (ptrToken == argS[ii]) {
                    (void) strcpy (argD[jj+2], argv[ii+3] + strlen (Token[jj])) ;
                    break ;
                }
            }
        }

        /* Check the destination type */
        type  = TYPE_NOK ;
        (void) strupr (argD[2]) ;
        if (strcmp (argD[2], "BAS") == 0)
            type = TYPE_BAS ;

        if (strcmp (argD[2], "IMG") == 0)
            type = TYPE_IMG ;

        if (type == TYPE_NOK) {
            printf ("\nERROR : Destination file type '%s%s' is not valid.\n", Token[0], argD[2]) ;
            PrintHelp ();
            break ;
        }

        /* Check the graphic conversion flag */
        graph = 0 ;
        if (type == TYPE_BAS) {
            (void) strupr (argD[3]) ;
            if (strcmp (argD[3], "YES") == 0)
                graph = 1 ;
        }

        /* Convert debug in a long */
        debug = strtol (argD[4], &ptrErr, 0) ;
        if (*ptrErr != 0) {
            debug = 0 ;
            break ;
        }

        /* Convert the Wav file */
        (void) ConvertWavToBin (argD[0], argD[1], type, graph, debug) ;

    } while (0) ;
}
