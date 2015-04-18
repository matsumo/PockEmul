/*** PC-E500 Emulator [SC62015.CC] ************************/
/* SC62015 CPU emulation                                  */
/*                       Copyright (C) Matsumo 1998,1999  */
/**********************************************************/

//TODO: transfert I/O to the pocket in/out instead of check imem

//#include <stdlib.h>
//#include <string.h>
#include "sc62015.h"
#include "sc62015d.h"
#include "Inter.h"
#include "Keyb.h"
//#include "e500.h"
//#include "rtc.h"
//#include "opt11.h"
//#include "ssfdc.h"
#include "sio.h"
#include "Lcdc.h"
#include "Log.h"
#include "ui/cregsz80widget.h"
#include "dialoganalog.h"

#include "lcc/parser/parser.h"


//#define	UN_DEFINE	printf("Undefined Code !!(pc=%05X)\n",reg.x.p);
#define		UN_DEFINE	debug.isdebug=1;
#define		OPR1			0		/* first operand by i-mem access */
#define		OPR2			1		/* second operand by i-mem access */
#define		Get_BP			imem[0xec]	/*	i-mem BP */
#define		Get_PX			imem[0xed]	/*	i-mem PX */
#define		Get_PY			imem[0xee]	/*	i-mem PY */
#define		Get_i8d(a)		imem[a&255]			//get i-mem direct
#define		Set_i8d(a,d)	imem[a&255]=d		//set i-mem direct
#define		Reset_Pre()		pre_1=1;pre_2=1	/* reset pre byte mode */

#define		GetBank()		(((imem[IMEM_EOL]>>5)&1)|((imem[IMEM_KOH]>>6)&2))
#define		SLOT1			0
#define		SLOT2			1
#define		SLOT3			2
#define		EMS				3
//#define		SLOT3EXT		4






UINT32	BASE_128[]={0x20000, 0x100000, 0x120000, 0x140000};
UINT32	BASE_64[] ={0x10000, 0x160000, 0x170000, 0x180000};
UINT32	SlotAdr[]={0x80000,0x40000,0xc0000,0x10000,0x20000};
WORD	SlotSize[]={256,256,256,192,128};

Csc62015::Csc62015(CPObject *parent)	: CCPU(parent)
{									//[constructor]
    halt=0;				//halt?(0:none, 1:halting)
    off=0;				//off?(0:none, 1:off)
    end=0;				//program end?(0:none, 1:end)
    save=0;				//end with memory save?(0:no, 1:yes)
    e6=0;				//E650 mode?(0:no, 1:yes)
    emsmode=0;			//EMS mode(0:none, 1-5:Delta 1-4 or Super)
    cpulog=0;				//execute log?(0:off, 1:on)
    logsw=0;				//log mode?(0:off, 1:on)
    CallSubLevel=0;

    div500	= div2	= false;
    ticks	= ticks2	= 0;

     pDEBUG	= new Cdebug_sc62015(this);
     regwidget = (CregCPU*) new Cregsz80Widget(0,this);
}

Csc62015::~Csc62015() {

}

#define		XTICKS      ( pPC->getfrequency() / 2)              // 0.5s counter
#define		XTICK2      ( pPC->getfrequency() / 1000 * 2)       // 2ms counter
#define		Set_ISR(d)	opr_imem(IMEM_ISR,OPR_OR,d)	// set status to ISR

inline void Csc62015::AddState(BYTE n)
{
    pPC->pTIMER->state+=(n);

    ticks+=(n);
    ticks2+=(n);

    div500  = (ticks >= XTICKS);
    div2    = (ticks2>= XTICK2);
    if (div500) {
        Set_ISR(INT_SLOW);
        ticks=0;
    }
    if (div2) {
        Set_ISR(INT_FAST);
        ticks2=0;
    }

    compute_xout();
    pPC->fillSoundBuffer((Get_Xout()?0xff:0x00));

}

/*****************************************************************************/
/* Convert to internal-RAM address with PRE byte							 */
/*  ENTRY :BYTE d=internal RAM address										 */
/*  RETURN:BYTE (internal RAM real address)									 */
/*****************************************************************************/
inline BYTE Csc62015::Conv_imemAdr(BYTE d,bool m)
{
/*printf("mode=%d\n",m);*/
    BYTE	r=0;
	if(m==0){
		switch(pre_1){					/* first operand */
		case	0:r=d;			   break;	/* (n) */
		case	1:r=d+Get_BP;	   break;	/* (BP+n) */
		case	2:r=d+Get_PX;	   break;	/* (PX+n) */
		case	3:r=Get_BP+Get_PX; break;	/* (BP+PX) */
		}
	}else{
		switch(pre_2){					/* second operand */
		case	0:r=d;			   break;	/* (n) */
		case	1:r=d+Get_BP;	   break;	/* (BP+n) */
		case	2:r=d+Get_PY;	   break;	/* (PY+n) */
		case	3:r=Get_BP+Get_PY; break;	/* (BP+PY) */
		}
	}
	return(r);
}

/*****************************************************************************/
/* Check for I/O access by internal-RAM address								 */
/*  ENTRY :BYTE d=internal RAM address, BYTE len=access bytes				 */
/*  RETURN:none																 */
/*****************************************************************************/
inline void Csc62015::Chk_imemAdr_Read(BYTE d,BYTE len)
{
    BYTE i;
    if (fp_log) fprintf(fp_log,"IMEM access : %02X  l=%d\n",d,len);
    for(i=0;i<len;i++){
        switch(d){
        case    IMEM_AMC: AddLog(LOG_CONSOLE,QString("read [EF]=%1\n").arg(imem[d],2,16,QChar('0')));break;
        case	IMEM_KOL:
        case	IMEM_KOH:/*((Ce500*)pPC)->getKey();*/ break;	// key matrix
//		case	IMEM_RxD:sio.si=1; break;		// sio RxD
//		case	IMEM_TxD:sio.so=1; break;		// sio TxD
//		case	IMEM_SCR:snd.scr=1; break;		// sound
//		case	IMEM_EOL:opt11.eio=IMEM_EOL; break;
//		case	IMEM_EIL:opt11.eio=IMEM_EIL; break;
        case IMEM_EOH:
        case IMEM_EOL:break;
        case IMEM_EIH:
        case IMEM_EIL: //if (mainwindow->dialoganalogic) mainwindow->dialoganalogic->setMarker(d);
                        AddLog(LOG_PRINTER,tr("READ [%1]").arg(d,2,16,QChar('0')));
                        break;
        }
        d++;
    }
}

inline void Csc62015::Chk_imemAdr(BYTE d,BYTE len,UINT32 data)
{
    BYTE i;
    if (fp_log) fprintf(fp_log,"IMEM access : %02X  l=%d\n",d,len);
    for(i=0;i<len;i++){
        switch(d){
//        case	IMEM_KOL:
//        case	IMEM_KOH:((Ce500*)pPC)->getKey(); break;	// key matrix
//		case	IMEM_RxD:sio.si=1; break;		// sio RxD
//		case	IMEM_TxD:sio.so=1; break;		// sio TxD
//		case	IMEM_SCR:snd.scr=1; break;		// sound
//		case	IMEM_EOL:opt11.eio=IMEM_EOL; break;
//		case	IMEM_EIL:opt11.eio=IMEM_EIL; break;
        case    IMEM_AMC: AddLog(LOG_CONSOLE,QString("write [EF]=%1\n").arg(data,4,16,QChar('0')));break;
        case IMEM_EIH:
        case IMEM_EIL:
        case IMEM_EOH:
        case IMEM_EOL: if (mainwindow->dialoganalogic) mainwindow->dialoganalogic->setMarker(data &0xff);
                AddLog(LOG_PRINTER,tr("WRITE [%1] = %2").arg(d,2,16,QChar('0')).arg(data,6,16,QChar('0')));
                break;
        }
        d++;
        data>>=8;
    }
}




/*****************************************************************************/
/* Get data from imem[]														 */
/*  ENTRY :BYTE a=internal RAM address										 */
/*  RETURN:(BYTE(8),WORD(16),DWORD(20,24) data)								 */
/*****************************************************************************/
inline BYTE Csc62015::Get_i8(BYTE a,bool m)
{
    BYTE	adr;
	adr=Conv_imemAdr(a,m);
    Chk_imemAdr_Read(adr,SIZE_8);
	return(imem[adr]);
}
inline WORD Csc62015::Get_i16(BYTE a,bool m)
{
    BYTE	adr;
	adr=Conv_imemAdr(a,m);
    Chk_imemAdr_Read(adr,SIZE_16);
	return(imem[adr]+(imem[adr+1]<<8));
}
inline UINT32 Csc62015::Get_i20(BYTE a,bool m)
{
    BYTE	adr;
	adr=Conv_imemAdr(a,m);
    Chk_imemAdr_Read(adr,SIZE_20);
	return((imem[adr]+(imem[adr+1]<<8)+(imem[adr+2]<<16))&MASK_20);
}
inline UINT32 Csc62015::Get_i24(BYTE a,bool m)
{
    BYTE	adr;
	adr=Conv_imemAdr(a,m);
    Chk_imemAdr_Read(adr,SIZE_20);
	return((imem[adr]+(imem[adr+1]<<8)+(imem[adr+2]<<16))&MASK_24);
}

/*****************************************************************************/
/* Set data to imem[]														 */
/*  ENTRY :BYTE a=internal RAM address, BYTE(8),WORD(16),DWORD(20,24) d=data */
/*****************************************************************************/
inline void Csc62015::Set_i8(BYTE a,BYTE d,bool m)
{
    BYTE	adr;
	adr=Conv_imemAdr(a,m);
    Chk_imemAdr(adr,SIZE_8,d);
	imem[adr]=d;
}
inline void Csc62015::Set_i16(BYTE a,WORD d,bool m)
{
    BYTE	adr;
	adr=Conv_imemAdr(a,m);
    Chk_imemAdr(adr,SIZE_16,d);
	imem[adr++]=d;
	imem[adr]=(d>>8);
}
inline void Csc62015::Set_i20(BYTE a,UINT32 d,bool m)
{
    BYTE	adr;
	adr=Conv_imemAdr(a,m);
    Chk_imemAdr(adr,SIZE_20,d);
	imem[adr++]=d;
	imem[adr++]=(d>>8);
	imem[adr]=(d>>16)&MASK_4;
}
inline void Csc62015::Set_i24(BYTE a,UINT32 d,bool m)
{
    BYTE	adr;
	adr=Conv_imemAdr(a,m);
    Chk_imemAdr(adr,SIZE_20,d);
	imem[adr++]=d;
	imem[adr++]=(d>>8);
	imem[adr]=(d>>16);
}

/*****************************************************************************/
/* Get data														 */
/*  ENTRY :BYTE r=No.(0-7)											 */
/*  RETURN:(BYTE(8),WORD(16),DWORD(20) data)								 */
/*****************************************************************************/
inline UINT32 Csc62015::Get_r(BYTE r)
{
	switch(r&7){
	case	0:return(reg.r.a ); break;
	case	1:return(reg.r.il); break;
	case	2:return(reg.x.ba); break;
	case	3:return(reg.x.i ); break;
	case	4:return(reg.x.x ); break;
	case	5:return(reg.x.y ); break;
	case	6:return(reg.x.u ); break;
	case	7:return(reg.x.s ); break;
	}
    return 0;
}

/*****************************************************************************/
/* Get data (modified to r2,r3)									 */
/*  ENTRY :BYTE r=No.(0-7)											 */
/*  RETURN:(WORD(16),DWORD(20) data)										 */
/*****************************************************************************/
inline UINT32 Csc62015::Get_r2(BYTE r)
{
	switch(r&7){
	case	0:return(reg.x.ba); break;
	case	1:return(reg.x.i ); break;
	case	2:return(reg.x.ba); break;
	case	3:return(reg.x.i ); break;
	case	4:return(reg.x.x ); break;
	case	5:return(reg.x.y ); break;
	case	6:return(reg.x.u ); break;
	case	7:return(reg.x.s ); break;
	}
    return 0;
}

/*****************************************************************************/
/* Set data														 */
/*  ENTRY :BYTE r=No.(0-7), BYTE(8),WORD(16),DWORD(20) d=data		 */
/*****************************************************************************/
inline void Csc62015::Set_r(BYTE r,UINT32 d)
{
	switch(r&7){
	case	0:reg.r.a = d		  ; break;
	case	1:reg.x.i =(d&MASK_8) ; break;
	case	2:reg.x.ba= d		  ; break;
	case	3:reg.x.i = d		  ; break;
	case	4:reg.x.x =(d&MASK_20); break;
	case	5:reg.x.y =(d&MASK_20); break;
	case	6:reg.x.u =(d&MASK_20); break;
	case	7:reg.x.s =(d&MASK_20); break;
	}
}

/*****************************************************************************/
/* Set data (modified to r2,r3)									 */
/*  ENTRY :BYTE r=No.(0-7), WORD(16),DWORD(20) d=data				 */
/*****************************************************************************/
inline void Csc62015::Set_r2(BYTE r,UINT32 d)
{
	switch(r&7){
	case	0:reg.x.ba= d		  ; break;
	case	1:reg.x.i = d		  ; break;
	case	2:reg.x.ba= d		  ; break;
	case	3:reg.x.i = d		  ; break;
	case	4:reg.x.x =(d&MASK_20); break;
	case	5:reg.x.y =(d&MASK_20); break;
	case	6:reg.x.u =(d&MASK_20); break;
	case	7:reg.x.s =(d&MASK_20); break;
	}
}

/*****************************************************************************/
/* Check flags(Zero & Carry)												 */
/*  ENTRY :DWORD d=data, BYTE len=data length(8bit=1,16bit=2,20bit=3)		 */
/*****************************************************************************/
inline void Csc62015::Chk_Flag(UINT32 d,BYTE len)
{
	UINT32	mask_z[]={0,MASK_8,MASK_16,MASK_20,MASK_24};
	UINT32	mask_c[]={0,MASK_8+1,MASK_16+1,MASK_20+1,MASK_24+1};
//	if(len>4) return;
	reg.r.z=(d&mask_z[len])==0?1:0;
	reg.r.c=(d&mask_c[len])==0?0:1;
}
inline void Csc62015::Chk_Zero(UINT32 d,BYTE len)
{
	UINT32	mask_z[]={0,MASK_8,MASK_16,MASK_20,MASK_24};
//	if(len>4) return;
	reg.r.z=(d&mask_z[len])==0?1:0;
}

/*****************************************************************************/
/* Make effective address from current reg.x.p	for MV r,[r3] or MV [r3],r	 */
/*  RETURN:DWORD address													 */
/*****************************************************************************/
inline UINT32 Csc62015::Get_d(BYTE len)
{
    BYTE	t;
    UINT32	a;
    t=pPC->Get_8(reg.x.p);
    reg.r.pc++;
    a=Get_r(t);												/* a=r3 ([r3]) */
    switch(t>>4){
    case   2:Set_r(t,(a+len)); break;							/* [r3++] */
    case   3:a-=len; Set_r(t,a); AddState(1); break;			/* [--r3] */
    case   8:a+=pPC->Get_8(reg.x.p);reg.r.pc++; AddState(2); break;	/* [r3+n] */
    case 0xc:a-=pPC->Get_8(reg.x.p);reg.r.pc++; AddState(2); break;	/* [r3-n] */
    }
    return(a&MASK_20);
}

