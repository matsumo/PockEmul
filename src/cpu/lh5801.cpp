/**********************************************************/
/* LH5801 CPU emulation                                  */
/**********************************************************/
#include <stdlib.h>
#include <QDebug>

#include "common.h"
#include "pcxxxx.h"
#include "lh5801.h"
#include "lh5801d.h"
#include "Keyb.h"
#include "Connect.h"
#include "Inter.h"

#include "Debug.h"
#include "Log.h"
#include "ui/cregslh5801widget.h"
#include "breakpoint.h"

#define IMEM_LEN    0x200

#define P	(lh5801.p.w)
#define S	(lh5801.s.w)
#define U	(lh5801.u.w)
#define UL	(lh5801.u.b.l)
#define UH	(lh5801.u.b.h)
#define X	(lh5801.x.w)
#define XL	(lh5801.x.b.l)
#define XH	(lh5801.x.b.h)
#define Y	(lh5801.y.w)
#define YL	(lh5801.y.b.l)
#define YH	(lh5801.y.b.h)
#define T	(lh5801.t)

#define C	(0x01)
#define IE	(0x02)
#define Z	(0x04)
#define V	(0x08)
#define H	(0x10)

#define F_C		(lh5801.t & C)
#define F_IE	(lh5801.t & IE)
#define F_Z		(lh5801.t & Z)
#define F_V		(lh5801.t & V)
#define F_H		(lh5801.t & H)

#define SET_C	(lh5801.t |= C)
#define SET_IE	(lh5801.t |= IE)
#define SET_Z	(lh5801.t |= Z)
#define SET_V	(lh5801.t |= V)
#define SET_H	(lh5801.t |= H)

#define UNSET_C		(lh5801.t &= ~C)
#define UNSET_IE	(lh5801.t &= ~IE)
#define UNSET_Z		(lh5801.t &= ~Z)
#define UNSET_V		(lh5801.t &= ~V)
#define UNSET_H		(lh5801.t &= ~H)

#define CHECK_Z(a)	{ ( !(a)? SET_Z : UNSET_Z);	}
#define CHECK_C(a)	{ ( (a) ? SET_C : UNSET_C);	}
#define CHECK_H(a)	{ ( (a) ? SET_H : UNSET_H);	}
#define CHECK_V(a)	{ ( (a) ? SET_V : UNSET_V);	}

#define ME1(a)		((a)|0x10000)
#define bool(b)		((b)?1:0)

long Oper_Use[1][0xff];

INLINE void Log_Oper(int set,int oper)
{
	oper=set=0;
//	Oper_Use[set][oper]++;
}

CLH5801::CLH5801(CPObject *parent)	: CCPU(parent)
{
    pDEBUG = new Cdebug_lh5801(this);
    fn_log = "lh5801.log";

    Is_Timer_Reached=false;
    step_Previous_State = 0;

    regwidget = (CregCPU*) new Cregslh5801Widget(0,this);
}

CLH5801::~CLH5801() {

}

bool CLH5801::init(void)
{
	Check_Log();
	pDEBUG->init();
	Reset();
	return(1);
}						//initialize

bool	CLH5801::exit(void)
{
    if(fp_log) fclose(fp_log);							//close log file
	pDEBUG->exit();
	return true;
}						//end

void CLH5801::Set_C(void)
{
	SET_C;
}
void CLH5801::UnSet_C(void)
{
	UNSET_C;
}

INLINE void CLH5801::TIMER_INC(void)
{
	// Shift right , b9=(b0 xor b4)
	lh5801.tm = (lh5801.tm >> 1) | (( (lh5801.tm & 0x01) ^ ((lh5801.tm & 0x10)>>4) )<<8 );
	
    Is_Timer_Reached=(lh5801.tm == 0x1FF ? true : false);
}

void CLH5801::step(void)
{

    quint64	Current_State;

    if (resetFlag) internalReset();

    if (Is_Timer_Reached) { lh5801.IR1=1; Is_Timer_Reached = false; }

	if (lh5801.IR0)
	{
		// Non-maskable Interrupt processing
		// NOT USED - Connected to Ground
	}
	else
	if ( (lh5801.IR1) && F_IE )
	{
		// Timer Interrupt Routine
		PUSH(lh5801.t);
		UNSET_IE;
		lh5801.IR1=0;
		PUSH_WORD(P);
		P = (UINT16) get_mem(0xFFFA,SIZE_16);
        CallSubLevel++;

	}
	else
	if ( (lh5801.IR2) && F_IE )
	{

		// Maskable Interrupt processing
		PUSH(lh5801.t);
		UNSET_IE;
        lh5801.HLT = false;
		lh5801.IR2=0;
		PUSH_WORD(P);
		P = (UINT16) get_mem(0xFFF8,SIZE_16);
        CallSubLevel++;

	}
	else
	if (lh5801.HLT)
	{
		// Do nothing
        AddState(2);
	}
	else
	{
		instruction();		
	}

#define TIMER_FREQUENCY 31250
#define NB_STATE_PER_TIMER	42

	// INCREMENT TIMER
	Current_State = pPC->pTIMER->state;

	if ((Current_State - step_Previous_State) >= 42)
	{
		TIMER_INC();
		step_Previous_State += (Current_State - step_Previous_State);
	}

}

void CLH5801::Load_Internal(QFile *file)
{
	char t[16];
	QDataStream in(file);	
	
	in.readRawData(t, 9);
	in.readRawData((char*)&lh5801,sizeof(lh5801));
    in.readRawData( (char *)imem,IMEM_LEN);	// Write Header
}													

void CLH5801::save_internal(QFile *file)
{
	QDataStream out(file);	

	out.writeRawData("LH5801STA", 9);					//header
	out.writeRawData((char*)&lh5801,sizeof(lh5801));			//reg
    out.writeRawData((char*)imem,IMEM_LEN);			//i-mem
}

void CLH5801::Load_Internal(QXmlStreamReader *xmlIn)
{
    if (xmlIn->readNextStartElement()) {
        if ( (xmlIn->name()=="cpu") &&
             (xmlIn->attributes().value("model").toString() == "lh5801")) {
            QByteArray ba_reg = QByteArray::fromBase64(xmlIn->attributes().value("registers").toString().toLatin1());
            memcpy((char *) &lh5801,ba_reg.data(),sizeof(lh5801));
            QByteArray ba_imem = QByteArray::fromBase64(xmlIn->attributes().value("iMem").toString().toLatin1());
            memcpy((char *) &imem,ba_imem.data(),IMEM_LEN);
        }
        xmlIn->skipCurrentElement();
    }
}

