/**********************************************************/
/* sc61860 CPU emulation                                  */
/**********************************************************/
#include <stdlib.h>
#include <QDebug>

#include "common.h"
#include "Log.h"
#include "pcxxxx.h"
#include "Lcdc.h"
#include "sc61860.h"
#include "Dasm.h"
#include "Inter.h"
#include "Debug.h"
#include "Keyb.h"
#include "dialoganalog.h"
#include "dialoglog.h"
#include "ui/cregssc61860widget.h"

#define ARG2x(i)	arg(i,2,16,QChar('0'))
#define ARG4x(i)	arg(i,4,16,QChar('0'))

FILE *fp_tmplog=NULL;
extern FILE *fp_tmp;

/*---------------------------------------------------------------------------*/

#define		XTICKS      ( pPC->getfrequency() / 2)              // 0.5s counter
#define		XTICK2      ( pPC->getfrequency() / 1000 * 2)       // 2ms counter
#define		XTICKRESET  ( pPC->getfrequency() / 2)              // Reset laptime (0.5s)

CSC61860::CSC61860(CPObject *parent)	: CCPU(parent)
{				//[constructor]
    end=0;				//program end?(0:none, 1:end)
    cpulog=0;				//execute log?(0:off, 1:on)
    logsw=false;			//log mode?(0:off, 1:on)
    fn_log="sc61860.log";
    CallSubLevel=0;

    div500	= 0;
    div2	= 0;
    ticks	= 0;
    ticks2	= 0;
    wait2khz = pPC->getfrequency()/1000/4;
    wait4khz = pPC->getfrequency()/1000/8;
    ticksReset = 0;
    DASMLOG=0;
    first_pass = true;
    pDEBUG	= new Cdebug_sc61860(parent);
    imemsize = 0x60;
#ifndef EMSCRIPTEN
    regwidget = (CregCPU*) new Cregssc61860Widget(parent,this);
    regwidget->hide();
#endif
    start2khz = 0;
    start4khz = 0;
    wait_loop_running = cup_loop_running = cdn_loop_running = false;
    op_local_counter = 0;
}

CSC61860::~CSC61860() {

}

INLINE void CSC61860::AddState(BYTE n)
{
	pPC->pTIMER->state+=(n);

    ticks+=(n);
	ticks2+=(n);

#if 1
    div500  = (ticks >= XTICKS);
    div2    = (ticks2>= XTICK2);
#else
    if (ticks >= XTICKS)
    {
        div500 = true;
        ticks =0;
    }
    if (ticks2 >= XTICK2)
    {
        div2 = true;
        ticks2 =0;
    }
#endif

    if (resetFlag) {
        ticksReset+=(n);
        if (ticksReset >= XTICKRESET)
        {
            qWarning()<<"OKOK";
            resetFlag = false;
            ticksReset = 0;
        }
    }

    backgroundTasks();
}


/*****************************************************************************/
/* Check for I/O access by internal-RAM address								 */
/*  ENTRY :BYTE d=internal RAM address, BYTE len=access bytes				 */
/*  RETURN:none																 */
/*****************************************************************************/

INLINE void CSC61860::Chk_imemAdr(BYTE d,BYTE len)
{
}

bool	CSC61860::Get_Xin(void)
{
	return(Xin);
}

void CSC61860::Set_Xin(bool data)
{

	Xin = data;
}

INLINE bool CSC61860::Get_Xout(void)
{
	return(Xout);
}

void CSC61860::Set_Xout(bool data)
{
    Xout = data;
}

void CSC61860::set_PC(UINT32 data)
{
    reg.d.pc = data;
}