/*****************************************************************************/
/* Make effective address from current reg.x.p & Get i-mem address			 */
/*	for MV (n),[r3] or MV [r3],(n)											 */
/*  RETURN:DWORD address, BYTE r=i-mem address								 */
/*****************************************************************************/
inline UINT32 Csc62015::Get_d2(BYTE len, BYTE *r)
{
    BYTE	t;
    UINT32	a;
    t=pPC->Get_8(reg.x.p);
    reg.r.pc++;
    *r=pPC->Get_8(reg.x.p);								/* r=(i-mem address) */
    reg.r.pc++;
    a=Get_r(t);												/* a=r3 ([r3])*/
    switch(t>>4){
    case   2:Set_r(t,(a+len)); break;							/* [r3++] */
    case   3:a-=len; Set_r(t,a); AddState(1); break;			/* [--r3] */
    case   8:a+=pPC->Get_8(reg.x.p);reg.r.pc++;AddState(2);break;	/* [r3+n] */
    case 0xc:a-=pPC->Get_8(reg.x.p);reg.r.pc++;AddState(2);break;	/* [r3-n] */
    }
    return(a&MASK_20);
}

/*****************************************************************************/
/* Make effective address from current reg.x.p	for MV r,[(n)] or MV [(n)],r */
/*  RETURN:DWORD address													 */
/*****************************************************************************/
inline UINT32 Csc62015::Get_i(void)
{
    BYTE	t;
    UINT32	a;
    t=pPC->Get_8(reg.x.p);
	reg.r.pc++;
    a=Get_i20(pPC->Get_8(reg.x.p),OPR1);								/* a=(n) */
	reg.r.pc++;
	switch(t>>4){
    case   8:a+=pPC->Get_8(reg.x.p);reg.r.pc++;AddState(2);break;	/* [(m)+n] */
    case 0xc:a-=pPC->Get_8(reg.x.p);reg.r.pc++;AddState(2);break;	/* [(m)-n] */
	}
	return(a&MASK_20);
}

/*****************************************************************************/
/* Make effective address from current reg.x.p								 */
/*	for MV (m),[(n)] or MV [(m)],(n)										 */
/*  ENTRY :BYTE r=mode(0:MV (m),[(n)] , 1:MV [(m)],(n) )					 */
/*  RETURN:DWORD address, BYTE r=i-mem address								 */
/*****************************************************************************/
inline UINT32 Csc62015::Get_i2(BYTE *r)
{
    BYTE	t,u,v,w;
    UINT32	a;
    t=pPC->Get_8(reg.x.p);
	reg.r.pc++;
    u=pPC->Get_8(reg.x.p);
	reg.r.pc++;
    v=pPC->Get_8(reg.x.p);
	reg.r.pc++;
	if(*r){w=u;u=v;v=w;}		/* swap u,v */
	a=Get_i20(v,1-*r);								/* a=(n) */
	*r=u;											/* *r=(i-mem address) */
	switch(t>>4){
    case   8:a+=pPC->Get_8(reg.x.p);reg.r.pc++;AddState(2);break;	/* [(m)+n] */
    case 0xc:a-=pPC->Get_8(reg.x.p);reg.r.pc++;AddState(2);break;	/* [(m)-n] */
	}
	return(a&MASK_20);
}

/*****************************************************************************/
/* Convert from BCD to HEX													 */
/*  ENTRY :BYTE d=BCD data(00h-99h)											 */
/*  RETURN:BYTE HEX data(00h-63h)											 */
/*****************************************************************************/
inline BYTE Csc62015::bcd2hex(BYTE d)
{
	return((d>>4)*10+(d&15));
}

/*****************************************************************************/
/* Convert from HEX to BCD													 */
/*  ENTRY :BYTE d=HEX data(00h-ffh)											 */
/*  RETURN:BYTE BCD data(00h-255h)											 */
/*****************************************************************************/
inline WORD Csc62015::hex2bcd(BYTE d)
{
    BYTE	a,b,c;
	a=d/100;
	b=d-(a*100);
	c=b/10;
	return((a<<8)+(c<<4)+b-(c*10));
}