void CLH5801::save_internal(QXmlStreamWriter *xmlOut)
{
    xmlOut->writeStartElement("cpu");
        xmlOut->writeAttribute("model","lh5801");
        QByteArray ba_reg((char*)&lh5801,sizeof(lh5801));
        xmlOut->writeAttribute("registers",ba_reg.toBase64());
        QByteArray ba_imem((char*)imem,IMEM_LEN);
        xmlOut->writeAttribute("iMem",ba_imem.toBase64());
    xmlOut->writeEndElement();
}

UINT32	CLH5801::get_mem(UINT32 adr,int size)
{
	switch(size)
	{
    case 8:
	case SIZE_8 :return( cpu_readmem(adr));
    case 16:
    case SIZE_16:return( cpu_readmem(adr+1)+(cpu_readmem(adr)<<8));
    case 20:
    case SIZE_20:return((cpu_readmem(adr+2)+(cpu_readmem(adr+1)<<8)+(cpu_readmem(adr)<<16))&MASK_20);
    case 24:
    case SIZE_24:return((cpu_readmem(adr+2)+(cpu_readmem(adr+1)<<8)+(cpu_readmem(adr)<<16))&MASK_24);
	}
	return(0);
}

void	CLH5801::set_mem(UINT32 adr,int size,UINT32 data)
{
	switch(size)
	{
    case 8:
    case SIZE_8 :
		cpu_writemem( adr, (BYTE) data);
		break;
	//case SIZE_16:
		//pPC->mem[adr]   = (BYTE) (data>>8);
		//pPC->mem[adr+1] = (BYTE) data;
		//break;
	//case SIZE_20:
		//pPC->mem[adr]   = (BYTE) data;
		//pPC->mem[adr+1] = (BYTE) (data>>8);
		//pPC->mem[adr+2] = (BYTE) ((data>>16)&MASK_4);
		//break;
	//case SIZE_24:
		//pPC->mem[adr]   = (BYTE) data;
		//pPC->mem[adr+1] = (BYTE) (data>>8);
		//pPC->mem[adr+2] = (BYTE) (data>>16);
		//break;
	}
}

bool	CLH5801::Get_Xin(void){return(0);}
void	CLH5801::Set_Xin(bool){}
bool	CLH5801::Get_Xout(void){return(0);}
void	CLH5801::Set_Xout(bool){}

UINT32	CLH5801::get_PC(void){return(P);}				//get Program Counter

INLINE UINT8 CLH5801::cpu_readmem(UINT32 adr)
{
    return (pPC->Get_8(adr));
}

INLINE void CLH5801::cpu_writemem(UINT32 addr, UINT8 data)
{
    pPC->Set_8(addr,data);
}

INLINE void CLH5801::change_pc(UINT16 addr)
{
	addr=0;
}


void CLH5801::Reset(void)
{
    resetFlag = true;
}

void CLH5801::internalReset(void)
{
    resetFlag = true;
    memset(imem,0,imemsize);
    P	= (UINT16) get_mem(0xFFFE,SIZE_16);
    lh5801.HLT=lh5801.IR0=lh5801.IR1=lh5801.IR2=0;
    S	= 0;
    U	= 0;
    UL	= 0;
    UH	= 0;
    X	= 0;
    XL	= 0;
    XH	= 0;
    Y	= 0;
    YL	= 0;
    YH	= 0;
    lh5801.tm=0; //9 bit
    lh5801.t=lh5801.a=lh5801.dp=lh5801.pu=lh5801.pv=0;
    lh5801.bf=1;
    CallSubLevel = 0;

    resetFlag = false;
}

INLINE void CLH5801::AddState(UINT8 n)
{
	pPC->pTIMER->state+=(n);
    ticks+=(n);
}

INLINE UINT8 CLH5801::cpu_readop(UINT32 adr)
{
    return (pPC->Get_8(adr));
}

INLINE UINT16 CLH5801::readop_word(void)
{
	return (UINT16) ((cpu_readop(P++) << 8) | cpu_readop(P++));
}

INLINE UINT8 CLH5801::add_generic(int left, int right, int carry)
{
	int res = left + right + carry;
	int v,c;

	lh5801.t&=~(H|V|Z|C);

	CHECK_Z(res & 0xff);

	c = res & 0x100;
	CHECK_C(c);

	if (((left & 0x0f)+(right & 0x0f) + carry) & 0x10) SET_H;
	v = ((left & 0x7f)+(right & 0x7f) + carry) & 0x80;
	if ( (c && !v)||(!c && v) ) SET_V;

	return (UINT8) (res);
}


INLINE void CLH5801::ADC(UINT8 data)
{
	lh5801.a = add_generic(lh5801.a,data,bool(F_C));
}

INLINE void CLH5801::ADD_MEM(UINT32 addr, UINT8 data)
{
	UINT8 v = add_generic(cpu_readmem(addr),data,0);
	cpu_writemem(addr,v);
}

INLINE void CLH5801::ADR(PAIR *reg)
{
	UINT8 loc_t = lh5801.t;		// Record Flags

	reg->b.l = add_generic(reg->b.l,lh5801.a,0);
	if (F_C) {
		reg->b.h++;
	}
	lh5801.t = loc_t;		// Restore Flags : OFFICIAL DOCUMENTATION IS WRONG Flags are not impacted 
}

INLINE void CLH5801::SBC(UINT8 data)
{
	lh5801.a = add_generic(lh5801.a,data ^ 0xff,bool(F_C));
}

INLINE void CLH5801::CPA(UINT8 a, UINT8 b)
{
	add_generic(a, b ^ 0xff, 1);
}

INLINE UINT8 CLH5801::decimaladd_generic(int left, int right, int carry)
{

	UINT8 a = add_generic(left,right,carry);
	if (!F_C && !F_H) a += 0x9a;
	else 
	if (!F_C &&  F_H) a += 0xa0;
	else
	if ( F_C && !F_H) a += 0xfa;

	return(a);
}

INLINE void CLH5801::DCA(UINT8 data)
{
	lh5801.a = decimaladd_generic(lh5801.a + 0x66, data, bool(F_C));
}	