extern FILE	*fp_tmp;
INLINE void CSC61860::compute_xout(void)
{
	qint64 delta;
	qint64 wait2khz = pPC->getfrequency()/1000/4;
	qint64 wait4khz = pPC->getfrequency()/1000/8;

	switch (pPC->IO_C>>4)
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
					delta = pPC->pTIMER->state - start2khz;
                    //while
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
					delta = pPC->pTIMER->state - start4khz;
                    //while
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


/*****************************************************************************/
/* Get data from imem[]														 */
/*  ENTRY :BYTE a=internal RAM address										 */
/*  RETURN:(BYTE(8),WORD(16),DWORD(20,24) data)								 */
/*****************************************************************************/
INLINE BYTE CSC61860::Get_i8(BYTE adr)
{
    if (fp_log) fprintf(fp_log,"LECTURE INT [%02x]=%02x (%c)\n",adr,imem[adr],imem[adr]);
	return(imem[adr]);
}
INLINE WORD CSC61860::Get_i16(BYTE adr)
{
    //if (fp_log) fprintf(fp_log,"LECTURE INT [%02x]=%04x (%c)(%c)\n",adr,imem[adr]+(imem[adr+1]<<8),imem[adr],(imem[adr+1]<<8));
    return(imem[adr]+(imem[adr+1]<<8));
}

/*****************************************************************************/
/* Set data to imem[]														 */
/*  ENTRY :BYTE a=internal RAM address, BYTE(8),WORD(16),DWORD(20,24) d=data */
/*****************************************************************************/
INLINE void CSC61860::Set_i8(BYTE adr,BYTE d)
{
    if (fp_log) fprintf(fp_log,"ECRITURE INT [%02x]=%02x (%c)\n",adr,d,d);
	imem[adr] = d;
}
INLINE void CSC61860::Set_i16(BYTE adr,WORD d)
{
    //if (fp_log) fprintf(fp_log,"ECRITURE INT [%04x]=%04x (%c)(%c)\n",adr,d,(BYTE) d),(BYTE) (d>>8);
	imem[adr  ]	=(BYTE) d;
	imem[adr+1]	=(BYTE) (d>>8);
}

/*****************************************************************************/
/* Get register data														 */
/*  ENTRY :BYTE r=register No.(0-7)											 */
/*  RETURN:(BYTE(8),WORD(16),DWORD(20) data)								 */
/*****************************************************************************/
INLINE  UINT32 CSC61860::Get_r(BYTE r)
{
	switch(r & 7)
	{
	case	0: return(I_REG_I); break;
	case	1: return(I_REG_J); break;
	case	2: return(I_REG_A); break;
	case	3: return(I_REG_B); break;
	case	4: return(I_REG_K); break;
	case	5: return(I_REG_L); break;
	case	6: return(I_REG_M); break;
	case	7: return(I_REG_N); break;
	}
	return(0);
}


/*****************************************************************************/
/* Set register data														 */
/*  ENTRY :BYTE r=register No.(0-7), BYTE(8),WORD(16),DWORD(20) d=data		 */
/*****************************************************************************/
INLINE  void CSC61860::Set_r(BYTE r,UINT32 d)
{
	switch(r&7)
	{
	case	0:I_REG_I = (BYTE) d	; break;
	case	1:I_REG_J = (BYTE) d	; break;
	case	2:I_REG_A = (BYTE) d	; break;
	case	3:I_REG_B = (BYTE) d	; break;
	case	4:I_REG_K = (BYTE) d	; break;
	case	5:I_REG_L = (BYTE) d	; break;
	case	6:I_REG_M = (BYTE) d	; break;
	case	7:I_REG_N = (BYTE) d	; break;
	}
}

/*****************************************************************************/
/* Check flags(Zero & Carry)												 */
/*  ENTRY :DWORD d=data, BYTE len=data length(8bit=1,16bit=2,20bit=3)		 */
/*****************************************************************************/
UINT32	mask_z[]={0,MASK_8,MASK_16,MASK_20,MASK_24};
UINT32	mask_c[]={0,MASK_8+1,MASK_16+1,MASK_20+1,MASK_24+1};

INLINE void CSC61860::Chk_Flag(UINT32 d,BYTE len)
{

	reg.r.z = ( (d & mask_z[len])==0 ) ? 1 : 0;
	reg.r.c = ( (d & mask_c[len])==0 ) ? 0 : 1;
}

INLINE void CSC61860::Chk_Zero(UINT32 d,BYTE len)
{
	reg.r.z = ( (d & mask_z[len])==0 ) ? 1 : 0;
}

/*****************************************************************************/
/* Convert from BCD to HEX													 */
/*  ENTRY :BYTE d=BCD data(00h-99h)											 */
/*  RETURN:BYTE HEX data(00h-63h)											 */
/*****************************************************************************/
INLINE BYTE bcd2hex(BYTE d)
{
	return((d>>4)*10+(d&15));
}

/*****************************************************************************/
/* Convert from HEX to BCD													 */
/*  ENTRY :BYTE d=HEX data(00h-ffh)											 */
/*  RETURN:BYTE BCD data(00h-255h)											 */
/*****************************************************************************/
INLINE WORD hex2bcd(BYTE d)
{
	BYTE	a,b,c;
	a=d/100;
	b=d-(a*100);
	c=b/10;
	return((a<<8)+(c<<4)+b-(c*10));
}


//---------------------------------------------------------------------------
//
//
//---------------------------------------------------------------------------
//--------------------------
//	LII n		n -> I
//--------------------------
INLINE void CSC61860::Op_00(void)
{
    I_REG_I = pPC->Get_PC(reg.d.pc);
	AddLog(LOG_CPU,tr("LII %1").ARG2x(I_REG_I));
	reg.d.pc++;
	AddState(4);
}
//--------------------------
//	LIJ n		n -> J
//--------------------------
INLINE void CSC61860::Op_01(void)
{
    I_REG_J = pPC->Get_PC(reg.d.pc);
	AddLog(LOG_CPU,tr("LIJ %1").ARG2x(I_REG_J));
	reg.d.pc++;
	AddState(4);
}
//--------------------------
//	LIA n		n -> A
//--------------------------
INLINE void CSC61860::Op_02(void)
{
    I_REG_A = pPC->Get_PC(reg.d.pc);
	AddLog(LOG_CPU,tr("LIA %1").ARG2x(I_REG_A));
	reg.d.pc++;
	AddState(4);
}
//--------------------------
//	LIB n		n -> B
//--------------------------
INLINE void CSC61860::Op_03(void)
{
    I_REG_B = pPC->Get_PC(reg.d.pc);
	AddLog(LOG_CPU,tr("LIB %1").ARG2x(I_REG_B));
	reg.d.pc++;
	AddState(4);
}
//--------------------------
//	IX
//--------------------------
INLINE void CSC61860::Op_04(void)
{
    reg.r.q=I_REG_Xh;
	if (++I_REG_Xl==0) I_REG_Xh++;
	reg.d.dp=I_REG_Xl|(I_REG_Xh<<8);
	AddLog(LOG_CPU,tr("IX"));
	AddState(6);
}
//--------------------------
//	DX
//--------------------------
INLINE void CSC61860::Op_05(void)
{
    reg.r.q=I_REG_Xh;
	if (--I_REG_Xl==0xff) I_REG_Xh--;
    reg.d.dp=I_REG_Xl|(I_REG_Xh<<8);	
	AddLog(LOG_CPU,"DX");
	AddState(6);
}
//--------------------------
//	IY
//--------------------------
INLINE void CSC61860::Op_06(void)
{
    reg.r.q=I_REG_Yh;
	if (++I_REG_Yl==0) I_REG_Yh++;
	reg.d.dp=I_REG_Yl|(I_REG_Yh<<8);
	AddLog(LOG_CPU,"IY");

	AddState(6);
}
//--------------------------
//	DY
//--------------------------
INLINE void CSC61860::Op_07(void)
{
    reg.r.q=I_REG_Yh;
	if (--I_REG_Yl==0xff) I_REG_Yh--;
    reg.d.dp=I_REG_Yl|(I_REG_Yh<<8);	
	AddLog(LOG_CPU,"DY");
	AddState(6);
}


//--------------------------
//	MVW
//--------------------------
INLINE void CSC61860::Op_08(void)
{
	BYTE l;
	l = I_REG_I;

    AddState(3);
	while(l != 0xFF)
	{
		Set_i8( reg.r.p,Get_i8(reg.r.q));
		reg.r.p++;	reg.r.p &=0x7F;
		reg.r.q++;	reg.r.q &=0x7F;
		l--;
        AddState(2);
		AddLog(LOG_CPU,tr("MVW  (i)=%1").ARG2x(l));
	}
}

//--------------------------
//	MVB
//--------------------------
INLINE void CSC61860::Op_0a(void)
{
	BYTE l;
	l = I_REG_J;
    AddState(3);
	AddLog(LOG_CPU,"MVB");
	while(l != 0xFF)
	{
		Set_i8( reg.r.p,Get_i8(reg.r.q));
		reg.r.p++;	reg.r.p &=0x7F;
		reg.r.q++;	reg.r.q &=0x7F;
		l--;
        AddState(2);
	}
}

//--------------------------
//	EXW
//--------------------------
INLINE void CSC61860::Op_09(void)
{
	BYTE t;
	BYTE l;
	l = I_REG_I;

    AddState(3);
	AddLog(LOG_CPU,"EXW");
	while(l != 0xFF)
	{
		t = Get_i8(reg.r.p);
		Set_i8( reg.r.p,Get_i8(reg.r.q));
		Set_i8( reg.r.q,t);
		reg.r.p++;	reg.r.p &=0x7F;
		reg.r.q++;	reg.r.q &=0x7F;
		l--;
        AddState(3);
	}
}

//--------------------------
//	EXB
//--------------------------
INLINE void CSC61860::Op_0b(void)
{
	BYTE t;
	BYTE l;
	l = I_REG_J;

    AddState(3);
	AddLog(LOG_CPU,"EXB");
	while(l != 0xFF)
	{
		t = Get_i8(reg.r.p);
		Set_i8( reg.r.p,Get_i8(reg.r.q));
		Set_i8( reg.r.q,t);
		reg.r.p++;	reg.r.p &=0x7F;
		reg.r.q++;	reg.r.q &=0x7F;
		l--;
        AddState(3);
	}
}
//--------------------------
// ADN
//--------------------------
INLINE void CSC61860::Op_0c(void)
{
	BYTE	d,b;
	WORD	w;
	BYTE l;
	l = I_REG_I;

    AddState(7);

	d=Get_i8(reg.r.p);
	w=hex2bcd(
		bcd2hex(Get_i8(reg.d.p)) + bcd2hex(I_REG_A)
		);
	Chk_Flag(w,SIZE_8);
	Set_i8(reg.r.p,(BYTE)w);
	reg.r.p--;reg.r.p &=0x7F;
	b = (BYTE) w;
	while(l!=0x00)
	{
		w=hex2bcd(
			bcd2hex(Get_i8(reg.d.p)) + reg.r.c
			);
		Chk_Flag(w,SIZE_8);
		Set_i8(reg.r.p,(BYTE)w);
		reg.r.p--;reg.r.p &=0x7F;
		b |= w;
		l--;
        AddState(3);
	}
	Chk_Zero(b,SIZE_8);
	AddLog(LOG_CPU,"ADN");
}
//--------------------------
// SBN
//--------------------------
INLINE void CSC61860::Op_0d(void)
{
	BYTE	b;
	WORD	w;
	BYTE l;
	l = I_REG_I;

    AddState(7);

	w=hex2bcd(100+bcd2hex(Get_i8(reg.d.p)) - bcd2hex(I_REG_A));
	reg.r.c=1-(w>>8);
	Set_i8(reg.r.p,(BYTE)w);
	reg.r.p--;reg.r.p &=0x7F;
	b = (BYTE) w;
	while(l!=0x00)
	{
		w=hex2bcd(100+bcd2hex(Get_i8(reg.d.p)) - reg.r.c);
		reg.r.c = 1-(w>>8);
		Set_i8(reg.r.p,(BYTE) w);
		reg.r.p--;reg.r.p &=0x7F;
		b |= w;
		l--;
        AddState(3);
	}
	Chk_Zero(b,SIZE_8);
	AddLog(LOG_CPU,"SBN");
}
//--------------------------
// ADW
//--------------------------
INLINE void CSC61860::Op_0e(void)
{
	BYTE	b;
	WORD	w;
	BYTE l;
	l = I_REG_I;


    AddState(7);

	w=hex2bcd(
		bcd2hex(Get_i8(reg.d.p)) + bcd2hex(Get_i8(reg.r.q))
		);
	Chk_Flag(w,SIZE_8);
	Set_i8(reg.r.p,(BYTE)w);
	reg.r.p--;reg.r.p &=0x7F;
	reg.r.q--;reg.r.q &=0x7F;
	b = (BYTE) w;
	while(l!=0x00)
	{
		w=hex2bcd(
			bcd2hex(Get_i8(reg.d.p)) + bcd2hex(Get_i8(reg.r.q))+reg.r.c
			);
		Chk_Flag(w,SIZE_8);
		Set_i8(reg.r.p,(BYTE)w);
		b |= w;
		reg.r.p--;reg.r.p &=0x7F;
		reg.r.q--;reg.r.q &=0x7F;
		l--;
        AddState(3);
	}
	Chk_Zero(b,SIZE_8);
	AddLog(LOG_CPU,"ADW");
}
//--------------------------
// SBW
//--------------------------
INLINE void CSC61860::Op_0f(void)
{
	BYTE	b;
	WORD	w;
	BYTE l;
	l = I_REG_I;

    AddState(7);

	w=hex2bcd(
		100 + bcd2hex(Get_i8(reg.d.p)) - bcd2hex(Get_i8(reg.r.q))
		);
	reg.r.c=1-(w>>8);
	Set_i8(reg.r.p,(BYTE)w);
	reg.r.p--;reg.r.p &=0x7F;
	reg.r.q--;reg.r.q &=0x7F;
	b = (BYTE) w;
	while(l!=0x00)
	{
		w=hex2bcd(
			100 + bcd2hex(Get_i8(reg.d.p)) - bcd2hex(Get_i8(reg.r.q))-reg.r.c
			);
		reg.r.c = 1-(w>>8);
		Set_i8(reg.r.p,(BYTE)w);
		b |= w;
		reg.r.p--;reg.r.p &=0x7F;
		reg.r.q--;reg.r.q &=0x7F;
		l--;
        AddState(3);
	}
	Chk_Zero(b,SIZE_8);
	AddLog(LOG_CPU,"SBW");
}
//--------------------------
//	LIDP nm		nm -> DP
//--------------------------
INLINE void CSC61860::Op_10(void)
{
    reg.d.dp=pPC->Get_16rPC(reg.d.pc);
	AddLog(LOG_CPU,tr("LIDP    %1").ARG4x(reg.d.dp));
	reg.d.pc+=SIZE_16;
	AddState(8);
}

//--------------------------
//	LIDL n		n -> DPl
//--------------------------
INLINE void CSC61860::Op_11(void)
{
    reg.d.dp = (reg.d.dp & 0xFF00) | pPC->Get_PC(reg.d.pc);
    //reg.r.dpl=pPC->Get_PC(reg.d.pc);
    //AddLog(LOG_CPU,tr("LIDL    %1").ARG2x(reg.r.dpl));
	reg.d.pc++;
	AddState(5);
}

//--------------------------
//	LIP n		n -> P
//--------------------------
INLINE void CSC61860::Op_12(void)
{
    reg.r.p=pPC->Get_PC(reg.d.pc);	reg.r.p &=0x7F;

	AddLog(LOG_CPU,tr("LIP     %1").ARG2x(reg.r.p));
	reg.d.pc++;
	AddState(4);
}
//--------------------------
//	LIQ n		n -> Q
//--------------------------
INLINE void CSC61860::Op_13(void)
{
    reg.r.q=pPC->Get_PC(reg.d.pc);
	reg.r.q &=0x7F;

	AddLog(LOG_CPU,tr("LIQ %1").ARG2x(reg.r.q));
	reg.d.pc++;
	AddState(4);
}

//--------------------------
//	ADB
//--------------------------
INLINE void CSC61860::Op_14(void)
{	
	UINT32 t;

	t = Get_i16(reg.r.p) + ( I_REG_A|(I_REG_B<<8) );	

	Chk_Flag(t,SIZE_16);
	Set_i16(reg.r.p, (WORD) t);
    reg.r.p++;	reg.r.p &= 0x7F;
	AddLog(LOG_CPU,"ADB");

	AddState(5);
}
//--------------------------
//	SBB
//--------------------------
INLINE void CSC61860::Op_15(void)
{	
	UINT32 t;

	t = Get_i16(reg.r.p) - ( I_REG_A|(I_REG_B<<8) );
	Chk_Flag(t,SIZE_16);
	Set_i16(reg.r.p, (WORD) t);
    reg.r.p++;	reg.r.p &= 0x7F;
	AddLog(LOG_CPU,"SBB");

	AddState(5);
}


//--------------------------
//	DATA
//--------------------------
INLINE void CSC61860::Op_35(void)	//ok
{
	BYTE l;
	WORD ba;

	l = I_REG_I;
	ba = ( I_REG_A|(I_REG_B<<8) );

    AddState(1);
	AddLog(LOG_CPU,"DATA");

	while(l != 0xFF)
	{
        Set_i8(reg.r.p,pPC->Get_PC(ba));
		reg.r.p++;	reg.r.p &= 0x7F;
		ba++;
		l--;
        AddState(4);
	}

}


//--------------------------
//	MVWD
//--------------------------
INLINE void CSC61860::Op_18(void)
{
	BYTE l;
	l = I_REG_I;

    AddState(1);
	while( l != 0xFF)
	{
		Set_i8((BYTE) reg.r.p,pPC->Get_8(reg.d.dp));
		reg.r.p++;	reg.r.p &=0x7F;
		reg.d.dp++;
		l--;
        AddState(4);
		AddLog(LOG_CPU,tr("MVWD  (i)=%1").ARG2x(l));
	}
}

//--------------------------
//	MVBD
//--------------------------
INLINE void CSC61860::Op_1a(void)
{
	BYTE l;
	l = I_REG_J;

    AddState(1);
	AddLog(LOG_CPU,"MVBD");
		while(l != 0xFF)
	{
		Set_i8((BYTE) reg.r.p,pPC->Get_8(reg.d.dp));
		reg.r.p++;	reg.r.p &=0x7F;
		reg.d.dp++;
		l--;
        AddState(4);
	}
}

//--------------------------
//	EXWD
//--------------------------
INLINE void CSC61860::Op_19(void)
{
	BYTE t;
	BYTE l;
	l = I_REG_I;

    AddState(1);
	AddLog(LOG_CPU,"EXWD");
		while(l != 0xFF)
	{
		t = Get_i8(reg.r.p);
		Set_i8( reg.r.p, pPC->Get_8(reg.d.dp));
		pPC->Set_8(reg.d.dp,t);
		reg.r.p++;	reg.r.p &=0x7F;
		reg.d.dp++;
		l--;
        AddState(6);
	}
}

//--------------------------
//	EXBD
//--------------------------
INLINE void CSC61860::Op_1b(void)
{
	BYTE t;
	BYTE l;
	l = I_REG_J;

    AddState(1);
	AddLog(LOG_CPU,"EXBD");
		while(l != 0xFF)
	{
		t = Get_i8(reg.r.p);
		Set_i8( reg.r.p,pPC->Get_8(reg.d.dp));
		pPC->Set_8(reg.d.dp,t);
		reg.r.p++;	reg.r.p &=0x7F;
		reg.d.dp++;
		l--;
        AddState(6);
	}
}

//--------------------------
// SRW
//--------------------------
INLINE void CSC61860::Op_1c(void)
{
	
	BYTE l,bp,a,b,t;
	l = I_REG_I;


    AddState(4);
	AddLog(LOG_CPU,"SRW");

	bp = 0;//Get_i8(reg.r.p-1)&0x0f;
		while(l != 0xFF)
	{
		t=Get_i8(reg.r.p);
		a = (t >> 4);
		b = (t & 0x0F);

		Set_i8(reg.r.p, (a)|(bp<<4));
		bp = b;
		reg.r.p++;	reg.r.p &=0x7F;
		l--;
        AddState(1);
	}

//	Chk_Zero(b,SIZE_8);
}
//--------------------------
// SLW
//--------------------------
INLINE void CSC61860::Op_1d(void)
{
	BYTE l,a,t,b,ap;
	AddLog(LOG_CPU,"SLW");
	l = I_REG_I;
    AddState(4);
	
	ap = 0;//Get_i8(reg.r.p+1)>>4;
		while(l != 0xFF)
	{
		t=Get_i8(reg.r.p);
		a = (t >> 4);
		b = (t & 0x0F);

		Set_i8(reg.r.p, (b<<4)|ap);
		ap = a;
		reg.r.p--;reg.r.p &=0x7F;
		l--;
        AddState(1);
	}

//	Chk_Zero(b,SIZE_8);
}
//--------------------------
//	FILM
//--------------------------
INLINE void CSC61860::Op_1e(void)
{
	BYTE l;
	l = I_REG_I;

    AddState(4);
	AddLog(LOG_CPU,"FILM");
		while(l != 0xFF)
	{
		Set_i8( reg.r.p,I_REG_A);
		reg.r.p++;	reg.r.p &=0x7F;
		l--;
        AddState(1);
	}
}


//--------------------------
//	FILD
//--------------------------
INLINE void CSC61860::Op_1f(void)
{
	BYTE l;
	l = I_REG_I;

    AddState(1);
	AddLog(LOG_CPU,"FILD");
		while(l != 0xFF)
	{
		pPC->Set_8( reg.d.dp,I_REG_A);
		reg.d.dp++;
		l--;
        AddState(3);
	}
//	reg.d.dp--;
}

//--------------------------
//	LDP		P -> A
//--------------------------
INLINE void CSC61860::Op_20(void)
{
	I_REG_A=reg.r.p;
	AddLog(LOG_CPU,"LDP");
	AddState(2);
}
//--------------------------
//	LDQ		Q -> A
//--------------------------
INLINE void CSC61860::Op_21(void)
{
	I_REG_A=reg.r.q;
	AddLog(LOG_CPU,"LDQ");
	AddState(2);
}
//--------------------------
//	LDR		R -> A
//--------------------------
INLINE void CSC61860::Op_22(void)
{
	I_REG_A=reg.r.r;
	AddLog(LOG_CPU,"LDR");
	AddState(2);
}

//--------------------------
//	IXL
//--------------------------
INLINE void CSC61860::Op_24(void)
{
    reg.r.q=I_REG_Xh;
	if (++I_REG_Xl==0) I_REG_Xh++;
	reg.d.dp=I_REG_Xl|(I_REG_Xh<<8);
	I_REG_A = pPC->Get_8(reg.d.dp);
	AddLog(LOG_CPU,"IXL");
	AddState(7);
}


//--------------------------
//	DXL
//--------------------------
INLINE void CSC61860::Op_25(void)
{
    reg.r.q=I_REG_Xh;
	if (--I_REG_Xl==0xff) I_REG_Xh--;
    reg.d.dp=I_REG_Xl|(I_REG_Xh<<8);	
	I_REG_A = pPC->Get_8(reg.d.dp);
	AddLog(LOG_CPU,"DXL");
	AddState(7);
}

//--------------------------
//	IYS
//--------------------------
INLINE void CSC61860::Op_26(void)
{
    reg.r.q=I_REG_Yh;
	if (++I_REG_Yl==0) I_REG_Yh++;
	reg.d.dp=I_REG_Yl|(I_REG_Yh<<8);
	pPC->Set_8(reg.d.dp,I_REG_A);
	AddLog(LOG_CPU,"IYS");
	AddState(6);
}

//--------------------------
//	DYS
//--------------------------
INLINE void CSC61860::Op_27(void)
{
    reg.r.q=I_REG_Yh;
	if (--I_REG_Yl==0xff) I_REG_Yh--;
    reg.d.dp=I_REG_Yl|(I_REG_Yh<<8);	
	pPC->Set_8(reg.d.dp,I_REG_A);
	AddLog(LOG_CPU,"DYS");
	AddState(6);
}
//----------------------------
// JRNZP n
//----------------------------
INLINE void CSC61860::Op_28(void)
{
	BYTE	t;
    t=pPC->Get_PC(reg.d.pc);
	AddLog(LOG_CPU,tr("JRNZP   %1").ARG2x(t));
	reg.d.pc++;
	if(!reg.r.z)
	{
		reg.d.pc+=(t-1);
		AddState(3);
	}
	AddState(4);
}
//----------------------------
// JRNZM  n
//----------------------------
INLINE void CSC61860::Op_29(void)
{
	BYTE	t;
    t=pPC->Get_PC(reg.d.pc);
	AddLog(LOG_CPU,tr("JRNZM   %1").ARG2x(t));
	reg.d.pc++;
	if(!reg.r.z)
	{
		reg.d.pc-=(t+1);
		AddState(3);
	}
	AddState(4);
}
//----------------------------
// JRNCP n 
//----------------------------
INLINE void CSC61860::Op_2a(void)
{
	BYTE	t;
    t=pPC->Get_PC(reg.d.pc);
	AddLog(LOG_CPU,tr("JRNCP   %1").ARG2x(t));
	reg.d.pc++;
	if(!reg.r.c)
	{
		reg.d.pc+=(t-1);
		AddState(3);
	}
	AddState(4);
}
//----------------------------
// JRNCM n
//----------------------------
INLINE void CSC61860::Op_2b(void)
{
	BYTE	t;
    t=pPC->Get_PC(reg.d.pc++);
	AddLog(LOG_CPU,tr("JRNCM   %1").ARG2x(t));
	
	if(!reg.r.c)
	{
		reg.d.pc-=(t+1);
		AddState(3);
	}
	AddState(4);
}
//----------------------------
// JRP n
//----------------------------
INLINE void CSC61860::Op_2c(void)
{
	BYTE	t;
    t=pPC->Get_PC(reg.d.pc++);
	AddLog(LOG_CPU,tr("JRP     %1").ARG2x(t));
	reg.d.pc += (t-1);
	AddState(7);
}

//----------------------------
// JRM n
//----------------------------
INLINE void CSC61860::Op_2d(void)
{
	BYTE t;
    t = pPC->Get_PC(reg.d.pc++);
	AddLog(LOG_CPU,tr("JRM     %1").ARG2x(t));
	reg.d.pc -= (t+1);
	AddState(7);
}
//--------------------------
//	STP		A -> P
//--------------------------
INLINE void CSC61860::Op_30(void)
{
	reg.r.p=I_REG_A;	reg.r.p &=0x7F;
	AddLog(LOG_CPU,"STP");
	AddState(2);
}
//--------------------------
//	STQ		A -> Q
//--------------------------
INLINE void CSC61860::Op_31(void)
{
	reg.r.q=I_REG_A;
	AddLog(LOG_CPU,"STQ");
	AddState(2);
}
//--------------------------
//	STR		A -> R
//--------------------------
INLINE void CSC61860::Op_32(void)
{
	reg.r.r=I_REG_A;
	AddLog(LOG_CPU,"STR");
	AddState(2);
}

//--------------------------
//	PUSH
//--------------------------
INLINE void CSC61860::Op_34(void)
{
	reg.r.r--;
	Set_i8(reg.r.r,I_REG_A);
	AddLog(LOG_CPU,"PUSH");
    if (reg.r.r > 0x5b)
	{
        if (fp_log) fprintf(fp_log,"STACK OVERFLOW!!!!!!!!\n");
		AddLog(0x00,"STACK OVERFLOW");
	}
	AddState(3);
}

//----------------------------
// JRZP n 
//----------------------------
INLINE void CSC61860::Op_38(void)
{
	register BYTE	t;
    t=pPC->Get_PC(reg.d.pc++);
	AddLog(LOG_CPU,tr("JRZP %1").ARG2x(t));
	
	if(reg.r.z)
	{
		reg.d.pc+=(t-1);
		AddState(3);
	}
	AddState(4);
}
//----------------------------
// JRZM n 
//----------------------------
INLINE void CSC61860::Op_39(void)
{
    // Due to synhronization issue (mainly about tape) 7 cycles in one shot is not acceptable
    // I split this instruction in 2 steps
#if 0
    BYTE	t;
    BYTE st=0;
    t=pPC->Get_PC(reg.d.pc++);
	AddLog(LOG_CPU,tr("JRZM %1").ARG2x(t));
	
	if(reg.r.z)
	{
		reg.d.pc-=(t+1);
        st=3;
	}
    AddState(st+4);
#else
    BYTE	t;
    BYTE st=0;
    if (first_pass) {
        AddState(4);
        first_pass=false;
        reg.d.pc--;
    }
    else {
        first_pass=true;
        t=pPC->Get_PC(reg.d.pc++);
        AddLog(LOG_CPU,tr("JRZM %1").ARG2x(t));

        if(reg.r.z)
        {
            reg.d.pc-=(t+1);
            AddState(3);
        }
    }

#endif
}
//----------------------------
// JRCP n
//----------------------------
INLINE void CSC61860::Op_3a(void)
{
	register BYTE	t;
    t=pPC->Get_PC(reg.d.pc++);
	AddLog(LOG_CPU,tr("JRCP %1").ARG2x(t));
	
	if(reg.r.c)
	{
		reg.d.pc+=(t-1);
		AddState(3);
	}
	AddState(4);
}
//----------------------------
// JRCM n
//----------------------------
INLINE void CSC61860::Op_3b(void)
{
	register BYTE	t;
    t=pPC->Get_PC(reg.d.pc++);
	AddLog(LOG_CPU,tr("JRCM %1").ARG2x(t));
	
	if(reg.r.c)
	{
		reg.d.pc-=(t+1);
		AddState(3);
	}
	AddState(4);
}

//----------------------------
// RTN
//----------------------------
INLINE void CSC61860::Op_37(void)
{

	reg.d.pc=Get_i16(reg.r.r);
	CallSubLevel--;
//    if (CallSubLevel <0) CallSubLevel = 0;
	reg.r.r+=SIZE_16;
	AddLog(LOG_CPU,"RTN");
	AddState(4);

}
//----------------------------
// INCr
//----------------------------
INLINE void CSC61860::Op_40(BYTE ind)
{
	UINT32 d;

	d = ((BYTE) Get_r(ind)) + 1;
	Chk_Flag(d,SIZE_8);
	Set_r(ind,(BYTE)d);

	switch (ind)
	{
		case 0 : AddLog(LOG_CPU,"INC I"); 
				 reg.r.q= 0;	break;
		case 1 : AddLog(LOG_CPU,"INC J"); 
				 reg.r.q= 1;	break;
		case 2 : AddLog(LOG_CPU,"INC A");
				 reg.r.q= 2;	break;
		case 3 : AddLog(LOG_CPU,"INC B");
				 reg.r.q= 3;	break;
		case 4 : AddLog(LOG_CPU,"INC K");
				 reg.r.q= 8;	break;
		case 5 : AddLog(LOG_CPU,"INC L");
				 reg.r.q= 9;	break;
		case 6 : AddLog(LOG_CPU,"INC M");
				 reg.r.q=10;	break;
		case 7 : AddLog(LOG_CPU,"INC N");
				 reg.r.q=11;	break;
	}
	AddState(4);
}
//----------------------------
// DECr
//----------------------------
INLINE void CSC61860::Op_41(BYTE ind)
{
	UINT32 d;

	d= ((BYTE) Get_r(ind)) - 1;
	Chk_Flag(d,SIZE_8);
	Set_r(ind,(BYTE)d);

	switch (ind){
		case 0 : AddLog(LOG_CPU,"DEC I");
				 reg.r.q= 0;	break;
		case 1 : AddLog(LOG_CPU,"DEC J"); 
				 reg.r.q= 1;	break;
		case 2 : AddLog(LOG_CPU,"DEC A"); 
				 reg.r.q= 2;	break;
		case 3 : AddLog(LOG_CPU,"DEC B"); 
				 reg.r.q= 3;	break;
		case 4 : AddLog(LOG_CPU,"DEC K"); 
				 reg.r.q= 8;	break;
		case 5 : AddLog(LOG_CPU,"DEC L"); 
				 reg.r.q= 9;	break;
		case 6 : AddLog(LOG_CPU,"DEC M"); 
				 reg.r.q=10;	break;
		case 7 : AddLog(LOG_CPU,"DEC N"); 
				 reg.r.q=11;	break;
	}
	AddState(4);
}

//----------------------------
// ADM		(P) + A -> (P)	C,Z 
//----------------------------
INLINE void CSC61860::Op_44(void)
{
	UINT32 t;

	t = Get_i8(reg.r.p) + I_REG_A;
	Chk_Flag(t,SIZE_8);
	Set_i8(reg.r.p, (BYTE) t);

	AddLog(LOG_CPU,"ADM");
	AddState(3);
}
//----------------------------
// SBM		(P) - A -> (P)	C,Z 
//----------------------------
INLINE void CSC61860::Op_45(void)
{
	UINT32 t;

	t = Get_i8(reg.r.p) - I_REG_A;
	Chk_Flag(t,SIZE_8);
	Set_i8(reg.r.p, (BYTE) t);

	AddLog(LOG_CPU,"SBM");
	AddState(3);
}
//----------------------------
// ADCM		(P) + A + C -> (P)	C,Z 
//----------------------------
INLINE void CSC61860::Op_c4(void)
{
	UINT32 t;

	t = Get_i8(reg.r.p) + I_REG_A + reg.r.c;
	Chk_Flag(t,SIZE_8);
	Set_i8(reg.r.p, (BYTE) t);

	AddLog(LOG_CPU,"ADCM");
	AddState(3);
}
//----------------------------
// SBCM		(P) - A - C -> (P)	C,Z 
//----------------------------
INLINE void CSC61860::Op_c5(void)
{
	UINT32 t;

	t = Get_i8(reg.r.p) - I_REG_A - reg.r.c;
	Chk_Flag(t,SIZE_8);
	Set_i8(reg.r.p, (BYTE) t);

	AddLog(LOG_CPU,"SBCM");
	AddState(3);
}


//----------------------------
// NOPW 
//----------------------------
INLINE void CSC61860::Op_4d(void)
{
	AddLog(LOG_CPU,"NOPW");
	AddState(2);
}


//----------------------------
// WAIT n
//----------------------------
// One by One STEP slow down the emulator speed
// BUT IT IS NEEDED FOR XIN XOUT ACCURACY
// TRY TO EMULATE 10 by 10 STEP
INLINE void CSC61860::Op_4e(void)
{
#if 1
	
    if (!wait_loop_running)
	{
        wait_loop_running = true;
		AddState(6);
        op_local_counter=pPC->Get_PC(reg.d.pc++);
        if (fp_log) fprintf(fp_log,"START WAIT -6: %i\n",op_local_counter);
		AddLog(LOG_CPU,tr("WAIT %1").ARG2x(op_local_counter));
		reg.d.pc--;
	}
//    if (op_local_counter>5)
//    {
//        AddState(5);
//        op_local_counter-=5;
//        if (fp_log) fprintf(fp_log,"current WAIT -5: %i\n",op_local_counter);
//        reg.d.pc--;
//    }
//    else
	if (op_local_counter--)
	{
		AddState(1);
//        if (fp_log) fprintf(fp_log,"current WAIT -1: %i\n",op_local_counter);
		reg.d.pc--;
	}
	else
	{
        wait_loop_running = false;
        if (fp_log) fprintf(fp_log,"FIN WAIT : %i\n",op_local_counter);
		reg.d.pc++;
	}
#else
    AddState(6+pPC->Get_PC(reg.d.pc++));
#endif
}

//----------------------------
// INCP
//----------------------------
INLINE void CSC61860::Op_50(void)
{
	reg.r.p++;	reg.r.p &= 0x7f;
	AddLog(LOG_CPU,"INCP");
	AddState(2);
}
//----------------------------
// DECP
//----------------------------
INLINE void CSC61860::Op_51(void)
{
	reg.r.p--;	reg.r.p &=0x7F;
	AddLog(LOG_CPU,"DECP");
	AddState(2);
}

//----------------------------
// STD		A -> (DP)
//----------------------------
INLINE void CSC61860::Op_52(void)
{
	pPC->Set_8(reg.d.dp,I_REG_A);
	AddLog(LOG_CPU,"STD");
	AddState(2);
}


//----------------------------
// LDD		(DP) -> A
//----------------------------
INLINE void CSC61860::Op_57(void)
{
	I_REG_A =  pPC->Get_8(reg.d.dp);
	AddLog(LOG_CPU,"LDP");
	AddState(3);
}

//----------------------------
// SWP (Swap A)
//----------------------------
INLINE void CSC61860::Op_58(void)
{
	I_REG_A=(I_REG_A>>4)+(I_REG_A<<4);
	AddLog(LOG_CPU,"SWP");
	AddState(2);
}
//----------------------------
// LDM		(P) -> A
//----------------------------
INLINE void CSC61860::Op_59(void)
{
	I_REG_A = Get_i8(reg.r.p);
	AddLog(LOG_CPU,"LDM");
	AddState(2);
}
//--------------------------
//	POP
//--------------------------
INLINE void CSC61860::Op_5b(void)
{
	I_REG_A=Get_i8(reg.r.r++);
	AddLog(LOG_CPU,"POP");
    if (reg.r.r > 0x5c)
	{
        if (fp_log) fprintf(fp_log,"STACK OVERFLOW!!!!!!!!\n");
		AddLog(0x80,"STACK OVERFLOW");
	}
	AddState(2);
}

//----------------------------
// ORIA		A V n -> A,Z 
//----------------------------
INLINE void CSC61860::Op_65(void)
{
	BYTE t,d,r ;

	d = I_REG_A;
    t = pPC->Get_PC(reg.d.pc++);
	AddLog(LOG_CPU,tr("ORIA %1").ARG2x(t));

	r = (d | t);
	I_REG_A = r;

	if (r == 0) 
		reg.r.z = 1;
	else
		reg.r.z = 0;

	AddState(4);
}

//----------------------------
// ORIM		(P) V n -> (P),Z 
//----------------------------
INLINE void CSC61860::Op_61(void)
{
	BYTE t,d,r ;

	d = Get_i8(reg.r.p);
    t = pPC->Get_PC(reg.d.pc++);
	AddLog(LOG_CPU,tr("ORIM %1").ARG2x(t));

	r = (d | t);
	Set_i8(reg.r.p,r);

	if (r == 0) 
		reg.r.z = 1;
	else
		reg.r.z = 0;

	AddState(4);
}
//----------------------------
// ORID		(DP) V n -> (DP),Z 
//----------------------------
INLINE void CSC61860::Op_d5(void)
{
	BYTE t,d,r ;

	d = pPC->Get_8(reg.d.dp);
    t = pPC->Get_PC(reg.d.pc++);
	AddLog(LOG_CPU,tr("ORID %1").ARG2x(t));

	r = (d | t);
	pPC->Set_8(reg.d.dp,r);

	if (r == 0) 
		reg.r.z = 1;
	else
		reg.r.z = 0;

	AddState(6);
}

//----------------------------
// ORMA		(P) V A -> (P),Z 
//----------------------------
INLINE void CSC61860::Op_47(void)
{
	BYTE t,d,r ;

	d = Get_i8(reg.r.p);
	t = I_REG_A;
	AddLog(LOG_CPU,"ORMA");

	r = (d | t);
	Set_i8(reg.r.p,r);

	if (r == 0) 
		reg.r.z = 1;
	else
		reg.r.z = 0;

	AddState(3);
}


//----------------------------
// ANIA		A ^ n -> A,Z 
//----------------------------
INLINE void CSC61860::Op_64(void)
{
	BYTE t,d,r ;

	d = I_REG_A;
    t = pPC->Get_PC(reg.d.pc++);
	AddLog(LOG_CPU,tr("ANIA %1").ARG2x(t));

	r = (d & t);
	I_REG_A = r;

	if (r == 0) 
		reg.r.z = 1;
	else
		reg.r.z = 0;

	AddState(4);
}

//----------------------------
// ANIM		(P) ^ n -> (P),Z 
//----------------------------
INLINE void CSC61860::Op_60(void)
{
	BYTE t,d,r ;

	d = Get_i8(reg.r.p);
    t = pPC->Get_PC(reg.d.pc++);
	AddLog(LOG_CPU,tr("ANIM %1").ARG2x(t));

	r = (d & t);
	Set_i8(reg.r.p,r);

	if (r == 0) 
		reg.r.z = 1;
	else
		reg.r.z = 0;

	AddState(4);
}
//----------------------------
// ANID		(DP) ^ n -> (DP),Z 
//----------------------------
INLINE void CSC61860::Op_d4(void)
{
	BYTE t,d,r ;

	d = pPC->Get_8(reg.d.dp);
    t = pPC->Get_PC(reg.d.pc++);
	AddLog(LOG_CPU,tr("ANID %1").ARG2x(t));

	r = (d & t);
	pPC->Set_8(reg.d.dp,r);

	if (r == 0) 
		reg.r.z = 1;
	else
		reg.r.z = 0;

	AddState(6);
}

//----------------------------
// ANMA		(P) ^ A -> (P),Z 
//----------------------------
INLINE void CSC61860::Op_46(void)
{
	BYTE t,d,r ;

	d = Get_i8(reg.r.p);
	t = I_REG_A;
	AddLog(LOG_CPU,"ANMA");

	r = (d & t);
	Set_i8(reg.r.p,r);

	if (r == 0) 
		reg.r.z = 1;
	else
		reg.r.z = 0;

	AddState(3);
}

//----------------------------
// TSIA		A ^ n -> Z 
//----------------------------
INLINE void CSC61860::Op_66(void)
{
	BYTE t,d,r ;

	d = I_REG_A;
    t = pPC->Get_PC(reg.d.pc++);
	AddLog(LOG_CPU,tr("TSIA %1").ARG2x(t));

	r = (d & t);

	if (r == 0) 
		reg.r.z = 1;
	else
		reg.r.z = 0;

	AddState(4);
}
//----------------------------
// TSIM		(P) ^ n -> Z 
//----------------------------
INLINE void CSC61860::Op_62(void)
{
	BYTE t,d,r ;

	d = Get_i8(reg.r.p);
    t = pPC->Get_PC(reg.d.pc++);
	AddLog(LOG_CPU,tr("TSIM %1").ARG2x(t));

	r = (d & t);

	if (r == 0) 
		reg.r.z = 1;
	else
		reg.r.z = 0;

	AddState(4);
}
//----------------------------
// TSID		(DP) ^ n -> Z 
//----------------------------
INLINE void CSC61860::Op_d6(void)
{
	BYTE t,d,r ;

	d = pPC->Get_8(reg.d.dp);
// R-1 used as temporarty storage
	Set_i8( reg.r.r-1,d); 

    t = pPC->Get_PC(reg.d.pc++);
	AddLog(LOG_CPU,tr("TSID %1").ARG2x(t));

	r = (d & t);

	if (r == 0) 
		reg.r.z = 1;
	else
		reg.r.z = 0;

	AddState(6);
}

//----------------------------
// CPIM		(P) - n   C,Z 
//----------------------------
INLINE void CSC61860::Op_63(void)
{
	BYTE t,d ;

	d = Get_i8(reg.r.p);
    t = pPC->Get_PC(reg.d.pc++);
	AddLog(LOG_CPU,tr("CPIM %1").ARG2x(t));

	if (d<t) { reg.r.c=1;reg.r.z=0;}
	else 
		if (d==t){ reg.r.c=0;reg.r.z=1;}
	else
		if (d>t) { reg.r.c=0;reg.r.z=0;}

	AddState(4);
}
//----------------------------
// CPIA		A - n   C,Z
//----------------------------
INLINE void CSC61860::Op_67(void)
{
	BYTE t,d ;

	d = I_REG_A;
    t = pPC->Get_PC(reg.d.pc++);
	AddLog(LOG_CPU,tr("CPIA %1").ARG2x(t));

	if (d<t) { reg.r.c=1;reg.r.z=0;}
	if (d==t){ reg.r.c=0;reg.r.z=1;}
	if (d>t) { reg.r.c=0;reg.r.z=0;}

	AddState(4);
}

//----------------------------
// TEST		n -> TEST 
//----------------------------
INLINE void CSC61860::Op_6b(void)
{
	BYTE	t;
    t = pPC->Get_PC(reg.d.pc++);

	reg.r.z = 0;

	switch (t)
	{
	case 0x01: 
        reg.r.z = 1;
                if (div500)
               {
                   reg.r.z = 0;
                   div500=false;
                   ticks=0;
                }
				break;
	case 0x02: 
        reg.r.z = 1;
                if (div2)
               {
                   reg.r.z = 0;
                   div2=false;
                   ticks2=0;
                }
				break;
	case 0x08: if (t==0x08)
				{
					pPC->pKEYB->CheckKon();
			//		AddLog(0x80,"TEST %02x : Div2=%i  Div500=%i   Kon=%i",t,div2,div500,pPC->pKEYB->Kon);
					if (pPC->pKEYB->Kon)
						 reg.r.z = 0;
					else reg.r.z = 1;
				}
				break;
	case 0x80: 	
				if (Get_Xin()) 
					reg.r.z=0;
				else
					reg.r.z=1;
//				if (fp_log) fprintf(fp_log,"    Z=%c\n",(reg.r.z?'1':'0'));
				break;
	case 0x40: 	
                if (resetFlag) {
                        //resetFlag = false;
                        reg.r.z = 0;
                    }
                else
                    reg.r.z = 1;
				break;
	}

        AddState(4);
}

//----------------------------
// ADIM		(P) + n -> (P)	C,Z 
//----------------------------
INLINE void CSC61860::Op_70(void)
{
	UINT32 t;
	BYTE n;

    n = pPC->Get_PC(reg.d.pc++);

	t = Get_i8(reg.r.p) + n;
	Chk_Flag(t,SIZE_8);
	Set_i8(reg.r.p, (BYTE) t);

	AddLog(LOG_CPU,tr("ADIM %1").ARG2x(n));
	AddState(4);
}

//----------------------------
// SBIM		(P) - n -> (P)	C,Z 
//----------------------------
INLINE void CSC61860::Op_71(void)
{
	UINT32 t;
	BYTE n;

    n = pPC->Get_PC(reg.d.pc++);
	AddLog(LOG_CPU,tr("ADIM %1").ARG2x(n));

	t = Get_i8(reg.r.p) - n;
	Chk_Flag(t,SIZE_8);
	Set_i8(reg.r.p, (BYTE) t);

	AddState(4);
}
//----------------------------
// ADIA		A+n -> A 
//----------------------------
INLINE void CSC61860::Op_74(void)
{
	UINT32	t;
	BYTE d;

    d = pPC->Get_PC(reg.d.pc++);
	AddLog(LOG_CPU,tr("ADIA %1").ARG2x(d));

	t = I_REG_A + d;

	Chk_Flag(t,SIZE_8);
	I_REG_A = (BYTE) t;
	AddState(4);
}
//----------------------------
// SBIA		A-n -> A 
//----------------------------
INLINE void CSC61860::Op_75(void)
{
	UINT32	t;
	BYTE	d;

    d = pPC->Get_PC(reg.d.pc++);
	AddLog(LOG_CPU,tr("SBIA %1").ARG2x(d));
	t = I_REG_A -  d;
	Chk_Flag(t,SIZE_8);
	I_REG_A = (BYTE) t;
	AddState(4);
}

//----------------------------
// CALL nm
//----------------------------
INLINE void CSC61860::Op_78(void)
{
	WORD	t;
    t=pPC->Get_16rPC(reg.d.pc);

	reg.d.pc+=SIZE_16;

	reg.r.r-=SIZE_16;
	Set_i16(reg.r.r, reg.d.pc);
	reg.d.pc=t;
	CallSubLevel++;
	AddLog(LOG_CPU,tr("CALL    %1").ARG4x(t));
	AddState(8);
}
//----------------------------
// JP nm
//----------------------------
INLINE void CSC61860::Op_79(void)
{
	register WORD	t;
    t=pPC->Get_16rPC(reg.d.pc);
	AddLog(LOG_CPU,tr("JP      %1").ARG4x(t));
	reg.d.pc+=SIZE_16;
	reg.d.pc=t;
	AddState(6);
}

//----------------------------
// JPNZ mn
//----------------------------
INLINE void CSC61860::Op_7c(void)
{
	register WORD	t;
    t=pPC->Get_16rPC(reg.d.pc);
	AddLog(LOG_CPU,tr("JPNZ    %1").ARG4x(t));
	reg.d.pc+=SIZE_16;
	if(!reg.r.z)
	{
		reg.d.pc=t;
	}
	AddState(6);
}
//----------------------------
// JPNC mn 
//----------------------------
INLINE void CSC61860::Op_7d(void)
{
	register WORD	t;
    t=pPC->Get_16rPC(reg.d.pc);
	AddLog(LOG_CPU,tr("JPNC    %1").ARG4x(t));
	reg.d.pc+=SIZE_16;
	if(!reg.r.c)
	{
		reg.d.pc=t;
	}
	AddState(6);
}
//----------------------------
// JPZ mn
//----------------------------
INLINE void CSC61860::Op_7e(void)
{
	register WORD	t;
    t=pPC->Get_16rPC(reg.d.pc);
	AddLog(LOG_CPU,tr("JPZ     %1").ARG4x(t));
	reg.d.pc+=SIZE_16;
	if(reg.r.z)
	{
		reg.d.pc=t;
	}
	AddState(6);
}
//----------------------------
// JPC mn 
//----------------------------
INLINE void CSC61860::Op_7f(void)
{
	register WORD	t;
    t=pPC->Get_16rPC(reg.d.pc);
	AddLog(LOG_CPU,tr("JPC     %1").ARG4x(t));
	reg.d.pc+=SIZE_16;
	if(reg.r.c)
	{
		reg.d.pc=t; 
	}
	AddState(6);
}
//--------------------------
//	LP l		l -> P
//--------------------------
INLINE void CSC61860::Op_80(BYTE Op)
{
	reg.r.p = (Op & 0x3f);

	AddLog(LOG_CPU,tr("LP%1").ARG2x(reg.r.p));
	AddState(2);
}
//----------------------------
// SR
//----------------------------
INLINE void CSC61860::Op_d2(void)
{
	BYTE	a,c;
	a=I_REG_A;
	c=reg.r.c;
	reg.r.c=a&1;
	a=(a>>1)|(c<<7);
	Chk_Zero(a,SIZE_8);
	I_REG_A=a;

	AddLog(LOG_CPU,"SR");  
	AddState(2);
}
//----------------------------
// SL
//----------------------------
INLINE void CSC61860::Op_5a(void)
{
	BYTE	a,c;
	a=I_REG_A;
	c=(BYTE) reg.r.c;
	reg.r.c=a>>7;
	a=(a<<1)|c;
	Chk_Zero(a,SIZE_8);
	I_REG_A=a;
	AddLog(LOG_CPU,"SL");  
	AddState(2);
}

//----------------------------
// SC 
//----------------------------
INLINE void CSC61860::Op_d0(void)
{
	reg.r.c=1;
	reg.r.z=1;
	AddLog(LOG_CPU,"SC");
	AddState(2);
}
//----------------------------
// RC
//----------------------------
INLINE void CSC61860::Op_d1(void)
{
	reg.r.c=0;
	reg.r.z=1;
	AddLog(LOG_CPU,"RC");
	AddState(2);
}

//----------------------------
// RZ
//----------------------------
INLINE void CSC61860::Op_77(void)
{
	reg.r.z=0;
	AddLog(LOG_CPU,"RZ");
	AddState(2);
}
//----------------------------
// LOOP n
//----------------------------
INLINE void CSC61860::Op_2f(void)
{
	BYTE	n;
	WORD	t;
    n = pPC->Get_PC(reg.d.pc++);
	t = Get_i8(reg.r.r);
	t--;
	Chk_Flag(t,SIZE_8);
	Set_i8(reg.r.r, (BYTE) t);
    if(!reg.r.c) {
        //reg.r.r++;
		reg.d.pc-=(n+1);
		AddState(3);
	}
	else
	{
        reg.r.r++;		// VERY IMPORTANT UNDOCUMENTED
	}
	AddLog(LOG_CPU,tr("LOOP    %1").ARG2x(n));
	AddState(7);
}

//----------------------------
// LEAVE
//----------------------------
INLINE void CSC61860::Op_d8(void)
{
	Set_i8(reg.r.r,0);

	AddLog(LOG_CPU,"LEAVE");

	AddState(2);
}

//----------------------------
// EXAB		A<->B
//----------------------------
INLINE void CSC61860::Op_da(void)
{
	BYTE	t;

	t = I_REG_B;
	I_REG_B = I_REG_A;
	I_REG_A = t;
	AddLog(LOG_CPU,"EXAB");
	AddState(3);
}
//----------------------------
// EXAM		A<->(P)
//----------------------------
INLINE void CSC61860::Op_db(void)
{
	BYTE t;

	t = I_REG_A;
	I_REG_A = Get_i8(reg.r.p);
	Set_i8(reg.r.p,t);
	AddLog(LOG_CPU,"EXAM");
	AddState(3);
}

//----------------------------
// MVMD		(DP) -> (P)
//----------------------------
INLINE void CSC61860::Op_55(void)
{
	Set_i8(reg.r.p,pPC->Get_8(reg.d.dp));

	AddLog(LOG_CPU,"MVMD");
	AddState(3);
}
//----------------------------
// MVDM		(P) -> (DP)
//----------------------------
INLINE void CSC61860::Op_53(void)
{
	pPC->Set_8(reg.d.dp,Get_i8(reg.r.p));

	AddLog(LOG_CPU,"MVDM");
	AddState(3);
}
//----------------------------
// READM	(PC) -> (P)
//----------------------------
INLINE void CSC61860::Op_54(void)
{
    Set_i8(reg.r.p,pPC->Get_PC(reg.d.pc++));
	AddLog(LOG_CPU,"READM");
	AddState(3);
}
//----------------------------
// READ		(PC) -> A
//----------------------------
INLINE void CSC61860::Op_56(void)
{

    I_REG_A = pPC->Get_PC(reg.d.pc++);
	AddLog(LOG_CPU,"READ");
	AddState(3);
}

//----------------------------
// TSCM		(P) ^ A   Z 
//----------------------------
INLINE void CSC61860::Op_c6(void)
{
	AddLog(LOG_CPU,"TSCM");
	AddState(4);

	reg.r.z = ((Get_i8(reg.r.p) & I_REG_A) == 0);
}
//----------------------------
// CPMA		(P) - A   C,Z 
//----------------------------
INLINE void CSC61860::Op_c7(void)
{
	BYTE t,d ;

	AddState(3);

	d = Get_i8(reg.r.p);
	t = I_REG_A;

	AddLog(LOG_CPU,"CPMA");

	if (d<t) { reg.r.c=1;reg.r.z=0;}
	if (d==t){ reg.r.c=0;reg.r.z=1;}
	if (d>t) { reg.r.c=0;reg.r.z=0;}

}
//----------------------------
// NOPT 
//----------------------------
INLINE void CSC61860::Op_ce(void)
{
	AddLog(LOG_CPU,"NOPT");
	AddState(3);
}


//----------------------------
// OUTC 
//----------------------------
INLINE void CSC61860::Op_df(void)
{
	pPC->Set_Port(PORT_C , Get_i8(IMEM_IC) );
	reg.r.q = IMEM_IC;
    AddState(2);
    pPC->setDisp_on((pPC->Get_Port(PORT_C) & 1));

    if (pPC->Get_Port(PORT_C) & 2)
	{
        ticks = ticks2 = 0;
        div500 = div2 = false;
    }
    power_on = !(pPC->Get_Port(PORT_C) & 8);
    //if ((pPC->Get_Port(PORT_C) & 2)
    start2khz = start4khz = 0;

    //compute_xout();

    if (fp_log) fprintf(fp_log,"check outc %lld %d\n",pPC->pTIMER->state,pPC->Get_Port(PORT_C)>>4);
	AddLog(LOG_CPU,"OUTC");

}
//----------------------------
// OUTA
//----------------------------
INLINE void CSC61860::Op_5d(void)
{
	pPC->Set_Port(PORT_A , Get_i8(IMEM_IA));
	reg.r.q = IMEM_IA;
	AddLog(LOG_CPU,"OUTA");
//	ShowPortsAuto(0);
    AddState(3);
}
//----------------------------
// OUTB 
//----------------------------
INLINE void CSC61860::Op_dd(void)
{
	pPC->Set_Port(PORT_B , Get_i8(IMEM_IB));
	reg.r.q = IMEM_IB;

	AddLog(LOG_CPU,"OUTB");
    AddState(2);
}
//----------------------------
// OUTF
//----------------------------
INLINE void CSC61860::Op_5f(void)
{
//	g_DasmStep=1;
	pPC->Set_Port(PORT_F , Get_i8(IMEM_FO));
	reg.r.q = IMEM_FO;
	
	AddLog(LOG_CPU,"OUTF");
    AddState(3);
}

//----------------------------
// INA
//----------------------------
INLINE void CSC61860::Op_4c(void)
{
	I_REG_A = pPC->Get_Port(PORT_A);
	Chk_Zero(I_REG_A,SIZE_8);
	AddLog(LOG_CPU,"INA");
    AddState(2);
}
//----------------------------
// INB
//----------------------------
INLINE void CSC61860::Op_cc(void)
{

	I_REG_A = pPC->Get_Port(PORT_B);
	Chk_Zero(I_REG_A,SIZE_8);
	AddLog(LOG_CPU,"INB");
    AddState(2);
}


//----------------------------
// CALl n
//----------------------------
INLINE void CSC61860::Op_e0(BYTE Op)
{

	BYTE h,l;
	WORD t;
	h = Op - 0xE0;
    l = pPC->Get_PC(reg.d.pc++);

	t = (h<<8)+l;

	reg.r.r-=SIZE_16;
	Set_i16(reg.r.r, reg.d.pc);
	reg.d.pc = (h<<8)+l;
	CallSubLevel++;
	AddLog(LOG_CPU,tr("CAL %1").ARG4x((h<<8)+l));
	AddState(7);
}


//--------------------------
//	CDN		tmp:=I;Z=1;{if(!Xin){Z=0;break;}}while(tmp--)
//--------------------------
INLINE void CSC61860::Op_6f(void)
{
    if (! cdn_loop_running) {
        cdn_loop_running = true;
		op_local_counter = I_REG_I;
		AddState(1);
	}

	reg.r.z = 1;

    op_local_counter--;
	reg.r.p++;	reg.r.p &=0x7F;

    if (!Get_Xin()) {
		reg.r.z = 0;
        cdn_loop_running = false;
	}
    else {
        if (op_local_counter )	{	reg.d.pc--;	}
    }
    AddState(4);
}

//--------------------------
//	CUP		tmp:=I;Z:=1;{if(Xin){Z:=0;break;}}while(tmp--)
// Transform to a reentrent function with static loop pointer
//--------------------------
INLINE void CSC61860::Op_4f(void)
{	
    if (! cup_loop_running) {
        cup_loop_running = true;
		op_local_counter = I_REG_I;
		AddState(1);
	}

	reg.r.z = 1;
    op_local_counter--;
    reg.r.p++;	reg.r.p &=0x7F;
    if (Get_Xin()) {
		reg.r.z = 0;
        cup_loop_running = false;
	}
    else {
        if (op_local_counter)	{	reg.d.pc--;	}
    }
    AddState(4);
}

//--------------------------
//	CLRA
//--------------------------
INLINE void CSC61860::Op_23(void)
{
	I_REG_A = 0;
	AddState(2);
	AddLog(LOG_CPU,"CLRA");
}


//----------------------------
// CASE 1 lnm
//----------------------------
INLINE void CSC61860::Op_7a(void)
{
	BYTE ind,d,op,r,n,m;
    //WORD	t;

    d=pPC->Get_PC(reg.d.pc++);		// number of brances

    n=pPC->Get_PC(reg.d.pc++);		// Return address Hi
    Set_i8(--reg.r.r, n);			// Push return address
    m=pPC->Get_PC(reg.d.pc++);		// Return address Lo
    Set_i8(--reg.r.r, m);			// Push return address

    CallSubLevel++;
    AddState(8);

    op=pPC->Get_PC(reg.d.pc++);
	if (op != 0x69)
	{
		AddLog(LOG_CPU,"ERREUR !!! CASE1 Without CASE2");
        emit msgError("ERREUR SC61860 !!! CASE1 Without CASE2");
	}

	ind = 0;
	AddState(5);
	while(ind < d)
	{
        r = pPC->Get_PC(reg.d.pc++);	// when A=r
        n = pPC->Get_PC(reg.d.pc++);	// Subroutine Jump address Hi
        m = pPC->Get_PC(reg.d.pc++);	// Subroutine Jump address Lo

//        reg.r.z = (I_REG_A == r);
//        if (reg.r.z)
        if (I_REG_A == r)
		{
			AddLog(LOG_CPU,tr(" JP CASE2 %1").ARG4x((n<<8)+m));
			reg.d.pc = (n<<8) + m;
			AddState(7);
			break;
		}
		ind++;
	}
 
	if (ind == d)	// Default jump
	{
        n = pPC->Get_PC(reg.d.pc++);
        m = pPC->Get_PC(reg.d.pc++);
		reg.d.pc = (n<<8) + m;
	}

	AddLog(LOG_CPU,"CASE 1 CASE 2");
}



INLINE void CSC61860::OpExec(BYTE Op)
{
    //pPC->pTIMER->state+=(1);
//    AddState(1);
//    return;

#if 0
	if (g_DasmStep)
	{
		pDEBUG->DisAsm_1(reg.d.pc-1);
		RefreshDasm();
	}
#endif
//debug.DisAsm_1(reg.d.pc-1);
//	AddLog(0x01,"PC=[%04X]",reg.d.pc-1 );

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
//	case 0x16 : Op_16();	break;
//	case 0x17 : Op_17();	break;
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
//	case 0x2e : Op_2e();	break;
	case 0x2f : Op_2f();	break;

	case 0x30 : Op_30();	break;
	case 0x31 : Op_31();	break;
	case 0x32 : Op_32();	break;
//	case 0x33 : Op_33();	break;
	case 0x34 : Op_34();	break;
	case 0x35 : Op_35();	break;
//	case 0x36 : Op_36();	break;
	case 0x37 : Op_37();	break;
	case 0x38 : Op_38();	break;
	case 0x39 : Op_39();	break;
	case 0x3a : Op_3a();	break;
	case 0x3b : Op_3b();	break;

	case 0x40 : Op_40(0);	break;
	case 0x41 : Op_41(0);	break;
	case 0x42 : Op_40(2);	break;
	case 0x43 : Op_41(2);	break;
	case 0x44 : Op_44();	break;
	case 0x45 : Op_45();	break;
	case 0x46 : Op_46();	break;
	case 0x47 : Op_47();	break;
	case 0x48 : Op_40(4);	break;
	case 0x49 : Op_41(4);	break;
	case 0x4a : Op_40(6);	break;
	case 0x4b : Op_41(6);	break;
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
//	case 0x5c : Op_5c();	break;
	case 0x5d : Op_5d();	break;
//	case 0x5e : Op_5e();	break;
	case 0x5f : Op_5f();	break;

	case 0x60 : Op_60();	break;
	case 0x61 : Op_61();	break;
	case 0x62 : Op_62();	break;
	case 0x63 : Op_63();	break;
	case 0x64 : Op_64();	break;
	case 0x65 : Op_65();	break;
	case 0x66 : Op_66();	break;
	case 0x67 : Op_67();	break;
//	case 0x68 : Op_68();	break;
//	case 0x69 : Op_69();	break;	With 7a
//	case 0x6a : Op_6a();	break;
	case 0x6b : Op_6b();	break;
//	case 0x6c : Op_6c();	break;
//	case 0x6d : Op_6d();	break;
//	case 0x6e : Op_6e();	break;
	case 0x6f : Op_6f();	break;

	case 0x70 : Op_70();	break;
	case 0x71 : Op_71();	break;
	case 0x72 : Op_77();	break;
	case 0x73 : Op_77();	break;
	case 0x74 : Op_74();	break;
	case 0x75 : Op_75();	break;
	case 0x76 : Op_77();	break;
	case 0x77 : Op_77();	break;
	case 0x78 : Op_78();	break;
	case 0x79 : Op_79();	break;
	case 0x7a : Op_7a();	break;
//	case 0x7b : Op_7b();	break;
	case 0x7c : Op_7c();	break;
	case 0x7d : Op_7d();	break;
	case 0x7e : Op_7e();	break;
	case 0x7f : Op_7f();	break;

	case 0x80 : Op_80(Op);	break;
	case 0x81 : Op_80(Op);	break;
	case 0x82 : Op_80(Op);	break;
	case 0x83 : Op_80(Op);	break;
	case 0x84 : Op_80(Op);	break;
	case 0x85 : Op_80(Op);	break;
	case 0x86 : Op_80(Op);	break;
	case 0x87 : Op_80(Op);	break;
	case 0x88 : Op_80(Op);	break;
	case 0x89 : Op_80(Op);	break;
	case 0x8a : Op_80(Op);	break;
	case 0x8b : Op_80(Op);	break;
	case 0x8c : Op_80(Op);	break;
	case 0x8d : Op_80(Op);	break;
	case 0x8e : Op_80(Op);	break;
	case 0x8f : Op_80(Op);	break;

	case 0x90 : Op_80(Op);	break;
	case 0x91 : Op_80(Op);	break;
	case 0x92 : Op_80(Op);	break;
	case 0x93 : Op_80(Op);	break;
	case 0x94 : Op_80(Op);	break;
	case 0x95 : Op_80(Op);	break;
	case 0x96 : Op_80(Op);	break;
	case 0x97 : Op_80(Op);	break;
	case 0x98 : Op_80(Op);	break;
	case 0x99 : Op_80(Op);	break;
	case 0x9a : Op_80(Op);	break;
	case 0x9b : Op_80(Op);	break;
	case 0x9c : Op_80(Op);	break;
	case 0x9d : Op_80(Op);	break;
	case 0x9e : Op_80(Op);	break;
	case 0x9f : Op_80(Op);	break;

	case 0xa0 : Op_80(Op);	break;
	case 0xa1 : Op_80(Op);	break;
	case 0xa2 : Op_80(Op);	break;
	case 0xa3 : Op_80(Op);	break;
	case 0xa4 : Op_80(Op);	break;
	case 0xa5 : Op_80(Op);	break;
	case 0xa6 : Op_80(Op);	break;
	case 0xa7 : Op_80(Op);	break;
	case 0xa8 : Op_80(Op);	break;
	case 0xa9 : Op_80(Op);	break;
	case 0xaa : Op_80(Op);	break;
	case 0xab : Op_80(Op);	break;
	case 0xac : Op_80(Op);	break;
	case 0xad : Op_80(Op);	break;
	case 0xae : Op_80(Op);	break;
	case 0xaf : Op_80(Op);	break;

	case 0xb0 : Op_80(Op);	break;
	case 0xb1 : Op_80(Op);	break;
	case 0xb2 : Op_80(Op);	break;
	case 0xb3 : Op_80(Op);	break;
	case 0xb4 : Op_80(Op);	break;
	case 0xb5 : Op_80(Op);	break;
	case 0xb6 : Op_80(Op);	break;
	case 0xb7 : Op_80(Op);	break;
	case 0xb8 : Op_80(Op);	break;
	case 0xb9 : Op_80(Op);	break;
	case 0xba : Op_80(Op);	break;
	case 0xbb : Op_80(Op);	break;
	case 0xbc : Op_80(Op);	break;
	case 0xbd : Op_80(Op);	break;
	case 0xbe : Op_80(Op);	break;
	case 0xbf : Op_80(Op);	break;

	case 0xc0 : Op_40(1);	break;
	case 0xc1 : Op_41(1);	break;
	case 0xc2 : Op_40(3);	break;
	case 0xc3 : Op_41(3);	break;
	case 0xc4 : Op_c4();	break;
	case 0xc5 : Op_c5();	break;
	case 0xc6 : Op_c6();	break;
	case 0xc7 : Op_c7();	break;
	case 0xc8 : Op_40(5);	break;
	case 0xc9 : Op_41(5);	break;
	case 0xca : Op_40(7);	break;
	case 0xcb : Op_41(7);	break;
	case 0xcc : Op_cc();	break;
//	case 0xcd : Op_cd();	break;
	case 0xce : Op_ce();	break;
//	case 0xcf : Op_cf();	break;

	case 0xd0 : Op_d0();	break;
	case 0xd1 : Op_d1();	break;
	case 0xd2 : Op_d2();	break;
	case 0xd4 : Op_d4();	break;
	case 0xd5 : Op_d5();	break;
	case 0xd6 : Op_d6();	break;
//	case 0xd7 : Op_d7();	break;	SZ
	case 0xd8 : Op_d8();	break;
//	case 0xd9 : Op_d9();	break;
	case 0xda : Op_da();	break;
	case 0xdb : Op_db();	break;
//	case 0xdc : Op_dc();	break;
	case 0xdd : Op_dd();	break;
//	case 0xde : Op_de();	break;
	case 0xdf : Op_df();	break;

	case 0xe0 : Op_e0(Op);	break;
	case 0xe1 : Op_e0(Op);	break;
	case 0xe2 : Op_e0(Op);	break;
	case 0xe3 : Op_e0(Op);	break;
	case 0xe4 : Op_e0(Op);	break;
	case 0xe5 : Op_e0(Op);	break;
	case 0xe6 : Op_e0(Op);	break;
	case 0xe7 : Op_e0(Op);	break;
	case 0xe8 : Op_e0(Op);	break;
	case 0xe9 : Op_e0(Op);	break;
	case 0xea : Op_e0(Op);	break;
	case 0xeb : Op_e0(Op);	break;
	case 0xec : Op_e0(Op);	break;
	case 0xed : Op_e0(Op);	break;
	case 0xee : Op_e0(Op);	break;
	case 0xef : Op_e0(Op);	break;
	case 0xf0 : Op_e0(Op);	break;
	case 0xf1 : Op_e0(Op);	break;
	case 0xf2 : Op_e0(Op);	break;
	case 0xf3 : Op_e0(Op);	break;
	case 0xf4 : Op_e0(Op);	break;
	case 0xf5 : Op_e0(Op);	break;
	case 0xf6 : Op_e0(Op);	break;
	case 0xf7 : Op_e0(Op);	break;
	case 0xf8 : Op_e0(Op);	break;
	case 0xf9 : Op_e0(Op);	break;
	case 0xfa : Op_e0(Op);	break;
	case 0xfb : Op_e0(Op);	break;
	case 0xfc : Op_e0(Op);	break;
	case 0xfd : Op_e0(Op);	break;
	case 0xfe : Op_e0(Op);	break;
	case 0xff : Op_e0(Op);	break;
	
	
	default : 	DASMLOG = 1;
                if (fp_log) fprintf(fp_log,"PC=[%04X]='%02X' : NOT YET EMULATED",reg.d.pc-1,Op);
//				AddLog(0x04,"PC=[%04X]='%02X' : NOT YET EMULATED",reg.d.pc-1,Op); 
		break;
	}
#if 0
	if (g_BreakPointAdr == (reg.d.pc))
	{
		pDEBUG->DisAsm_1(reg.d.pc);
		ListBox_ResetContent(g_hWndListDasm);
		g_DasmFlag = 1;
		RefreshDasm();
		halt = 1;
	}

	if (g_BreakSubLevel == CallSubLevel)
	{
		g_BreakSubLevel = -1;
		pDEBUG->DisAsm_1(reg.d.pc);
		ListBox_ResetContent(g_hWndListDasm);
		g_DasmFlag = 1;
		RefreshDasm();
		halt = 1;
	}
#endif

}