/*---------------------------------------------------------------------------*/
/* NOP */
inline void Csc62015::Op_00(void)
{
	AddState(1);
}
/* RETI */
inline void Csc62015::Op_01(void)
{
    imem[IMEM_IMR]=pPC->Get_8(reg.x.s);
	reg.x.s++;
    reg.x.f=pPC->Get_8(reg.x.s);
	reg.x.s++;
    reg.x.p=pPC->Get_20(reg.x.s);
	reg.x.s+=SIZE_20;
    CallSubLevel--;
	AddState(7);
}
/* JP mn */
inline void Csc62015::Op_02(void)
{
    reg.r.pc=pPC->Get_16(reg.x.p);
	AddState(4);
}
/* JPF lmn */
inline void Csc62015::Op_03(void)
{
    reg.x.p=pPC->Get_20(reg.x.p);
	AddState(5);
}
/* CALL mn */
inline void Csc62015::Op_04(void)
{
    WORD	t;
    t=pPC->Get_16(reg.x.p);
	reg.r.pc+=SIZE_16;
	reg.x.s-=SIZE_16;
    pPC->Set_16(reg.x.s, reg.r.pc);
	reg.r.pc=t;
    CallSubLevel++;
	AddState(6);
}
/* CALLF lmn */
inline void Csc62015::Op_05(void)
{
    UINT32	t;
    t=pPC->Get_20(reg.x.p);
	reg.r.pc+=SIZE_20;
	reg.x.s-=SIZE_20;
    pPC->Set_20(reg.x.s, reg.x.p);
	reg.x.p=t;
    CallSubLevel++;
	AddState(8);
}
/* RET */
inline void Csc62015::Op_06(void)
{
    reg.r.pc=pPC->Get_16(reg.x.s);
	reg.x.s+=SIZE_16;
    CallSubLevel--;
	AddState(4);
}
/* RETF */
inline void Csc62015::Op_07(void)
{
    reg.x.p=pPC->Get_20(reg.x.s);
	reg.x.s+=SIZE_20;
    CallSubLevel--;
	AddState(5);
}
/* MV A,n */
inline void Csc62015::Op_08(void)
{
    reg.r.a=pPC->Get_8(reg.x.p);
	reg.r.pc++;
	AddState(2);
}
/* MV IL,n */
inline void Csc62015::Op_09(void)
{
    reg.r.il=pPC->Get_8(reg.x.p);
	reg.r.pc++;
	reg.r.ih=0;
	AddState(3);
}
/* MV BA,mn */
inline void Csc62015::Op_0a(void)
{
    reg.x.ba=pPC->Get_16(reg.x.p);
	reg.r.pc+=SIZE_16;
	AddState(3);
}
/* MV I,mn */
inline void Csc62015::Op_0b(void)
{
    reg.x.i=pPC->Get_16(reg.x.p);
	reg.r.pc+=SIZE_16;
	AddState(3);
}
/* MV X,lmn */
inline void Csc62015::Op_0c(void)
{
    reg.x.x=pPC->Get_20(reg.x.p);
	reg.r.pc+=SIZE_20;
	AddState(4);
}
/* MV Y,lmn */
inline void Csc62015::Op_0d(void)
{
    reg.x.y=pPC->Get_20(reg.x.p);
	reg.r.pc+=SIZE_20;
	AddState(4);
}
/* MV U,lmn */
inline void Csc62015::Op_0e(void)
{
    reg.x.u=pPC->Get_20(reg.x.p);
	reg.r.pc+=SIZE_20;
	AddState(4);
}
/* MV S,lmn */
inline void Csc62015::Op_0f(void)
{
    reg.x.s=pPC->Get_20(reg.x.p);
	reg.r.pc+=SIZE_20;
	AddState(4);
}
/* JP (n) */
inline void Csc62015::Op_10(void)
{
    reg.x.p=Get_i20(pPC->Get_8(reg.x.p),OPR1);
	AddState(6);
}
/* JP r3 */
inline void Csc62015::Op_11(void)
{
    reg.x.p=Get_r(pPC->Get_8(reg.x.p));
	AddState(4);
}
/* JR +n */
inline void Csc62015::Op_12(void)
{
    reg.r.pc+=pPC->Get_8(reg.x.p);
	reg.r.pc++;
	AddState(3);
}
/* JR -n */
inline void Csc62015::Op_13(void)
{
    reg.r.pc-=pPC->Get_8(reg.x.p);
	reg.r.pc++;
	AddState(3);
}
/* JPZ mn */
inline void Csc62015::Op_14(void)
{
    WORD	t;
    t=pPC->Get_16(reg.x.p);
	reg.r.pc+=SIZE_16;
	if(reg.r.z){reg.r.pc=t; AddState(1);}
	AddState(3);
}
/* JPNZ mn */
inline void Csc62015::Op_15(void)
{
    WORD	t;
    t=pPC->Get_16(reg.x.p);
	reg.r.pc+=SIZE_16;
	if(!reg.r.z){ reg.r.pc=t; AddState(1);}
	AddState(3);
}
/* JPC mn */
inline void Csc62015::Op_16(void)
{
    WORD	t;
    t=pPC->Get_16(reg.x.p);
	reg.r.pc+=SIZE_16;
	if(reg.r.c){ reg.r.pc=t; AddState(1);}
	AddState(3);
}
/* JPNC mn */
inline void Csc62015::Op_17(void)
{
    WORD	t;
    t=pPC->Get_16(reg.x.p);
	reg.r.pc+=SIZE_16;
	if(!reg.r.c){ reg.r.pc=t; AddState(1);}
	AddState(3);
}
/* JRZ +n */
inline void Csc62015::Op_18(void)
{
    BYTE	t;
    t=pPC->Get_8(reg.x.p);
	reg.r.pc++;
	if(reg.r.z){ reg.r.pc+=t; AddState(1);}
	AddState(2);
}
/* JRZ -n */
inline void Csc62015::Op_19(void)
{
    BYTE	t;
    t=pPC->Get_8(reg.x.p);
	reg.r.pc++;
	if(reg.r.z){ reg.r.pc-=t; AddState(1);}
	AddState(2);
}
/* JRNZ +n */
inline void Csc62015::Op_1a(void)
{
    BYTE	t;
    t=pPC->Get_8(reg.x.p);
	reg.r.pc++;
	if(!reg.r.z){ reg.r.pc+=t; AddState(1);}
	AddState(2);
}
/* JRNZ -n */
inline void Csc62015::Op_1b(void)
{
    BYTE	t;
    t=pPC->Get_8(reg.x.p);
	reg.r.pc++;
	if(!reg.r.z){ reg.r.pc-=t; AddState(1);}
	AddState(2);
}
/* JRC +n */
inline void Csc62015::Op_1c(void)
{
    BYTE	t;
    t=pPC->Get_8(reg.x.p);
	reg.r.pc++;
	if(reg.r.c){ reg.r.pc+=t; AddState(1);}
	AddState(2);
}
/* JRC -n */
inline void Csc62015::Op_1d(void)
{
    BYTE	t;
    t=pPC->Get_8(reg.x.p);
	reg.r.pc++;
	if(reg.r.c){ reg.r.pc-=t; AddState(1);}
	AddState(2);
}
/* JRNC +n */
inline void Csc62015::Op_1e(void)
{
    BYTE	t;
    t=pPC->Get_8(reg.x.p);
	reg.r.pc++;
	if(!reg.r.c){ reg.r.pc+=t; AddState(1);}
	AddState(2);
}
/* JRNC -n */
inline void Csc62015::Op_1f(void)
{
    BYTE	t;
    t=pPC->Get_8(reg.x.p);
	reg.r.pc++;
	if(!reg.r.c){ reg.r.pc-=t; AddState(1);}
	AddState(2);
}
/* Undefined Code */
inline void Csc62015::Op_20(void)
{
//UN_DEFINE
	AddState(1);
}
/* PRE 21h */
inline void Csc62015::Op_21(void)
{
	pre_1=1;
	pre_2=3;
	AddState(1);
	Step_sc62015_();
}
/* PRE 22h */
inline void Csc62015::Op_22(void)
{
	pre_1=1;
	pre_2=0;
	AddState(1);
	Step_sc62015_();
}
/* PRE 23h */
inline void Csc62015::Op_23(void)
{
	pre_1=1;
	pre_2=2;
	AddState(1);
	Step_sc62015_();
}
/* PRE 24h */
inline void Csc62015::Op_24(void)
{
	pre_1=3;
	pre_2=1;
	AddState(1);
	Step_sc62015_();
}
/* PRE 25h */
inline void Csc62015::Op_25(void)
{
	pre_1=3;
	pre_2=3;
	AddState(1);
	Step_sc62015_();
}
/* PRE 26h */
inline void Csc62015::Op_26(void)
{
	pre_1=3;
	pre_2=0;
	AddState(1);
	Step_sc62015_();
}
/* PRE 27h */
inline void Csc62015::Op_27(void)
{
	pre_1=3;
	pre_2=2;
	AddState(1);
	Step_sc62015_();
}
/* PUSHU A */
inline void Csc62015::Op_28(void)
{
	reg.x.u--;
    pPC->Set_8(reg.x.u,reg.r.a);
	AddState(3);
}
/* PUSH IL */
inline void Csc62015::Op_29(void)
{
	reg.x.u--;
    pPC->Set_8(reg.x.u,reg.r.il);
	AddState(3);
}
/* PUSHU BA */
inline void Csc62015::Op_2a(void)
{
	reg.x.u-=SIZE_16;
    pPC->Set_16(reg.x.u,reg.x.ba);
	AddState(4);
}
/* PUSHU I */
inline void Csc62015::Op_2b(void)
{
	reg.x.u-=SIZE_16;
    pPC->Set_16(reg.x.u,reg.x.i);
	AddState(4);
}
/* PUSHU X */
inline void Csc62015::Op_2c(void)
{
	reg.x.u-=SIZE_20;
    pPC->Set_20(reg.x.u,reg.x.x);
	AddState(5);
}
/* PUSHU Y */
inline void Csc62015::Op_2d(void)
{
	reg.x.u-=SIZE_20;
    pPC->Set_20(reg.x.u,reg.x.y);
	AddState(5);
}
/* PUSHU F */
inline void Csc62015::Op_2e(void)
{
	reg.x.u--;
    pPC->Set_8(reg.x.u,reg.x.f);
	AddState(3);
}
/* PUSHU IMR */
inline void Csc62015::Op_2f(void)
{
	reg.x.u--;
    pPC->Set_8(reg.x.u,imem[IMEM_IMR]);
	imem[IMEM_IMR]&=0x7f;			/* disable interrupt */
	AddState(3);
}
/* PRE 30h */
inline void Csc62015::Op_30(void)
{
	pre_1=0;
	pre_2=1;
	AddState(1);
	Step_sc62015_();
}
/* PRE 31h */
inline void Csc62015::Op_31(void)
{
	pre_1=0;
	pre_2=3;
	AddState(1);
	Step_sc62015_();
}
/* PRE 32h */
inline void Csc62015::Op_32(void)
{
	pre_1=0;
	pre_2=0;
	AddState(1);
	Step_sc62015_();
}
/* PRE 33h */
inline void Csc62015::Op_33(void)
{
	pre_1=0;
	pre_2=2;
	AddState(1);
	Step_sc62015_();
}
/* PRE 34h */
inline void Csc62015::Op_34(void)
{
	pre_1=2;
	pre_2=1;
	AddState(1);
	Step_sc62015_();
}
/* PRE 35h */
inline void Csc62015::Op_35(void)
{
	pre_1=2;
	pre_2=3;
	AddState(1);
	Step_sc62015_();
}
/* PRE 36h */
inline void Csc62015::Op_36(void)
{
	pre_1=2;
	pre_2=0;
	AddState(1);
	Step_sc62015_();
}
/* PRE 37h */
inline void Csc62015::Op_37(void)
{
	pre_1=2;
	pre_2=2;
	AddState(1);
	Step_sc62015_();
}
/* POPU A */
inline void Csc62015::Op_38(void)
{
    reg.r.a=pPC->Get_8(reg.x.u);
	reg.x.u++;
	AddState(2);
}
/* POPU IL */
inline void Csc62015::Op_39(void)
{
    reg.r.il=pPC->Get_8(reg.x.u);
	reg.x.u++;
	reg.r.ih=0;
	AddState(3);
}
/* POPU BA */
inline void Csc62015::Op_3a(void)
{
    reg.x.ba=pPC->Get_16(reg.x.u);
	reg.x.u+=SIZE_16;
	AddState(3);
}
/* POPU I */
inline void Csc62015::Op_3b(void)
{
    reg.x.i=pPC->Get_16(reg.x.u);
	reg.x.u+=SIZE_16;
	AddState(3);
}
/* POPU X */
inline void Csc62015::Op_3c(void)
{
    reg.x.x=pPC->Get_20(reg.x.u);
	reg.x.u+=SIZE_20;
	AddState(4);
}
/* POPU Y */
inline void Csc62015::Op_3d(void)
{
    reg.x.y=pPC->Get_20(reg.x.u);
	reg.x.u+=SIZE_20;
	AddState(4);
}
/* POPU F */
inline void Csc62015::Op_3e(void)
{
    reg.x.f=pPC->Get_8(reg.x.u);
	reg.x.u++;
	AddState(2);
}
/* POPU IMR */
inline void Csc62015::Op_3f(void)
{
    imem[IMEM_IMR]=pPC->Get_8(reg.x.u);
	reg.x.u++;
	AddState(2);
}
/* ADD A,n */
inline void Csc62015::Op_40(void)
{
    UINT32	t;
    t=reg.r.a+pPC->Get_8(reg.x.p);
	reg.r.pc++;
	Chk_Flag(t,SIZE_8);
	reg.r.a=t;
	AddState(3);
}
/* ADD (m),n */
inline void Csc62015::Op_41(void)
{
    BYTE	a;
    UINT32	t;
    a=pPC->Get_8(reg.x.p);
	reg.r.pc++;
    t=Get_i8(a,OPR1)+pPC->Get_8(reg.x.p);
	reg.r.pc++;
	Chk_Flag(t,SIZE_8);
	Set_i8(a,t,OPR1);
	AddState(4);
}
/* ADD A,(n) */
inline void Csc62015::Op_42(void)
{
    UINT32	t;
    t=reg.r.a+Get_i8(pPC->Get_8(reg.x.p),OPR1);
	reg.r.pc++;
	Chk_Flag(t,SIZE_8);
	reg.r.a=t;
	AddState(4);
}
/* ADD (n),A */
inline void Csc62015::Op_43(void)
{
    BYTE	a;
    UINT32	t;
    a=pPC->Get_8(reg.x.p);
	reg.r.pc++;
	t=reg.r.a+Get_i8(a,OPR1);
	Chk_Flag(t,SIZE_8);
	Set_i8(a,t,OPR1);
	AddState(4);
}
/* ADD r2,r2' */
inline void Csc62015::Op_44(void)
{
    BYTE	t;
    UINT32	d;
    t=pPC->Get_8(reg.x.p);
	reg.r.pc++;
	d=(Get_r(t>>4)&MASK_16)+(Get_r(t)&MASK_16);
	Chk_Flag(d,SIZE_16);
	Set_r(t>>4,d);
	AddState(5);
}
/* ADD r3,r' */
inline void Csc62015::Op_45(void)
{
    BYTE	t;
    UINT32	d;
    t=pPC->Get_8(reg.x.p);
	reg.r.pc++;
	d=Get_r(t>>4)+Get_r(t);
	Chk_Flag(d,SIZE_20);
	Set_r(t>>4,d);
	AddState(7);
}
/* ADD r1,r1' */
inline void Csc62015::Op_46(void)
{
    BYTE	t;
    UINT32	d;
    t=pPC->Get_8(reg.x.p);
	reg.r.pc++;
	d=(Get_r(t>>4)&MASK_8)+(Get_r(t)&MASK_8);
	Chk_Flag(d,SIZE_8);
	Set_r(t>>4,d);
	AddState(3);
}
/* PMDF (m),n */
inline void Csc62015::Op_47(void)
{
    BYTE	a;
    a=pPC->Get_8(reg.x.p);
	reg.r.pc++;
    Set_i8(a,Get_i8(a,OPR1)+pPC->Get_8(reg.x.p),OPR1);
	reg.r.pc++;
	AddState(4);
}
/* SUB A,n */
inline void Csc62015::Op_48(void)
{
    UINT32	t;
    t=reg.r.a-pPC->Get_8(reg.x.p);
	reg.r.pc++;
	Chk_Flag(t,SIZE_8);
	reg.r.a=t;
	AddState(3);
}
/* SUB (m),n */
inline void Csc62015::Op_49(void)
{
    BYTE	a;
    UINT32	t;
    a=pPC->Get_8(reg.x.p);
	reg.r.pc++;
    t=Get_i8(a,OPR1)-pPC->Get_8(reg.x.p);
	reg.r.pc++;
	Chk_Flag(t,SIZE_8);
	Set_i8(a,t,OPR1);
	AddState(4);
}
/* SUB A,(n) */
inline void Csc62015::Op_4a(void)
{
    UINT32	t;
    t=reg.r.a-Get_i8(pPC->Get_8(reg.x.p),OPR1);
	reg.r.pc++;
	Chk_Flag(t,SIZE_8);
	reg.r.a=t;
	AddState(4);
}
/* SUB (n),A */
inline void Csc62015::Op_4b(void)
{
    BYTE	a;
    UINT32	t;
    a=pPC->Get_8(reg.x.p);
	reg.r.pc++;
	t=Get_i8(a,OPR1)-reg.r.a;
	Chk_Flag(t,SIZE_8);
	Set_i8(a,t,OPR1);
	AddState(4);
}
/* SUB r2,r2' */
inline void Csc62015::Op_4c(void)
{
    BYTE	t;
    UINT32	d;
    t=pPC->Get_8(reg.x.p);
	reg.r.pc++;
	d=(Get_r(t>>4)&MASK_16)-(Get_r(t)&MASK_16);
	Chk_Flag(d,SIZE_16);
	Set_r(t>>4,d);
	AddState(5);
}
/* SUB r3,r' */
inline void Csc62015::Op_4d(void)
{
    BYTE	t;
    UINT32	d;
    t=pPC->Get_8(reg.x.p);
	reg.r.pc++;
	d=Get_r(t>>4)-Get_r(t);
	Chk_Flag(d,SIZE_20);
	Set_r(t>>4,d);
	AddState(7);
}
/* SUB r1,r1' */
inline void Csc62015::Op_4e(void)
{
    BYTE	t;
    UINT32	d;
    t=pPC->Get_8(reg.x.p);
	reg.r.pc++;
	d=(Get_r(t>>4)&MASK_8)-(Get_r(t)&MASK_8);
	Chk_Flag(d,SIZE_8);
	Set_r(t>>4,d);
	AddState(3);
}
/* PUSHS F */
inline void Csc62015::Op_4f(void)
{
	reg.x.s--;
    pPC->Set_8(reg.x.s,reg.x.f);
	AddState(3);
}
/* ADC A,n */
inline void Csc62015::Op_50(void)
{
    UINT32	t;
    t=reg.r.a+pPC->Get_8(reg.x.p)+reg.r.c;
	reg.r.pc++;
	Chk_Flag(t,SIZE_8);
	reg.r.a=t;
	AddState(3);
}
/* ADC (m),n */
inline void Csc62015::Op_51(void)
{
    BYTE	a;
    UINT32	t;
    a=pPC->Get_8(reg.x.p);
	reg.r.pc++;
    t=Get_i8(a,OPR1)+pPC->Get_8(reg.x.p)+reg.r.c;
	reg.r.pc++;
	Chk_Flag(t,SIZE_8);
	Set_i8(a,t,OPR1);
	AddState(4);
}
/* ADC A,(n) */
inline void Csc62015::Op_52(void)
{
    UINT32	t;
    t=reg.r.a+Get_i8(pPC->Get_8(reg.x.p),OPR1)+reg.r.c;
	reg.r.pc++;
	Chk_Flag(t,SIZE_8);
	reg.r.a=t;
	AddState(4);
}
/* ADC (n),A */
inline void Csc62015::Op_53(void)
{
    BYTE	a;
    UINT32	t;
    a=pPC->Get_8(reg.x.p);
	reg.r.pc++;
	t=Get_i8(a,OPR1)+reg.r.a+reg.r.c;
	Chk_Flag(t,SIZE_8);
	Set_i8(a,t,OPR1);
	AddState(4);
}
/* ADCL (m),(n) */
inline void Csc62015::Op_54(void)
{
    BYTE	m,n;
    UINT32	t,b;
    AddState(5);
    m=Conv_imemAdr(pPC->Get_8(reg.x.p),OPR1);
	reg.r.pc++;
    n=Conv_imemAdr(pPC->Get_8(reg.x.p),OPR2);
	reg.r.pc++;
	t=Get_i8d(m)+Get_i8d(n++);
	Chk_Flag(t,SIZE_8);
	Set_i8d(m++,t);
	b=t;
	while(--reg.x.i!=0){
        AddState(2);
		t=Get_i8d(m)+Get_i8d(n++)+reg.r.c;
		Chk_Flag(t,SIZE_8);
		Set_i8d(m++,t);
		b|=t;
	}
	Chk_Zero(b,SIZE_8);
}
/* ADCL (n),A */
inline void Csc62015::Op_55(void)
{
    BYTE	a;
    UINT32	t,b;
    AddState(4);
    a=Conv_imemAdr(pPC->Get_8(reg.x.p),OPR1);
	reg.r.pc++;
	t=Get_i8d(a)+reg.r.a;
	Chk_Flag(t,SIZE_8);
	Set_i8d(a++,t);
	b=t;
	while(--reg.x.i!=0){
        AddState(1);
		t=Get_i8d(a)+reg.r.c;
		Chk_Flag(t,SIZE_8);
		Set_i8d(a++,t);
		b|=t;
	}
	Chk_Zero(b,SIZE_8);
}
/* MVL (m),[r3+-n] */
inline void Csc62015::Op_56(void)
{
    BYTE	d;
    UINT32	s;
    AddState(5);
	s=Get_d2(SIZE_8,&d);
	d=Conv_imemAdr(d,OPR1);
    while(--reg.x.i!=0) {
        AddState(2);
        Set_i8d(d++,pPC->Get_8(s++));
    }
    Set_i8d(d,pPC->Get_8(s));
}
/* PMDF (n),A */
inline void Csc62015::Op_57(void)
{
    BYTE	a;
    a=pPC->Get_8(reg.x.p);
	reg.r.pc++;
	Set_i8(a,reg.r.a+Get_i8(a,OPR1),OPR1);
	AddState(4);
}
/* SBC A,n */
inline void Csc62015::Op_58(void)
{
    UINT32	t;
    t=reg.r.a-pPC->Get_8(reg.x.p)-reg.r.c;
	reg.r.pc++;
	Chk_Flag(t,SIZE_8);
	reg.r.a=t;
	AddState(3);
}
/* SBC (m),n */
inline void Csc62015::Op_59(void)
{
    BYTE	a;
    UINT32	t;
    a=pPC->Get_8(reg.x.p);
	reg.r.pc++;
    t=Get_i8(a,OPR1)-pPC->Get_8(reg.x.p)-reg.r.c;
	reg.r.pc++;
	Chk_Flag(t,SIZE_8);
	Set_i8(a,t,OPR1);
	AddState(4);
}
/* SBC A,(n) */
inline void Csc62015::Op_5a(void)
{
    UINT32	t;
    t=reg.r.a-Get_i8(pPC->Get_8(reg.x.p),OPR1)-reg.r.c;
	reg.r.pc++;
	Chk_Flag(t,SIZE_8);
	reg.r.a=t;
	AddState(4);
}
/* SBC (n),A */
inline void Csc62015::Op_5b(void)
{
    BYTE	a;
    UINT32	t;
    a=pPC->Get_8(reg.x.p);
	reg.r.pc++;
	t=Get_i8(a,OPR1)-reg.r.a-reg.r.c;
	Chk_Flag(t,SIZE_8);
	Set_i8(a,t,OPR1);
	AddState(4);
}
/* SBCL (m),(n) */
inline void Csc62015::Op_5c(void)
{
    BYTE	m,n;
    UINT32	t,b;
    AddState(5);
    m=Conv_imemAdr(pPC->Get_8(reg.x.p),OPR1);
	reg.r.pc++;
    n=Conv_imemAdr(pPC->Get_8(reg.x.p),OPR2);
	reg.r.pc++;
	t=Get_i8d(m)-Get_i8d(n++);
	Chk_Flag(t,SIZE_8);
	Set_i8d(m++,t);
	b=t;
	while(--reg.x.i!=0){
        AddState(2);
		t=Get_i8d(m)-Get_i8d(n++)-reg.r.c;
		Chk_Flag(t,SIZE_8);
		Set_i8d(m++,t);
		b|=t;
	}
	Chk_Zero(b,SIZE_8);
}
/* SBCL (n),A */
inline void Csc62015::Op_5d(void)
{
    BYTE	a;
    UINT32	t,b;
    AddState(4);
    a=Conv_imemAdr(pPC->Get_8(reg.x.p),OPR1);
	reg.r.pc++;
	t=Get_i8d(a)-reg.r.a;
	Chk_Flag(t,SIZE_8);
	Set_i8d(a++,t);
	b=t;
	while(--reg.x.i!=0){
        AddState(1);
		t=Get_i8d(a)-reg.r.c;
		Chk_Flag(t,SIZE_8);
		Set_i8d(a++,t);
		b|=t;
	}
	Chk_Zero(b,SIZE_8);
}
/* MVL [r3+-n],(m) */
inline void Csc62015::Op_5e(void)
{
    BYTE	s;
    UINT32	d;
    AddState(5);
	d=Get_d2(SIZE_8,&s);
	s=Conv_imemAdr(s,OPR1);
    while(--reg.x.i!=0) {
        AddState(2);
        pPC->Set_8(d++,Get_i8d(s++));
    }
    pPC->Set_8(d,Get_i8d(s));
}
/* POPS F */
inline void Csc62015::Op_5f(void)
{
    reg.x.f=pPC->Get_8(reg.x.s);
	reg.x.s++;
	AddState(2);
}
/* CMP A,n */
inline void Csc62015::Op_60(void)
{
    UINT32	t;
    t=reg.r.a-pPC->Get_8(reg.x.p);
	reg.r.pc++;
	Chk_Flag(t,SIZE_8);
	AddState(3);
}
/* CMP (m),n */
inline void Csc62015::Op_61(void)
{
    BYTE	a;
    UINT32	t;
    a=pPC->Get_8(reg.x.p);
	reg.r.pc++;
    t=Get_i8(a,OPR1)-pPC->Get_8(reg.x.p);
	reg.r.pc++;
	Chk_Flag(t,SIZE_8);
	AddState(4);
}
/* CMP [klm],n */
inline void Csc62015::Op_62(void)
{
    BYTE	a;
    UINT32	t;
    a=pPC->Get_8(pPC->Get_20(reg.x.p));
	reg.r.pc+=SIZE_20;
    t=a-pPC->Get_8(reg.x.p);
	reg.r.pc++;
	Chk_Flag(t,SIZE_8);
	AddState(6);
}
/* CMP (n),A */
inline void Csc62015::Op_63(void)
{
    BYTE	a;
    UINT32	t;
    a=pPC->Get_8(reg.x.p);
	reg.r.pc++;
	t=Get_i8(a,OPR1)-reg.r.a;
	Chk_Flag(t,SIZE_8);
	AddState(4);
}
/* TEST A,n */
inline void Csc62015::Op_64(void)
{
    UINT32	t;
    t=reg.r.a&pPC->Get_8(reg.x.p);
	reg.r.pc++;
	Chk_Zero(t,SIZE_8);
	AddState(3);
}
/* TEST (m),n */
inline void Csc62015::Op_65(void)
{
    BYTE	a;
    UINT32	t;
    a=pPC->Get_8(reg.x.p);
	reg.r.pc++;
    t=Get_i8(a,OPR1)&pPC->Get_8(reg.x.p);
	reg.r.pc++;
	Chk_Zero(t,SIZE_8);
	AddState(4);
}
/* TEST [klm],n */
inline void Csc62015::Op_66(void)
{
    UINT32	t,a;
    a=pPC->Get_20(reg.x.p);
	reg.r.pc+=SIZE_20;
    t=pPC->Get_8(a)&pPC->Get_8(reg.x.p);
	reg.r.pc++;
	Chk_Zero(t,SIZE_8);
	AddState(6);
}
/* TEST (n),A */
inline void Csc62015::Op_67(void)
{
    UINT32	t;
    t=reg.r.a&Get_i8(pPC->Get_8(reg.x.p),OPR1);
	reg.r.pc++;
	Chk_Zero(t,SIZE_8);
	AddState(4);
}
/* XOR A,n */
inline void Csc62015::Op_68(void)
{
    UINT32	t;
    t=reg.r.a^pPC->Get_8(reg.x.p);
	reg.r.pc++;
	Chk_Zero(t,SIZE_8);
	reg.r.a=t;
	AddState(3);
}
/* XOR (m),n */
inline void Csc62015::Op_69(void)
{
    BYTE	a;
    UINT32	t;
    a=pPC->Get_8(reg.x.p);
	reg.r.pc++;
    t=Get_i8(a,OPR1)^pPC->Get_8(reg.x.p);
	reg.r.pc++;
	Chk_Zero(t,SIZE_8);
	Set_i8(a,t,OPR1);
	AddState(4);
}
/* XOR [klm],n */
inline void Csc62015::Op_6a(void)
{
    UINT32	t,a;
    a=pPC->Get_20(reg.x.p);
	reg.r.pc+=SIZE_20;
    t=pPC->Get_8(a)^pPC->Get_8(reg.x.p);
	reg.r.pc++;
	Chk_Zero(t,SIZE_8);
    pPC->Set_8(a,t);
	AddState(7);
}
/* XOR (n),A */
inline void Csc62015::Op_6b(void)
{
    BYTE	a;
    UINT32	t;
    a=pPC->Get_8(reg.x.p);
	reg.r.pc++;
	t=reg.r.a^Get_i8(a,OPR1);
	Chk_Zero(t,SIZE_8);
	Set_i8(a,t,OPR1);
	AddState(4);
}
/* INC r */
inline void Csc62015::Op_6c(void)
{
	BYTE	reg_size[]={SIZE_8 ,SIZE_8 ,SIZE_16,SIZE_16,
						SIZE_20,SIZE_20,SIZE_20,SIZE_20};
    BYTE	t;
    UINT32	d;
    t=pPC->Get_8(reg.x.p);
	reg.r.pc++;
	d=Get_r(t);
	Chk_Zero(++d,reg_size[t&7]);
	Set_r(t,d);
	AddState(3);
}
/* INC (n) */
inline void Csc62015::Op_6d(void)
{
    BYTE	t;
    UINT32	d;
    t=pPC->Get_8(reg.x.p);
	reg.r.pc++;
	d=Get_i8(t,OPR1);
	Chk_Zero(++d,SIZE_8);
	Set_i8(t,d,OPR1);
	AddState(3);
}
/* XOR (m),(n) */
inline void Csc62015::Op_6e(void)
{
    BYTE	a;
    UINT32	t;
    a=pPC->Get_8(reg.x.p);
	reg.r.pc++;
    t=Get_i8(a,OPR1)^Get_i8(pPC->Get_8(reg.x.p),OPR2);
	reg.r.pc++;
	Chk_Zero(t,SIZE_8);
	Set_i8(a,t,OPR1);
	AddState(6);
}
/* XOR A,(n) */
inline void Csc62015::Op_6f(void)
{
    UINT32	t;
    t=reg.r.a^Get_i8(pPC->Get_8(reg.x.p),OPR1);
	reg.r.pc++;
	Chk_Zero(t,SIZE_8);
	reg.r.a=t;
	AddState(4);
}
/* AND A,n */
inline void Csc62015::Op_70(void)
{
    UINT32	t;
    t=reg.r.a&pPC->Get_8(reg.x.p);
	reg.r.pc++;
	Chk_Zero(t,SIZE_8);
	reg.r.a=t;
	AddState(3);
}
/* AND (m),n */
inline void Csc62015::Op_71(void)
{
    BYTE	a;
    UINT32	t;
    a=pPC->Get_8(reg.x.p);
	reg.r.pc++;
    t=Get_i8(a,OPR1)&pPC->Get_8(reg.x.p);
	reg.r.pc++;
	Chk_Zero(t,SIZE_8);
	Set_i8(a,t,OPR1);
	AddState(4);
}
/* AND [klm],n */
inline void Csc62015::Op_72(void)
{
    UINT32	t,a;
    a=pPC->Get_20(reg.x.p);
	reg.r.pc+=SIZE_20;
    t=pPC->Get_8(a)&pPC->Get_8(reg.x.p);
	reg.r.pc++;
	Chk_Zero(t,SIZE_8);
    pPC->Set_8(a,t);
	AddState(7);
}
/* AND (n),A */
inline void Csc62015::Op_73(void)
{
    BYTE	a;
    UINT32	t;
    a=pPC->Get_8(reg.x.p);
	reg.r.pc++;
	t=reg.r.a&Get_i8(a,OPR1);
	Chk_Zero(t,SIZE_8);
	Set_i8(a,t,OPR1);
	AddState(4);
}
/* MV A,B */
inline void Csc62015::Op_74(void)
{
	reg.r.a=reg.r.b;
	AddState(1);
}
/* MV B,A */
inline void Csc62015::Op_75(void)
{
	reg.r.b=reg.r.a;
	AddState(1);
}
/* AND (m),(n) */
inline void Csc62015::Op_76(void)
{
    BYTE	a;
    UINT32	t;
    a=pPC->Get_8(reg.x.p);
	reg.r.pc++;
    t=Get_i8(a,OPR1)&Get_i8(pPC->Get_8(reg.x.p),OPR2);
	reg.r.pc++;
	Chk_Zero(t,SIZE_8);
	Set_i8(a,t,OPR1);
	AddState(6);
}
/* AND A,(n) */
inline void Csc62015::Op_77(void)
{
    UINT32	t;
    t=reg.r.a&Get_i8(pPC->Get_8(reg.x.p),OPR1);
	reg.r.pc++;
	Chk_Zero(t,SIZE_8);
	reg.r.a=t;
	AddState(4);
}
/* OR A,n */
inline void Csc62015::Op_78(void)
{
    UINT32	t;
    t=reg.r.a|pPC->Get_8(reg.x.p);
	reg.r.pc++;
	Chk_Zero(t,SIZE_8);
	reg.r.a=t;
	AddState(3);
}
/* OR (m),n */
inline void Csc62015::Op_79(void)
{
    BYTE	a;
    UINT32	t;
    a=pPC->Get_8(reg.x.p);
	reg.r.pc++;
    t=Get_i8(a,OPR1)|pPC->Get_8(reg.x.p);
	reg.r.pc++;
	Chk_Zero(t,SIZE_8);
	Set_i8(a,t,OPR1);
	AddState(4);
}
/* OR [klm],n */
inline void Csc62015::Op_7a(void)
{
    UINT32	t,a;
    a=pPC->Get_20(reg.x.p);
	reg.r.pc+=SIZE_20;
    t=pPC->Get_8(a)|pPC->Get_8(reg.x.p);
	reg.r.pc++;
	Chk_Zero(t,SIZE_8);
    pPC->Set_8(a,t);
	AddState(7);
}
/* OR (n),A */
inline void Csc62015::Op_7b(void)
{
    BYTE	a;
    UINT32	t;
    a=pPC->Get_8(reg.x.p);
	reg.r.pc++;
	t=reg.r.a|Get_i8(a,OPR1);
	Chk_Zero(t,SIZE_8);
	Set_i8(a,t,OPR1);
	AddState(4);
}
/* DEC r */
inline void Csc62015::Op_7c(void)
{
	BYTE	reg_size[]={SIZE_8 ,SIZE_8 ,SIZE_16,SIZE_16,
						SIZE_20,SIZE_20,SIZE_20,SIZE_20};
    BYTE	t;
    UINT32	d;
    t=pPC->Get_8(reg.x.p);
	reg.r.pc++;
	d=Get_r(t);
	Chk_Zero(--d,reg_size[t&7]);
	Set_r(t,d);
	AddState(3);
}
/* DEC (n) */
inline void Csc62015::Op_7d(void)
{
    BYTE	t;
    UINT32	d;
    t=pPC->Get_8(reg.x.p);
	reg.r.pc++;
	d=Get_i8(t,OPR1);
	Chk_Zero(--d,SIZE_8);
	Set_i8(t,d,OPR1);
	AddState(3);
}
/* OR (m),(n) */
inline void Csc62015::Op_7e(void)
{
    BYTE	a;
    UINT32	t;
    a=pPC->Get_8(reg.x.p);
	reg.r.pc++;
    t=Get_i8(a,OPR1)|Get_i8(pPC->Get_8(reg.x.p),OPR2);
	reg.r.pc++;
	Chk_Zero(t,SIZE_8);
	Set_i8(a,t,OPR1);
	AddState(6);
}
/* OR A,(n) */
inline void Csc62015::Op_7f(void)
{
    UINT32	t;
    t=reg.r.a|Get_i8(pPC->Get_8(reg.x.p),OPR1);
	reg.r.pc++;
	Chk_Zero(t,SIZE_8);
	reg.r.a=t;
	AddState(4);
}
/* MV A,(n) */
inline void Csc62015::Op_80(void)
{
    reg.r.a=Get_i8(pPC->Get_8(reg.x.p),OPR1);
	reg.r.pc++;
	AddState(3);
}
/* MV IL,(n) */
inline void Csc62015::Op_81(void)
{
    reg.r.il=Get_i8(pPC->Get_8(reg.x.p),OPR1);
	reg.r.pc++;
	reg.r.ih=0;
	AddState(4);
}
/* MV BA,(n) */
inline void Csc62015::Op_82(void)
{
    reg.x.ba=Get_i16(pPC->Get_8(reg.x.p),OPR1);
	reg.r.pc++;
	AddState(4);
}
/* MV I,(n) */
inline void Csc62015::Op_83(void)
{
    reg.x.i=Get_i16(pPC->Get_8(reg.x.p),OPR1);
	reg.r.pc++;
	AddState(4);
}
/* MV X,(n) */
inline void Csc62015::Op_84(void)
{
    reg.x.x=Get_i20(pPC->Get_8(reg.x.p),OPR1);
	reg.r.pc++;
	AddState(5);
}
/* MV Y,(n) */
inline void Csc62015::Op_85(void)
{
    reg.x.y=Get_i20(pPC->Get_8(reg.x.p),OPR1);
	reg.r.pc++;
	AddState(5);
}
/* MV U,(n) */
inline void Csc62015::Op_86(void)
{
    reg.x.u=Get_i20(pPC->Get_8(reg.x.p),OPR1);
	reg.r.pc++;
	AddState(5);
}
/* MV S,(n) */
inline void Csc62015::Op_87(void)
{
    reg.x.s=Get_i20(pPC->Get_8(reg.x.p),OPR1);
	reg.r.pc++;
	AddState(5);
}
/* MV A,[lmn] */
inline void Csc62015::Op_88(void)
{
    reg.r.a=pPC->Get_8(pPC->Get_20(reg.x.p));
	reg.r.pc+=SIZE_20;
	AddState(6);
}
/* MV IL,[lmn] */
inline void Csc62015::Op_89(void)
{
    reg.r.il=pPC->Get_8(pPC->Get_20(reg.x.p));
	reg.r.pc+=SIZE_20;
	reg.r.ih=0;
	AddState(6);
}
/* MV BA,[lmn] */
inline void Csc62015::Op_8a(void)
{
    reg.x.ba=pPC->Get_16(pPC->Get_20(reg.x.p));
	reg.r.pc+=SIZE_20;
	AddState(7);
}
/* MV I,[lmn] */
inline void Csc62015::Op_8b(void)
{
    reg.x.i=pPC->Get_16(pPC->Get_20(reg.x.p));
	reg.r.pc+=SIZE_20;
	AddState(7);
}
/* MV X,[lmn] */
inline void Csc62015::Op_8c(void)
{
    reg.x.x=pPC->Get_20(pPC->Get_20(reg.x.p));
	reg.r.pc+=SIZE_20;
	AddState(8);
}
/* MV Y,[lmn] */
inline void Csc62015::Op_8d(void)
{
    reg.x.y=pPC->Get_20(pPC->Get_20(reg.x.p));
	reg.r.pc+=SIZE_20;
	AddState(8);
}
/* MV U,[lmn] */
inline void Csc62015::Op_8e(void)
{
    reg.x.u=pPC->Get_20(pPC->Get_20(reg.x.p));
	reg.r.pc+=SIZE_20;
	AddState(8);
}
/* MV S,[lmn] */
inline void Csc62015::Op_8f(void)
{
    reg.x.s=pPC->Get_20(pPC->Get_20(reg.x.p));
	reg.r.pc+=SIZE_20;
	AddState(8);
}
/* MV A,[r3] */
inline void Csc62015::Op_90(void)
{
    reg.r.a=pPC->Get_8(Get_d(SIZE_8));
	AddState(4);
}
/* MV IL,[r3] */
inline void Csc62015::Op_91(void)
{
    reg.x.i=pPC->Get_8(Get_d(SIZE_8));
	AddState(5);
}
/* MV BA,[r3] */
inline void Csc62015::Op_92(void)
{
    reg.x.ba=pPC->Get_16(Get_d(SIZE_16));
	AddState(5);
}
/* MV I,[r3] */
inline void Csc62015::Op_93(void)
{
    reg.x.i=pPC->Get_16(Get_d(SIZE_16));
	AddState(5);
}
/* MV X,[r3] */
inline void Csc62015::Op_94(void)
{
    reg.x.x=pPC->Get_20(Get_d(SIZE_20));
	AddState(6);
}
/* MV Y,[r3] */
inline void Csc62015::Op_95(void)
{
    reg.x.y=pPC->Get_20(Get_d(SIZE_20));
	AddState(6);
}
/* MV U,[r3] */
inline void Csc62015::Op_96(void)
{
    reg.x.u=pPC->Get_20(Get_d(SIZE_20));
	AddState(6);
}
/* SC */
inline void Csc62015::Op_97(void)
{
	reg.r.c=1;
	AddState(1);
}
/* MV A,[(n)] */
inline void Csc62015::Op_98(void)
{
    reg.r.a=pPC->Get_8(Get_i());
	AddState(9);
}
/* MV IL,[(n)] */
inline void Csc62015::Op_99(void)
{
    reg.r.il=pPC->Get_8(Get_i());
	reg.r.ih=0;
	AddState(10);
}
/* MV BA,[(n)] */
inline void Csc62015::Op_9a(void)
{
    reg.x.ba=pPC->Get_16(Get_i());
	AddState(10);
}
/* MV I,[(n)] */
inline void Csc62015::Op_9b(void)
{
    reg.x.i=pPC->Get_16(Get_i());
	AddState(10);
}
/* MV X,[(n)] */
inline void Csc62015::Op_9c(void)
{
    reg.x.x=pPC->Get_20(Get_i());
	AddState(11);
}
/* MV Y,[(n)] */
inline void Csc62015::Op_9d(void)
{
    reg.x.y=pPC->Get_20(Get_i());
	AddState(11);
}
/* MV U,[(n)] */
inline void Csc62015::Op_9e(void)
{
    reg.x.u=pPC->Get_20(Get_i());
	AddState(11);
}
/* RC */
inline void Csc62015::Op_9f(void)
{
	reg.r.c=0;
	AddState(1);
}
/* MV (n),A */
inline void Csc62015::Op_a0(void)
{
    Set_i8(pPC->Get_8(reg.x.p),reg.r.a,OPR1);
	reg.r.pc++;
	AddState(3);
}
/* MV (n),IL */
inline void Csc62015::Op_a1(void)
{
    Set_i8(pPC->Get_8(reg.x.p),reg.r.il,OPR1);
	reg.r.pc++;
	AddState(3);
}
/* MV (n),BA */
inline void Csc62015::Op_a2(void)
{
    Set_i16(pPC->Get_8(reg.x.p),reg.x.ba,OPR1);
	reg.r.pc++;
	AddState(4);
}
/* MV (n),I */
inline void Csc62015::Op_a3(void)
{
    Set_i16(pPC->Get_8(reg.x.p),reg.x.i,OPR1);
	reg.r.pc++;
	AddState(4);
}
/* MV (n),X */
inline void Csc62015::Op_a4(void)
{
    Set_i20(pPC->Get_8(reg.x.p),reg.x.x,OPR1);
	reg.r.pc++;
	AddState(5);
}
/* MV (n),Y */
inline void Csc62015::Op_a5(void)
{
    Set_i20(pPC->Get_8(reg.x.p),reg.x.y,OPR1);
	reg.r.pc++;
	AddState(5);
}
/* MV (n),U */
inline void Csc62015::Op_a6(void)
{
    Set_i20(pPC->Get_8(reg.x.p),reg.x.u,OPR1);
	reg.r.pc++;
	AddState(5);
}
/* MV (n),S */
inline void Csc62015::Op_a7(void)
{
    Set_i20(pPC->Get_8(reg.x.p),reg.x.s,OPR1);
	reg.r.pc++;
	AddState(5);
}
/* MV [lmn],A */
inline void Csc62015::Op_a8(void)
{
    pPC->Set_8(pPC->Get_20(reg.x.p),reg.r.a);
	reg.r.pc+=SIZE_20;
	AddState(5);
}
/* MV [lmn],IL */
inline void Csc62015::Op_a9(void)
{
    pPC->Set_8(pPC->Get_20(reg.x.p),reg.r.il);
	reg.r.pc+=SIZE_20;
	AddState(5);
}
/* MV [lmn],BA */
inline void Csc62015::Op_aa(void)
{
    pPC->Set_16(pPC->Get_20(reg.x.p),reg.x.ba);
	reg.r.pc+=SIZE_20;
	AddState(6);
}
/* MV [lmn],I */
inline void Csc62015::Op_ab(void)
{
    pPC->Set_16(pPC->Get_20(reg.x.p),reg.x.i);
	reg.r.pc+=SIZE_20;
	AddState(6);
}
/* MV [lmn],X */
inline void Csc62015::Op_ac(void)
{
    pPC->Set_20(pPC->Get_20(reg.x.p),reg.x.x);
	reg.r.pc+=SIZE_20;
	AddState(7);
}
/* MV [lmn],Y */
inline void Csc62015::Op_ad(void)
{
    pPC->Set_20(pPC->Get_20(reg.x.p),reg.x.y);
	reg.r.pc+=SIZE_20;
	AddState(7);
}
/* MV [lmn],U */
inline void Csc62015::Op_ae(void)
{
    pPC->Set_20(pPC->Get_20(reg.x.p),reg.x.u);
	reg.r.pc+=SIZE_20;
	AddState(7);
}
/* MV [lmn],S */
inline void Csc62015::Op_af(void)
{
    pPC->Set_20(pPC->Get_20(reg.x.p),reg.x.s);
	reg.r.pc+=SIZE_20;
	AddState(7);
}
/* MV [r3],A */
inline void Csc62015::Op_b0(void)
{
    pPC->Set_8(Get_d(SIZE_8),reg.r.a);
	AddState(4);
}
/* MV [r3],IL */
inline void Csc62015::Op_b1(void)
{
    pPC->Set_8(Get_d(SIZE_8),reg.r.il);
	AddState(4);
}
/* MV [r3],BA */
inline void Csc62015::Op_b2(void)
{
    pPC->Set_16(Get_d(SIZE_16),reg.x.ba);
	AddState(5);
}
/* MV [r3],I */
inline void Csc62015::Op_b3(void)
{
    pPC->Set_16(Get_d(SIZE_16),reg.x.i);
	AddState(5);
}
/* MV [r3],X */
inline void Csc62015::Op_b4(void)
{
    pPC->Set_20(Get_d(SIZE_20),reg.x.x);
	AddState(6);
}
/* MV [r3],Y */
inline void Csc62015::Op_b5(void)
{
    pPC->Set_20(Get_d(SIZE_20),reg.x.y);
	AddState(6);
}
/* MV [r3],U */
inline void Csc62015::Op_b6(void)
{
    pPC->Set_20(Get_d(SIZE_20),reg.x.u);
	AddState(6);
}
/* CMP (m),(n) */
inline void Csc62015::Op_b7(void)
{
    UINT32	t;
    t=Get_i8(pPC->Get_8(reg.x.p),OPR1);
	reg.r.pc++;
    t-=Get_i8(pPC->Get_8(reg.x.p),OPR2);
	reg.r.pc++;
	Chk_Flag(t,SIZE_8);
	AddState(6);
}
/* MV [(n)],A */
inline void Csc62015::Op_b8(void)
{
    pPC->Set_8(Get_i(),reg.r.a);
	AddState(9);
}
/* MV [(n)],IL */
inline void Csc62015::Op_b9(void)
{
    pPC->Set_8(Get_i(),reg.r.il);
	AddState(9);
}
/* MV [(n)],BA */
inline void Csc62015::Op_ba(void)
{
    pPC->Set_16(Get_i(),reg.x.ba);
	AddState(10);
}
/* MV [(n)],I */
inline void Csc62015::Op_bb(void)
{
    pPC->Set_16(Get_i(),reg.x.i);
	AddState(10);
}
/* MV [(n)],X */
inline void Csc62015::Op_bc(void)
{
    pPC->Set_20(Get_i(),reg.x.x);
	AddState(11);
}
/* MV [(n)],Y */
inline void Csc62015::Op_bd(void)
{
    pPC->Set_20(Get_i(),reg.x.y);
	AddState(11);
}
/* MV [(n)],U */
inline void Csc62015::Op_be(void)
{
    pPC->Set_20(Get_i(),reg.x.u);
	AddState(11);
}
/* Undefined Code */
inline void Csc62015::Op_bf(void)
{
//UN_DEFINE
	AddState(1);
}
/* EX (m),(n) */
inline void Csc62015::Op_c0(void)
{
    BYTE	d,s;
    BYTE	w;
    d=pPC->Get_8(reg.x.p);
	reg.r.pc++;
    s=pPC->Get_8(reg.x.p);
	reg.r.pc++;
	w=Get_i8(d,OPR1); Set_i8(d,Get_i8(s,OPR2),OPR1); Set_i8(s,w,OPR2);
	AddState(7);
}
/* EXW (m),(n) */
inline void Csc62015::Op_c1(void)
{
    BYTE	d,s;
    WORD	w;
    d=pPC->Get_8(reg.x.p);
	reg.r.pc++;
    s=pPC->Get_8(reg.x.p);
	reg.r.pc++;
	w=Get_i16(d,OPR1); Set_i16(d,Get_i16(s,OPR2),OPR1); Set_i16(s,w,OPR2);
	AddState(10);
}
/* EXP (m),(n) */
inline void Csc62015::Op_c2(void)
{
    BYTE	d,s;
    UINT32	w;
    d=pPC->Get_8(reg.x.p);
	reg.r.pc++;
    s=pPC->Get_8(reg.x.p);
	reg.r.pc++;
	w=Get_i24(d,OPR1); Set_i24(d,Get_i24(s,OPR2),OPR1); Set_i24(s,w,OPR2);
	AddState(13);
}
/* EXL (m),(n) */
inline void Csc62015::Op_c3(void)
{
    BYTE	d,s,w;
    AddState(5);
    d=Conv_imemAdr(pPC->Get_8(reg.x.p),OPR1);
	reg.r.pc++;
    s=Conv_imemAdr(pPC->Get_8(reg.x.p),OPR2);
	reg.r.pc++;
	while(--reg.x.i!=0){
        AddState(3);
		w=Get_i8d(d); Set_i8d(d,Get_i8d(s)); Set_i8d(s,w);
		s++; d++;
	}
	w=Get_i8d(d); Set_i8d(d,Get_i8d(s)); Set_i8d(s,w);
}
/* DADL (m),(n) */
inline void Csc62015::Op_c4(void)
{
    BYTE	d,s,b;
    WORD	w;
    AddState(5);
    d=Conv_imemAdr(pPC->Get_8(reg.x.p),OPR1);
	reg.r.pc++;
    s=Conv_imemAdr(pPC->Get_8(reg.x.p),OPR2);
	reg.r.pc++;
	w=hex2bcd(bcd2hex(Get_i8d(d))+bcd2hex(Get_i8d(s--)));
	Chk_Flag(w,SIZE_8);
	Set_i8d(d--,w);
	b=w;
	while(--reg.x.i!=0){
        AddState(2);
		w=hex2bcd(bcd2hex(Get_i8d(d))+bcd2hex(Get_i8d(s--))+reg.r.c);
		Chk_Flag(w,SIZE_8);
		Set_i8d(d--,w);
		b|=w;
	}
	Chk_Zero(b,SIZE_8);
//printf("DADL (m),(n)\n");
}
/* DADL (n),A */
inline void Csc62015::Op_c5(void)
{
    BYTE	d,b;
    WORD	w;
    AddState(4);
    d=Conv_imemAdr(pPC->Get_8(reg.x.p),OPR1);
	reg.r.pc++;
	w=hex2bcd(bcd2hex(Get_i8d(d))+bcd2hex(reg.r.a));
	Chk_Flag(w,SIZE_8);
	Set_i8d(d--,w);
	b=w;
	while(--reg.x.i!=0){
        AddState(1);
		w=hex2bcd(bcd2hex(Get_i8d(d))+reg.r.c);
		Chk_Flag(w,SIZE_8);
		Set_i8d(d--,w);
		b|=w;
	}
	Chk_Zero(b,SIZE_8);
//printf("DADL (n),A\n");
}
/* CMPW (m),(n) */
inline void Csc62015::Op_c6(void)
{
    UINT32	t;
    t=Get_i16(pPC->Get_8(reg.x.p),OPR1);
	reg.r.pc++;
    t-=Get_i16(pPC->Get_8(reg.x.p),OPR2);
	reg.r.pc++;
	Chk_Flag(t,SIZE_16);
	AddState(8);
}
/* CMPP (m),(n) */
inline void Csc62015::Op_c7(void)
{
    UINT32	t;
    t=Get_i24(pPC->Get_8(reg.x.p),OPR1);
	reg.r.pc++;
    t-=Get_i24(pPC->Get_8(reg.x.p),OPR2);
	reg.r.pc++;
	Chk_Flag(t,SIZE_24);
	AddState(10);
}
/* MV (m),(n) */
inline void Csc62015::Op_c8(void)
{
    BYTE	t;
    t=pPC->Get_8(reg.x.p);
	reg.r.pc++;
    Set_i8(t,Get_i8(pPC->Get_8(reg.x.p),OPR2),OPR1);
	reg.r.pc++;
	AddState(6);
}
/* MVW (m),(n) */
inline void Csc62015::Op_c9(void)
{
    BYTE	t;
    t=pPC->Get_8(reg.x.p);
	reg.r.pc++;
    Set_i16(t,Get_i16(pPC->Get_8(reg.x.p),OPR2),OPR1);
	reg.r.pc++;
	AddState(8);
}
/* MVP (m),(n) */
inline void Csc62015::Op_ca(void)
{
    BYTE	t;
    t=pPC->Get_8(reg.x.p);
	reg.r.pc++;
    Set_i24(t,Get_i24(pPC->Get_8(reg.x.p),OPR2),OPR1);
	reg.r.pc++;
	AddState(10);
}
/* MVL (n),(m) */
inline void Csc62015::Op_cb(void)
{
    BYTE	d,s;
    AddState(5);
    d=Conv_imemAdr(pPC->Get_8(reg.x.p),OPR1);
	reg.r.pc++;
    s=Conv_imemAdr(pPC->Get_8(reg.x.p),OPR2);
	reg.r.pc++;
    while(--reg.x.i!=0) {
        AddState(2);
        Set_i8d(d++,Get_i8d(s++));
    }
	Set_i8d(d,Get_i8d(s));
}
/* MV (m),n */
inline void Csc62015::Op_cc(void)
{
    BYTE	t;
    t=pPC->Get_8(reg.x.p);
	reg.r.pc++;
    Set_i8(t,pPC->Get_8(reg.x.p),OPR1);
	reg.r.pc++;
	AddState(3);
}
/* MVW (l),mn */
inline void Csc62015::Op_cd(void)
{
    BYTE	t;
    t=pPC->Get_8(reg.x.p);
	reg.r.pc++;
    Set_i16(t,pPC->Get_16(reg.x.p),OPR1);
	reg.r.pc+=SIZE_16;
	AddState(4);
}
/* TCL ??? */
inline void Csc62015::Op_ce(void)
{
//UN_DEFINE
    AddState(1);
}
/* MVLD (m),(n) */
inline void Csc62015::Op_cf(void)
{
    BYTE	d,s;
    AddState(5);
    d=Conv_imemAdr(pPC->Get_8(reg.x.p),OPR1);
	reg.r.pc++;
    s=Conv_imemAdr(pPC->Get_8(reg.x.p),OPR2);
	reg.r.pc++;
    while(--reg.x.i!=0) {
        AddState(2);
        Set_i8d(d--,Get_i8d(s--));
    }
	Set_i8d(d,Get_i8d(s));
}
/* MV (k),[lmn] */
inline void Csc62015::Op_d0(void)
{
    BYTE	t;
    t=pPC->Get_8(reg.x.p);
	reg.r.pc++;
    Set_i8(t,pPC->Get_8(pPC->Get_20(reg.x.p)),OPR1);
	reg.r.pc+=SIZE_20;
	AddState(7);
}
/* MVW (k),[lmn] */
inline void Csc62015::Op_d1(void)
{
    BYTE	t;
    t=pPC->Get_8(reg.x.p);
	reg.r.pc++;
    Set_i16(t,pPC->Get_16(pPC->Get_20(reg.x.p)),OPR1);
	reg.r.pc+=SIZE_20;
	AddState(8);
}
/* MVP (k),[lmn] */
inline void Csc62015::Op_d2(void)
{
    BYTE	t;
    t=pPC->Get_8(reg.x.p);
	reg.r.pc++;
    Set_i24(t,pPC->Get_24(pPC->Get_20(reg.x.p)),OPR1);
	reg.r.pc+=SIZE_20;
	AddState(9);
}
/* MVL (k),[lmn] */
inline void Csc62015::Op_d3(void)
{
    BYTE	d;
    UINT32	s;
    AddState(6);
    d=Conv_imemAdr(pPC->Get_8(reg.x.p),OPR1);
	reg.r.pc++;
    s=pPC->Get_20(reg.x.p);
	reg.r.pc+=SIZE_20;
    while(--reg.x.i!=0) {
        AddState(2);
        Set_i8d(d++,pPC->Get_8(s++));
    }
    Set_i8d(d,pPC->Get_8(s));
}
/* DSBL (m),(n) */
inline void Csc62015::Op_d4(void)
{
    BYTE	d,s,b;
    WORD	w;
    AddState(5);
    d=Conv_imemAdr(pPC->Get_8(reg.x.p),OPR1);
	reg.r.pc++;
    s=Conv_imemAdr(pPC->Get_8(reg.x.p),OPR2);
	reg.r.pc++;
	w=hex2bcd(100+bcd2hex(Get_i8d(d))-bcd2hex(Get_i8d(s--)));
	reg.r.c=1-(w>>8);
	Set_i8d(d--,w);
	b=w;
	while(--reg.x.i!=0){
        AddState(2);
		w=hex2bcd(100+bcd2hex(Get_i8d(d))-bcd2hex(Get_i8d(s--))-reg.r.c);
		reg.r.c=1-(w>>8);
		Set_i8d(d--,w);
		b|=w;
	}
	Chk_Zero(b,SIZE_8);
//printf("DSBL (m),(n)\n");
}
/* DSBL (n),A */
inline void Csc62015::Op_d5(void)
{
    BYTE	d,b;
    WORD	w;
    AddState(4);
    d=Conv_imemAdr(pPC->Get_8(reg.x.p),OPR1);
	reg.r.pc++;
	w=hex2bcd(100+bcd2hex(Get_i8d(d))-bcd2hex(reg.r.a));
	reg.r.c=1-(w>>8);
	Set_i8d(d--,w);
	b=w;
	while(--reg.x.i!=0){
        AddState(1);
		w=hex2bcd(100+bcd2hex(Get_i8d(d))-reg.r.c);
		reg.r.c=1-(w>>8);
		Set_i8d(d--,w);
		b|=w;
	}
	Chk_Zero(b,SIZE_8);
//printf("DSBL (n),A\n");
}
/* CMPW (n),r2 */
inline void Csc62015::Op_d6(void)
{
    BYTE	a;
    UINT32	t;
    a=pPC->Get_8(reg.x.p);
	reg.r.pc++;
    t=Get_i16(pPC->Get_8(reg.x.p),OPR1)-(Get_r(a)&MASK_16);
	reg.r.pc++;
	Chk_Flag(t,SIZE_16);
	AddState(7);
}
/* CMPP (n),r3 */
inline void Csc62015::Op_d7(void)
{
    BYTE	a;
    UINT32	t;
    a=pPC->Get_8(reg.x.p);
	reg.r.pc++;
    t=Get_i20(pPC->Get_8(reg.x.p),OPR1)-Get_r(a);
	reg.r.pc++;
	Chk_Flag(t,SIZE_20);
	AddState(9);
}
/* MV [klm],(n) */
inline void Csc62015::Op_d8(void)
{
    UINT32	t;
    t=pPC->Get_20(reg.x.p);
	reg.r.pc+=SIZE_20;
    pPC->Set_8(t,Get_i8(pPC->Get_8(reg.x.p),OPR1));
	reg.r.pc++;
	AddState(6);
}
/* MVW [klm],(n) */
inline void Csc62015::Op_d9(void)
{
    UINT32	t;
    t=pPC->Get_20(reg.x.p);
	reg.r.pc+=SIZE_20;
    pPC->Set_16(t,Get_i16(pPC->Get_8(reg.x.p),OPR1));
	reg.r.pc++;
	AddState(7);
}
/* MVP [klm],(n) */
inline void Csc62015::Op_da(void)
{
    UINT32	t;
    t=pPC->Get_20(reg.x.p);
	reg.r.pc+=SIZE_20;
    pPC->Set_24(t,Get_i24(pPC->Get_8(reg.x.p),OPR1));
	reg.r.pc++;
	AddState(8);
}
/* MVL [klm],(n) */
inline void Csc62015::Op_db(void)
{
    BYTE	s;
    UINT32	d;
    AddState(6);
    d=pPC->Get_20(reg.x.p);
	reg.r.pc+=SIZE_20;
    s=Conv_imemAdr(pPC->Get_8(reg.x.p),OPR1);
	reg.r.pc++;
    while(--reg.x.i!=0) {
        AddState(2);
        pPC->Set_8(d++,Get_i8d(s++));
    }
    pPC->Set_8(d,Get_i8d(s));
}
/* MVP (k),lmn */
inline void Csc62015::Op_dc(void)
{
    BYTE	t;
    t=pPC->Get_8(reg.x.p);
	reg.r.pc++;
    Set_i24(t,pPC->Get_24(reg.x.p),OPR1);
	reg.r.pc+=SIZE_20;
	AddState(5);
}
/* EX A,B */
inline void Csc62015::Op_dd(void)
{
    BYTE	t;
	t=reg.r.a; reg.r.a=reg.r.b; reg.r.b=t;
	AddState(3);
}
/* HALT */
inline void Csc62015::Op_de(void)
{
//AddLog(LOG_TEMP,"System HALT!!");/*exit(1);*/
//debug.isdebug=1;
    halt=1;
	AddState(2);
}

