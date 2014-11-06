/*** PC-1350 Emulator [DEBUG.CC] **************************/
/* debugger                                               */
/**********************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <QDebug>


#include "common.h"
#include "Debug.h"
#include "pcxxxx.h"
#include "cpu.h"
#include "lh5801.h"
#include "sc61860.h"
#include "lcc/parser/parser.h"

//#include "inter.h"

DisAsmTbl	AsmTbl_lh5801[]={
    {		 2,"LII   %02X",	"LII"},					/* 00h */
    {		 2,"LIJ   %02X",	"LIJ"},
    {		 2,"LIA   %02X",	"LIA"},
    {		 2,"LIB   %02X",	"LIB"},
    {		 1,"IX",			"IX"},
    {		 1,"DX",			"DX"},
    {		 1,"IY",			"IY"},
    {		 1,"DY",			"DY"},
    {		 1,"MVW",			"MVW"},
    {		 1,"EXW",			"EXW"},
    {		 1,"MVB",			"MVB"},
    {		 1,"EXB",			"EXB"},
    {		 1,"ADN",			"ADN"},
    {		 1,"SBN",			"SBN"},
    {		 1,"ADW",			"ADW"},
    {		 1,"SBW",			"SBW"},
    {		 3,"LIDP  %02X%02X","LIDP"},					/* 10h */
    {		 2,"LIDL  %02X",	"LIDL"},
    {		 2,"LIP   %02X",	"LIP"},
    {		 2,"LIQ   %02X",	"LIQ"},
    {		 1,"ADB",			"ADB"},
    {		 1,"SBB",			"SBB"},
    {		 1,"???   (16)",	"???   (16)"},
    {		 1,"???   (17)",	"???   (17)"},
    {		 1,"MVWD",			"MVWD"},
    {		 1,"EXWD",			"EXWD"},
    {		 1,"MVBD",			"MVBD"},
    {		 1,"EXBD",			"EXBD"},
    {		 1,"SRW",			"SRW"},
    {		 1,"SLW",			"SLW"},
    {		 1,"FILM",			"FILM"},
    {		 1,"FILD",			"FILD"},
    {		 1,"LDP",			"LDP"},							/* 20h */
    {		 1,"LDQ",			"LDQ"},
    {		 1,"LDR",			"LDR"},
    {         1,"CLRA",          "CLRA"},
    {		 1,"IXL",			"IXL"},
    {		 1,"DXL",			"DXL"},
    {		 1,"IYS",			"IYS"},
    {		 1,"DYS",			"DYS"},
    {		 2,"JRNZP %02X",	"JRNZP"},
    {		 2,"JRNZM %02X",	"JRNZM"},
    {		 2,"JRNCP %02X",	"JRNCP"},
    {		 2,"JRNCM %02X",	"JRNCM"},
    {		 2,"JRP   %02X",	"JRP"},
    {		 2,"JRM   %02X",	"JRM"},
    {		 1,"???   (2E)",	"???   (2E)"},
    {		 2,"LOOP  %02X",	"LOOP"},
    {		 1,"STP",			"STP"},							// 30h
    {		 1,"STQ",			"STQ"},
    {		 1,"STR",			"STR"},
    {		 1,"???   (33)",	"???   (33)"},
    {		 1,"PUSH",			"PUSH"},
    {		 1,"DATA",			"DATA"},
    {		 1,"???	  (36)",	"???   (36)"},
    {		 1,"RTN",			"RTN"},
    {		 2,"JRZP  %02X",	"JRZP"},
    {		 2,"JRZM  %02X",	"JRZM"},
    {		 2,"JRCP  %02X",	"JRCP"},
    {		 2,"JRCM  %02X",	"JRCM"},
    {		 1,"???   (3C)",	"???   (3C)"},
    {		 1,"???   (3D)",	"???   (3D)"},
    {		 1,"???   (3E)",	"???   (3E)"},
    {		 1,"???   (3F)",	"???   (3F)"},
    {		 1,"INCI",			"INCI"},							/* 40h */
    {		 1,"DECI",			"DECI"},
    {		 1,"INCA",			"INCA"},
    {		 1,"DECA",			"DECA"},
    {		 1,"ADM",			"ADM"},
    {		 1,"SBM",			"SBM"},
    {		 1,"ANMA",			"ANMA"},
    {		 1,"ORMA",			"ORMA"},
    {		 1,"INCK",			"INCK"},
    {		 1,"DECK",			"DECK"},
    {		 1,"INCM",			"INCM"},
    {		 1,"DECM",			"DECM"},
    {         1,"IN A",			"IN A"},
    {		 1,"NOPW",			"NOPW"},
    {		 2,"WAIT  %02X",	"WAIT"},
    {		 1,"CUP",			"CUP"},
    {		 1,"INCP",			"INCP"},							/* 50h */
    {		 1,"DECP",			"DECP"},
    {		 1,"STD",			"STD"},
    {		 1,"MVDM",			"MVDM"},
    {		 1,"READM",			"READM"},
    {		 1,"MVMD",			"MVMD"},
    {		 1,"READ",			"READ"},
    {		 1,"LDD",			"LDD"},
    {		 1,"SWP",			"SWP"},
    {		 1,"LDM",			"LDM"},
    {		 1,"SL",			"SL"},
    {		 1,"POP",			"POP"},
    {		 1,"???   (5C)",	"???   (5C)"},
    {         1,"OUT A",			"OUT A"},
    {		 1,"???   (5E)",	"???   (5E)"},
    {		 1,"OUT F",			"OUT F"},
    {		 2,"ANIM  %02X",	"ANIM"},						// 60h
    {		 2,"ORIM  %02X",	"ORIM"},
    {		 2,"TSIM  %02X",	"TSIM"},
    {		 2,"CPIM  %02X",	"CPIM"},
    {		 2,"ANIA  %02X",	"ANIA"},
    {		 2,"ORIA  %02X",	"ORIA"},
    {		 2,"TSIA  %02X",	"TSIA"},
    {		 2,"CPIA  %02X",	"CPIA"},
    {		 1,"???   (68)",	"???   (68)"},
    {		-3,"CASE 2",		"CASE 2"},
    {		 1,"???   (6A)",	"???   (6A)"},
    {		 2,"TEST  %02X",	"TEST"},
    {		 1,"???   (6C)",	"???   (6C)"},
    {		 1,"???   (6D)",	"???   (6D)"},
    {		 1,"???   (6E)",	"???   (6E)"},
    {		 1,"CDN",			"CDN"},
    {		 2,"ADIM  %02X",	"ADIM"},						// 70h
    {		 2,"SBIM  %02X",	"SBIM"},
    {		 1,"???   (72)",	"???   (72)"},
    {		 1,"???   (73)",	"???   (73)"},
    {		 2,"ADIA  %02X",	"ADIA"},
    {		 2,"SBIA  %02X",	"SBIA"},
    {		 1,"???   (76)",	"???   (76)"},
    {		 1,"???   (77)",	"???   (77)"},
    {		 3,"CALL  %02X%02X","CALL"},
    {		 3,"JP    %02X%02X","JP"},
    {        -4,"CASE1 %02X-(%02X%02X)","CASE1"},
    {		 1,"???   (7B)",	"???   (7B)"},
    {		 3,"JPNZ  %02X%02X","JPNZ"},
    {		 3,"JPNC  %02X%02X","JPNC"},
    {		 3,"JPZ   %02X%02X","JPZ"},
    {		 3,"JPC   %02X%02X","JPC"},
    {		-1,"LP%02X",		"LP"},							// 80h
    {		-1,"LP%02X",		"LP"},
    {		-1,"LP%02X",		"LP"},
    {		-1,"LP%02X",		"LP"},
    {		-1,"LP%02X",		"LP"},
    {		-1,"LP%02X",		"LP"},
    {		-1,"LP%02X",		"LP"},
    {		-1,"LP%02X",		"LP"},
    {		-1,"LP%02X",		"LP"},
    {		-1,"LP%02X",		"LP"},
    {		-1,"LP%02X",		"LP"},
    {		-1,"LP%02X",		"LP"},
    {		-1,"LP%02X",		"LP"},
    {		-1,"LP%02X",		"LP"},
    {		-1,"LP%02X",		"LP"},
    {		-1,"LP%02X",		"LP"},

    {		-1,"LP%02X",		"LP"},
    {		-1,"LP%02X",		"LP"},
    {		-1,"LP%02X",		"LP"},
    {		-1,"LP%02X",		"LP"},
    {		-1,"LP%02X",		"LP"},
    {		-1,"LP%02X",		"LP"},
    {		-1,"LP%02X",		"LP"},
    {		-1,"LP%02X",		"LP"},
    {		-1,"LP%02X",	"LP"},
    {		-1,"LP%02X",	"LP"},
    {		-1,"LP%02X",	"LP"},
    {		-1,"LP%02X",	"LP"},
    {		-1,"LP%02X",	"LP"},
    {		-1,"LP%02X",	"LP"},
    {		-1,"LP%02X",	"LP"},
    {		-1,"LP%02X",	"LP"},

    {		-1,"LP%02X",	"LP"},
    {		-1,"LP%02X",	"LP"},
    {		-1,"LP%02X",	"LP"},
    {		-1,"LP%02X",	"LP"},
    {		-1,"LP%02X",	"LP"},
    {		-1,"LP%02X",	"LP"},
    {		-1,"LP%02X",	"LP"},
    {		-1,"LP%02X",	"LP"},
    {		-1,"LP%02X",	"LP"},
    {		-1,"LP%02X",	"LP"},
    {		-1,"LP%02X",	"LP"},
    {		-1,"LP%02X",	"LP"},
    {		-1,"LP%02X",	"LP"},
    {		-1,"LP%02X",	"LP"},
    {		-1,"LP%02X",	"LP"},
    {		-1,"LP%02X",	"LP"},

    {		-1,"LP%02X",	"LP"},
    {		-1,"LP%02X",	"LP"},
    {		-1,"LP%02X",	"LP"},
    {		-1,"LP%02X",	"LP"},
    {		-1,"LP%02X",	"LP"},
    {		-1,"LP%02X",	"LP"},
    {		-1,"LP%02X",	"LP"},
    {		-1,"LP%02X",	"LP"},
    {		-1,"LP%02X",	"LP"},
    {		-1,"LP%02X",	"LP"},
    {		-1,"LP%02X",	"LP"},
    {		-1,"LP%02X",	"LP"},
    {		-1,"LP%02X",	"LP"},
    {		-1,"LP%02X",	"LP"},
    {		-1,"LP%02X",	"LP"},
    {		-1,"LP%02X",	"LP"},

    {		 1,"INCJ",		"INCJ"},						// C0h
    {		 1,"DECJ","DECJ"},
    {		 1,"INCB","INCB"},
    {		 1,"DECB","DECB"},
    {		 1,"ADCM","ADCM"},
    {		 1,"SBCM","SBCM"},
    {		 1,"??? ","??? "},
    {		 1,"CPMA","CPMA"},
    {		 1,"INCL","INCL"},
    {		 1,"DECL","DECL"},
    {		 1,"INCN","INCN"},
    {		 1,"DECN","DECN"},
    {         1,"IN B ","IN B "},
    {		 1,"??? ","??? "},
    {		 1,"NOPT","NOPT"},
    {		 1,"??? ","??? "},

    {		 1,"SC  ","SC  "},
    {		 1,"RC  ","RC  "},
    {		 1,"SR  ","SR  "},
    {		 1,"??? ","??? "},
    {		 2,"ANID  %02X","ANID"},
    {		 2,"ORID  %02X","ORID"},
    {		 2,"TSID  %02X","TSID"},
    {		 1,"???   (D7)","???   (D7)"},
    {		 1,"LEAVE","LEAVE"},
    {		 1,"???   (D9)","???   (D9)"},
    {		 1,"EXAB","EXAB"},
    {		 1,"EXAM","EXAM"},
    {		 1,"???   (DC)","???   (DC)"},
    {         1,"OUT B","OUT B"},
    {		 1,"???   (DE)","???   (DE)"},
    {         1,"OUT C","OUT C"},

    {	    -2,"CAL%02X%02X","CAL"},
    {		-2,"CAL%02X%02X","CAL"},
    {		-2,"CAL%02X%02X","CAL"},
    {		-2,"CAL%02X%02X","CAL"},
    {		-2,"CAL%02X%02X","CAL"},
    {		-2,"CAL%02X%02X","CAL"},
    {		-2,"CAL%02X%02X","CAL"},
    {		-2,"CAL%02X%02X","CAL"},
    {		-2,"CAL%02X%02X","CAL"},
    {		-2,"CAL%02X%02X","CAL"},
    {		-2,"CAL%02X%02X","CAL"},
    {		-2,"CAL%02X%02X","CAL"},
    {		-2,"CAL%02X%02X","CAL"},
    {		-2,"CAL%02X%02X","CAL"},
    {		-2,"CAL%02X%02X","CAL"},
    {		-2,"CAL%02X%02X","CAL"},

    {		-2,"CAL%02X%02X","CAL"},
    {		-2,"CAL%02X%02X","CAL"},
    {		-2,"CAL%02X%02X","CAL"},
    {		-2,"CAL%02X%02X","CAL"},
    {		-2,"CAL%02X%02X","CAL"},
    {		-2,"CAL%02X%02X","CAL"},
    {		-2,"CAL%02X%02X","CAL"},
    {		-2,"CAL%02X%02X","CAL"},
    {		-2,"CAL%02X%02X","CAL"},
    {		-2,"CAL%02X%02X","CAL"},
    {		-2,"CAL%02X%02X","CAL"},
    {		-2,"CAL%02X%02X","CAL"},
    {		-2,"CAL%02X%02X","CAL"},
    {		-2,"CAL%02X%02X","CAL"},
    {		-2,"CAL%02X%02X","CAL"},
    {		-2,"CAL%02X%02X","CAL"}
    };