/*---------------------------------------------------------------------------*/
/*****************************************************************************/
/* Initialize sc61860 CPU emulator (memory, register)						 */
/* RETURN: 0=error, 1=success												 */
/*****************************************************************************/
												

bool CSC61860::init(void)
{
    Check_Log();
	AddLog(0x01,"MEMORY initializing...");

	ticks = ticks2 = div500 = div2 = 0;
    Xin = Xout = false;

    power_on = 1;
 
	AddLog(0x01,"done.\n");
 
    Reset();

    //if(logsw) fp_log=fopen("sc61860.log","wt");			//open log file
#ifndef EMSCRIPTEN
	pDEBUG->init();
#endif

	return(1);
}

void CSC61860::Reset(void)
{
    memset(&reg,0,sizeof(reg));
    resetFlag = true;
    ticksReset = 0;
	memset(imem,0,MAX_IMEM);
	I_REG_A  = 0;
	I_REG_B  = 0;
	I_REG_I  = 0;
	I_REG_Xl = 0; I_REG_Xh = 0;
	I_REG_Yl = 0; I_REG_Yh = 0;
	reg.d.pc= 0;
}

/************************************************/
/* Load memory and register of the sc61860 CPU	*/
/************************************************/
void CSC61860::Load_Internal(QFile *file)
{
	QDataStream in(file);	

	char t[16];
	in.readRawData( t,HEAD_LEN);	// Write Header
	in.readRawData( (char*) &reg,REG_LEN);	// Write Header
	in.readRawData( (char *)imem,IMEM_LEN);	// Write Header
}	
/************************************************/
/* Save memory and register of the sc61860 CPU	*/
/************************************************/
void CSC61860::save_internal(QFile *file)
{
	QDataStream out(file);	
	
	out.writeRawData(HEAD_STR, HEAD_LEN);	//header
	out.writeRawData((char*)&reg,REG_LEN);			//reg
	out.writeRawData((char*)imem,IMEM_LEN);			//i-mem
}