/* OFF */
inline void Csc62015::Op_df(void)
{
//AddLog(LOG_TEMP,"Software POWER OFF!!");/*exit(1);*/

	imem[IMEM_ISR]=0;
    off=1;
	AddState(2);
}

/* MV (n),[r3] */
inline void Csc62015::Op_e0(void)
{
    BYTE	t;
    UINT32	a;
	a=Get_d2(SIZE_8, &t);
    Set_i8(t,pPC->Get_8(a),OPR1);
	AddState(6);
}
/* MVW (n),[r3] */
inline void Csc62015::Op_e1(void)
{
    BYTE	t;
    UINT32	a;
	a=Get_d2(SIZE_16, &t);
    Set_i16(t,pPC->Get_16(a),OPR1);
	AddState(7);
}
/* MVP (n),[r3] */
inline void Csc62015::Op_e2(void)
{
    BYTE	t;
    UINT32	a;
	a=Get_d2(SIZE_20, &t);
    Set_i24(t,pPC->Get_24(a),OPR1);
	AddState(8);
}
/* MVL (n),[r3] */
inline void Csc62015::Op_e3(void)
{
    BYTE	d,t;
    UINT32	s;
    AddState(10);
    t=pPC->Get_8(reg.x.p);
	reg.r.pc++;
    d=Conv_imemAdr(pPC->Get_8(reg.x.p),OPR1);
	reg.r.pc++;
	s=Get_r(t);
    while(--reg.x.i!=0) {
        AddState(2);
        Set_i8d(((t&0x10)==0?d++:d--),pPC->Get_8(((t&0x10)==0?s++:--s)));
    }
    Set_i8d(d,pPC->Get_8(((t&0x10)==0?s++:--s)));
	Set_r(t,s);
}
/* ROR A */
inline void Csc62015::Op_e4(void)
{
    BYTE	a,c;
	a=reg.r.a;
	c=a&1;					/* b0 */
	a=(a>>1)|(c<<7);
	reg.r.c=c;
	Chk_Zero(a,SIZE_8);
	reg.r.a=a;
	AddState(2);
}
/* ROR (n) */
inline void Csc62015::Op_e5(void)
{
    BYTE	a,c,t;
    t=pPC->Get_8(reg.x.p);
	reg.r.pc++;
	a=Get_i8(t,OPR1);
	c=a&1;					/* b0 */
	a=(a>>1)|(c<<7);
	reg.r.c=c;
	Chk_Zero(a,SIZE_8);
	Set_i8(t,a,OPR1);
	AddState(3);
}
/* ROL A */
inline void Csc62015::Op_e6(void)
{
    BYTE	a,c;
	a=reg.r.a;
	c=(a>>7)&1;				/* b7 */
	a=(a<<1)|c;
	reg.r.c=c;
	Chk_Zero(a,SIZE_8);
	reg.r.a=a;
	AddState(2);
}
/* ROL (n) */
inline void Csc62015::Op_e7(void)
{
    BYTE	a,c,t;
    t=pPC->Get_8(reg.x.p);
	reg.r.pc++;
	a=Get_i8(t,OPR1);
	c=(a>>7)&1;				/* b7 */
	a=(a<<1)|c;
	reg.r.c=c;
	Chk_Zero(a,SIZE_8);
	Set_i8(t,a,OPR1);
	AddState(3);
}
/* MV [r3],(n) */
inline void Csc62015::Op_e8(void)
{
    BYTE	t;
    UINT32	a;
	a=Get_d2(SIZE_8, &t);
    pPC->Set_8(a,Get_i8(t,OPR1));
	AddState(6);
}
/* MVW [r3],(n) */
inline void Csc62015::Op_e9(void)
{
    BYTE	t;
    UINT32	a;
	a=Get_d2(SIZE_16, &t);
    pPC->Set_16(a,Get_i16(t,OPR1));
	AddState(7);
}
/* MVP [r3],(n) */
inline void Csc62015::Op_ea(void)
{
    BYTE	t;
    UINT32	a;
	a=Get_d2(SIZE_20, &t);
    pPC->Set_24(a,Get_i24(t,OPR1));
	AddState(8);
}
/* MVL [r3],(n) */
inline void Csc62015::Op_eb(void)
{
    BYTE	s,t;
    UINT32	d;
    AddState(10);
    t=pPC->Get_8(reg.x.p);
	reg.r.pc++;
    s=Conv_imemAdr(pPC->Get_8(reg.x.p),OPR1);
	reg.r.pc++;
	d=Get_r(t);
    while(--reg.x.i!=0) {
        AddState(2);
        pPC->Set_8(((t&0x10)==0?d++:--d),Get_i8d(((t&0x10)==0?s++:s--)));
    }
    pPC->Set_8(((t&0x10)==0?d++:--d),Get_i8d(s));
	Set_r(t,d);
}
/* DSLL (n) */
inline void Csc62015::Op_ec(void)
{
    BYTE	n;
    UINT32	s,t,u,b;
    AddState(4);
    n=Conv_imemAdr(pPC->Get_8(reg.x.p),OPR1);
	reg.r.pc++;
	t=Get_i8d(n);
	s=t<<4;
	Set_i8d(n--,s);
	u=t>>4;
	b=s;
	while(--reg.x.i!=0){
        AddState(1);
		t=Get_i8d(n);
		s=t<<4|u;
		Set_i8d(n--,s);
		u=t>>4;
		b|=s;
	}
	Chk_Zero(b,SIZE_8);
}
/* EX r,r' */
inline void Csc62015::Op_ed(void)
{
    BYTE	t;
    UINT32	w;
    t=pPC->Get_8(reg.x.p);
	reg.r.pc++;
	w=Get_r2(t); Set_r2(t,Get_r2(t>>4)); Set_r2(t>>4,w);
	AddState(4);
}
/* SWAP A */
inline void Csc62015::Op_ee(void)
{
	reg.r.a=(reg.r.a>>4)|(reg.r.a<<4);
	Chk_Zero(reg.r.a,SIZE_8);
	AddState(3);
}
/* WAIT */
inline void Csc62015::Op_ef(void)
{
    AddState(1);
    while(--reg.x.i!=0) {AddState(1);}				/* dummy !!! */
//	reg.x.i=0;							// for a little speed up!!
}
/* MV (m),[(n)] */
inline void Csc62015::Op_f0(void)
{
    BYTE	t=0;
    UINT32	a;
	a=Get_i2(&t);
    Set_i8(t,pPC->Get_8(a),OPR1);
	AddState(11);
}
/* MVW (m),[(n)] */
inline void Csc62015::Op_f1(void)
{
    BYTE	t=0;
    UINT32	a;
	a=Get_i2(&t);
    Set_i16(t,pPC->Get_16(a),OPR1);
	AddState(12);
}
/* MVP (m),[(n)] */
inline void Csc62015::Op_f2(void)
{
    BYTE	t=0;
    UINT32	a;
	a=Get_i2(&t);
    Set_i24(t,pPC->Get_24(a),OPR1);
	AddState(13);
}
/* MVL (l),[(n)] */
inline void Csc62015::Op_f3(void)
{
    BYTE	d=0;
    UINT32	s;
    AddState(10);
	s=Get_i2(&d);
	d=Conv_imemAdr(d,OPR1);
    while(--reg.x.i!=0) {
        AddState(2);
        Set_i8d(d++,pPC->Get_8(s++));
    }
    Set_i8d(d,pPC->Get_8(s));
}
/* SHR A */
inline void Csc62015::Op_f4(void)
{
    BYTE	a,c;
	a=reg.r.a;
	c=reg.r.c;
	reg.r.c=a&1;
	a=(a>>1)|(c<<7);
	Chk_Zero(a,SIZE_8);
	reg.r.a=a;
	AddState(2);
}
/* SHR (n) */
inline void Csc62015::Op_f5(void)
{
    BYTE	a,c,t;
    t=pPC->Get_8(reg.x.p);
	reg.r.pc++;
	a=Get_i8(t,OPR1);
	c=reg.r.c;
	reg.r.c=a&1;
	a=(a>>1)|(c<<7);
	Chk_Zero(a,SIZE_8);
	Set_i8(t,a,OPR1);
	AddState(3);
}
/* SHL A */
inline void Csc62015::Op_f6(void)
{
    BYTE	a,c;
	a=reg.r.a;
	c=reg.r.c;
	reg.r.c=a>>7;
	a=(a<<1)|c;
	Chk_Zero(a,SIZE_8);
	reg.r.a=a;
	AddState(2);
}
/* SHL (n) */
inline void Csc62015::Op_f7(void)
{
    BYTE	a,c,t;
    t=pPC->Get_8(reg.x.p);
	reg.r.pc++;
	a=Get_i8(t,OPR1);
	c=reg.r.c;
	reg.r.c=a>>7;
	a=(a<<1)|c;
	Chk_Zero(a,SIZE_8);
	Set_i8(t,a,OPR1);
	AddState(3);
}
/* MV [(m)],(n) */
inline void Csc62015::Op_f8(void)
{
    BYTE	t=1;
    UINT32	a;
	a=Get_i2(&t);
    pPC->Set_8(a,Get_i8(t,OPR2));
	AddState(11);
}
/* MVW [(m)].(n) */
inline void Csc62015::Op_f9(void)
{
    BYTE	t=1;
    UINT32	a;
	a=Get_i2(&t);
    pPC->Set_16(a,Get_i16(t,OPR2));
	AddState(12);
}
/* MVP [(m)],(n) */
inline void Csc62015::Op_fa(void)
{
    BYTE	t=1;
    UINT32	a;
	a=Get_i2(&t);
    pPC->Set_24(a,Get_i24(t,OPR2));
	AddState(13);
}
/* MVL [(m)],(n) */
inline void Csc62015::Op_fb(void)
{
    BYTE	s=1;
    UINT32	d;
    AddState(10);
	d=Get_i2(&s);
	s=Conv_imemAdr(s,OPR2);
    while(--reg.x.i!=0) {
        AddState(2);
        pPC->Set_8(d++,Get_i8d(s++));
    }
    pPC->Set_8(d,Get_i8d(s));
}
/* DSRL (n) */
inline void Csc62015::Op_fc(void)
{
    BYTE	n;
    UINT32	s,t,u,b;
    AddState(4);
    n=Conv_imemAdr(pPC->Get_8(reg.x.p),OPR1);
	reg.r.pc++;
	t=Get_i8d(n);
	s=t>>4;
	Set_i8d(n++,s);
	u=t&MASK_4;
	b=s;
	while(--reg.x.i!=0){
        AddState(1);
		t=Get_i8d(n);
		s=t>>4|u<<4;
		Set_i8d(n++,s);
		u=t&MASK_4;
		b|=s;
	}
	Chk_Zero(b,SIZE_8);
}
/* MV r,r' */
inline void Csc62015::Op_fd(void)
{
    BYTE	t;
    t=pPC->Get_8(reg.x.p);
	reg.r.pc++;
	Set_r2(t>>4,Get_r2(t));
	AddState(2);
}
/* IR */
inline void Csc62015::Op_fe(void)
{
	reg.x.s-=SIZE_20;
    pPC->Set_20(reg.x.s,reg.x.p-1);
	reg.x.s--;
    pPC->Set_8(reg.x.s,reg.x.f);
	reg.x.s--;
    pPC->Set_8(reg.x.s,imem[IMEM_IMR]);
    reg.x.p=pPC->Get_20(VECT_IR);
    CallSubLevel++;
	AddState(1);
}
/* RESET */
inline void Csc62015::Op_ff(void)
{
    reg.x.p=pPC->Get_20(VECT_RESET);
	AddState(1);
}