INLINE void CLH5801::DCS(UINT8 data)
{
	lh5801.a = decimaladd_generic(lh5801.a, data^0xff, bool(F_C));
}

INLINE void CLH5801::AND(UINT8 data)
{
	lh5801.a &= data;
	CHECK_Z(lh5801.a);
}

INLINE void CLH5801::AND_MEM(UINT32 addr, UINT8 data)
{
	data &= cpu_readmem(addr);
	CHECK_Z(data);
	cpu_writemem(addr,data);	
}

INLINE void CLH5801::BIT(UINT8 a, UINT8 b)
{
	CHECK_Z(a & b);
}

INLINE void CLH5801::EOR(UINT8 data)		// EXCLUSIVE OR
{
	lh5801.a ^= data;
	CHECK_Z(lh5801.a);
}

INLINE void CLH5801::ORA(UINT8 data)
{
	lh5801.a |= data;
	CHECK_Z(lh5801.a);
}

INLINE void CLH5801::ORA_MEM(UINT32 addr, UINT8 data)
{
	data |= cpu_readmem(addr);
	CHECK_Z(data);
	cpu_writemem(addr,data);	
}

INLINE void CLH5801::LDA(UINT8 data)
{
	lh5801.a = data;
	CHECK_Z(data);
}

INLINE void CLH5801::LDE(PAIR *reg)
{
	// or z flag depends on reg
	lh5801.a = cpu_readmem(reg->w--);
	CHECK_Z(lh5801.a);
}

INLINE void CLH5801::SDE(PAIR *reg)
{
	cpu_writemem(reg->w--, lh5801.a);
}

INLINE void CLH5801::LIN(PAIR *reg)
{
	// or z flag depends on reg
	lh5801.a = cpu_readmem(reg->w++);
	CHECK_Z(lh5801.a);
}

INLINE void CLH5801::SIN(PAIR *reg)
{
	cpu_writemem(reg->w++, lh5801.a);
}

INLINE void CLH5801::DEC(UINT8 *adr)
{
	*adr=add_generic(*adr,0xff,0);
}

INLINE void CLH5801::INC(UINT8 *adr)
{
	*adr=add_generic(*adr,1,0);
}

void CLH5801::RTN(void)
{
	P =  cpu_readmem(++S)<<8;
	P |= cpu_readmem(++S);
	change_pc(P);
    CallSubLevel--;
}

INLINE void CLH5801::RTI(void)
{
	RTN();
	// flags
	T = cpu_readmem(++S);
	
}

INLINE void CLH5801::PUSH(UINT8 data)
{
	cpu_writemem(S--, data);
}

INLINE void CLH5801::PUSH_WORD(UINT16 data)
{
	PUSH( (UINT8) (data & 0xff));
	PUSH( (UINT8) (data >> 8));
}

INLINE void CLH5801::POP(void)
{
	lh5801.a = cpu_readmem(++S);
	CHECK_Z(lh5801.a);
}

INLINE void CLH5801::POP_WORD(PAIR *reg)
{
	reg->b.h = cpu_readmem(++S);
	reg->b.l = cpu_readmem(++S);
}


INLINE void CLH5801::JMP(UINT32 adr)
{
	P = (UINT16) adr;
	change_pc(P);
}

INLINE void CLH5801::BRANCH_PLUS(int doit)
{
	UINT16 t = cpu_readop(P++);
	if (doit) {
		AddState(2);
		P += t;
		change_pc(P);
	}
}

INLINE void CLH5801::BRANCH_MINUS(int doit)
{
	UINT8 t=cpu_readop(P++);
	if (doit) {
		AddState(3);
		P -= t;
		change_pc(P);
	}
}

INLINE void CLH5801::LOP(void)
{
	UINT8 t = cpu_readop(P++);

	AddState(8);

	if (UL--) {
		AddState(3);
		P -= t;
		change_pc(P);		
	}
}

INLINE void CLH5801::SJP(void)
{
	UINT16 t=readop_word();
	PUSH_WORD(P);
	P = t;
	change_pc(t);	
    CallSubLevel++;
}

INLINE void CLH5801::VECTOR(int doit, int nr)
{
	if (doit) {
		PUSH_WORD(P);
		P =  (cpu_readmem(0xff00+nr) << 8) | cpu_readmem(0xff00+nr+1);
		change_pc(P);
		AddState(21-8);
        CallSubLevel++;
	}
	UNSET_Z;
}

INLINE void CLH5801::AEX(void)
{
	UINT8 l = lh5801.a;
	lh5801.a = (l<<4) | (l>>4);
}

INLINE void CLH5801::DRL(UINT32 adr)
{
	UINT16 l = lh5801.a | (cpu_readmem(adr)<<8);

	lh5801.a = l>>8;
	cpu_writemem( adr , l>>4 );
}

INLINE void CLH5801::DRR(UINT32 adr)
{
	UINT16 l = cpu_readmem(adr) | (lh5801.a<<8);

	lh5801.a = (UINT8) l;
	cpu_writemem(adr,l>>4);
}

INLINE void CLH5801::ROL(void)
{
	// maybe use of the adder
	int l = lh5801.a;
	lh5801.a=(lh5801.a << 1) | F_C;

	CHECK_C( l & 0x80 );				// OK
	CHECK_Z( lh5801.a );				// OK
	CHECK_H( lh5801.a & 0x10 );			// OK
	CHECK_V( (l >= 0x40) && (l<0xc0) );	// OK

}

INLINE void CLH5801::ROR(void)
{
	int l = lh5801.a;
	lh5801.a = ((lh5801.a | (F_C << 8)) >> 1);

	// flags cvhz
	CHECK_C(l & 0x01);					// OK
	CHECK_Z(lh5801.a);					// OK
	CHECK_H(lh5801.a & 0x08);			// OK
	CHECK_V( ( (l&0x01)&&(lh5801.a&0x02) ) || ((l&0x02)&&(lh5801.a&0x01)));	// OK
}

INLINE void CLH5801::SHL(void)		// FLAGS OK
{
	int l = lh5801.a;
	lh5801.a<<=1;

	CHECK_C(l & 0x80);					// OK
	CHECK_Z(lh5801.a);					// OK
	CHECK_H(l & 0x08);					// OK
	CHECK_V((l>=0x40)&&(l<0xc0));		// OK
}