/************************************************/
/* Load memory and register of the sc61860 CPU	*/
/************************************************/
void CSC61860::Load_Internal(QXmlStreamReader *xmlIn)
{
    if (xmlIn->readNextStartElement()) {
        if ( (xmlIn->name()=="cpu") &&
             (xmlIn->attributes().value("model").toString() == "sc61860")) {
            QByteArray ba_reg = QByteArray::fromBase64(xmlIn->attributes().value("registers").toString().toLatin1());
            memcpy((char *) &reg,ba_reg.data(),REG_LEN);
            QByteArray ba_imem = QByteArray::fromBase64(xmlIn->attributes().value("iMem").toString().toLatin1());
            memcpy((char *) &imem,ba_imem.data(),IMEM_LEN);
        }
        xmlIn->skipCurrentElement();
    }
}

/************************************************/
/* Save memory and register of the sc61860 CPU	*/
/************************************************/
void CSC61860::save_internal(QXmlStreamWriter *xmlOut)
{
    xmlOut->writeStartElement("cpu");
        xmlOut->writeAttribute("model","sc61860");
        QByteArray ba_reg((char*)&reg,REG_LEN);
        xmlOut->writeAttribute("registers",ba_reg.toBase64());
        QByteArray ba_imem((char*)imem,IMEM_LEN);
        xmlOut->writeAttribute("iMem",ba_imem.toBase64());
    xmlOut->writeEndElement();
}