/* execute one operation code(for after pre byte) */
void Csc62015::Step_sc62015_(void)
{
    UINT32 t;
	t=reg.x.p;
	reg.r.pc++;
    OpExec(pPC->Get_PC(t));
	Reset_Pre();
}
/*---------------------------------------------------------------------------*/
/*****************************************************************************/
/* Load Memory from file													 */
/*  ENTRY :BYTE s=Slot No.(SLOT1-3)											 */
/*  RETURN:1:success, 0:error												 */
/*****************************************************************************/
bool Csc62015::Mem_Load(BYTE s)
{
#if 0
    bool r=0;
	FILE *fp;
/*	char *MacName[]={"???","E500","E500","E500"
					,"1480U","???","1490U","E550(E500)"
					,"E650?","U6000?"};*/
	printf(" SLOT%c:Loading...",0x31+s);
	if((fp=fopen(SlotName[s],"rb"))!=NULL){
		if(s==SLOT3){
			fread(&mem[SlotAdr[s]],1024,SlotSize[s],fp);
/*			printf("PC-%s(Ver.%d.%d)",MacName[mem[0xffff0]&15]
				,mem[0xffff0],mem[0xffff1]);*/
			printf("[ROM Ver.%d.%d]",mem[0xffff0],mem[0xffff1]);
			if(mem[0xffff0]>7){
				e6=1;							// E650&U6000mode
				emsmode=(emsmode&16)+(emsmode&7)<4?0:4;
				fread(&mem[SlotAdr[SLOT3EXT]],1024,SlotSize[SLOT3EXT],fp);
			}
			printf("success.\n"); r=1;
		}else{
			if(check_filesize(fp,SlotSize[s],UNIT_KB)){
				fread(&mem[SlotAdr[s]],1024,SlotSize[s],fp);
				printf("success.\n"); r=1;
			}else printf("skipped.\n");
		}
		fclose(fp);
	}else printf("file not found(%s).\n",SlotName[s]);
	return(r);
#endif
    return true;
}
/*****************************************************************************/
/* Load EMS from file														 */
/*  ENTRY :none																 */
/*  RETURN:1:success, 0:error												 */
/*****************************************************************************/
bool Csc62015::EMS_Load(void)
{
#if 0
	int  i;
    bool r=0;
	FILE *fp;
//	char *bankstr[]={"192KB","2x128KB","4x128KB","4x64KB","4x192KB"};
	int banksize[]={0,192,256,512,256,768};
//	printf(" EMS  :Loading...[%s]",bankstr[(emsmode&7)-1]);
	printf(" EMS  :Loading...");
	if((fp=fopen(SlotName[EMS],"rb"))!=NULL){
		if(check_filesize(fp,banksize[emsmode&7],UNIT_KB)){
			switch((emsmode&7)){
			case 1:									//  192K(for DELTA1)
				fread(&mem[SlotAdr[EMS]],1024,SlotSize[EMS],fp); break;
			case 2:									//2x128K(for DELTA2)
				fread(&mem[BASE_128[1]],1024,128,fp);
				fread(&mem[BASE_128[0]],1024,128,fp); break;
			case 3:									//4x128K(for DELTA3)
				for(i=0;i<4;i++){
					fread(&mem[BASE_128[3-i]],1024,128,fp);
				} break;
			case 4:									//4x 64K(for DELTA4)
				for(i=0;i<4;i++){
					fread(&mem[BASE_64[3-i]],1024,64,fp);
				} break;
			case 5:									//4x192K(for super-DELTA)
				for(i=0;i<4;i++){
					fread(&mem[BASE_64[3-i]],1024,64,fp);
					fread(&mem[BASE_128[3-i]],1024,128,fp);
				} break;
			}
			printf("success.\n"); r=1;
		}else printf("skipped.\n");
		fclose(fp);
	}else printf("file not found(%s).\n",SlotName[EMS]);
	return(r);
#endif
    return true;
}
/*****************************************************************************/
/* Save Memory to file														 */
/*  ENTRY :BYTE s=Slot No.(SLOT1-3)											 */
/*  RETURN:none																 */
/*****************************************************************************/
void Csc62015::Mem_Save(BYTE s)
{
#if 0
	FILE *fp;
	printf(" SLOT%c:Saving...",0x31+s);
	if((fp=fopen(SlotName[s],"wb"))!=NULL){
		fwrite(&mem[SlotAdr[s]],1024,SlotSize[s],fp);
		fclose(fp);
		printf("done.\n");
	}else printf("file cannot open.\n");
#endif
}
/*****************************************************************************/
/* Save EMS to file															 */
/*  ENTRY :none																 */
/*  RETURN:none																 */
/*****************************************************************************/
void Csc62015::EMS_Save(void)
{
#if 0
	int  i;
	FILE *fp;
	printf(" EMS  :Saving...");
	if((fp=fopen(SlotName[EMS],"wb"))!=NULL){
		switch((emsmode&7)){
		case 1:										//  192K(for DELTA1)
			fwrite(&mem[SlotAdr[EMS]],1024,SlotSize[EMS],fp); break;
		case 2:										//2x128K(for DELTA2)
			fwrite(&mem[BASE_128[1]],1024,128,fp);
			fwrite(&mem[BASE_128[0]],1024,128,fp); break;
		case 3:										//4x128K(for DELTA3)
			for(i=0;i<4;i++){
				fwrite(&mem[BASE_128[3-i]],1024,128,fp);
			} break;
		case 4:										//4x 64K(for DELTA4)
			for(i=0;i<4;i++){
				fwrite(&mem[BASE_64[3-i]],1024,64,fp);
			} break;
		case 5:										//4x192K(for super-DELTA)
			for(i=0;i<4;i++){
				fwrite(&mem[BASE_64[3-i]],1024,64,fp);
				fwrite(&mem[BASE_128[3-i]],1024,128,fp);
			} break;
		}
		fclose(fp);
		printf("done.\n");
	}else printf("file cannot open.\n");
#endif
}