INLINE void CLH5801::SHR(void)		// FLAGS OK
{
	int l=lh5801.a;
	lh5801.a>>=1;

	CHECK_C(l & 0x01);										// OK
	CHECK_Z(lh5801.a);										// OK
	CHECK_H(lh5801.a & 0x08);								// OK
	CHECK_V( ( (l&0x01)&&(lh5801.a&0x02) ) || ((l&0x02)&&(lh5801.a&0x01)));	// OK
}

INLINE void CLH5801::AM(int value)
{
	lh5801.tm=value;
}

INLINE void CLH5801::ITA(void)
{
    lh5801.a=pPC->in(0);
	CHECK_Z(lh5801.a);
}


INLINE void CLH5801::instruction_fd(void)
{
	int oper;
	int adr;

	oper = cpu_readop(P++);
	
//	Log_Oper(1,oper);

	switch (oper) {
	case 0x01:	SBC(cpu_readmem(ME1(X)));						AddState(11);	break;
	case 0x03:	ADC(cpu_readmem(ME1(X)));						AddState(11);	break;
	case 0x05:	LDA(cpu_readmem(ME1(X)));						AddState(10);/**/	break;
	case 0x07:	CPA(lh5801.a, cpu_readmem(ME1(X))); 			AddState(11);	break;
	case 0x08:	X=X;	AddLog(LOG_MASTER,"X=X op08");				AddState(11);	break;
	case 0x09:	AND(cpu_readmem(ME1(X)));						AddState(11);	break;
	case 0x0a:	POP_WORD(&lh5801.x);							AddState(15);	break;
	case 0x0b:	ORA(cpu_readmem(ME1(X)));						AddState(11);	break;
	case 0x0c:	DCS(cpu_readmem(ME1(X)));						AddState(17);/**/ 	break;
	case 0x0d:	EOR(cpu_readmem(ME1(X)));						AddState(11);	break;
	case 0x0e:	cpu_writemem(ME1(X),lh5801.a);					AddState(10);	break;
	case 0x0f:	BIT(cpu_readmem(ME1(X)),lh5801.a); 				AddState(11);	break;
	case 0x11:	SBC(cpu_readmem(ME1(Y)));						AddState(11);	break;
	case 0x13:	ADC(cpu_readmem(ME1(Y)));						AddState(11);	break;
	case 0x15:	LDA(cpu_readmem(ME1(Y)));						AddState(10);	break;
	case 0x17:	CPA(lh5801.a, cpu_readmem(ME1(Y))); 			AddState(11);	break;
	case 0x18:	X=Y;											AddState(11);	break;
	case 0x19:	AND(cpu_readmem(ME1(Y)));						AddState(11);	break;
	case 0x1a:	POP_WORD(&lh5801.y);							AddState(15);	break;
	case 0x1b:	ORA(cpu_readmem(ME1(Y)));						AddState(11);	break;
	case 0x1c:	DCS(cpu_readmem(ME1(Y)));						AddState(17);/**/ 	break;
	case 0x1d:	EOR(cpu_readmem(ME1(Y))); 						AddState(11);	break;
	case 0x1e:	cpu_writemem(ME1(Y),lh5801.a); 					AddState(10);/**/	break;
	case 0x1f:	BIT(cpu_readmem(ME1(Y)),lh5801.a);				AddState(11);	break;
	case 0x21:	SBC(cpu_readmem(ME1(U)));						AddState(11);	break;
	case 0x23:	ADC(cpu_readmem(ME1(U)));						AddState(11);	break;
	case 0x25:	LDA(cpu_readmem(ME1(U)));						AddState(10);	break;
	case 0x27:	CPA(lh5801.a, cpu_readmem(ME1(U))); 			AddState(11);	break;
	case 0x28:	X=U;											AddState(11);	break;
	case 0x29:	AND(cpu_readmem(ME1(U)));						AddState(11);	break;
	case 0x2a:	POP_WORD(&lh5801.u);							AddState(15);	break;
	case 0x2b:	ORA(cpu_readmem(ME1(U)));						AddState(11);	break;
	case 0x2c:	DCS(cpu_readmem(ME1(U)));						AddState(17);/**/ 	break;
	case 0x2d:	EOR(cpu_readmem(ME1(U)));						AddState(11);	break;
	case 0x2e:	cpu_writemem(ME1(U),lh5801.a); 					AddState(10);	break;
	case 0x2f:	BIT(cpu_readmem(ME1(U)),lh5801.a); 				AddState(11);	break;
    case 0x3a:	S++;S++;            							AddState(15);	break;
    case 0x40:	INC(&XH);										AddState(9);	break;
	case 0x42:	DEC(&XH);										AddState(9);	break;
	case 0x48:	X=S;											AddState(11);	break;
	case 0x49:	AND_MEM(ME1(X), cpu_readop(P++));				AddState(17);	break;
	case 0x4a:	X=X;	AddLog(LOG_MASTER,"X=X op4a");			AddState(11);	break; //!!!
	case 0x4b:	ORA_MEM(ME1(X), cpu_readop(P++)); 				AddState(17);	break;
	case 0x4c:	lh5801.bf=0;/*off ! LOOK*/						AddState(8);	break;
	case 0x4d:	BIT(cpu_readmem(ME1(X)), cpu_readop(P++));		AddState(14);/**/	break;
	case 0x4e:	S=X;											AddState(11);	break;
	case 0x4f:	ADD_MEM(ME1(X), cpu_readop(P++)); 				AddState(17);	break;
	case 0x50:	INC(&YH);										AddState(9);	break;
	case 0x52:	DEC(&YH);										AddState(9);	break;
	case 0x58:	X=P;											AddState(11);	break;
	case 0x59:	AND_MEM(ME1(Y), cpu_readop(P++));				AddState(17);	break;
	case 0x5a:	Y=X;											AddState(11);	break;
	case 0x5b:	ORA_MEM(ME1(Y), cpu_readop(P++)); 				AddState(17);	break;
	case 0x5d:	BIT(cpu_readmem(ME1(Y)), cpu_readop(P++));		AddState(14);/**/	break;
    case 0x5e:	JMP(X);	CallSubLevel--;										AddState(11);	break; // P=X
	case 0x5f:	ADD_MEM(ME1(Y), cpu_readop(P++));				AddState(17);	break;
	case 0x60:	INC(&UH);										AddState(9);	break;
	case 0x62:	DEC(&UH);										AddState(9);	break;
	case 0x69:	AND_MEM(ME1(U), cpu_readop(P++)); 				AddState(17);	break;
	case 0x6a:	U=X;											AddState(11);	break;
	case 0x6b:	ORA_MEM(ME1(U), cpu_readop(P++)); 				AddState(17);	break;
	case 0x6d:	BIT(cpu_readmem(ME1(X)), cpu_readop(P++));		AddState(14);/**/	break;
	case 0x6f:	ADD_MEM(ME1(U), cpu_readop(P++)); 				AddState(17);	break;
	case 0x81:	SET_IE; /*sie !*/								AddState(8);/**/	break;
	case 0x88:	PUSH_WORD(X);									AddState(14);	break;
	case 0x8a:	POP();											AddState(12);	break;
	case 0x8c:	DCA(cpu_readmem(ME1(X)));						AddState(19); 	break; 
//	case 0x8e:	/*cdv clears internal devider*/		/* LOOK*/	AddState(8);	break;
	case 0x98:	PUSH_WORD(Y);									AddState(14);	break;
	case 0x9c:	DCA(cpu_readmem(ME1(Y)));						AddState(19); 	break;
	case 0xa1:	SBC(cpu_readmem(ME1(readop_word()))); 			AddState(17);	break;
	case 0xa3:	ADC(cpu_readmem(ME1(readop_word()))); 			AddState(17);	break;
	case 0xa5:	LDA(cpu_readmem(ME1(readop_word()))); 			AddState(16);/**/	break;
	case 0xa7:	CPA(lh5801.a, cpu_readmem(ME1(readop_word())));	AddState(17);	break;
	case 0xa8:	PUSH_WORD(U);									AddState(14);	break;
	case 0xa9:	AND(cpu_readmem(ME1(readop_word()))); 			AddState(17);	break;
	case 0xaa:	LDA(lh5801.t);CHECK_Z(lh5801.t);				AddState(9);	break;
	case 0xab:	ORA(cpu_readmem(ME1(readop_word()))); 			AddState(17);	break;
	case 0xac:	DCA(cpu_readmem(ME1(U)));						AddState(19); 	break;
	case 0xad:	EOR(cpu_readmem(ME1(readop_word()))); 			AddState(17);	break;
	case 0xae:	cpu_writemem(ME1(readop_word()),lh5801.a);		AddState(16);	break;
	case 0xaf:	BIT(cpu_readmem(ME1(readop_word())),lh5801.a);	AddState(17);	break;
    case 0xb1:	lh5801.HLT=0;AddLog(0x01,"HALT");/* LOOK */		AddState(8);	break;
    case 0xb8:	PUSH_WORD((lh5801.s.b.h)<<8);   				AddState(14);	break;
    case 0xba:	ITA();											AddState(9);	break;
	case 0xbe:	UNSET_IE; /*rie !*/								AddState(8);/**/	break;
	case 0xc0:	lh5801.dp=0; /*rdp !*/							AddState(8);	break;
	case 0xc1:	lh5801.dp=1; /*sdp !*/							AddState(8);	break;
	case 0xc8:	PUSH(lh5801.a);									AddState(11);	break;
	case 0xca:	ADR(&lh5801.x);									AddState(11);	break;
//	case 0xcc:	/*atp sends a to data bus*/		/* LOOK */		AddState(9);	break;
	case 0xce:	AM(lh5801.a);									AddState(9); 	break;
	case 0xd3:	DRR(ME1(X)); 									AddState(16);/**/ 	break;
	case 0xd7:	DRL(ME1(X));									AddState(16);/**/ 	break;
	case 0xda:	ADR(&lh5801.y);									AddState(11);	break;
	case 0xde:	AM(lh5801.a|0x100);								AddState(9); 	break;
	case 0xea:	ADR(&lh5801.u);									AddState(11);	break;
	case 0xe9:	adr=ME1(readop_word());AND_MEM(adr, cpu_readop(P++)); 
																AddState(23);	break;
	case 0xeb: 	adr=ME1(readop_word());ORA_MEM(adr, cpu_readop(P++)); 
																AddState(23);	break;
	case 0xec:	lh5801.t=lh5801.a & 0x1F;						AddState(9);	break;
	case 0xed:	adr=ME1(readop_word());BIT(cpu_readmem(adr), cpu_readop(P++)); 
																AddState(20);/**/	break;
	case 0xef:	adr=ME1(readop_word());ADD_MEM(adr, cpu_readop(P++)); 
																AddState(23);	break;
	default:
        if (!resetFlag) {
				AddLog(LOG_MASTER,tr("lh5801 illegal opcode at %1  fd%2").arg((P-2),4,16,QChar('0')).arg((int)oper,2,16,QChar('0')));
                qWarning()<<tr("lh5801 illegal opcode at %1  fd%2").arg((P-2),4,16,QChar('0')).arg((int)oper,2,16,QChar('0'));
                pPC->BreakSubLevel = 99999;
                pPC->DasmStep = true;
                pPC->DasmFlag = false;
                pPC->pBreakpointManager->breakMsg=tr("ill op at %1 %2").arg(P-1,4,16,QChar('0')).arg(oper,4,16,QChar('0'));
                emit showDasm();
        }
                break;
	}
}