/*****************************************************************************/
/* Exitting sc61860 CPU emulator (save memory, register)					 */
/*****************************************************************************/
bool CSC61860::exit(void)
{
    if(fp_log) fclose(fp_log);							//close log file

	pDEBUG->exit();

	return true;
}

/*****************************************************************************/
/* execute one operation code												 */
/*****************************************************************************/
void CSC61860::step(void)
{
	UINT32 t;

	if(logsw)
	{
        if(cpulog && fp_log)						//check log mode
		{
//			debug.DisAsm_1(reg.d.pc,fp_log);		//write execute log
		}
	}
	t=reg.d.pc;
	reg.d.pc++;
//	fprintf(fp_tmp,"%2X - ",pPC->Get_8(t));
    OpExec(pPC->Get_PC(t));
//    backgroundTasks();

}

INLINE void CSC61860::backgroundTasks(void) {
    compute_xout();
    pPC->fillSoundBuffer((Get_Xout()?0xff:0x00));
}

/*---------------------------------------------------------------------------*/
/*****************************************************************************/
/* Get data from register													 */
/*  ENTRY :REGNAME regname=REG_xx											 */
/*  RETURN:DWORD value														 */
/*****************************************************************************/
UINT32 CSC61860::get_reg(REGNAME regname)
{
	switch(regname)
	{
	case REG_A :return(I_REG_A);	break;
	case REG_B :return(I_REG_B);	break;
	case REG_BA:return(I_REG_A|(I_REG_B<<8));	break;
	case REG_I :return(I_REG_I);	break;
	case REG_J :return(I_REG_J);	break;
	case REG_X :return(I_REG_Xl|(I_REG_Xh<<8));	break;
	case REG_Y :return(I_REG_Yl|(I_REG_Yh<<8));	break;
	case REG_PC:return(reg.d.pc);	break;
	case REG_DP:return(reg.d.dp);	break;
	case REG_C :return(reg.r.c);	break;
	case REG_Z :return(reg.r.z);	break;
    case REG_P :return(reg.r.p);    break;
    case REG_Q :return(reg.r.q);    break;
    case REG_R :return(reg.r.r);    break;
	}
	return(0);
}