/*---------------------------------------------------------------------------*/
/*****************************************************************************/
/* Initialize SC62015 CPU emulator (memory, register)						 */
/* RETURN: 0=error, 1=success												 */
/*****************************************************************************/
bool Csc62015::init(void)
{
    Check_Log();

	return(1);
}

void Csc62015::Reset(void) {

    imem[IMEM_ACM] &= 0x7F;
    imem[IMEM_UCR] = 0;
    imem[IMEM_USR] &= 0xD8;
    imem[IMEM_IMR] = 0;
    imem[IMEM_SCR] = 0;
    imem[IMEM_SSR] &= 0xFB;
    imem[IMEM_USR] |= 0x18;
    imem[IMEM_EIH] = 0;
    imem[IMEM_EIL] = 0;
    imem[IMEM_EOH] = 0;
    imem[IMEM_EOL] = 0;
    reg.x.p=pPC->Get_20(VECT_RESET);
   halt = false;

}

/*****************************************************************************/
/* Exitting SC62015 CPU emulator (save memory, register)					 */
/*****************************************************************************/
bool Csc62015::exit(void)
{
	Mem_Save(SLOT1);
	Mem_Save(SLOT2);
//	Mem_Save(SLOT3);
	if(emsmode>0&&emsmode<8) EMS_Save();
	if(logsw) fclose(fp_log);							//close log file

    return true;
}