INLINE void CLH5801::instruction(void)
{
	int oper;
	int adr;


	oper=cpu_readop(P++);
	

//	Log_Oper(0,oper);

	switch (oper) {
	case 0x00: SBC(XL);									AddState(6);/**/	break;	//OK SBC XL
	case 0x01: SBC(cpu_readmem(X));						AddState(7);/**/	break;	//OK SBC(X)
	case 0x02: ADC(XL);									AddState(6);/**/	break;	//OK ADC XL
	case 0x03: ADC(cpu_readmem(X));						AddState(7);/**/	break;	//OK ADC(X)
	case 0x04: LDA(XL);									AddState(5);/**/	break;	//OK LDA XL
	case 0x05: LDA(cpu_readmem(X));						AddState(6);/**/	break;	//OK LDA(X)
	case 0x06: CPA(lh5801.a, XL);						AddState(6);/**/	break;	//OK CPA XL
	case 0x07: CPA(lh5801.a, cpu_readmem(X));			AddState(7);/**/	break;	//OK CPA(X)
	case 0x08: XH=lh5801.a;								AddState(5);/**/	break;	//OK STA XH
	case 0x09: AND(cpu_readmem(X));						AddState(7);/**/	break;	//OK AND(X)
	case 0x0a: XL=lh5801.a;								AddState(5);/**/	break;	//OK STA XL
	case 0x0b: ORA(cpu_readmem(X));						AddState(7);/**/	break;	//OK ORA(X)
	case 0x0c: DCS(cpu_readmem(X));						AddState(13);/**/	break;	//OK DCS(X)
	case 0x0d: EOR(cpu_readmem(X));						AddState(7);/**/	break;	// EOR(X)
	case 0x0e: cpu_writemem(X,lh5801.a);				AddState(6);/**/	break;	// STA(X)
	case 0x0f: BIT(cpu_readmem(X),lh5801.a);			AddState(7);/**/	break;	// BIT(X)
	case 0x10: SBC(YL);									AddState(6);/**/	break;	// SBC YL
	case 0x11: SBC(cpu_readmem(Y));						AddState(7);/**/	break;	// SBC(Y)
	case 0x12: ADC(YL);									AddState(6);/**/	break;	//OK ADC YL
	case 0x13: ADC(cpu_readmem(Y));						AddState(7);/**/	break;	//OK ADC(Y)
	case 0x14: LDA(YL);									AddState(5);/**/	break;
	case 0x15: LDA(cpu_readmem(Y));						AddState(6);/**/	break;
	case 0x16: CPA(lh5801.a, YL);						AddState(6);/**/	break;
	case 0x17: CPA(lh5801.a, cpu_readmem(Y));			AddState(7);/**/	break;
	case 0x18: YH=lh5801.a;								AddState(5);/**/	break;
	case 0x19: AND(cpu_readmem(Y));						AddState(7);/**/	break;
	case 0x1a: YL=lh5801.a;								AddState(5);/**/	break;
	case 0x1b: ORA(cpu_readmem(Y));						AddState(7);/**/	break;
	case 0x1c: DCS(cpu_readmem(Y));						AddState(13);/**/	break;
	case 0x1d: EOR(cpu_readmem(Y));						AddState(7);/**/	break;
	case 0x1e: cpu_writemem(Y,lh5801.a);				AddState(6);/**/	break;
	case 0x1f: BIT(cpu_readmem(Y),lh5801.a);			AddState(7);/**/	break;
	case 0x20: SBC(UL);									AddState(6);/**/	break;	
	case 0x21: SBC(cpu_readmem(U));						AddState(7);/**/	break;
	case 0x22: ADC(UL);									AddState(6);/**/	break;
	case 0x23: ADC(cpu_readmem(U));						AddState(7);/**/	break;
	case 0x24: LDA(UL);									AddState(5);/**/	break;
	case 0x25: LDA(cpu_readmem(U));						AddState(6);/**/	break;
	case 0x26: CPA(lh5801.a, UL);						AddState(6);/**/	break;
	case 0x27: CPA(lh5801.a, cpu_readmem(U));			AddState(7);/**/	break;
	case 0x28: UH = lh5801.a;							AddState(5);/**/	break;
	case 0x29: AND(cpu_readmem(U));						AddState(7);/**/	break;
	case 0x2a: UL = lh5801.a;							AddState(5);/**/	break;
	case 0x2b: ORA(cpu_readmem(U));						AddState(7);/**/	break;
	case 0x2c: DCS(cpu_readmem(U));						AddState(13);/**/	break;
	case 0x2d: EOR(cpu_readmem(U));						AddState(7);/**/	break;
	case 0x2e: cpu_writemem(U,lh5801.a);				AddState(6);/**/	break;
	case 0x2f: BIT(cpu_readmem(U),lh5801.a);			AddState(7);/**/	break;
    case 0x30: SBC(0);									AddState(6);/**/	break;
    case 0x32: ADC(0);									AddState(6);/**/	break;
    case 0x34: LDA(0);									AddState(5);/**/	break;
    case 0x36: CPA(lh5801.a, 0);						AddState(6);/**/	break;
    case 0x38: /*nop*/									AddState(5);/**/	break;
	case 0x40: INC(&XL);								AddState(5);/**/	break;
	case 0x41: SIN(&lh5801.x);							AddState(6);/**/	break;
	case 0x42: DEC(&XL);								AddState(5);/**/	break;
	case 0x43: SDE(&lh5801.x);							AddState(6);/**/	break;
	case 0x44: X++;										AddState(5);/**/	break;
	case 0x45: LIN(&lh5801.x);							AddState(6);/**/	break;
	case 0x46: X--;										AddState(5);/**/	break;
	case 0x47: LDE(&lh5801.x);							AddState(6);/**/	break;
	case 0x48: XH=cpu_readop(P++);						AddState(6);/**/	break;
	case 0x49: AND_MEM(X, cpu_readop(P++));				AddState(13);/**/	break;
	case 0x4a: XL=cpu_readop(P++);						AddState(6);/**/	break;
	case 0x4b: ORA_MEM(X, cpu_readop(P++));				AddState(13);/**/	break;
	case 0x4c: CPA(XH, cpu_readop(P++));				AddState(7);/**/	break;
	case 0x4d: BIT(cpu_readmem(X), cpu_readop(P++));	AddState(10);/**/	break;
	case 0x4e: CPA(XL, cpu_readop(P++));				AddState(7);/**/	break;
	case 0x4f: ADD_MEM(X, cpu_readop(P++));				AddState(13);/**/	break;
	case 0x50: INC(&YL);								AddState(5);/**/	break;
	case 0x51: SIN(&lh5801.y);							AddState(6);/**/	break;
	case 0x52: DEC(&YL);								AddState(5);/**/	break;
	case 0x53: SDE(&lh5801.y);							AddState(6);/**/	break;
	case 0x54: Y++;										AddState(5);/**/	break;
	case 0x55: LIN(&lh5801.y);							AddState(6);/**/	break;
	case 0x56: Y--;										AddState(5);/**/	break;
	case 0x57: LDE(&lh5801.y);							AddState(6);/**/	break;
	case 0x58: YH=cpu_readop(P++);						AddState(6);/**/	break;
	case 0x59: AND_MEM(Y, cpu_readop(P++));				AddState(13);/**/	break;
	case 0x5a: YL=cpu_readop(P++);						AddState(6);/**/	break;
	case 0x5b: ORA_MEM(Y, cpu_readop(P++));				AddState(13);/**/	break;
	case 0x5c: CPA(YH, cpu_readop(P++));				AddState(7);/**/	break;
	case 0x5d: BIT(cpu_readmem(Y), cpu_readop(P++));	AddState(10);/**/	break;
	case 0x5e: CPA(YL, cpu_readop(P++));				AddState(7);/**/	break;
	case 0x5f: ADD_MEM(Y, cpu_readop(P++));				AddState(13);/**/	break;
	case 0x60: INC(&UL);								AddState(5);/**/	break;
	case 0x61: SIN(&lh5801.u);							AddState(6);/**/	break;
	case 0x62: DEC(&UL);								AddState(5);/**/	break;
	case 0x63: SDE(&lh5801.u);							AddState(6);/**/	break;
	case 0x64: U++;										AddState(5);/**/	break;
	case 0x65: LIN(&lh5801.u);							AddState(6);/**/	break;
	case 0x66: U--;										AddState(5);/**/	break;
	case 0x67: LDE(&lh5801.u);							AddState(6);/**/	break;
	case 0x68: UH=cpu_readop(P++);						AddState(6);/**/	break;
	case 0x69: AND_MEM(U, cpu_readop(P++));				AddState(13);/**/	break;
	case 0x6a: UL=cpu_readop(P++);						AddState(6);/**/	break;
	case 0x6b: ORA_MEM(U, cpu_readop(P++));				AddState(13);/**/	break;
	case 0x6c: CPA(UH, cpu_readop(P++));				AddState(7);/**/	break;
	case 0x6d: BIT(cpu_readmem(U), cpu_readop(P++));	AddState(10);/**/	break;
	case 0x6e: CPA(UL, cpu_readop(P++));				AddState(7);/**/	break;
	case 0x6f: ADD_MEM(U, cpu_readop(P++));				AddState(13);/**/	break;
	case 0x80: SBC(XH);									AddState(6);/**/	break;
	case 0x81: BRANCH_PLUS(!F_C);						AddState(8);/**/	break;
	case 0x82: ADC(XH);									AddState(6);/**/	break;
	case 0x83: BRANCH_PLUS(F_C);						AddState(8);/**/	break;
	case 0x84: LDA(XH);									AddState(5);/**/	break;
	case 0x85: BRANCH_PLUS(!F_H);						AddState(8);/**/	break;
	case 0x86: CPA(lh5801.a, XH);						AddState(6);/**/	break;
	case 0x87: BRANCH_PLUS(F_H);						AddState(8);/**/	break;
	case 0x88: LOP();												/**/	break;
	case 0x89: BRANCH_PLUS(!F_Z);						AddState(8);/**/	break;
	case 0x8a: RTI();									AddState(14);/**/	break;
	case 0x8b: BRANCH_PLUS(F_Z);						AddState(8);/**/	break;
	case 0x8c: DCA(cpu_readmem(X));						AddState(15);/**/	break;
	case 0x8d: BRANCH_PLUS(!F_V);						AddState(8);/**/	break;
	case 0x8e: BRANCH_PLUS(1);							AddState(6);/**/	break;
	case 0x8f: BRANCH_PLUS(F_V);						AddState(8);/**/	break;
	case 0x90: SBC(YH);									AddState(6);/**/	break;
	case 0x91: BRANCH_MINUS(!F_C);						AddState(8);/**/	break;
	case 0x92: ADC(YH);									AddState(6);/**/	break;
	case 0x93: BRANCH_MINUS(F_C);						AddState(8);/**/	break;
	case 0x94: LDA(YH);									AddState(5);/**/	break;
	case 0x95: BRANCH_MINUS(!F_H);						AddState(8);/**/	break;
	case 0x96: CPA(lh5801.a, YH);						AddState(6);/**/	break;
	case 0x97: BRANCH_MINUS(F_H);						AddState(8);/**/	break;
	case 0x99: BRANCH_MINUS(!F_Z);						AddState(8);/**/	break;
	case 0x9a: RTN();									AddState(11);/**/	break;
	case 0x9b: BRANCH_MINUS(F_Z);						AddState(8);/**/	break;
	case 0x9c: DCA(cpu_readmem(Y));						AddState(15);/**/	break;
	case 0x9d: BRANCH_MINUS(!F_V);						AddState(8);/**/	break;
	case 0x9e: BRANCH_MINUS(1);							AddState(6);/**/	break;
	case 0x9f: BRANCH_MINUS(F_V);						AddState(8);/**/	break;
	case 0xa0: SBC(UH);									AddState(6);/**/	break;
	case 0xa2: ADC(UH);									AddState(6);/**/	break;
	case 0xa1: SBC(cpu_readmem(readop_word()));			AddState(13);/**/	break;
	case 0xa3: ADC(cpu_readmem(readop_word()));			AddState(13);/**/	break;
	case 0xa4: LDA(UH);									AddState(5);/**/	break;
	case 0xa5: LDA(cpu_readmem(readop_word()));			AddState(12);/**/	break;
	case 0xa6: CPA(lh5801.a, UH);						AddState(6);/**/	break;
	case 0xa7: CPA(lh5801.a,cpu_readmem(readop_word()));AddState(13);/**/	break;
	case 0xa8: lh5801.pv=1;/*spv!*/						AddState(4);/**/	break;
	case 0xa9: AND(cpu_readmem(readop_word()));			AddState(13);/**/	break;
	case 0xaa: S=readop_word();							AddState(12);/**/	break;
	case 0xab: ORA(cpu_readmem(readop_word()));			AddState(13);/**/	break;
	case 0xac: DCA(cpu_readmem(U));						AddState(15);/**/	break;
	case 0xad: EOR(cpu_readmem(readop_word()));			AddState(13);/**/	break;
	case 0xae: cpu_writemem(readop_word(),lh5801.a);	AddState(12);/**/	break;
	case 0xaf: BIT(cpu_readmem(readop_word()),lh5801.a);AddState(13);/**/	break;
	case 0xb1: SBC(cpu_readop(P++));					AddState(7);/**/	break;
	case 0xb3: ADC(cpu_readop(P++));					AddState(7);/**/	break;
	case 0xb5: LDA(cpu_readop(P++));					AddState(6);/**/	break;
	case 0xb7: CPA(lh5801.a, cpu_readop(P++));			AddState(7);/**/	break;
	case 0xb8: lh5801.pv=0;/*rpv!*/						AddState(4);/**/	break;
	case 0xb9: AND(cpu_readop(P++));					AddState(7);/**/	break;
	case 0xba: JMP(readop_word());						AddState(12);/**/	break;
	case 0xbb: ORA(cpu_readop(P++));					AddState(7);/**/	break;
	case 0xbd: EOR(cpu_readop(P++));					AddState(7);/**/	break;
	case 0xbe: SJP();									AddState(19);/**/	break;
	case 0xbf: BIT(lh5801.a,cpu_readop(P++));			AddState(7);/**/	break;
	case 0xc1: VECTOR(!F_C, cpu_readop(P++));			AddState(8);/**/	break;
	case 0xc3: VECTOR(F_C,  cpu_readop(P++));			AddState(8);/**/	break;
	case 0xc5: VECTOR(!F_H, cpu_readop(P++));			AddState(8);/**/	break;
	case 0xc7: VECTOR(F_H , cpu_readop(P++));			AddState(8);/**/	break;
	case 0xc9: VECTOR(!F_Z, cpu_readop(P++));			AddState(8);/**/	break;
	case 0xcb: VECTOR(F_Z , cpu_readop(P++));			AddState(8);/**/	break;
	case 0xcd: VECTOR(  1 , cpu_readop(P++));			AddState(7);/**/	break;
	case 0xcf: VECTOR(F_V , cpu_readop(P++));			AddState(8);/**/	break;
	case 0xd1: ROR();									AddState(9);/**/	break;
	case 0xd3: DRR(X);									AddState(12);/**/	break;
	case 0xd5: SHR();									AddState(9);/**/	break;
	case 0xd7: DRL(X);									AddState(12);/**/	break;
	case 0xd9: SHL();									AddState(6);/**/	break;
	case 0xdb: ROL();									AddState(8);/**/	break;
	case 0xdd: INC(&lh5801.a);							AddState(5);/**/	break;
	case 0xdf: DEC(&lh5801.a);							AddState(5);/**/	break;
	case 0xe1: lh5801.pu=1;/*spu!*/						AddState(4);/**/	break;
	case 0xe3: lh5801.pu=0;/*rpu!*/						AddState(4);/**/	break;
	case 0xe9: adr=readop_word();
			   AND_MEM(adr, cpu_readop(P++)); 			AddState(19);	break;
	case 0xeb: adr=readop_word();
			   ORA_MEM(adr, cpu_readop(P++));			AddState(19);	break;
	case 0xed: 
		adr=readop_word();BIT(cpu_readmem(adr), cpu_readop(P++)); 
														AddState(16);	break;
	case 0xef: 
		adr=readop_word();ADD_MEM(adr, cpu_readop(P++));
														AddState(19);	break;
	case 0xf1: AEX();									AddState(6);/**/	break;
	case 0xf5: cpu_writemem(Y++, cpu_readmem(X++));		AddState(7);	break; //TIN
	case 0xf7: CPA(lh5801.a, cpu_readmem(X++));			AddState(7);	break; //CIN
	case 0xf9: UNSET_C;									AddState(4);/**/	break;
	case 0xfb: SET_C;									AddState(4);/**/	break;
	case 0xfd: instruction_fd();										break;
	case 0xc0: case 0xc2: case 0xc4: case 0xc6: 
	case 0xc8: case 0xca: case 0xcc: case 0xce:
	case 0xd0: case 0xd2: case 0xd4: case 0xd6: 
	case 0xd8: case 0xda: case 0xdc: case 0xde:
	case 0xe0: case 0xe2: case 0xe4: case 0xe6: 
	case 0xe8: case 0xea: case 0xec: case 0xee:
	case 0xf0: case 0xf2: case 0xf4: case 0xf6: 
				VECTOR(1, oper);						AddState(4);/**/	break;
	default:
        if (!resetFlag) {
            AddLog(LOG_MASTER,tr("lh5801 illegal opcode at %1 %2").arg(P-1,4,16,QChar('0')).arg(oper,4,16,QChar('0')));
            qWarning()<<tr("lh5801 illegal opcode at %1 %2").arg(P-1,4,16,QChar('0')).arg(oper,4,16,QChar('0'));

            pPC->BreakSubLevel = 99999;
            pPC->DasmStep = true;
            pPC->DasmFlag = false;
            pPC->pBreakpointManager->breakMsg=tr("ill op at %1 %2").arg(P-1,4,16,QChar('0')).arg(oper,4,16,QChar('0'));
            emit showDasm();
        }
        break;
	}

}