/*!
 \brief Set data to register

 \fn Csc::set_reg
 \param regname REG_xx
 \param data    value
*/
void CSC61860::set_reg(REGNAME regname,UINT32 data)
{
	switch(regname)
	{
	case REG_A :I_REG_A = (BYTE) data; break;
	case REG_B :I_REG_B = (BYTE) data; break;
	case REG_BA:I_REG_A = (BYTE) data;
				I_REG_B = (BYTE) (data>>8); break;
	case REG_I :I_REG_I = (BYTE) data; break;
	case REG_J :I_REG_J = (BYTE) data; break;
	case REG_X :I_REG_Xl= (BYTE) data;
				I_REG_Xh= (BYTE) (data>>8); break;
	case REG_Y :I_REG_Yl= (BYTE) data;
				I_REG_Yh= (BYTE) (data>>8); break;
	case REG_PC:reg.d.pc= (WORD) data; break;	
	case REG_DP:reg.d.dp= (WORD) data; break;	
	case REG_C : reg.r.c= (BYTE) data; break;
	case REG_Z : reg.r.z= (BYTE) data; break;
    case REG_P : reg.r.p= (BYTE) data; break;
    case REG_Q : reg.r.q= (BYTE) data; break;
    case REG_R : reg.r.r= (BYTE) data; break;

	}
}