/*****************************************************************************/
/* execute one operation code												 */
/*****************************************************************************/
void Csc62015::step(void)
{
    UINT32 t;

    if (halt) {
        AddState(3);
        return;
    }
//	if(timer.state<timer.chkspeed)
    {					//check for overrun
/*		if(reg.x.p==0xfffe8){						//hook IOCS
			if(imem[0xd6]==2 && reg.x.i==0x26) debug.isdebug=1;
		}*/
		if(logsw){
            if(cpulog && fp_log!=0){					//check log mode
				if(reg.x.p==0xfffe8)				//check calling IOCS
					fprintf(fp_log,"IOCS:(cx)=%04X,i=%04X\n",imem[0xd6]+imem[0xd7]*256,reg.x.i);
//				debug.DisAsm_1(reg.x.p,fp_log);		//write execute log
			}
		}
		t=reg.x.p;
		reg.r.pc++;
        OpExec(pPC->Get_PC(t));
	}
}

inline void Csc62015::OpExec(BYTE Op)
{

    switch(Op)	{
    case 0x00 : Op_00();	break;
    case 0x01 : Op_01();	break;
    case 0x02 : Op_02();	break;
    case 0x03 : Op_03();	break;
    case 0x04 : Op_04();	break;
    case 0x05 : Op_05();	break;
    case 0x06 : Op_06();	break;
    case 0x07 : Op_07();	break;
    case 0x08 : Op_08();	break;
    case 0x09 : Op_09();	break;
    case 0x0a : Op_0a();	break;
    case 0x0b : Op_0b();	break;
    case 0x0c : Op_0c();	break;
    case 0x0d : Op_0d();	break;
    case 0x0e : Op_0e();	break;
    case 0x0f : Op_0f();	break;

    case 0x10 : Op_10();	break;
    case 0x11 : Op_11();	break;
    case 0x12 : Op_12();	break;
    case 0x13 : Op_13();	break;
    case 0x14 : Op_14();	break;
    case 0x15 : Op_15();	break;
    case 0x16 : Op_16();	break;
    case 0x17 : Op_17();	break;
    case 0x18 : Op_18();	break;
    case 0x19 : Op_19();	break;
    case 0x1a : Op_1a();	break;
    case 0x1b : Op_1b();	break;
    case 0x1c : Op_1c();	break;
    case 0x1d : Op_1d();	break;
    case 0x1e : Op_1e();	break;
    case 0x1f : Op_1f();	break;

    case 0x20 : Op_20();	break;
    case 0x21 : Op_21();	break;
    case 0x22 : Op_22();	break;
    case 0x23 : Op_23();	break;
    case 0x24 : Op_24();	break;
    case 0x25 : Op_25();	break;
    case 0x26 : Op_26();	break;
    case 0x27 : Op_27();	break;
    case 0x28 : Op_28();	break;
    case 0x29 : Op_29();	break;
    case 0x2a : Op_2a();	break;
    case 0x2b : Op_2b();	break;
    case 0x2c : Op_2c();	break;
    case 0x2d : Op_2d();	break;
    case 0x2e : Op_2e();	break;
    case 0x2f : Op_2f();	break;

    case 0x30 : Op_30();	break;
    case 0x31 : Op_31();	break;
    case 0x32 : Op_32();	break;
    case 0x33 : Op_33();	break;
    case 0x34 : Op_34();	break;
    case 0x35 : Op_35();	break;
    case 0x36 : Op_36();	break;
    case 0x37 : Op_37();	break;
    case 0x38 : Op_38();	break;
    case 0x39 : Op_39();	break;
    case 0x3a : Op_3a();	break;
    case 0x3b : Op_3b();	break;
    case 0x3c : Op_3c();	break;
    case 0x3d : Op_3d();	break;
    case 0x3e : Op_3e();	break;
    case 0x3f : Op_3f();	break;

    case 0x40 : Op_40();	break;
    case 0x41 : Op_41();	break;
    case 0x42 : Op_42();	break;
    case 0x43 : Op_43();	break;
    case 0x44 : Op_44();	break;
    case 0x45 : Op_45();	break;
    case 0x46 : Op_46();	break;
    case 0x47 : Op_47();	break;
    case 0x48 : Op_48();	break;
    case 0x49 : Op_49();	break;
    case 0x4a : Op_4a();	break;
    case 0x4b : Op_4b();	break;
    case 0x4c : Op_4c();	break;
    case 0x4d : Op_4d();	break;
    case 0x4e : Op_4e();	break;
    case 0x4f : Op_4f();	break;

    case 0x50 : Op_50();	break;
    case 0x51 : Op_51();	break;
    case 0x52 : Op_52();	break;
    case 0x53 : Op_53();	break;
    case 0x54 : Op_54();	break;
    case 0x55 : Op_55();	break;
    case 0x56 : Op_56();	break;
    case 0x57 : Op_57();	break;
    case 0x58 : Op_58();	break;
    case 0x59 : Op_59();	break;
    case 0x5a : Op_5a();	break;
    case 0x5b : Op_5b();	break;
    case 0x5c : Op_5c();	break;
    case 0x5d : Op_5d();	break;
    case 0x5e : Op_5e();	break;
    case 0x5f : Op_5f();	break;

    case 0x60 : Op_60();	break;
    case 0x61 : Op_61();	break;
    case 0x62 : Op_62();	break;
    case 0x63 : Op_63();	break;
    case 0x64 : Op_64();	break;
    case 0x65 : Op_65();	break;
    case 0x66 : Op_66();	break;
    case 0x67 : Op_67();	break;
    case 0x68 : Op_68();	break;
    case 0x69 : Op_69();	break;
    case 0x6a : Op_6a();	break;
    case 0x6b : Op_6b();	break;
    case 0x6c : Op_6c();	break;
    case 0x6d : Op_6d();	break;
    case 0x6e : Op_6e();	break;
    case 0x6f : Op_6f();	break;

    case 0x70 : Op_70();	break;
    case 0x71 : Op_71();	break;
    case 0x72 : Op_72();	break;
    case 0x73 : Op_73();	break;
    case 0x74 : Op_74();	break;
    case 0x75 : Op_75();	break;
    case 0x76 : Op_76();	break;
    case 0x77 : Op_77();	break;
    case 0x78 : Op_78();	break;
    case 0x79 : Op_79();	break;
    case 0x7a : Op_7a();	break;
    case 0x7b : Op_7b();	break;
    case 0x7c : Op_7c();	break;
    case 0x7d : Op_7d();	break;
    case 0x7e : Op_7e();	break;
    case 0x7f : Op_7f();	break;

    case 0x80 : Op_80();	break;
    case 0x81 : Op_81();	break;
    case 0x82 : Op_82();	break;
    case 0x83 : Op_83();	break;
    case 0x84 : Op_84();	break;
    case 0x85 : Op_85();	break;
    case 0x86 : Op_86();	break;
    case 0x87 : Op_87();	break;
    case 0x88 : Op_88();	break;
    case 0x89 : Op_89();	break;
    case 0x8a : Op_8a();	break;
    case 0x8b : Op_8b();	break;
    case 0x8c : Op_8c();	break;
    case 0x8d : Op_8d();	break;
    case 0x8e : Op_8e();	break;
    case 0x8f : Op_8f();	break;

    case 0x90 : Op_90();	break;
    case 0x91 : Op_91();	break;
    case 0x92 : Op_92();	break;
    case 0x93 : Op_93();	break;
    case 0x94 : Op_94();	break;
    case 0x95 : Op_95();	break;
    case 0x96 : Op_96();	break;
    case 0x97 : Op_97();	break;
    case 0x98 : Op_98();	break;
    case 0x99 : Op_99();	break;
    case 0x9a : Op_9a();	break;
    case 0x9b : Op_9b();	break;
    case 0x9c : Op_9c();	break;
    case 0x9d : Op_9d();	break;
    case 0x9e : Op_9e();	break;
    case 0x9f : Op_9f();	break;

    case 0xa0 : Op_a0();	break;
    case 0xa1 : Op_a1();	break;
    case 0xa2 : Op_a2();	break;
    case 0xa3 : Op_a3();	break;
    case 0xa4 : Op_a4();	break;
    case 0xa5 : Op_a5();	break;
    case 0xa6 : Op_a6();	break;
    case 0xa7 : Op_a7();	break;
    case 0xa8 : Op_a8();	break;
    case 0xa9 : Op_a9();	break;
    case 0xaa : Op_aa();	break;
    case 0xab : Op_ab();	break;
    case 0xac : Op_ac();	break;
    case 0xad : Op_ad();	break;
    case 0xae : Op_ae();	break;
    case 0xaf : Op_af();	break;

    case 0xb0 : Op_b0();	break;
    case 0xb1 : Op_b1();	break;
    case 0xb2 : Op_b2();	break;
    case 0xb3 : Op_b3();	break;
    case 0xb4 : Op_b4();	break;
    case 0xb5 : Op_b5();	break;
    case 0xb6 : Op_b6();	break;
    case 0xb7 : Op_b7();	break;
    case 0xb8 : Op_b8();	break;
    case 0xb9 : Op_b9();	break;
    case 0xba : Op_ba();	break;
    case 0xbb : Op_bb();	break;
    case 0xbc : Op_bc();	break;
    case 0xbd : Op_bd();	break;
    case 0xbe : Op_be();	break;
    case 0xbf : Op_bf();	break;

    case 0xc0 : Op_c0();	break;
    case 0xc1 : Op_c1();	break;
    case 0xc2 : Op_c2();	break;
    case 0xc3 : Op_c3();	break;
    case 0xc4 : Op_c4();	break;
    case 0xc5 : Op_c5();	break;
    case 0xc6 : Op_c6();	break;
    case 0xc7 : Op_c7();	break;
    case 0xc8 : Op_c8();	break;
    case 0xc9 : Op_c9();	break;
    case 0xca : Op_ca();	break;
    case 0xcb : Op_cb();	break;
    case 0xcc : Op_cc();	break;
    case 0xcd : Op_cd();	break;
    case 0xce : Op_ce();	break;
    case 0xcf : Op_cf();	break;

    case 0xd0 : Op_d0();	break;
    case 0xd1 : Op_d1();	break;
    case 0xd2 : Op_d2();	break;
    case 0xd3 : Op_d3();	break;
    case 0xd4 : Op_d4();	break;
    case 0xd5 : Op_d5();	break;
    case 0xd6 : Op_d6();	break;
    case 0xd7 : Op_d7();	break;
    case 0xd8 : Op_d8();	break;
    case 0xd9 : Op_d9();	break;
    case 0xda : Op_da();	break;
    case 0xdb : Op_db();	break;
    case 0xdc : Op_dc();	break;
    case 0xdd : Op_dd();	break;
    case 0xde : Op_de();	break;
    case 0xdf : Op_df();	break;

    case 0xe0 : Op_e0();	break;
    case 0xe1 : Op_e1();	break;
    case 0xe2 : Op_e2();	break;
    case 0xe3 : Op_e3();	break;
    case 0xe4 : Op_e4();	break;
    case 0xe5 : Op_e5();	break;
    case 0xe6 : Op_e6();	break;
    case 0xe7 : Op_e7();	break;
    case 0xe8 : Op_e8();	break;
    case 0xe9 : Op_e9();	break;
    case 0xea : Op_ea();	break;
    case 0xeb : Op_eb();	break;
    case 0xec : Op_ec();	break;
    case 0xed : Op_ed();	break;
    case 0xee : Op_ee();	break;
    case 0xef : Op_ef();	break;

    case 0xf0 : Op_f0();	break;
    case 0xf1 : Op_f1();	break;
    case 0xf2 : Op_f2();	break;
    case 0xf3 : Op_f3();	break;
    case 0xf4 : Op_f4();	break;
    case 0xf5 : Op_f5();	break;
    case 0xf6 : Op_f6();	break;
    case 0xf7 : Op_f7();	break;
    case 0xf8 : Op_f8();	break;
    case 0xf9 : Op_f9();	break;
    case 0xfa : Op_fa();	break;
    case 0xfb : Op_fb();	break;
    case 0xfc : Op_fc();	break;
    case 0xfd : Op_fd();	break;
    case 0xfe : Op_fe();	break;
    case 0xff : Op_ff();	break;



    default : 	//DASMLOG = 1;
                //if (fp_log) fprintf(fp_log,"PC=[%04X]='%02X' : NOT YET EMULATED",reg.d.pc-1,Op);
//				AddLog(0x04,"PC=[%04X]='%02X' : NOT YET EMULATED",reg.d.pc-1,Op);
        break;
    }


}