UINT32 Cdebug::DisAsm_File(UINT32 adr,FILE *fp)
{
	UINT32 Result;

	Result = DisAsm_1(adr);
	fprintf(fp,"%s",Buffer);
	return(Result);
}


void Cdebug::header(UINT32 adr,int l) {
    sprintf(Buffer,"%05X:",(uint) adr);
    for(int i=0;i<l;i++)
        sprintf(Buffer,"%s%02X",Buffer,(uint)pCPU->get_mem(adr+i,SIZE_8));
}

Cdebug::Cdebug(CCPU *parent)	: QObject(parent)
{							//[constructor]
    breakf		= 0;
    breakadr	= 0;
    debugged	= 0;	//break point(0:disable, 1:enable)
    isdebug		= 0;			//debug?(0:none, 1:debugging)

    pCPU = (CCPU *) parent;

    loadSymbolMap();
}

char *Cdebug::toSymbol(quint32 adr,int size)
{
    if (symbolMap.contains(adr)) {
        return symbolMap[adr]->toLbl().toLatin1().data();
    }
    else {
        sprintf(tmpSymbolLabel,"%.*x",size,adr & ( (1<<(size*4))-1));
        return (char*)tmpSymbolLabel;
    }
}

/*****************************************************************************/
/* Initializeing Debugger													 */
/*  ENTRY :none																 */
/*  RETURN:none																 */
/*****************************************************************************/
bool Cdebug::init(void)
{
	return true;
}

/*****************************************************************************/
/* Exitting Debugger														 */
/*  ENTRY :none																 */
/*  RETURN:none																 */
/*****************************************************************************/
bool Cdebug::exit(void)
{
	return true;
}



QString Csymbol::toLbl()
{
    if (name.contains("$$")){
        return name.mid(name.indexOf("$$")+2);
    }
    return name;
}
