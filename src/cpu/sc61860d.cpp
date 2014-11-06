#include "sc61860d.h"
#include "sc61860.h"
#include "lcc/parser/parser.h"

DisAsmTbl	AsmTbl_sc61860[]={
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


UINT32 Cdebug_sc61860::DisAsm_1(UINT32 adr)
{

    //const char	*reg[]={"A","IL","BA","I","X","Y","U","S"};
    char	l;
//	char	b[16];
    char	s[2000];
//	BYTE	t;
    BYTE	i;

    Buffer[0] = '\0';
    DasmAdr = adr;
    if(adr==20076) {
        adr=20076;
    }
    int a = pCPU->pPC->get_mem(adr,SIZE_8);
    l=abs(AsmTbl[a].len);
    switch(AsmTbl[pCPU->pPC->Get_PC(adr)].len)
    {
    case  1:sprintf(s,AsmTbl[pCPU->pPC->get_mem(adr,SIZE_8)].nim);
            break;
    case  2:sprintf(s,AsmTbl[pCPU->pPC->get_mem(adr,SIZE_8)].nim,pCPU->pPC->get_mem(adr+1,SIZE_8));
            break;
    case  3:sprintf(s,AsmTbl[pCPU->pPC->get_mem(adr,SIZE_8)].nim,pCPU->pPC->get_mem(adr+1,SIZE_8),pCPU->pPC->get_mem(adr+2,SIZE_8));
            break;
    case  4:sprintf(s,AsmTbl[pCPU->pPC->get_mem(adr,SIZE_8)].nim,pCPU->pPC->get_mem(adr+1,SIZE_8),pCPU->pPC->get_mem(adr+2,SIZE_8),pCPU->pPC->get_mem(adr+3,SIZE_8));
            break;

    case -1:sprintf(s,AsmTbl[pCPU->pPC->get_mem(adr,SIZE_8)].nim,(pCPU->pPC->get_mem(adr,SIZE_8)&0x3F));
            break;
    case -2:sprintf(s,AsmTbl[pCPU->pPC->get_mem(adr,SIZE_8)].nim,(pCPU->pPC->get_mem(adr,SIZE_8)&0x1F),pCPU->pPC->get_mem(adr+1,SIZE_8));
            break;
    case -4:sprintf(s,AsmTbl[pCPU->pPC->get_mem(adr,SIZE_8)].nim,pCPU->pPC->get_mem(adr+1,SIZE_8),pCPU->pPC->get_mem(adr+2,SIZE_8),pCPU->pPC->get_mem(adr+3,SIZE_8));

            sprintf(Buffer,"%05X:",(uint)adr);
            for(i=0;i<l;i++)
                sprintf(Buffer,"%s%02X",Buffer,(uint)pCPU->pPC->get_mem(adr+i,SIZE_8));
            sprintf(Buffer,"%s%*s%s\n",Buffer,16-(l<<1)," ",s);

            {
            int pc,nb,rh,rl;

            pc = adr;

            pc++; nb = pCPU->pPC->get_mem(pc,SIZE_8);
            pc++; rh = pCPU->pPC->get_mem(pc,SIZE_8);
            pc++; rl = pCPU->pPC->get_mem(pc,SIZE_8);
            pc++; // CASE 2
            sprintf(s,"CASE 2 ");

            pc++;
            for (int i=0;i<nb;i++)
            {
                sprintf(s,"%s,%02X:%02X%02X",
                    s,
                    (uint)pCPU->pPC->get_mem(pc,SIZE_8),
                    (uint)pCPU->pPC->get_mem(pc+1,SIZE_8),
                    (uint)pCPU->pPC->get_mem(pc+2,SIZE_8));
                pc+=3;
            }
            adr +=4;
            l = 5+3*nb;
            }
            break;


    }


    sprintf(Buffer,"%s%05X:",Buffer,(uint)adr);
    if (l<5)
        for(i=0;i<l;i++)
            sprintf(Buffer,"%s%02X",Buffer,(uint)pCPU->pPC->get_mem(adr+i,SIZE_8));
    int decal;
    if (l<5) decal = l;
    else	decal = 0;
    sprintf(Buffer,"%s%*s%s",Buffer,16-(decal<<1)," ",s);


    debugged = 1;
    NextDasmAdr = ((adr+l)&MASK_20);

    return((adr+l)&MASK_20);
}

void Cdebug_sc61860::injectReg(Parser *p)
{

    CSC61860 * _sc = (CSC61860*)(pCPU->pPC);

    p->symbols_ ["P"]=	_sc->get_reg(REG_P);
    p->symbols_ ["Q"]=	_sc->get_reg(REG_Q);
    p->symbols_ ["R"]=	_sc->get_reg(REG_R);
    p->symbols_ ["I"]=	_sc->get_reg(REG_I);
    p->symbols_ ["J"]=	_sc->get_reg(REG_J);
    p->symbols_ ["A"]=	_sc->get_reg(REG_A);
    p->symbols_ ["B"]=	_sc->get_reg(REG_B);
    p->symbols_ ["X"]=	_sc->get_reg(REG_X);
    p->symbols_ ["Y"]=	_sc->get_reg(REG_Y);
    p->symbols_ ["DP"]=	_sc->get_reg(REG_DP);
    p->symbols_ ["PC"]=	_sc->get_reg(REG_PC);

}

Cdebug_sc61860::Cdebug_sc61860(CCPU *parent)	: Cdebug(parent)
{
    AsmTbl = AsmTbl_sc61860;
}