/*---------------------------------------------------------------------------*/
/*****************************************************************************/
/* Get data from register													 */
/*  ENTRY :REGNAME regname=REG_xx											 */
/*  RETURN:DWORD value														 */
/*****************************************************************************/
UINT32 Csc62015::get_reg(REGNAME regname)
{
	switch(regname){
	case REG_A :return(reg.r.a);
	case REG_IL:return(reg.r.il);
	case REG_BA:return(reg.x.ba);
	case REG_I :return(reg.x.i);
	case REG_X :return(reg.x.x);
	case REG_Y :return(reg.x.y);
	case REG_U :return(reg.x.u);
	case REG_S :return(reg.x.s);
	case REG_P :return(reg.x.p);
	case REG_F :return(reg.x.f);
    case REG_PC :return(reg.r.pc);
	}
    return 0;
}

/*****************************************************************************/
/* Set data to register														 */
/*  ENTRY :REGNAME regname=REG_xx, DWORD data=value							 */
/*  RETURN:none																 */
/*****************************************************************************/
void Csc62015::set_reg(REGNAME regname,UINT32 data)
{
	switch(regname){
	case REG_A :reg.r.a =data; break;
	case REG_IL:reg.r.il=data; break;
	case REG_BA:reg.x.ba=data; break;
	case REG_I :reg.x.i =data; break;
	case REG_X :reg.x.x =data; break;
	case REG_Y :reg.x.y =data; break;
	case REG_U :reg.x.u =data; break;
	case REG_S :reg.x.s =data; break;
	case REG_P :reg.x.p =data; break;
	case REG_F :reg.x.f =data; break;
    default: break;
	}
}


/*****************************************************************************/
/* Get data from internal memory											 */
/*  ENTRY :BYTE adr=address													 */
/*  RETURN:BYTE value														 */
/*****************************************************************************/
UINT32 Csc62015::get_imem(BYTE adr)
{
	return(imem[adr]);
}
/*****************************************************************************/
/* Set data to internal memory												 */
/*  ENTRY :BYTE adr=address, BYTE data=value								 */
/*  RETURN:none																 */
/*****************************************************************************/
inline void Csc62015::set_imem(BYTE adr,BYTE data)
{
	imem[adr]=data;
}
/*****************************************************************************/
/* Oparation internal memory												 */
/*  ENTRY :BYTE adr=address, OPRMODE opr=OPR_xxx DWORD data=value			 */
/*  RETURN:none																 */
/*****************************************************************************/
void Csc62015::opr_imem(BYTE adr,OPRMODE opr,BYTE data)
{
	switch(opr){
	case OPR_AND:imem[adr]&=data; break;
	case OPR_OR :imem[adr]|=data; break;
	case OPR_XOR:imem[adr]^=data; break;
	case OPR_ADD:imem[adr]+=data; break;
	case OPR_SUB:imem[adr]-=data; break;
	}
}

void Csc62015::Load_Internal(QXmlStreamReader *xmlIn)
{
    if (xmlIn->readNextStartElement()) {
        if ( (xmlIn->name()=="cpu") &&
             (xmlIn->attributes().value("model").toString() == "sc62015")) {
            off = xmlIn->attributes().value("off").toString().toInt(0,16);
            halt = xmlIn->attributes().value("halt").toString().toInt(0,16);
            QByteArray ba_reg = QByteArray::fromBase64(xmlIn->attributes().value("registers").toString().toLatin1());
            memcpy((char *) &reg,ba_reg.data(),sizeof(reg));
            QByteArray ba_imem = QByteArray::fromBase64(xmlIn->attributes().value("iMem").toString().toLatin1());
            memcpy((char *) &imem,ba_imem.data(),sizeof(imem));
        }
        xmlIn->skipCurrentElement();
    }
}

void Csc62015::save_internal(QXmlStreamWriter *xmlOut)
{
    xmlOut->writeStartElement("cpu");
     xmlOut->writeAttribute("model","sc62015");
     xmlOut->writeAttribute("off",QString("%1").arg(off));
     xmlOut->writeAttribute("halt",QString("%1").arg(halt));
     QByteArray ba_reg((char*)&reg,sizeof(reg));
     xmlOut->writeAttribute("registers",ba_reg.toBase64());
     QByteArray ba_imem((char*)imem,sizeof(imem));
     xmlOut->writeAttribute("iMem",ba_imem.toBase64());
    xmlOut->writeEndElement();
}

#if 0
/*!
 \brief Get data from memory

 \fn Csc::get_mem
 \param adr     address
 \param size    SIZE_08 or SIZE_16 or SIZE_20 or SIZE_24
 \return DWORD  value
*/
DWORD Csc62015::get_mem(DWORD adr,int size)
{
    switch(size)
    {
    case SIZE_8 :return(pPC->Get_PC(adr));
    case SIZE_16:return(pPC->Get_PC(adr)+(pPC->Get_PC(adr+1)<<8));
    case SIZE_20:return((pPC->Get_PC(adr)+(pPC->Get_PC(adr+1)<<8)+(pPC->Get_PC(adr+2)<<16))&MASK_20);
    case SIZE_24:return((pPC->Get_PC(adr)+(pPC->Get_PC(adr+1)<<8)+(pPC->Get_PC(adr+2)<<16))&MASK_24);
    }
    return(0);
}
/*****************************************************************************/
/* Set data to memory														 */
/*  ENTRY :DOWRD adr=address, int size=SIZE_xx, DWORD data=value			 */
/*  RETURN:none																 */
/*****************************************************************************/
void Csc62015::set_mem(DWORD adr,int size,DWORD data)
{
    switch(size)
    {
    case SIZE_8 :
        pPC->Set_8(adr , (BYTE) data);
        break;
    case SIZE_16:
        pPC->Set_8(adr , (BYTE) data);
        pPC->Set_8(adr+1 , (BYTE) (data>>8));
        break;
    case SIZE_20:
        pPC->Set_8(adr , (BYTE) data);
        pPC->Set_8(adr+1 , (BYTE) (data>>8));
        pPC->Set_8(adr+2 , (BYTE) ((data>>16)&MASK_4));
        break;
    case SIZE_24:
        pPC->Set_8(adr , (BYTE) data);
        pPC->Set_8(adr+1 , (BYTE) (data>>8));
        pPC->Set_8(adr+2 , (BYTE) (data>>16));
        break;
    }
}
#endif

void Csc62015::Regs_Info(UINT8 Type)
{

    sprintf(Regs_String," ");

    switch(Type)
    {
    case 0:			// Monitor Registers Dialog
    case 2:			// For Log File
        sprintf(Regs_String,	"BA:%.4x i:%.4x p:%.6x x:%.6x y:%.6x u:%.6x s:%.6x f:%.2x z:%s c:%s",
            reg.x.ba,reg.x.i,reg.x.p,reg.x.x,reg.x.y,reg.x.u,reg.x.s,reg.x.f,
                reg.r.z?"1":"0",
                reg.r.c?"1":"0");
        break;
    case 1:			// For Log File
        sprintf(Regs_String,	"BA:%.4x i:%.4x p:%.6x x:%.6x y:%.6x u:%.6x s:%.6x f:%.2x z:%s c:%s",
            reg.x.ba,reg.x.i,reg.x.p,reg.x.x,reg.x.y,reg.x.u,reg.x.s,reg.x.f,
                reg.r.z?"1":"0",
                reg.r.c?"1":"0");
        break;
    }
}



bool	Csc62015::Get_Xin(void)
{
    Xin = (imem[IMEM_SSR] >> 1) & 0x01;
    return(Xin);
}

void Csc62015::Set_Xin(bool data)
{

    Xin = data;
    setImemBit(IMEM_SSR,2,data);
}

bool Csc62015::Get_Xout(void)
{
    return(Xout);
}

void Csc62015::Set_Xout(bool data)
{
    Xout = data;
}

INLINE void Csc62015::compute_xout(void)
{

    qint64 wait2khz = pPC->getfrequency()/1000/4;
    qint64 wait4khz = pPC->getfrequency()/1000/8;

    switch (imem[IMEM_SCR]>>4)
    {
        case 0x00 : Xout = false;
                    start2khz = 0;
                    start4khz = 0;
                    //if (fp_log) fprintf(fp_log,"XOUT LOW\n");
                    break;

        case 0x01 : Xout = true;
                    start2khz = 0;
                    start4khz = 0;
                    //if (fp_log) fprintf(fp_log,"XOUT HIGH\n");
                    break;

        case 0x02 : // 2khz
                    start4khz = 0;
                    //if (fp_log) fprintf(fp_log,"XOUT 2Khz\n");
                    if (start2khz == 0){
                        start2khz = pPC->pTIMER->state;
                        if (fp_log) fprintf(fp_log,"XOUT 2Khz INIT\n");
                        Xout = true;
                    }

                    if ((pPC->pTIMER->state - start2khz) >= wait2khz){
                        Xout = !Xout;
                        start2khz += wait2khz;
                        if (fp_log) fprintf(fp_log,"XOUT 2Khz switch\n");
                    }
                    break;

        case 0x03 : // 4khz
                    start2khz = 0;
                    //if (fp_log) fprintf(fp_log,"XOUT 4Khz\n");
                    if (start4khz==0)
                    {
                        start4khz = pPC->pTIMER->state;
                        if (fp_log) fprintf(fp_log,"XOUT 4Khz INIT\n");
                        Xout = true;
                    }

                    if (( pPC->pTIMER->state - start4khz) >= wait4khz)
                    {
                        Xout = !Xout;
                        start4khz += wait4khz;
//                        if (fp_tmp) fprintf(fp_tmp,"%s\n",tr("switch XOUT to %1 : wait = %2  -  delta=%3  new:%4 - old:%5 ").arg(Xout).arg(wait4khz).arg(pPC->pTIMER->state - start4khz).arg(pPC->pTIMER->state).arg(start4khz).toLocal8Bit().data());


                        if (fp_log) fprintf(fp_log,"XOUT 4Khz switch\n");
                    }
                    break;

        case 0x04 : Xout = false;
                    start2khz = 0;
                    start4khz = 0;
                    //if (fp_log) fprintf(fp_log,"XOUT LOW");
                    break;

        case 0x05 : Xout = true;
                    start2khz = 0;
                    start4khz = 0;
                    //if (fp_log) fprintf(fp_log,"XOUT HIGH\n");
                    break;

        case 0x06 :
        case 0x07 : // Xin -> Xout
                    Xout = Xin;
                    start2khz = 0;
                    start4khz = 0;
                    break;
    }

}


UINT32 Csc62015::get_PC(void)
{
    return(get_reg(REG_P));
}