void CLH5801::Regs_Info(UINT8 Type)
{
	switch(Type)
	{
	case 0:			// Monitor Registers Dialog
        sprintf(Regs_String,	"LH5801 : P:%.4x S:%.4x U:%.4x X:%.4x Y:%.4x T:%.2x A:%.2x TM:%.3x IN:%.2x PV:%x PU:%x BF:%x DP:%x FLAGS:%s%s%s%s%s%s%s%s",
							lh5801.p.w,lh5801.s.w,lh5801.u.w,
							lh5801.x.w,lh5801.y.w,lh5801.t,
							lh5801.a,lh5801.tm,pPC->pKEYB->KStrobe,
							lh5801.pv,lh5801.pu,lh5801.bf,lh5801.dp,
							lh5801.t&0x80?"1":"0", 
							lh5801.t&0x40?"1":"0", 
							lh5801.t&0x20?"1":"0",
							lh5801.t&0x10?"H":".", 
							lh5801.t&8?"V":".", 
							lh5801.t&4?"Z":".", 
							lh5801.t&2?"I":".", 
							lh5801.t&1?"C":".");
		break;
	case 1:			// For Log File
		sprintf(Regs_String,	"P:%.4x S:%.4x U:%.4x X:%.4x Y:%.4x T:%.2x A:%.2x TM:%.3x IN:%.2x PV:%x  PU:%x BF:%x DP:%x F:%s%s%s%s%s%s%s%s",
							lh5801.p.w,lh5801.s.w,lh5801.u.w,
							lh5801.x.w,lh5801.y.w,lh5801.t,
							lh5801.a,lh5801.tm,pPC->pKEYB->KStrobe,
							lh5801.pv,lh5801.pu,lh5801.bf,lh5801.dp,
							lh5801.t&0x80?"1":"0", 
							lh5801.t&0x40?"1":"0", 
							lh5801.t&0x20?"1":"0",
							lh5801.t&0x10?"H":".", 
							lh5801.t&8?"V":".", 
							lh5801.t&4?"Z":".", 
							lh5801.t&2?"I":".", 
							lh5801.t&1?"C":".");
		break;
	}

}

