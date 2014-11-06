#include "sc62015d.h"
#include "sc62015.h"
#include "lcc/parser/parser.h"

UINT32 Cdebug_sc62015::DisAsm_1(UINT32 adr)
{
    typedef	struct{
        char	len;
        const char	*nim;
    }	DisAsmTbl;

    DisAsmTbl	d[]={
         {1,"NOP"},						/* 00h */
         {1,"RETI"},
        {-3,"JP    %02X%02X"},
        {-4,"JPF   %1X%02X%02X"},
        {-3,"CALL  %02X%02X"},
        {-4,"CALLF %1X%02X%02X"},
         {1,"RET"},
         {1,"RETF"},
         {2,"MV    A,%02X"},
         {2,"MV    IL,%02X"},
        {-3,"MV    BA,%02X%02X"},
        {-3,"MV    I,%02X%02X"},
        {-4,"MV    X,%1X%02X%02X"},
        {-4,"MV    Y,%1X%02X%02X"},
        {-4,"MV    U,%1X%02X%02X"},
        {-4,"MV    S,%1X%02X%02X"},
         {2,"JP    (%02X)"},					/* 10h */
        {10,"JP    %s"},
         {2,"JR    +%02X"},
         {2,"JR    -%02X"},
        {-3,"JPZ   %02X%02X"},
        {-3,"JPNZ  %02X%02X"},
        {-3,"JPC   %02X%02X"},
        {-3,"JPNC  %02X%02X"},
        { 2,"JRZ   +%02X"},
        { 2,"JRZ   -%02X"},
        { 2,"JRNZ  +%02X"},
        { 2,"JRNZ  -%02X"},
        { 2,"JRC   +%02X"},
        { 2,"JRC   -%02X"},
        { 2,"JRNC  +%02X"},
        { 2,"JRNC  -%02X"},
        { 1,"???   (20)"},					/* 20h */
        { 1,"PRE   21"},
        { 1,"PRE   22"},
        { 1,"PRE   23"},
        { 1,"PRE   24"},
        { 1,"PRE   25"},
        { 1,"PRE   26"},
        { 1,"PRE   27"},
        { 1,"PUSHU A"},
        { 1,"PUSHU IL"},
        { 1,"PUSHU BA"},
        { 1,"PUSHU I"},
        { 1,"PUSHU X"},
        { 1,"PUSHU Y"},
        { 1,"PUSHU F"},
        { 1,"PUSHU IMR"},
        { 1,"PRE   30"},					/* 30h */
        { 1,"PRE   31"},
        { 1,"PRE   32"},
        { 1,"PRE   33"},
        { 1,"PRE   34"},
        { 1,"PRE   35"},
        { 1,"PRE   36"},
        { 1,"PRE   37"},
        { 1,"POPU  A"},
        { 1,"POPU  IL"},
        { 1,"POPU  BA"},
        { 1,"POPU  I"},
        { 1,"POPU  X"},
        { 1,"POPU  Y"},
        { 1,"POPU  F"},
        { 1,"POPU  IMR"},
        { 2,"ADD   A,%02X"},						/* 40h */
        { 3,"ADD   (%02X),%02X"},
        { 2,"ADD   A,(%02X)"},
        { 2,"ADD   (%02X),A"},
        {11,"ADD   %s,%s"},
        {11,"ADD   %s,%s"},
        {11,"ADD   %s,%s"},
        { 3,"PMDF  (%02X),%02X"},
        { 2,"SUB   A,%02X"},
       {  3,"SUB   (%02X),%02X"},
         {2,"SUB   A,(%02X)"},
         {2,"SUB   (%02X),A"},
        {11,"SUB   %s,%s"},
        {11,"SUB   %s,%s"},
        {11,"SUB   %s,%s"},
        { 1,"PUSHS F"},
        { 2,"ADC   A,%02X"},						/* 50h */
        { 3,"ADC   (%02X),%02X"},
        { 2,"ADC   A,(%02X)"},
        { 2,"ADC   (%02X),A"},
        { 3,"ADCL  (%02X),(%02X)"},
        { 2,"ADCL  (%02X),A"},
        {12,"MVL   (%02X),[%s%s%02X]"},
        { 2,"PMDF  (%02X),a"},
        { 2,"SBC   A,%02X"},
        { 3,"SBC   (%02X),%02X"},
        { 2,"SBC   A,(%02X)"},
        { 2,"SBC   (%02X),A"},
        { 3,"SBCL  (%02X),(%02X)"},
        { 2,"SBCL  (%02X),A"},
        {13,"MVL   [%s%s%02X],(%02X)"},
        { 1,"POPS  F"},
        { 2,"CMP   A,%02X"},						/* 60h */
        { 3,"CMP   (%02X),%02X"},
        { 5,"CMP   [%1X%02X%02X],%02X"},
        { 2,"CMP   (%02X),A"},
        { 2,"TEST  A,%02X"},
        { 3,"TEST  (%02X),%02X"},
        { 5,"TEST  [%1X%02X%02X],%02X"},
        { 2,"TEST  (%02X),A"},
        { 2,"XOR   A,%02X"},
        { 3,"XOR   (%02X),%02X"},
        { 5,"XOR   [%1X%02X%02X],%02X"},
        { 2,"XOR   (%02X),A"},
        {10,"INC   %s"},
        { 2,"INC   (%02X)"},
        { 3,"XOR   (%02X),(%02X)"},
        { 2,"XOR   A,(%02X)"},
        { 2,"AND   A,%02X"},						/* 70h */
        { 3,"AND   (%02X),%02X"},
        { 5,"AND   [%1X%02X%02X],%02X"},
        { 2,"AND   (%02X),A"},
        { 1,"MV    A,B"},
        { 1,"MV    B,A"},
        { 3,"AND   (%02X),(%02X)"},
        { 2,"AND   A,(%02X)"},
        { 2,"OR    A,%02X"},
        { 3,"OR    (%02X),%02X"},
        { 5,"OR    [%1X%02X%02X],%02X"},
        { 2,"OR    (%02X),A"},
        {10,"DEC   %s"},
        { 2,"DEC   (%02X)"},
        { 3,"OR    (%02X),(%02X)"},
        { 2,"OR    A,(%02X)"},
        { 2,"MV    A,(%02X)"},					/* 80h */
        { 2,"MV    IL,(%02X)"},
        { 2,"MV    BA,(%02X)"},
        { 2,"MV    I,(%02X)"},
        { 2,"MV    X,(%02X)"},
        { 2,"MV    Y,(%02X)"},
        { 2,"MV    U,(%02X)"},
        { 2,"MV    S,(%02X)"},
        {-4,"MV    A,[%1X%02X%02X]"},
        {-4,"MV    IL,[%1X%02X%02X]"},
        {-4,"MV    BA,[%1X%02X%02X]"},
        {-4,"MV    I,[%1X%02X%02X]"},
        {-4,"MV    X,[%1X%02X%02X]"},
        {-4,"MV    Y,[%1X%02X%02X]"},
        {-4,"MV    U,[%1X%02X%02X]"},
        {-4,"MV    S,[%1X%02X%02X]"},
        {14,"MV    A,[%s]"},						/* 90h */
        {14,"MV    IL,[%s]"},
        {14,"MV    BA,[%s]"},
        {14,"MV    I,[%s]"},
        {14,"MV    X,[%s]"},
        {14,"MV    Y,[%s]"},
        {14,"MV    U,[%s]"},
        { 1,"SC"},
        {15,"MV    A,[(%02X)%s]"},
        {15,"MV    IL,[(%02X)%s]"},
        {15,"MV    BA,[(%02X)%s]"},
        {15,"MV    I,[(%02X)%s]"},
        {15,"MV    X,[(%02X)%s]"},
        {15,"MV    Y,[(%02X)%s]"},
        {15,"MV    U,[(%02X)%s]"},
        { 1,"RC"},
        { 2,"MV    (%02X),A"},					/* a0h */
        { 2,"MV    (%02X),IL"},
        { 2,"MV    (%02X),BA"},
        { 2,"MV    (%02X),I"},
        { 2,"MV    (%02X),X"},
        { 2,"MV    (%02X),Y"},
        { 2,"MV    (%02X),U"},
        { 2,"MV    (%02X),S"},
        {-4,"MV    [%1X%02X%02X],A"},
        {-4,"MV    [%1X%02X%02X],IL"},
        {-4,"MV    [%1X%02X%02X],BA"},
        {-4,"MV    [%1X%02X%02X],I"},
        {-4,"MV    [%1X%02X%02X],X"},
        {-4,"MV    [%1X%02X%02X],Y"},
        {-4,"MV    [%1X%02X%02X],U"},
        {-4,"MV    [%1X%02X%02X],S"},
        {14,"MV    [%s],A"},						/* b0h */
        {14,"MV    [%s],IL"},
        {14,"MV    [%s],BA"},
        {14,"MV    [%s],I"},
        {14,"MV    [%s],X"},
        {14,"MV    [%s],Y"},
        {14,"MV    [%s],U"},
        { 3,"CMP   (%02X),(%02X)"},
        {15,"MV    [(%02X)%s],A"},
        {15,"MV    [(%02X)%s],IL"},
        {15,"MV    [(%02X)%s],BA"},
        {15,"MV    [(%02X)%s],I"},
        {15,"MV    [(%02X)%s],X"},
        {15,"MV    [(%02X)%s],Y"},
        {15,"MV    [(%02X)%s],U"},
        { 1,"???   (BF)"},
        { 3,"EX    (%02X),(%02X)"},				/* c0h */
        { 3,"EXW   (%02X),(%02X)"},
        { 3,"EXP   (%02X),(%02X)"},
        { 3,"EXL   (%02X),(%02X)"},
        { 3,"DADL  (%02X),(%02X)"},
        { 2,"DADL  (%02X),A"},
        { 3,"CMPW  (%02X),(%02X)"},
        { 3,"CMPP  (%02X),(%02X)"},
        { 3,"MV    (%02X),(%02X)"},
        { 3,"MVW   (%02X),(%02X)"},
        { 3,"MVP   (%02X),(%02X)"},
        { 3,"MVL   (%02X),(%02X)"},
        { 3,"MV    (%02X),%02X"},
        { 4,"MVW   (%02X),%02X%02X"},
        { 1,"TCL"},
        { 3,"MVLD  (%02X),(%02X)"},
        {-5,"MV    (%02X),[%1X%02X%02X]"},				/* d0h */
        {-5,"MVW   (%02X),[%1X%02X%02X]"},
        {-5,"MVP   (%02X),[%1X%02X%02X]"},
        {-5,"MVL   (%02X),[%1X%02X%02X]"},
        { 3,"DSBL  (%02X),(%02X)"},
        { 2,"DSBL  (%02X),A"},
        {16,"CMPW  (%02X),%s"},
        {16,"CMPP  (%02X),%s"},
        { 5,"MV    [%1X%02X%02X],(%02X)"},
        { 5,"MVW   [%1X%02X%02X],(%02X)"},
        { 5,"MVP   [%1X%02X%02X],(%02X)"},
        { 5,"MVL   [%1X%02X%02X],(%02X)"},
        {-5,"MVP   (%02X),%1X%02X%02X"},
        { 1,"EX    A,B"},
        { 1,"HALT"},
        { 1,"OFF"},
        {17,"MV    (%02X),[%s]"},						/* e0h */
        {17,"MVW   (%02X),[%s]"},
        {17,"MVP   (%02X),[%s]"},
        {17,"MVL   (%02X),[%s]"},
        { 1,"ROR   A"},
        { 2,"ROR   (%02X)"},
        { 1,"ROL   A"},
        { 2,"ROL   (%02X)"},
        {18,"MV    [%s],(%02X)"},
        {18,"MVW   [%s],(%02X)"},
        {18,"MVP   [%s],(%02X)"},
        {18,"MVL   [%s],(%02X)"},
        { 2,"DSLL  (%02X)"},
        {11,"EX    %s,%s"},
        { 1,"SWAP  A"},
        { 1,"WAIT"},
        {19,"MV    (%02X),[(%02X)%s]"},					/* f0h */
        {19,"MVW   (%02X),[(%02X)%s]"},
        {19,"MVP   (%02X),[(%02X)%s]"},
        {19,"MVL   (%02X),[(%02X)%s]"},
        { 1,"SHR   A"},
        { 2,"SHR   (%02X)"},
        { 1,"SHL   A"},
        { 2,"SHL   (%02X)"},
        {20,"MV    [(%02X)%s],(%02X)"},
        {20,"MVW   [(%02X)%s],(%02X)"},
        {20,"MVP   [(%02X)%s],(%02X)"},
        {20,"MVL   [(%02X)%s],(%02X)"},
        { 2,"DSRL  (%02X)"},
        {11,"MV    %s,%s"},
        { 1,"IR"},
        { 1,"RESET"}};
    const char	*reg[]={"A","IL","BA","I","X","Y","U","S"};
    char	l,b[16],s[32];
    BYTE	t,i;

    DasmAdr=adr;

    l=abs(d[pCPU->pPC->get_mem(adr,SIZE_8)].len);
    switch(d[pCPU->pPC->get_mem(adr,SIZE_8)].len){
    case  1:sprintf(s,d[pCPU->pPC->get_mem(adr,SIZE_8)].nim); break;
    case  2:sprintf(s,d[pCPU->pPC->get_mem(adr,SIZE_8)].nim,pCPU->pPC->get_mem(adr+1,SIZE_8)); break;
    case  3:sprintf(s,d[pCPU->pPC->get_mem(adr,SIZE_8)].nim,pCPU->pPC->get_mem(adr+1,SIZE_8),pCPU->pPC->get_mem(adr+2,SIZE_8)); break;
    case  4:sprintf(s,d[pCPU->pPC->get_mem(adr,SIZE_8)].nim,pCPU->pPC->get_mem(adr+1,SIZE_8),pCPU->pPC->get_mem(adr+3,SIZE_8),pCPU->pPC->get_mem(adr+2,SIZE_8)); break;
    case  5:sprintf(s,d[pCPU->pPC->get_mem(adr,SIZE_8)].nim,pCPU->pPC->get_mem(adr+3,SIZE_8),pCPU->pPC->get_mem(adr+2,SIZE_8),pCPU->pPC->get_mem(adr+1,SIZE_8),pCPU->pPC->get_mem(adr+4,SIZE_8)); break;
    case -3:sprintf(s,d[pCPU->pPC->get_mem(adr,SIZE_8)].nim,pCPU->pPC->get_mem(adr+2,SIZE_8),pCPU->pPC->get_mem(adr+1,SIZE_8)); break;
    case -4:sprintf(s,d[pCPU->pPC->get_mem(adr,SIZE_8)].nim,pCPU->pPC->get_mem(adr+3,SIZE_8),pCPU->pPC->get_mem(adr+2,SIZE_8),pCPU->pPC->get_mem(adr+1,SIZE_8)); break;
    case -5:sprintf(s,d[pCPU->pPC->get_mem(adr,SIZE_8)].nim,pCPU->pPC->get_mem(adr+1,SIZE_8),pCPU->pPC->get_mem(adr+4,SIZE_8),pCPU->pPC->get_mem(adr+3,SIZE_8),pCPU->pPC->get_mem(adr+2,SIZE_8)); break;
    case 10:sprintf(s,d[pCPU->pPC->get_mem(adr,SIZE_8)].nim,reg[pCPU->pPC->get_mem(adr+1,SIZE_8)&7]); l=2;break;
    case 11:sprintf(s,d[pCPU->pPC->get_mem(adr,SIZE_8)].nim,reg[(pCPU->pPC->get_mem(adr+1,SIZE_8)>>4)&7],reg[pCPU->pPC->get_mem(adr+1,SIZE_8)&7]); l=2;break;
    case 12:sprintf(s,d[pCPU->pPC->get_mem(adr,SIZE_8)].nim,pCPU->pPC->get_mem(adr+2,SIZE_8),reg[pCPU->pPC->get_mem(adr+1,SIZE_8)&7],(pCPU->pPC->get_mem(adr+1,SIZE_8)&0x40)==0?"+":"-",pCPU->pPC->get_mem(adr+3,SIZE_8)); l=4;break;
    case 13:sprintf(s,d[pCPU->pPC->get_mem(adr,SIZE_8)].nim,reg[pCPU->pPC->get_mem(adr+1,SIZE_8)&7],(pCPU->pPC->get_mem(adr+1,SIZE_8)&0x40)==0?"+":"-",pCPU->pPC->get_mem(adr+3,SIZE_8),pCPU->pPC->get_mem(adr+2,SIZE_8)); l=4;break;
    case 14:
        if((pCPU->pPC->get_mem(adr+1,SIZE_8)&0x80)==0){
            sprintf(b,"%s%s%s",(pCPU->pPC->get_mem(adr+1,SIZE_8)&0x30)!=0x30?"":"--",reg[pCPU->pPC->get_mem(adr+1,SIZE_8)&7],(pCPU->pPC->get_mem(adr+1,SIZE_8)&0x30)!=0x20?"":"++");l=2;
        }else{
            sprintf(b,"%s%s%02X",reg[pCPU->pPC->get_mem(adr+1,SIZE_8)&7],(pCPU->pPC->get_mem(adr+1,SIZE_8)&0x40)==0?"+":"-",pCPU->pPC->get_mem(adr+2,SIZE_8));l=3;
        }
        sprintf(s,d[pCPU->pPC->get_mem(adr,SIZE_8)].nim,b); break;
    case 15:
        if((pCPU->pPC->get_mem(adr+1,SIZE_8)&0x80)==0){ b[0]=0;l=3;
        }else{
            sprintf(b,"%s%02X",(pCPU->pPC->get_mem(adr+1,SIZE_8)&0x40)==0?"+":"-",pCPU->pPC->get_mem(adr+3,SIZE_8));l=4;
        }
        sprintf(s,d[pCPU->pPC->get_mem(adr,SIZE_8)].nim,pCPU->pPC->get_mem(adr+2,SIZE_8),b);break;
    case 16:sprintf(s,d[pCPU->pPC->get_mem(adr,SIZE_8)].nim,pCPU->pPC->get_mem(adr+2,SIZE_8),reg[pCPU->pPC->get_mem(adr+1,SIZE_8)&7]); l=3;break;
    case 17:
        if((pCPU->pPC->get_mem(adr+1,SIZE_8)&0x80)==0){
            sprintf(b,"%s%s%s",(pCPU->pPC->get_mem(adr+1,SIZE_8)&0x30)!=0x30?"":"--",reg[pCPU->pPC->get_mem(adr+1,SIZE_8)&7],(pCPU->pPC->get_mem(adr+1,SIZE_8)&0x30)!=0x20?"":"++");
            l=3;t=pCPU->pPC->get_mem(adr+2,SIZE_8);
        }else{
            sprintf(b,"%s%s%02X",reg[pCPU->pPC->get_mem(adr+1,SIZE_8)&7],(pCPU->pPC->get_mem(adr+1,SIZE_8)&0x40)==0?"+":"-",pCPU->pPC->get_mem(adr+3,SIZE_8));
            l=4;t=pCPU->pPC->get_mem(adr+2,SIZE_8);
        }
        sprintf(s,d[pCPU->pPC->get_mem(adr,SIZE_8)].nim,t,b); break;
    case 18:
        if((pCPU->pPC->get_mem(adr+1,SIZE_8)&0x80)==0){
            sprintf(b,"%s%s%s",(pCPU->pPC->get_mem(adr+1,SIZE_8)&0x30)!=0x30?"":"--",reg[pCPU->pPC->get_mem(adr+1,SIZE_8)&7],(pCPU->pPC->get_mem(adr+1,SIZE_8)&0x30)!=0x20?"":"++");
            l=3;t=pCPU->pPC->get_mem(adr+2,SIZE_8);
        }else{
            sprintf(b,"%s%s%02X",reg[pCPU->pPC->get_mem(adr+1,SIZE_8)&7],(pCPU->pPC->get_mem(adr+1,SIZE_8)&0x40)==0?"+":"-",pCPU->pPC->get_mem(adr+3,SIZE_8));
            l=4;t=pCPU->pPC->get_mem(adr+2,SIZE_8);
        }
        sprintf(s,d[pCPU->pPC->get_mem(adr,SIZE_8)].nim,b,t); break;
    case 19:
        if((pCPU->pPC->get_mem(adr+1,SIZE_8)&0x80)==0){ b[0]=0;l=4;
        }else{
            sprintf(b,"%s%02X",(pCPU->pPC->get_mem(adr+1,SIZE_8)&0x40)==0?"+":"-",pCPU->pPC->get_mem(adr+4,SIZE_8));l=5;
        }
        sprintf(s,d[pCPU->pPC->get_mem(adr,SIZE_8)].nim,pCPU->pPC->get_mem(adr+2,SIZE_8),pCPU->pPC->get_mem(adr+3,SIZE_8),b);break;
    case 20:
        if((pCPU->pPC->get_mem(adr+1,SIZE_8)&0x80)==0){ b[0]=0;l=4;
        }else{
            sprintf(b,"%s%02X",(pCPU->pPC->get_mem(adr+1,SIZE_8)&0x40)==0?"+":"-",pCPU->pPC->get_mem(adr+4,SIZE_8));l=5;
        }
        sprintf(s,d[pCPU->pPC->get_mem(adr,SIZE_8)].nim,pCPU->pPC->get_mem(adr+2,SIZE_8),b,pCPU->pPC->get_mem(adr+3,SIZE_8));break;
    }
    sprintf(Buffer,"%05X:",adr);
    for(i=0;i<l;i++) sprintf(Buffer,"%s%02X",Buffer,pCPU->pPC->get_mem(adr+i,SIZE_8));
    sprintf(Buffer,"%s%*s%s ",Buffer,16-(l<<1)," ",s);
    if((pCPU->pPC->get_mem(adr,SIZE_8)>0x20 && pCPU->pPC->get_mem(adr,SIZE_8)<0x28)||(pCPU->pPC->get_mem(adr,SIZE_8)>0x2f && pCPU->pPC->get_mem(adr,SIZE_8)<0x38)){
        adr=DisAsm_1((adr+l)&MASK_20); l=0;
    }

    debugged = true;
    NextDasmAdr = (adr+l)&MASK_20;
    return((adr+l)&MASK_20);
}

void Cdebug_sc62015::injectReg(Parser *p)
{

    Csc62015 * _sc = (Csc62015*)(pCPU);

    p->symbols_ ["A"]=	_sc->get_reg(REG_A);
    p->symbols_ ["IL"]=	_sc->get_reg(REG_IL);
    p->symbols_ ["BA"]=	_sc->get_reg(REG_BA);
    p->symbols_ ["I"]=	_sc->get_reg(REG_I);
    p->symbols_ ["X"]=	_sc->get_reg(REG_X);
    p->symbols_ ["Y"]=	_sc->get_reg(REG_Y);
    p->symbols_ ["U"]=	_sc->get_reg(REG_U);
    p->symbols_ ["S"]=	_sc->get_reg(REG_S);
    p->symbols_ ["P"]=	_sc->get_reg(REG_P);
    p->symbols_ ["F"]=	_sc->get_reg(REG_F);
    p->symbols_ ["PC"]=	_sc->get_reg(REG_PC);

}

Cdebug_sc62015::Cdebug_sc62015(CCPU *parent)	: Cdebug(parent)
{

}