/*!
 \brief Get data from memory

 \fn Csc::get_mem
 \param adr     address
 \param size    SIZE_08 or SIZE_16 or SIZE_20 or SIZE_24
 \return DWORD  value
*/
UINT32 CSC61860::get_mem(UINT32 adr,int size)
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
void CSC61860::set_mem(UINT32 adr,int size,UINT32 data)
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



void CSC61860::Regs_Info(UINT8 Type)
{
	switch(Type)
	{
    case 0:			// Monitor Registers Dialog
	case 2:			// For Log File
		sprintf(Regs_String,	"I:%.2x J:%.2x A:%.2x B:%.2x \r\nX:%.2x%.2x  Y:%.2x%.2x \r\nK:%.2x L:%.2x M:%.2x N:%.2x \r\nP:%.2x Q:%.2x R:%.2x DP:%.4x \r\nC:%s Z:%s",
			I_REG_I,I_REG_J,I_REG_A,I_REG_B,
			I_REG_Xh,I_REG_Xl,I_REG_Yh,I_REG_Yl,
			I_REG_K,I_REG_L,I_REG_M,I_REG_N,
			reg.r.p,reg.r.q,reg.r.r,reg.d.dp,
			reg.r.c?"1":".",reg.r.z?"1":".");
		break;
	case 1:			// For Log File
		sprintf(Regs_String,	"I:%.2x J:%.2x A:%.2x B:%.2x X:%.2x%.2x  Y:%.2x%.2x K:%.2x L:%.2x M:%.2x N:%.2x P:%.2x Q:%.2x R:%.2x DP:%.4x C:%s Z:%s",
			I_REG_I,I_REG_J,I_REG_A,I_REG_B,
			I_REG_Xh,I_REG_Xl,I_REG_Yh,I_REG_Yl,
			I_REG_K,I_REG_L,I_REG_M,I_REG_N,
			reg.r.p,reg.r.q,reg.r.r,reg.d.dp,
			reg.r.c?"1":".",reg.r.z?"1":".");

		break;
	}

}

