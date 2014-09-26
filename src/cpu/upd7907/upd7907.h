#ifndef __UPD7907_H__
#define __UPD7907_H__


/*
  all types have internal ram at 0xff00-0xffff
  7810
  7811 (4kbyte),7812(8),7814(16) have internal rom at 0x0000
*/
#undef IN
#undef OUT

#include "cpu.h"






// I/O line definitions
enum
{
    // input lines
    MAX_INPUT_LINES = 32+3,
    INPUT_LINE_IRQ0 = 0,
    INPUT_LINE_IRQ1 = 1,
    INPUT_LINE_IRQ2 = 2,
    INPUT_LINE_IRQ3 = 3,
    INPUT_LINE_IRQ4 = 4,
    INPUT_LINE_IRQ5 = 5,
    INPUT_LINE_IRQ6 = 6,
    INPUT_LINE_IRQ7 = 7,
    INPUT_LINE_IRQ8 = 8,
    INPUT_LINE_IRQ9 = 9,
    INPUT_LINE_NMI = MAX_INPUT_LINES - 3,

    // special input lines that are implemented in the core
    INPUT_LINE_RESET = MAX_INPUT_LINES - 2,
    INPUT_LINE_HALT = MAX_INPUT_LINES - 1
};


// unfortunatly memory configuration differs with internal rom size
typedef enum
{
	TYPE_7801,
	TYPE_78C05,
	TYPE_78C06,
	TYPE_7810,
	TYPE_7810_GAMEMASTER, // a few modifications until internal rom dumped
	TYPE_7807
//  TYPE_78C10, // stop instruction added
//  TYPE_78IV,
//  TYPE_78K0,
//  TYPE_78K0S
//  millions of subtypes
} UPD7907_TYPE;

/* Supply an instance of this function in your driver code:
 * It will be called whenever an output signal changes or a new
 * input line state is to be sampled.
 */
//typedef int (*upd7907_io_callback)(device_t *device, int ioline, int state);

// use it as reset parameter in the Machine struct
typedef struct {
    UPD7907_TYPE type;
//    upd7907_io_callback io_callback;
} UPD7907_CONFIG;

enum
{
    UPD7907_PC=1, UPD7907_SP, UPD7907_PSW,
    UPD7907_EA, UPD7907_V, UPD7907_A, UPD7907_VA,
    UPD7907_BC, UPD7907_B, UPD7907_C, UPD7907_DE, UPD7907_D, UPD7907_E, UPD7907_HL, UPD7907_H, UPD7907_L,
    UPD7907_EA2, UPD7907_V2, UPD7907_A2, UPD7907_VA2,
    UPD7907_BC2, UPD7907_B2, UPD7907_C2, UPD7907_DE2, UPD7907_D2, UPD7907_E2, UPD7907_HL2, UPD7907_H2, UPD7907_L2,
    UPD7907_CNT0, UPD7907_CNT1, UPD7907_TM0, UPD7907_TM1, UPD7907_ECNT, UPD7907_ECPT, UPD7907_ETM0, UPD7907_ETM1,
    UPD7907_MA, UPD7907_MB, UPD7907_MCC, UPD7907_MC, UPD7907_MM, UPD7907_MF,
    UPD7907_TMM, UPD7907_ETMM, UPD7907_EOM, UPD7907_SML, UPD7907_SMH,
    UPD7907_ANM, UPD7907_MKL, UPD7907_MKH, UPD7907_ZCM,
    UPD7907_TXB, UPD7907_RXB, UPD7907_CR0, UPD7907_CR1, UPD7907_CR2, UPD7907_CR3,
    UPD7907_TXD, UPD7907_RXD, UPD7907_SCK, UPD7907_TI, UPD7907_TO, UPD7907_CI, UPD7907_CO0, UPD7907_CO1
};

/* port numbers for PA,PB,PC,PD and PF */
enum offs_t
{
    UPD7907_PORTA, UPD7907_PORTB, UPD7907_PORTC, UPD7907_PORTD, UPD7907_PORTF,
    UPD7907_PORTT,UPD7907_PORTE
};

enum
{
	UPD7807_PORTA, UPD7807_PORTB, UPD7807_PORTC, UPD7807_PORTD, UPD7807_PORTF,
	UPD7807_PORTT
};

/* IRQ lines */
#define UPD7907_INTF1		0
#define UPD7907_INTF2		1
#define UPD7907_INTF0		2
#define UPD7907_INTFE1      4

typedef qint8	INT8;
typedef qint32	INT32;
typedef quint32	UINT32;

typedef union {
#ifdef POCKEMUL_BIG_ENDIAN
    struct { quint8 h3,h2,h,l; } b;
    struct { quint16 h,l; } w;
#else
    struct { quint8 l,h,h2,h3; } b;
    struct { quint16 l,h; } w;
#endif
    quint32 d;
}	PAIR;

class Cupd7907;

//typedef struct _upd7907_state upd7907_state;
struct upd7907_state
{
    PAIR	ppc;	/* previous program counter */
    PAIR	pc; 	/* program counter */
    PAIR	sp; 	/* stack pointer */
    UINT8	op; 	/* opcode */
    UINT8	op2;	/* opcode part 2 */
    UINT8	iff;	/* interrupt enable flip flop */
    UINT8	psw;	/* processor status word */
    PAIR	ea; 	/* extended accumulator */
    PAIR	va; 	/* accumulator + vector register */
    PAIR	bc; 	/* 8bit B and C registers / 16bit BC register */
    PAIR	de; 	/* 8bit D and E registers / 16bit DE register */
    PAIR	hl; 	/* 8bit H and L registers / 16bit HL register */
    PAIR	ea2;	/* alternate register set */
    PAIR	va2;
    PAIR	bc2;
    PAIR	de2;
    PAIR	hl2;
    PAIR	cnt;	/* 8 bit timer counter */
    PAIR	tm; 	/* 8 bit timer 0/1 comparator inputs */
    PAIR	ecnt;	/* timer counter register / capture register */
    PAIR	etm;	/* timer 0/1 comparator inputs */
    UINT8	ma; 	/* port A input or output mask */
    UINT8	mb; 	/* port B input or output mask */
    UINT8	mcc;	/* port C control/port select */
    UINT8	mc; 	/* port C input or output mask */
    UINT8	mm; 	/* memory mapping */
    UINT8	mf; 	/* port F input or output mask */
    UINT8	tmm;	/* timer 0 and timer 1 operating parameters */
    UINT8	etmm;	/* 16-bit multifunction timer/event counter */
    UINT8	eom;	/* 16-bit timer/event counter output control */
    UINT8	sml;	/* serial interface parameters low */
    UINT8	smh;	/* -"- high */
    UINT8	anm;	/* analog to digital converter operating parameters */
    UINT8	mkl;	/* interrupt mask low */
    UINT8	mkh;	/* -"- high */
    UINT8	zcm;	/* bias circuitry for ac zero-cross detection */
    UINT8	pa_in;	/* port A,B,C,D,F inputs */
    UINT8	pb_in;
    UINT8	pc_in;
    UINT8	pd_in;
    UINT8	pf_in;
    UINT8	pa_out; /* port A,B,C,D,F outputs */
    UINT8	pb_out;
    UINT8	pc_out;
    UINT8	pd_out;
    UINT8	pf_out;
    UINT8	cr0;	/* analog digital conversion register 0 */
    UINT8	cr1;	/* analog digital conversion register 1 */
    UINT8	cr2;	/* analog digital conversion register 2 */
    UINT8	cr3;	/* analog digital conversion register 3 */
    UINT8	txb;	/* transmitter buffer */
    UINT8	rxb;	/* receiver buffer */
    UINT8	txd;	/* port C control line states */
    UINT8	rxd;
    UINT8	sck;
    UINT8	ti;
    UINT8	to;
    UINT8	ci;
    UINT8	co0;
    UINT8	co1;
    UINT16	irr;	/* interrupt request register */
    UINT16	itf;	/* interrupt test flag register */
    int		int1;	/* keep track of current int1 state. Needed for 7801 irq checking. */
    int		int2;	/* keep track to current int2 state. Needed for 7801 irq checking. */

/* internal helper variables */
    UINT16	txs;	/* transmitter shift register */
    UINT16	rxs;	/* receiver shift register */
    UINT8	txcnt;	/* transmitter shift register bit count */
    UINT8	rxcnt;	/* receiver shift register bit count */
    UINT8	txbuf;	/* transmitter buffer was written */
    INT32	ovc0;	/* overflow counter for timer 0 (for clock div 12/384) */
    INT32	ovc1;	/* overflow counter for timer 0 (for clock div 12/384) */
    INT32	ovce;	/* overflow counter for ecnt */
    INT32	ovcf;	/* overflow counter for fixed clock div 3 mode */
    INT32	ovcs;	/* overflow counter for serial I/O */
    UINT8	edges;	/* rising/falling edge flag for serial I/O */

    void (* handle_timers)(upd7907_state *cpustate, int cycles);
    UPD7907_CONFIG config;
//    device_irq_acknowledge_callback irq_callback;
//    legacy_cpu_device *device;
//    address_space *program;
//    direct_read_data *direct;
//    address_space *io;
    int icount;
    quint8 imem[0x10];
    CpcXXXX *pPC;

    const struct opcode_s *opXX;	/* opcode table */
    const struct opcode_s *op48;
    const struct opcode_s *op4C;
    const struct opcode_s *op4D;
    const struct opcode_s *op60;
    const struct opcode_s *op64;
    const struct opcode_s *op70;
    const struct opcode_s *op74;

    bool serialPending;
};
typedef void (* funcPtr)(upd7907_state *);
struct opcode_s {
    funcPtr opfunc;
    UINT8 oplen;
    UINT8 cycles;
    UINT8 cycles_skip;
    UINT8 mask_l0_l1;
};

class Cupd7907 : public CCPU
{
public:


    Cupd7907(CPObject *parent);
    virtual ~Cupd7907();

    virtual	bool	init(void);						//initialize
    virtual	bool	exit(void);						//end
    virtual void	step(void);						//step SC61860
    virtual void	Reset(void);

    virtual	void	Load_Internal(QXmlStreamReader *);
    virtual	void	save_internal(QXmlStreamWriter *);

    virtual bool Get_Xin(){ return true;}
    virtual void Set_Xin(bool){}
    virtual bool Get_Xout(){ return true;}
    virtual void Set_Xout(bool){}
    virtual UINT32 get_PC();
    virtual void Regs_Info(UINT8);

    upd7907_state upd7907stat;



    bool softi;
    static UINT8 RP(upd7907_state *cpustate, offs_t port);
    static void WP(upd7907_state *cpustate, offs_t port, UINT8 data);
    static void upd7907_take_irq(upd7907_state *cpustate);
    static void upd7907_write_EOM(upd7907_state *cpustate);
    static void upd7907_write_TXB(upd7907_state *cpustate);
    static void upd7907_sio_output(upd7907_state *cpustate);
    static void upd7907_sio_input(upd7907_state *cpustate);
    static void upd7907_timers(upd7907_state *cpustate, int cycles);
    static void upd7801_timers(upd7907_state *cpustate, int cycles);
    static void upd78c05_timers(upd7907_state *cpustate, int cycles);
    static void set_irq_line(upd7907_state *cpustate, int irqline, int state);
    static void execute(upd7907_state *cpustate);





    static const struct opcode_s op48_7810[];
    static const struct opcode_s op48_7801[];
    static const struct opcode_s op48_78c05[];
    static const struct opcode_s op48_78c06[];
    static const struct opcode_s op48_7907[];
    static const struct opcode_s op4C_7810[];
    static const struct opcode_s op4C_7801[];
    static const struct opcode_s op4C_78c05[];
    static const struct opcode_s op4C_78c06[];
    static const struct opcode_s op4C_7907[];
    static const struct opcode_s op4D_7810[];
    static const struct opcode_s op4D_7801[];
    static const struct opcode_s op4D_78c05[];
    static const struct opcode_s op4D_78c06[];
    static const struct opcode_s op4D_7907[];
    static const struct opcode_s op60_7810[];
    static const struct opcode_s op60_7801[];
    static const struct opcode_s op60_78c05[];
    static const struct opcode_s op60_78c06[];
    static const struct opcode_s op60_7907[];
    static const struct opcode_s op64_7810[];
    static const struct opcode_s op64_7801[];
    static const struct opcode_s op64_78c05[];
    static const struct opcode_s op64_78c06[];
    static const struct opcode_s op64_7907[];
    static const struct opcode_s op70_7810[];
    static const struct opcode_s op70_7801[];
    static const struct opcode_s op70_78c05[];
    static const struct opcode_s op70_78c06[];
    static const struct opcode_s op70_7907[];
    static const struct opcode_s op74_7810[];
    static const struct opcode_s op74_7801[];
    static const struct opcode_s op74_78c05[];
    static const struct opcode_s op74_78c06[];
    static const struct opcode_s op74_7907[];
    static const struct opcode_s opXX_7810[];
    static const struct opcode_s opXX_7807[];
    static const struct opcode_s opXX_7801[];
    static const struct opcode_s opXX_78c05[];
    static const struct opcode_s opXX_78c06[];
    static const struct opcode_s opXX_7907[];

    static void illegal(upd7907_state *cpustate);
    static void illegal2(upd7907_state *cpustate);

    static void ACI_ANM_xx(upd7907_state *cpustate);
    static void ACI_A_xx(upd7907_state *cpustate);
    static void ACI_B_xx(upd7907_state *cpustate);
    static void ACI_C_xx(upd7907_state *cpustate);
    static void ACI_D_xx(upd7907_state *cpustate);
    static void ACI_EOM_xx(upd7907_state *cpustate);
    static void ACI_E_xx(upd7907_state *cpustate);
    static void ACI_H_xx(upd7907_state *cpustate);
    static void ACI_L_xx(upd7907_state *cpustate);
    static void ACI_MKH_xx(upd7907_state *cpustate);
    static void ACI_MKL_xx(upd7907_state *cpustate);
    static void ACI_PA_xx(upd7907_state *cpustate);
    static void ACI_PB_xx(upd7907_state *cpustate);
    static void ACI_PC_xx(upd7907_state *cpustate);
    static void ACI_PD_xx(upd7907_state *cpustate);
    static void ACI_PF_xx(upd7907_state *cpustate);
    static void ACI_SMH_xx(upd7907_state *cpustate);
    static void ACI_TMM_xx(upd7907_state *cpustate);
    static void ACI_V_xx(upd7907_state *cpustate);
    static void ADCW_wa(upd7907_state *cpustate);
    static void ADCX_B(upd7907_state *cpustate);
    static void ADCX_D(upd7907_state *cpustate);
    static void ADCX_Dm(upd7907_state *cpustate);
    static void ADCX_Dp(upd7907_state *cpustate);
    static void ADCX_H(upd7907_state *cpustate);
    static void ADCX_Hm(upd7907_state *cpustate);
    static void ADCX_Hp(upd7907_state *cpustate);
    static void ADC_A_A(upd7907_state *cpustate);
    static void ADC_A_B(upd7907_state *cpustate);
    static void ADC_A_C(upd7907_state *cpustate);
    static void ADC_A_D(upd7907_state *cpustate);
    static void ADC_A_E(upd7907_state *cpustate);
    static void ADC_A_H(upd7907_state *cpustate);
    static void ADC_A_L(upd7907_state *cpustate);
    static void ADC_A_V(upd7907_state *cpustate);
    static void ADC_B_A(upd7907_state *cpustate);
    static void ADC_C_A(upd7907_state *cpustate);
    static void ADC_D_A(upd7907_state *cpustate);
    static void ADC_E_A(upd7907_state *cpustate);
    static void ADC_H_A(upd7907_state *cpustate);
    static void ADC_L_A(upd7907_state *cpustate);
    static void ADC_V_A(upd7907_state *cpustate);
    static void ADDNCW_wa(upd7907_state *cpustate);
    static void ADDNCX_B(upd7907_state *cpustate);
    static void ADDNCX_D(upd7907_state *cpustate);
    static void ADDNCX_Dm(upd7907_state *cpustate);
    static void ADDNCX_Dp(upd7907_state *cpustate);
    static void ADDNCX_H(upd7907_state *cpustate);
    static void ADDNCX_Hm(upd7907_state *cpustate);
    static void ADDNCX_Hp(upd7907_state *cpustate);
    static void ADDNC_A_A(upd7907_state *cpustate);
    static void ADDNC_A_B(upd7907_state *cpustate);
    static void ADDNC_A_C(upd7907_state *cpustate);
    static void ADDNC_A_D(upd7907_state *cpustate);
    static void ADDNC_A_E(upd7907_state *cpustate);
    static void ADDNC_A_H(upd7907_state *cpustate);
    static void ADDNC_A_L(upd7907_state *cpustate);
    static void ADDNC_A_V(upd7907_state *cpustate);
    static void ADDNC_B_A(upd7907_state *cpustate);
    static void ADDNC_C_A(upd7907_state *cpustate);
    static void ADDNC_D_A(upd7907_state *cpustate);
    static void ADDNC_E_A(upd7907_state *cpustate);
    static void ADDNC_H_A(upd7907_state *cpustate);
    static void ADDNC_L_A(upd7907_state *cpustate);
    static void ADDNC_V_A(upd7907_state *cpustate);
    static void ADDW_wa(upd7907_state *cpustate);
    static void ADDX_B(upd7907_state *cpustate);
    static void ADDX_D(upd7907_state *cpustate);
    static void ADDX_Dm(upd7907_state *cpustate);
    static void ADDX_Dp(upd7907_state *cpustate);
    static void ADDX_H(upd7907_state *cpustate);
    static void ADDX_Hm(upd7907_state *cpustate);
    static void ADDX_Hp(upd7907_state *cpustate);
    static void ADD_A_A(upd7907_state *cpustate);
    static void ADD_A_B(upd7907_state *cpustate);
    static void ADD_A_C(upd7907_state *cpustate);
    static void ADD_A_D(upd7907_state *cpustate);
    static void ADD_A_E(upd7907_state *cpustate);
    static void ADD_A_H(upd7907_state *cpustate);
    static void ADD_A_L(upd7907_state *cpustate);
    static void ADD_A_V(upd7907_state *cpustate);
    static void ADD_B_A(upd7907_state *cpustate);
    static void ADD_C_A(upd7907_state *cpustate);
    static void ADD_D_A(upd7907_state *cpustate);
    static void ADD_E_A(upd7907_state *cpustate);
    static void ADD_H_A(upd7907_state *cpustate);
    static void ADD_L_A(upd7907_state *cpustate);
    static void ADD_V_A(upd7907_state *cpustate);
    static void ADINC_ANM_xx(upd7907_state *cpustate);
    static void ADINC_A_xx(upd7907_state *cpustate);
    static void ADINC_B_xx(upd7907_state *cpustate);
    static void ADINC_C_xx(upd7907_state *cpustate);
    static void ADINC_D_xx(upd7907_state *cpustate);
    static void ADINC_EOM_xx(upd7907_state *cpustate);
    static void ADINC_E_xx(upd7907_state *cpustate);
    static void ADINC_H_xx(upd7907_state *cpustate);
    static void ADINC_L_xx(upd7907_state *cpustate);
    static void ADINC_MKH_xx(upd7907_state *cpustate);
    static void ADINC_MKL_xx(upd7907_state *cpustate);
    static void ADINC_PA_xx(upd7907_state *cpustate);
    static void ADINC_PB_xx(upd7907_state *cpustate);
    static void ADINC_PC_xx(upd7907_state *cpustate);
    static void ADINC_PD_xx(upd7907_state *cpustate);
    static void ADINC_PF_xx(upd7907_state *cpustate);
    static void ADINC_SMH_xx(upd7907_state *cpustate);
    static void ADINC_TMM_xx(upd7907_state *cpustate);
    static void ADINC_V_xx(upd7907_state *cpustate);
    static void ADI_ANM_xx(upd7907_state *cpustate);
    static void ADI_A_xx(upd7907_state *cpustate);
    static void ADI_B_xx(upd7907_state *cpustate);
    static void ADI_C_xx(upd7907_state *cpustate);
    static void ADI_D_xx(upd7907_state *cpustate);
    static void ADI_EOM_xx(upd7907_state *cpustate);
    static void ADI_E_xx(upd7907_state *cpustate);
    static void ADI_H_xx(upd7907_state *cpustate);
    static void ADI_L_xx(upd7907_state *cpustate);
    static void ADI_MKH_xx(upd7907_state *cpustate);
    static void ADI_MKL_xx(upd7907_state *cpustate);
    static void ADI_PA_xx(upd7907_state *cpustate);
    static void ADI_PB_xx(upd7907_state *cpustate);
    static void ADI_PC_xx(upd7907_state *cpustate);
    static void ADI_PD_xx(upd7907_state *cpustate);
    static void ADI_PF_xx(upd7907_state *cpustate);
    static void ADI_SMH_xx(upd7907_state *cpustate);
    static void ADI_TMM_xx(upd7907_state *cpustate);
    static void ADI_V_xx(upd7907_state *cpustate);
    static void ANAW_wa(upd7907_state *cpustate);
    static void ANAX_B(upd7907_state *cpustate);
    static void ANAX_D(upd7907_state *cpustate);
    static void ANAX_Dm(upd7907_state *cpustate);
    static void ANAX_Dp(upd7907_state *cpustate);
    static void ANAX_H(upd7907_state *cpustate);
    static void ANAX_Hm(upd7907_state *cpustate);
    static void ANAX_Hp(upd7907_state *cpustate);
    static void ANA_A_A(upd7907_state *cpustate);
    static void ANA_A_B(upd7907_state *cpustate);
    static void ANA_A_C(upd7907_state *cpustate);
    static void ANA_A_D(upd7907_state *cpustate);
    static void ANA_A_E(upd7907_state *cpustate);
    static void ANA_A_H(upd7907_state *cpustate);
    static void ANA_A_L(upd7907_state *cpustate);
    static void ANA_A_V(upd7907_state *cpustate);
    static void ANA_B_A(upd7907_state *cpustate);
    static void ANA_C_A(upd7907_state *cpustate);
    static void ANA_D_A(upd7907_state *cpustate);
    static void ANA_E_A(upd7907_state *cpustate);
    static void ANA_H_A(upd7907_state *cpustate);
    static void ANA_L_A(upd7907_state *cpustate);
    static void ANA_V_A(upd7907_state *cpustate);
    static void ANIW_wa_xx(upd7907_state *cpustate);
    static void ANI_ANM_xx(upd7907_state *cpustate);
    static void ANI_A_xx(upd7907_state *cpustate);
    static void ANI_B_xx(upd7907_state *cpustate);
    static void ANI_C_xx(upd7907_state *cpustate);
    static void ANI_D_xx(upd7907_state *cpustate);
    static void ANI_EOM_xx(upd7907_state *cpustate);
    static void ANI_E_xx(upd7907_state *cpustate);
    static void ANI_H_xx(upd7907_state *cpustate);
    static void ANI_L_xx(upd7907_state *cpustate);
    static void ANI_MKH_xx(upd7907_state *cpustate);
    static void ANI_MKL_xx(upd7907_state *cpustate);
    static void ANI_PA_xx(upd7907_state *cpustate);
    static void ANI_PB_xx(upd7907_state *cpustate);
    static void ANI_PC_xx(upd7907_state *cpustate);
    static void ANI_PD_xx(upd7907_state *cpustate);
    static void ANI_PF_xx(upd7907_state *cpustate);
    static void ANI_SMH_xx(upd7907_state *cpustate);
    static void ANI_TMM_xx(upd7907_state *cpustate);
    static void ANI_V_xx(upd7907_state *cpustate);
    static void BIT_0_wa(upd7907_state *cpustate);
    static void BIT_1_wa(upd7907_state *cpustate);
    static void BIT_2_wa(upd7907_state *cpustate);
    static void BIT_3_wa(upd7907_state *cpustate);
    static void BIT_4_wa(upd7907_state *cpustate);
    static void BIT_5_wa(upd7907_state *cpustate);
    static void BIT_6_wa(upd7907_state *cpustate);
    static void BIT_7_wa(upd7907_state *cpustate);
    static void BLOCK(upd7907_state *cpustate);
    static void CALB(upd7907_state *cpustate);
    static void CALF(upd7907_state *cpustate);
    static void CALL_w(upd7907_state *cpustate);
    static void CALT(upd7907_state *cpustate);
    static void CLC(upd7907_state *cpustate);
    static void CLR(upd7907_state *cpustate);
    static void DAA(upd7907_state *cpustate);
    static void DADC_EA_BC(upd7907_state *cpustate);
    static void DADC_EA_DE(upd7907_state *cpustate);
    static void DADC_EA_HL(upd7907_state *cpustate);
    static void DADDNC_EA_BC(upd7907_state *cpustate);
    static void DADDNC_EA_DE(upd7907_state *cpustate);
    static void DADDNC_EA_HL(upd7907_state *cpustate);
    static void DADD_EA_BC(upd7907_state *cpustate);
    static void DADD_EA_DE(upd7907_state *cpustate);
    static void DADD_EA_HL(upd7907_state *cpustate);
    static void DAN_EA_BC(upd7907_state *cpustate);
    static void DAN_EA_DE(upd7907_state *cpustate);
    static void DAN_EA_HL(upd7907_state *cpustate);
    static void DCRW_wa(upd7907_state *cpustate);
    static void DCR_A(upd7907_state *cpustate);
    static void DCR_B(upd7907_state *cpustate);
    static void DCR_C(upd7907_state *cpustate);
    static void DCX_BC(upd7907_state *cpustate);
    static void DCX_DE(upd7907_state *cpustate);
    static void DCX_EA(upd7907_state *cpustate);
    static void DCX_HL(upd7907_state *cpustate);
    static void DCX_SP(upd7907_state *cpustate);
    static void DEQ_EA_BC(upd7907_state *cpustate);
    static void DEQ_EA_DE(upd7907_state *cpustate);
    static void DEQ_EA_HL(upd7907_state *cpustate);
    static void DGT_EA_BC(upd7907_state *cpustate);
    static void DGT_EA_DE(upd7907_state *cpustate);
    static void DGT_EA_HL(upd7907_state *cpustate);
    static void DI(upd7907_state *cpustate);
    static void DIV_A(upd7907_state *cpustate);
    static void DIV_B(upd7907_state *cpustate);
    static void DIV_C(upd7907_state *cpustate);
    static void DLT_EA_BC(upd7907_state *cpustate);
    static void DLT_EA_DE(upd7907_state *cpustate);
    static void DLT_EA_HL(upd7907_state *cpustate);
    static void DMOV_BC_EA(upd7907_state *cpustate);
    static void DMOV_DE_EA(upd7907_state *cpustate);
    static void DMOV_EA_BC(upd7907_state *cpustate);
    static void DMOV_EA_DE(upd7907_state *cpustate);
    static void DMOV_EA_ECNT(upd7907_state *cpustate);
    static void DMOV_EA_ECPT(upd7907_state *cpustate);
    static void DMOV_EA_HL(upd7907_state *cpustate);
    static void DMOV_ETM0_EA(upd7907_state *cpustate);
    static void DMOV_ETM1_EA(upd7907_state *cpustate);
    static void DMOV_HL_EA(upd7907_state *cpustate);
    static void DNE_EA_BC(upd7907_state *cpustate);
    static void DNE_EA_DE(upd7907_state *cpustate);
    static void DNE_EA_HL(upd7907_state *cpustate);
    static void DOFF_EA_BC(upd7907_state *cpustate);
    static void DOFF_EA_DE(upd7907_state *cpustate);
    static void DOFF_EA_HL(upd7907_state *cpustate);
    static void DON_EA_BC(upd7907_state *cpustate);
    static void DON_EA_DE(upd7907_state *cpustate);
    static void DON_EA_HL(upd7907_state *cpustate);
    static void DOR_EA_BC(upd7907_state *cpustate);
    static void DOR_EA_DE(upd7907_state *cpustate);
    static void DOR_EA_HL(upd7907_state *cpustate);
    static void DRLL_EA(upd7907_state *cpustate);
    static void DRLR_EA(upd7907_state *cpustate);
    static void DSBB_EA_BC(upd7907_state *cpustate);
    static void DSBB_EA_DE(upd7907_state *cpustate);
    static void DSBB_EA_HL(upd7907_state *cpustate);
    static void DSLL_EA(upd7907_state *cpustate);
    static void DSLR_EA(upd7907_state *cpustate);
    static void DSUBNB_EA_BC(upd7907_state *cpustate);
    static void DSUBNB_EA_DE(upd7907_state *cpustate);
    static void DSUBNB_EA_HL(upd7907_state *cpustate);
    static void DSUB_EA_BC(upd7907_state *cpustate);
    static void DSUB_EA_DE(upd7907_state *cpustate);
    static void DSUB_EA_HL(upd7907_state *cpustate);
    static void DXR_EA_BC(upd7907_state *cpustate);
    static void DXR_EA_DE(upd7907_state *cpustate);
    static void DXR_EA_HL(upd7907_state *cpustate);
    static void EADD_EA_A(upd7907_state *cpustate);
    static void EADD_EA_B(upd7907_state *cpustate);
    static void EADD_EA_C(upd7907_state *cpustate);
    static void EI(upd7907_state *cpustate);
    static void EQAW_wa(upd7907_state *cpustate);
    static void EQAX_B(upd7907_state *cpustate);
    static void EQAX_D(upd7907_state *cpustate);
    static void EQAX_Dm(upd7907_state *cpustate);
    static void EQAX_Dp(upd7907_state *cpustate);
    static void EQAX_H(upd7907_state *cpustate);
    static void EQAX_Hm(upd7907_state *cpustate);
    static void EQAX_Hp(upd7907_state *cpustate);
    static void EQA_A_A(upd7907_state *cpustate);
    static void EQA_A_B(upd7907_state *cpustate);
    static void EQA_A_C(upd7907_state *cpustate);
    static void EQA_A_D(upd7907_state *cpustate);
    static void EQA_A_E(upd7907_state *cpustate);
    static void EQA_A_H(upd7907_state *cpustate);
    static void EQA_A_L(upd7907_state *cpustate);
    static void EQA_A_V(upd7907_state *cpustate);
    static void EQA_B_A(upd7907_state *cpustate);
    static void EQA_C_A(upd7907_state *cpustate);
    static void EQA_D_A(upd7907_state *cpustate);
    static void EQA_E_A(upd7907_state *cpustate);
    static void EQA_H_A(upd7907_state *cpustate);
    static void EQA_L_A(upd7907_state *cpustate);
    static void EQA_V_A(upd7907_state *cpustate);
    static void EQIW_wa_xx(upd7907_state *cpustate);
    static void EQI_ANM_xx(upd7907_state *cpustate);
    static void EQI_A_xx(upd7907_state *cpustate);
    static void EQI_B_xx(upd7907_state *cpustate);
    static void EQI_C_xx(upd7907_state *cpustate);
    static void EQI_D_xx(upd7907_state *cpustate);
    static void EQI_EOM_xx(upd7907_state *cpustate);
    static void EQI_E_xx(upd7907_state *cpustate);
    static void EQI_H_xx(upd7907_state *cpustate);
    static void EQI_L_xx(upd7907_state *cpustate);
    static void EQI_MKH_xx(upd7907_state *cpustate);
    static void EQI_MKL_xx(upd7907_state *cpustate);
    static void EQI_PA_xx(upd7907_state *cpustate);
    static void EQI_PB_xx(upd7907_state *cpustate);
    static void EQI_PC_xx(upd7907_state *cpustate);
    static void EQI_PD_xx(upd7907_state *cpustate);
    static void EQI_PF_xx(upd7907_state *cpustate);
    static void EQI_SMH_xx(upd7907_state *cpustate);
    static void EQI_TMM_xx(upd7907_state *cpustate);
    static void EQI_V_xx(upd7907_state *cpustate);
    static void ESUB_EA_A(upd7907_state *cpustate);
    static void ESUB_EA_B(upd7907_state *cpustate);
    static void ESUB_EA_C(upd7907_state *cpustate);
    static void EXA(upd7907_state *cpustate);
    static void EXH(upd7907_state *cpustate);
    static void EXX(upd7907_state *cpustate);
    static void EXR(upd7907_state *cpustate);
    static void GTAW_wa(upd7907_state *cpustate);
    static void GTAX_B(upd7907_state *cpustate);
    static void GTAX_D(upd7907_state *cpustate);
    static void GTAX_Dm(upd7907_state *cpustate);
    static void GTAX_Dp(upd7907_state *cpustate);
    static void GTAX_H(upd7907_state *cpustate);
    static void GTAX_Hm(upd7907_state *cpustate);
    static void GTAX_Hp(upd7907_state *cpustate);
    static void GTA_A_A(upd7907_state *cpustate);
    static void GTA_A_B(upd7907_state *cpustate);
    static void GTA_A_C(upd7907_state *cpustate);
    static void GTA_A_D(upd7907_state *cpustate);
    static void GTA_A_E(upd7907_state *cpustate);
    static void GTA_A_H(upd7907_state *cpustate);
    static void GTA_A_L(upd7907_state *cpustate);
    static void GTA_A_V(upd7907_state *cpustate);
    static void GTA_B_A(upd7907_state *cpustate);
    static void GTA_C_A(upd7907_state *cpustate);
    static void GTA_D_A(upd7907_state *cpustate);
    static void GTA_E_A(upd7907_state *cpustate);
    static void GTA_H_A(upd7907_state *cpustate);
    static void GTA_L_A(upd7907_state *cpustate);
    static void GTA_V_A(upd7907_state *cpustate);
    static void GTIW_wa_xx(upd7907_state *cpustate);
    static void GTI_ANM_xx(upd7907_state *cpustate);
    static void GTI_A_xx(upd7907_state *cpustate);
    static void GTI_B_xx(upd7907_state *cpustate);
    static void GTI_C_xx(upd7907_state *cpustate);
    static void GTI_D_xx(upd7907_state *cpustate);
    static void GTI_EOM_xx(upd7907_state *cpustate);
    static void GTI_E_xx(upd7907_state *cpustate);
    static void GTI_H_xx(upd7907_state *cpustate);
    static void GTI_L_xx(upd7907_state *cpustate);
    static void GTI_MKH_xx(upd7907_state *cpustate);
    static void GTI_MKL_xx(upd7907_state *cpustate);
    static void GTI_PA_xx(upd7907_state *cpustate);
    static void GTI_PB_xx(upd7907_state *cpustate);
    static void GTI_PC_xx(upd7907_state *cpustate);
    static void GTI_PD_xx(upd7907_state *cpustate);
    static void GTI_PF_xx(upd7907_state *cpustate);
    static void GTI_SMH_xx(upd7907_state *cpustate);
    static void GTI_TMM_xx(upd7907_state *cpustate);
    static void GTI_V_xx(upd7907_state *cpustate);
    static void HALT(upd7907_state *cpustate);
    static void IN(upd7907_state *cpustate);
    static void INRW_wa(upd7907_state *cpustate);
    static void INR_A(upd7907_state *cpustate);
    static void INR_B(upd7907_state *cpustate);
    static void INR_C(upd7907_state *cpustate);
    static void INX_BC(upd7907_state *cpustate);
    static void INX_DE(upd7907_state *cpustate);
    static void INX_EA(upd7907_state *cpustate);
    static void INX_HL(upd7907_state *cpustate);
    static void INX_SP(upd7907_state *cpustate);
    static void JB(upd7907_state *cpustate);
    static void JEA(upd7907_state *cpustate);
    static void JMP_w(upd7907_state *cpustate);
    static void JR(upd7907_state *cpustate);
    static void JRE(upd7907_state *cpustate);
    static void LBCD_w(upd7907_state *cpustate);
    static void LDAW_wa(upd7907_state *cpustate);
    static void LDAX_B(upd7907_state *cpustate);
    static void LDAX_D(upd7907_state *cpustate);
    static void LDAX_D_xx(upd7907_state *cpustate);
    static void LDAX_Dm(upd7907_state *cpustate);
    static void LDAX_Dp(upd7907_state *cpustate);
    static void LDAX_H(upd7907_state *cpustate);
    static void LDAX_H_A(upd7907_state *cpustate);
    static void LDAX_H_B(upd7907_state *cpustate);
    static void LDAX_H_EA(upd7907_state *cpustate);
    static void LDAX_H_xx(upd7907_state *cpustate);
    static void LDAX_Hm(upd7907_state *cpustate);
    static void LDAX_Hp(upd7907_state *cpustate);
    static void LDEAX_D(upd7907_state *cpustate);
    static void LDEAX_D_xx(upd7907_state *cpustate);
    static void LDEAX_Dp(upd7907_state *cpustate);
    static void LDEAX_H(upd7907_state *cpustate);
    static void LDEAX_H_A(upd7907_state *cpustate);
    static void LDEAX_H_B(upd7907_state *cpustate);
    static void LDEAX_H_EA(upd7907_state *cpustate);
    static void LDEAX_H_xx(upd7907_state *cpustate);
    static void LDEAX_Hp(upd7907_state *cpustate);
    static void LDED_w(upd7907_state *cpustate);
    static void LHLD_w(upd7907_state *cpustate);
    static void LSPD_w(upd7907_state *cpustate);
    static void LTAW_wa(upd7907_state *cpustate);
    static void LTAX_B(upd7907_state *cpustate);
    static void LTAX_D(upd7907_state *cpustate);
    static void LTAX_Dm(upd7907_state *cpustate);
    static void LTAX_Dp(upd7907_state *cpustate);
    static void LTAX_H(upd7907_state *cpustate);
    static void LTAX_Hm(upd7907_state *cpustate);
    static void LTAX_Hp(upd7907_state *cpustate);
    static void LTA_A_A(upd7907_state *cpustate);
    static void LTA_A_B(upd7907_state *cpustate);
    static void LTA_A_C(upd7907_state *cpustate);
    static void LTA_A_D(upd7907_state *cpustate);
    static void LTA_A_E(upd7907_state *cpustate);
    static void LTA_A_H(upd7907_state *cpustate);
    static void LTA_A_L(upd7907_state *cpustate);
    static void LTA_A_V(upd7907_state *cpustate);
    static void LTA_B_A(upd7907_state *cpustate);
    static void LTA_C_A(upd7907_state *cpustate);
    static void LTA_D_A(upd7907_state *cpustate);
    static void LTA_E_A(upd7907_state *cpustate);
    static void LTA_H_A(upd7907_state *cpustate);
    static void LTA_L_A(upd7907_state *cpustate);
    static void LTA_V_A(upd7907_state *cpustate);
    static void LTIW_wa_xx(upd7907_state *cpustate);
    static void LTI_ANM_xx(upd7907_state *cpustate);
    static void LTI_A_xx(upd7907_state *cpustate);
    static void LTI_B_xx(upd7907_state *cpustate);
    static void LTI_C_xx(upd7907_state *cpustate);
    static void LTI_D_xx(upd7907_state *cpustate);
    static void LTI_EOM_xx(upd7907_state *cpustate);
    static void LTI_E_xx(upd7907_state *cpustate);
    static void LTI_H_xx(upd7907_state *cpustate);
    static void LTI_L_xx(upd7907_state *cpustate);
    static void LTI_MKH_xx(upd7907_state *cpustate);
    static void LTI_MKL_xx(upd7907_state *cpustate);
    static void LTI_PA_xx(upd7907_state *cpustate);
    static void LTI_PB_xx(upd7907_state *cpustate);
    static void LTI_PC_xx(upd7907_state *cpustate);
    static void LTI_PD_xx(upd7907_state *cpustate);
    static void LTI_PF_xx(upd7907_state *cpustate);
    static void LTI_SMH_xx(upd7907_state *cpustate);
    static void LTI_TMM_xx(upd7907_state *cpustate);
    static void LTI_V_xx(upd7907_state *cpustate);
    static void LXI_B_w(upd7907_state *cpustate);
    static void LXI_D_w(upd7907_state *cpustate);
    static void LXI_EA_s(upd7907_state *cpustate);
    static void LXI_H_w(upd7907_state *cpustate);
    static void LXI_S_w(upd7907_state *cpustate);
    static void MOV_ANM_A(upd7907_state *cpustate);
    static void MOV_A_ANM(upd7907_state *cpustate);
    static void MOV_A_B(upd7907_state *cpustate);
    static void MOV_A_C(upd7907_state *cpustate);
    static void MOV_A_CR0(upd7907_state *cpustate);
    static void MOV_A_CR1(upd7907_state *cpustate);
    static void MOV_A_CR2(upd7907_state *cpustate);
    static void MOV_A_CR3(upd7907_state *cpustate);
    static void MOV_A_D(upd7907_state *cpustate);
    static void MOV_A_E(upd7907_state *cpustate);
    static void MOV_A_EAH(upd7907_state *cpustate);
    static void MOV_A_EAL(upd7907_state *cpustate);
    static void MOV_A_EOM(upd7907_state *cpustate);
    static void MOV_A_H(upd7907_state *cpustate);
    static void MOV_A_L(upd7907_state *cpustate);
    static void MOV_A_MKH(upd7907_state *cpustate);
    static void MOV_A_MKL(upd7907_state *cpustate);
    static void MOV_A_PA(upd7907_state *cpustate);
    static void MOV_A_PB(upd7907_state *cpustate);
    static void MOV_A_PC(upd7907_state *cpustate);
    static void MOV_A_PD(upd7907_state *cpustate);
    static void MOV_A_PF(upd7907_state *cpustate);
    static void MOV_A_RXB(upd7907_state *cpustate);
    static void MOV_A_S(upd7907_state *cpustate);
    static void MOV_A_SMH(upd7907_state *cpustate);
    static void MOV_A_TMM(upd7907_state *cpustate);
    static void MOV_A_PT(upd7907_state *cpustate);
    static void MOV_A_w(upd7907_state *cpustate);
    static void MOV_B_A(upd7907_state *cpustate);
    static void MOV_B_w(upd7907_state *cpustate);
    static void MOV_C_A(upd7907_state *cpustate);
    static void MOV_C_w(upd7907_state *cpustate);
    static void MOV_D_A(upd7907_state *cpustate);
    static void MOV_D_w(upd7907_state *cpustate);
    static void MOV_EAH_A(upd7907_state *cpustate);
    static void MOV_EAL_A(upd7907_state *cpustate);
    static void MOV_EOM_A(upd7907_state *cpustate);
    static void MOV_ETMM_A(upd7907_state *cpustate);
    static void MOV_E_A(upd7907_state *cpustate);
    static void MOV_E_w(upd7907_state *cpustate);
    static void MOV_H_A(upd7907_state *cpustate);
    static void MOV_H_w(upd7907_state *cpustate);
    static void MOV_L_A(upd7907_state *cpustate);
    static void MOV_L_w(upd7907_state *cpustate);
    static void MOV_MA_A(upd7907_state *cpustate);
    static void MOV_MB_A(upd7907_state *cpustate);
    static void MOV_MCC_A(upd7907_state *cpustate);
    static void MOV_MC_A(upd7907_state *cpustate);
    static void MOV_MF_A(upd7907_state *cpustate);
    static void MOV_MKH_A(upd7907_state *cpustate);
    static void MOV_MKL_A(upd7907_state *cpustate);
    static void MOV_MM_A(upd7907_state *cpustate);
    static void MOV_PA_A(upd7907_state *cpustate);
    static void MOV_PB_A(upd7907_state *cpustate);
    static void MOV_PC_A(upd7907_state *cpustate);
    static void MOV_PD_A(upd7907_state *cpustate);
    static void MOV_PF_A(upd7907_state *cpustate);
    static void MOV_S_A(upd7907_state *cpustate);
    static void MOV_SMH_A(upd7907_state *cpustate);
    static void MOV_SML_A(upd7907_state *cpustate);
    static void MOV_TM0_A(upd7907_state *cpustate);
    static void MOV_TM1_A(upd7907_state *cpustate);
    static void MOV_TMM_A(upd7907_state *cpustate);
    static void MOV_TXB_A(upd7907_state *cpustate);
    static void MOV_V_w(upd7907_state *cpustate);
    static void MOV_ZCM_A(upd7907_state *cpustate);
    static void MOV_w_A(upd7907_state *cpustate);
    static void MOV_w_B(upd7907_state *cpustate);
    static void MOV_w_C(upd7907_state *cpustate);
    static void MOV_w_D(upd7907_state *cpustate);
    static void MOV_w_E(upd7907_state *cpustate);
    static void MOV_w_H(upd7907_state *cpustate);
    static void MOV_w_L(upd7907_state *cpustate);
    static void MOV_w_V(upd7907_state *cpustate);
    static void MUL_A(upd7907_state *cpustate);
    static void MUL_B(upd7907_state *cpustate);
    static void MUL_C(upd7907_state *cpustate);
    static void MVIW_wa_xx(upd7907_state *cpustate);
    static void MVIX_BC_xx(upd7907_state *cpustate);
    static void MVIX_DE_xx(upd7907_state *cpustate);
    static void MVIX_HL_xx(upd7907_state *cpustate);
    static void MVI_ANM_xx(upd7907_state *cpustate);
    static void MVI_A_xx(upd7907_state *cpustate);
    static void MVI_B_xx(upd7907_state *cpustate);
    static void MVI_C_xx(upd7907_state *cpustate);
    static void MVI_D_xx(upd7907_state *cpustate);
    static void MVI_EOM_xx(upd7907_state *cpustate);
    static void MVI_E_xx(upd7907_state *cpustate);
    static void MVI_H_xx(upd7907_state *cpustate);
    static void MVI_L_xx(upd7907_state *cpustate);
    static void MVI_MKH_xx(upd7907_state *cpustate);
    static void MVI_MKL_xx(upd7907_state *cpustate);
    static void MVI_PA_xx(upd7907_state *cpustate);
    static void MVI_PB_xx(upd7907_state *cpustate);
    static void MVI_PC_xx(upd7907_state *cpustate);
    static void MVI_PD_xx(upd7907_state *cpustate);
    static void MVI_PF_xx(upd7907_state *cpustate);
    static void MVI_SMH_xx(upd7907_state *cpustate);
    static void MVI_TMM_xx(upd7907_state *cpustate);
    static void MVI_V_xx(upd7907_state *cpustate);
    static void NEAW_wa(upd7907_state *cpustate);
    static void NEAX_B(upd7907_state *cpustate);
    static void NEAX_D(upd7907_state *cpustate);
    static void NEAX_Dm(upd7907_state *cpustate);
    static void NEAX_Dp(upd7907_state *cpustate);
    static void NEAX_H(upd7907_state *cpustate);
    static void NEAX_Hm(upd7907_state *cpustate);
    static void NEAX_Hp(upd7907_state *cpustate);
    static void NEA_A_A(upd7907_state *cpustate);
    static void NEA_A_B(upd7907_state *cpustate);
    static void NEA_A_C(upd7907_state *cpustate);
    static void NEA_A_D(upd7907_state *cpustate);
    static void NEA_A_E(upd7907_state *cpustate);
    static void NEA_A_H(upd7907_state *cpustate);
    static void NEA_A_L(upd7907_state *cpustate);
    static void NEA_A_V(upd7907_state *cpustate);
    static void NEA_B_A(upd7907_state *cpustate);
    static void NEA_C_A(upd7907_state *cpustate);
    static void NEA_D_A(upd7907_state *cpustate);
    static void NEA_E_A(upd7907_state *cpustate);
    static void NEA_H_A(upd7907_state *cpustate);
    static void NEA_L_A(upd7907_state *cpustate);
    static void NEA_V_A(upd7907_state *cpustate);
    static void NEGA(upd7907_state *cpustate);
    static void NEIW_wa_xx(upd7907_state *cpustate);
    static void NEI_ANM_xx(upd7907_state *cpustate);
    static void NEI_A_xx(upd7907_state *cpustate);
    static void NEI_B_xx(upd7907_state *cpustate);
    static void NEI_C_xx(upd7907_state *cpustate);
    static void NEI_D_xx(upd7907_state *cpustate);
    static void NEI_EOM_xx(upd7907_state *cpustate);
    static void NEI_E_xx(upd7907_state *cpustate);
    static void NEI_H_xx(upd7907_state *cpustate);
    static void NEI_L_xx(upd7907_state *cpustate);
    static void NEI_MKH_xx(upd7907_state *cpustate);
    static void NEI_MKL_xx(upd7907_state *cpustate);
    static void NEI_PA_xx(upd7907_state *cpustate);
    static void NEI_PB_xx(upd7907_state *cpustate);
    static void NEI_PC_xx(upd7907_state *cpustate);
    static void NEI_PD_xx(upd7907_state *cpustate);
    static void NEI_PF_xx(upd7907_state *cpustate);
    static void NEI_SMH_xx(upd7907_state *cpustate);
    static void NEI_TMM_xx(upd7907_state *cpustate);
    static void NEI_V_xx(upd7907_state *cpustate);
    static void NOP(upd7907_state *cpustate);
    static void OFFAW_wa(upd7907_state *cpustate);
    static void OFFAX_B(upd7907_state *cpustate);
    static void OFFAX_D(upd7907_state *cpustate);
    static void OFFAX_Dm(upd7907_state *cpustate);
    static void OFFAX_Dp(upd7907_state *cpustate);
    static void OFFAX_H(upd7907_state *cpustate);
    static void OFFAX_Hm(upd7907_state *cpustate);
    static void OFFAX_Hp(upd7907_state *cpustate);
    static void OFFA_A_A(upd7907_state *cpustate);
    static void OFFA_A_B(upd7907_state *cpustate);
    static void OFFA_A_C(upd7907_state *cpustate);
    static void OFFA_A_D(upd7907_state *cpustate);
    static void OFFA_A_E(upd7907_state *cpustate);
    static void OFFA_A_H(upd7907_state *cpustate);
    static void OFFA_A_L(upd7907_state *cpustate);
    static void OFFA_A_V(upd7907_state *cpustate);
    static void OFFIW_wa_xx(upd7907_state *cpustate);
    static void OFFI_ANM_xx(upd7907_state *cpustate);
    static void OFFI_A_xx(upd7907_state *cpustate);
    static void OFFI_B_xx(upd7907_state *cpustate);
    static void OFFI_C_xx(upd7907_state *cpustate);
    static void OFFI_D_xx(upd7907_state *cpustate);
    static void OFFI_EOM_xx(upd7907_state *cpustate);
    static void OFFI_E_xx(upd7907_state *cpustate);
    static void OFFI_H_xx(upd7907_state *cpustate);
    static void OFFI_L_xx(upd7907_state *cpustate);
    static void OFFI_MKH_xx(upd7907_state *cpustate);
    static void OFFI_MKL_xx(upd7907_state *cpustate);
    static void OFFI_PA_xx(upd7907_state *cpustate);
    static void OFFI_PB_xx(upd7907_state *cpustate);
    static void OFFI_PC_xx(upd7907_state *cpustate);
    static void OFFI_PD_xx(upd7907_state *cpustate);
    static void OFFI_PF_xx(upd7907_state *cpustate);
    static void OFFI_SMH_xx(upd7907_state *cpustate);
    static void OFFI_TMM_xx(upd7907_state *cpustate);
    static void OFFI_V_xx(upd7907_state *cpustate);
    static void ONAW_wa(upd7907_state *cpustate);
    static void ONAX_B(upd7907_state *cpustate);
    static void ONAX_D(upd7907_state *cpustate);
    static void ONAX_Dm(upd7907_state *cpustate);
    static void ONAX_Dp(upd7907_state *cpustate);
    static void ONAX_H(upd7907_state *cpustate);
    static void ONAX_Hm(upd7907_state *cpustate);
    static void ONAX_Hp(upd7907_state *cpustate);
    static void ONA_A_A(upd7907_state *cpustate);
    static void ONA_A_B(upd7907_state *cpustate);
    static void ONA_A_C(upd7907_state *cpustate);
    static void ONA_A_D(upd7907_state *cpustate);
    static void ONA_A_E(upd7907_state *cpustate);
    static void ONA_A_H(upd7907_state *cpustate);
    static void ONA_A_L(upd7907_state *cpustate);
    static void ONA_A_V(upd7907_state *cpustate);
    static void ONIW_wa_xx(upd7907_state *cpustate);
    static void ONI_ANM_xx(upd7907_state *cpustate);
    static void ONI_A_xx(upd7907_state *cpustate);
    static void ONI_B_xx(upd7907_state *cpustate);
    static void ONI_C_xx(upd7907_state *cpustate);
    static void ONI_D_xx(upd7907_state *cpustate);
    static void ONI_EOM_xx(upd7907_state *cpustate);
    static void ONI_E_xx(upd7907_state *cpustate);
    static void ONI_H_xx(upd7907_state *cpustate);
    static void ONI_L_xx(upd7907_state *cpustate);
    static void ONI_MKH_xx(upd7907_state *cpustate);
    static void ONI_MKL_xx(upd7907_state *cpustate);
    static void ONI_PA_xx(upd7907_state *cpustate);
    static void ONI_PB_xx(upd7907_state *cpustate);
    static void ONI_PC_xx(upd7907_state *cpustate);
    static void ONI_PD_xx(upd7907_state *cpustate);
    static void ONI_PF_xx(upd7907_state *cpustate);
    static void ONI_SMH_xx(upd7907_state *cpustate);
    static void ONI_TMM_xx(upd7907_state *cpustate);
    static void ONI_V_xx(upd7907_state *cpustate);
    static void ORAW_wa(upd7907_state *cpustate);
    static void ORAX_B(upd7907_state *cpustate);
    static void ORAX_D(upd7907_state *cpustate);
    static void ORAX_Dm(upd7907_state *cpustate);
    static void ORAX_Dp(upd7907_state *cpustate);
    static void ORAX_H(upd7907_state *cpustate);
    static void ORAX_Hm(upd7907_state *cpustate);
    static void ORAX_Hp(upd7907_state *cpustate);
    static void ORA_A_A(upd7907_state *cpustate);
    static void ORA_A_B(upd7907_state *cpustate);
    static void ORA_A_C(upd7907_state *cpustate);
    static void ORA_A_D(upd7907_state *cpustate);
    static void ORA_A_E(upd7907_state *cpustate);
    static void ORA_A_H(upd7907_state *cpustate);
    static void ORA_A_L(upd7907_state *cpustate);
    static void ORA_A_V(upd7907_state *cpustate);
    static void ORA_B_A(upd7907_state *cpustate);
    static void ORA_C_A(upd7907_state *cpustate);
    static void ORA_D_A(upd7907_state *cpustate);
    static void ORA_E_A(upd7907_state *cpustate);
    static void ORA_H_A(upd7907_state *cpustate);
    static void ORA_L_A(upd7907_state *cpustate);
    static void ORA_V_A(upd7907_state *cpustate);
    static void ORIW_wa_xx(upd7907_state *cpustate);
    static void ORI_ANM_xx(upd7907_state *cpustate);
    static void ORI_A_xx(upd7907_state *cpustate);
    static void ORI_B_xx(upd7907_state *cpustate);
    static void ORI_C_xx(upd7907_state *cpustate);
    static void ORI_D_xx(upd7907_state *cpustate);
    static void ORI_EOM_xx(upd7907_state *cpustate);
    static void ORI_E_xx(upd7907_state *cpustate);
    static void ORI_H_xx(upd7907_state *cpustate);
    static void ORI_L_xx(upd7907_state *cpustate);
    static void ORI_MKH_xx(upd7907_state *cpustate);
    static void ORI_MKL_xx(upd7907_state *cpustate);
    static void ORI_PA_xx(upd7907_state *cpustate);
    static void ORI_PB_xx(upd7907_state *cpustate);
    static void ORI_PC_xx(upd7907_state *cpustate);
    static void ORI_PD_xx(upd7907_state *cpustate);
    static void ORI_PF_xx(upd7907_state *cpustate);
    static void ORI_SMH_xx(upd7907_state *cpustate);
    static void ORI_TMM_xx(upd7907_state *cpustate);
    static void ORI_V_xx(upd7907_state *cpustate);
    static void OUT(upd7907_state *cpustate);
    static void PEN(upd7907_state *cpustate);
    static void PER(upd7907_state *cpustate);
    static void PEX(upd7907_state *cpustate);
    static void POP_BC(upd7907_state *cpustate);
    static void POP_DE(upd7907_state *cpustate);
    static void POP_EA(upd7907_state *cpustate);
    static void POP_HL(upd7907_state *cpustate);
    static void POP_VA(upd7907_state *cpustate);
    static void PRE_48(upd7907_state *cpustate);
    static void PRE_4C(upd7907_state *cpustate);
    static void PRE_4D(upd7907_state *cpustate);
    static void PRE_60(upd7907_state *cpustate);
    static void PRE_64(upd7907_state *cpustate);
    static void PRE_70(upd7907_state *cpustate);
    static void PRE_74(upd7907_state *cpustate);
    static void PUSH_BC(upd7907_state *cpustate);
    static void PUSH_DE(upd7907_state *cpustate);
    static void PUSH_EA(upd7907_state *cpustate);
    static void PUSH_HL(upd7907_state *cpustate);
    static void PUSH_VA(upd7907_state *cpustate);
    static void RET(upd7907_state *cpustate);
    static void RETI(upd7907_state *cpustate);
    static void RETS(upd7907_state *cpustate);
    static void RLD(upd7907_state *cpustate);
    static void RLL_A(upd7907_state *cpustate);
    static void RLL_B(upd7907_state *cpustate);
    static void RLL_C(upd7907_state *cpustate);
    static void RLR_A(upd7907_state *cpustate);
    static void RLR_B(upd7907_state *cpustate);
    static void RLR_C(upd7907_state *cpustate);
    static void RRD(upd7907_state *cpustate);
    static void SBBW_wa(upd7907_state *cpustate);
    static void SBBX_B(upd7907_state *cpustate);
    static void SBBX_D(upd7907_state *cpustate);
    static void SBBX_Dm(upd7907_state *cpustate);
    static void SBBX_Dp(upd7907_state *cpustate);
    static void SBBX_H(upd7907_state *cpustate);
    static void SBBX_Hm(upd7907_state *cpustate);
    static void SBBX_Hp(upd7907_state *cpustate);
    static void SBB_A_A(upd7907_state *cpustate);
    static void SBB_A_B(upd7907_state *cpustate);
    static void SBB_A_C(upd7907_state *cpustate);
    static void SBB_A_D(upd7907_state *cpustate);
    static void SBB_A_E(upd7907_state *cpustate);
    static void SBB_A_H(upd7907_state *cpustate);
    static void SBB_A_L(upd7907_state *cpustate);
    static void SBB_A_V(upd7907_state *cpustate);
    static void SBB_B_A(upd7907_state *cpustate);
    static void SBB_C_A(upd7907_state *cpustate);
    static void SBB_D_A(upd7907_state *cpustate);
    static void SBB_E_A(upd7907_state *cpustate);
    static void SBB_H_A(upd7907_state *cpustate);
    static void SBB_L_A(upd7907_state *cpustate);
    static void SBB_V_A(upd7907_state *cpustate);
    static void SBCD_w(upd7907_state *cpustate);
    static void SBI_ANM_xx(upd7907_state *cpustate);
    static void SBI_A_xx(upd7907_state *cpustate);
    static void SBI_B_xx(upd7907_state *cpustate);
    static void SBI_C_xx(upd7907_state *cpustate);
    static void SBI_D_xx(upd7907_state *cpustate);
    static void SBI_EOM_xx(upd7907_state *cpustate);
    static void SBI_E_xx(upd7907_state *cpustate);
    static void SBI_H_xx(upd7907_state *cpustate);
    static void SBI_L_xx(upd7907_state *cpustate);
    static void SBI_MKH_xx(upd7907_state *cpustate);
    static void SBI_MKL_xx(upd7907_state *cpustate);
    static void SBI_PA_xx(upd7907_state *cpustate);
    static void SBI_PB_xx(upd7907_state *cpustate);
    static void SBI_PC_xx(upd7907_state *cpustate);
    static void SBI_PD_xx(upd7907_state *cpustate);
    static void SBI_PF_xx(upd7907_state *cpustate);
    static void SBI_SMH_xx(upd7907_state *cpustate);
    static void SBI_TMM_xx(upd7907_state *cpustate);
    static void SBI_V_xx(upd7907_state *cpustate);
    static void SDED_w(upd7907_state *cpustate);
    static void SETB(upd7907_state *cpustate);
    static void SHLD_w(upd7907_state *cpustate);
    static void SIO(upd7907_state *cpustate);
    static void SK_bit(upd7907_state *cpustate);
    static void SKN_bit(upd7907_state *cpustate);
    static void SKIT_AN4(upd7907_state *cpustate);
    static void SKIT_AN5(upd7907_state *cpustate);
    static void SKIT_AN6(upd7907_state *cpustate);
    static void SKIT_AN7(upd7907_state *cpustate);
    static void SKIT_ER(upd7907_state *cpustate);
    static void SKIT_F0(upd7907_state *cpustate);
    static void SKIT_F1(upd7907_state *cpustate);
    static void SKIT_F2(upd7907_state *cpustate);
    static void SKIT_FAD(upd7907_state *cpustate);
    static void SKIT_FE0(upd7907_state *cpustate);
    static void SKIT_FE1(upd7907_state *cpustate);
    static void SKIT_FEIN(upd7907_state *cpustate);
    static void SKIT_FSR(upd7907_state *cpustate);
    static void SKIT_FST(upd7907_state *cpustate);
    static void SKIT_FT0(upd7907_state *cpustate);
    static void SKIT_FT1(upd7907_state *cpustate);
    static void SKIT_NMI(upd7907_state *cpustate);
    static void SKIT_OV(upd7907_state *cpustate);
    static void SKIT_SB(upd7907_state *cpustate);
    static void SKNIT_AN4(upd7907_state *cpustate);
    static void SKNIT_AN5(upd7907_state *cpustate);
    static void SKNIT_AN6(upd7907_state *cpustate);
    static void SKNIT_AN7(upd7907_state *cpustate);
    static void SKNIT_ER(upd7907_state *cpustate);
    static void SKNIT_F0(upd7907_state *cpustate);
    static void SKNIT_F1(upd7907_state *cpustate);
    static void SKNIT_F2(upd7907_state *cpustate);
    static void SKNIT_FAD(upd7907_state *cpustate);
    static void SKNIT_FE0(upd7907_state *cpustate);
    static void SKNIT_FE1(upd7907_state *cpustate);
    static void SKNIT_FEIN(upd7907_state *cpustate);
    static void SKNIT_FSR(upd7907_state *cpustate);
    static void SKNIT_FST(upd7907_state *cpustate);
    static void SKNIT_FT0(upd7907_state *cpustate);
    static void SKNIT_FT1(upd7907_state *cpustate);
    static void SKNIT_NMI(upd7907_state *cpustate);
    static void SKNIT_OV(upd7907_state *cpustate);
    static void SKNIT_SB(upd7907_state *cpustate);
    static void SKN_CY(upd7907_state *cpustate);
    static void SKN_HC(upd7907_state *cpustate);
    static void SKN_NV(upd7907_state *cpustate);
    static void SKN_Z(upd7907_state *cpustate);
    static void SK_CY(upd7907_state *cpustate);
    static void SK_HC(upd7907_state *cpustate);
    static void SK_NV(upd7907_state *cpustate);
    static void SK_Z(upd7907_state *cpustate);
    static void SLLC_A(upd7907_state *cpustate);
    static void SLLC_B(upd7907_state *cpustate);
    static void SLLC_C(upd7907_state *cpustate);
    static void SLL_A(upd7907_state *cpustate);
    static void SLL_B(upd7907_state *cpustate);
    static void SLL_C(upd7907_state *cpustate);
    static void SLRC_A(upd7907_state *cpustate);
    static void SLRC_B(upd7907_state *cpustate);
    static void SLRC_C(upd7907_state *cpustate);
    static void SLR_A(upd7907_state *cpustate);
    static void SLR_B(upd7907_state *cpustate);
    static void SLR_C(upd7907_state *cpustate);
    static void SOFTI(upd7907_state *cpustate);
    static void SSPD_w(upd7907_state *cpustate);
    static void STAW_wa(upd7907_state *cpustate);
    static void STAX_B(upd7907_state *cpustate);
    static void STAX_D(upd7907_state *cpustate);
    static void STAX_D_xx(upd7907_state *cpustate);
    static void STAX_Dm(upd7907_state *cpustate);
    static void STAX_Dp(upd7907_state *cpustate);
    static void STAX_H(upd7907_state *cpustate);
    static void STAX_H_A(upd7907_state *cpustate);
    static void STAX_H_B(upd7907_state *cpustate);
    static void STAX_H_EA(upd7907_state *cpustate);
    static void STAX_H_xx(upd7907_state *cpustate);
    static void STAX_Hm(upd7907_state *cpustate);
    static void STAX_Hp(upd7907_state *cpustate);
    static void STC(upd7907_state *cpustate);
    static void STEAX_D(upd7907_state *cpustate);
    static void STEAX_D_xx(upd7907_state *cpustate);
    static void STEAX_Dp(upd7907_state *cpustate);
    static void STEAX_H(upd7907_state *cpustate);
    static void STEAX_H_A(upd7907_state *cpustate);
    static void STEAX_H_B(upd7907_state *cpustate);
    static void STEAX_H_EA(upd7907_state *cpustate);
    static void STEAX_H_xx(upd7907_state *cpustate);
    static void STEAX_Hp(upd7907_state *cpustate);
    static void STM(upd7907_state *cpustate);
    static void STOP(upd7907_state *cpustate);
    static void SUBNBW_wa(upd7907_state *cpustate);
    static void SUBNBX_B(upd7907_state *cpustate);
    static void SUBNBX_D(upd7907_state *cpustate);
    static void SUBNBX_Dm(upd7907_state *cpustate);
    static void SUBNBX_Dp(upd7907_state *cpustate);
    static void SUBNBX_H(upd7907_state *cpustate);
    static void SUBNBX_Hm(upd7907_state *cpustate);
    static void SUBNBX_Hp(upd7907_state *cpustate);
    static void SUBNB_A_A(upd7907_state *cpustate);
    static void SUBNB_A_B(upd7907_state *cpustate);
    static void SUBNB_A_C(upd7907_state *cpustate);
    static void SUBNB_A_D(upd7907_state *cpustate);
    static void SUBNB_A_E(upd7907_state *cpustate);
    static void SUBNB_A_H(upd7907_state *cpustate);
    static void SUBNB_A_L(upd7907_state *cpustate);
    static void SUBNB_A_V(upd7907_state *cpustate);
    static void SUBNB_B_A(upd7907_state *cpustate);
    static void SUBNB_C_A(upd7907_state *cpustate);
    static void SUBNB_D_A(upd7907_state *cpustate);
    static void SUBNB_E_A(upd7907_state *cpustate);
    static void SUBNB_H_A(upd7907_state *cpustate);
    static void SUBNB_L_A(upd7907_state *cpustate);
    static void SUBNB_V_A(upd7907_state *cpustate);
    static void SUBW_wa(upd7907_state *cpustate);
    static void SUBX_B(upd7907_state *cpustate);
    static void SUBX_D(upd7907_state *cpustate);
    static void SUBX_Dm(upd7907_state *cpustate);
    static void SUBX_Dp(upd7907_state *cpustate);
    static void SUBX_H(upd7907_state *cpustate);
    static void SUBX_Hm(upd7907_state *cpustate);
    static void SUBX_Hp(upd7907_state *cpustate);
    static void SUB_A_A(upd7907_state *cpustate);
    static void SUB_A_B(upd7907_state *cpustate);
    static void SUB_A_C(upd7907_state *cpustate);
    static void SUB_A_D(upd7907_state *cpustate);
    static void SUB_A_E(upd7907_state *cpustate);
    static void SUB_A_H(upd7907_state *cpustate);
    static void SUB_A_L(upd7907_state *cpustate);
    static void SUB_A_V(upd7907_state *cpustate);
    static void SUB_B_A(upd7907_state *cpustate);
    static void SUB_C_A(upd7907_state *cpustate);
    static void SUB_D_A(upd7907_state *cpustate);
    static void SUB_E_A(upd7907_state *cpustate);
    static void SUB_H_A(upd7907_state *cpustate);
    static void SUB_L_A(upd7907_state *cpustate);
    static void SUB_V_A(upd7907_state *cpustate);
    static void SUINB_ANM_xx(upd7907_state *cpustate);
    static void SUINB_A_xx(upd7907_state *cpustate);
    static void SUINB_B_xx(upd7907_state *cpustate);
    static void SUINB_C_xx(upd7907_state *cpustate);
    static void SUINB_D_xx(upd7907_state *cpustate);
    static void SUINB_EOM_xx(upd7907_state *cpustate);
    static void SUINB_E_xx(upd7907_state *cpustate);
    static void SUINB_H_xx(upd7907_state *cpustate);
    static void SUINB_L_xx(upd7907_state *cpustate);
    static void SUINB_MKH_xx(upd7907_state *cpustate);
    static void SUINB_MKL_xx(upd7907_state *cpustate);
    static void SUINB_PA_xx(upd7907_state *cpustate);
    static void SUINB_PB_xx(upd7907_state *cpustate);
    static void SUINB_PC_xx(upd7907_state *cpustate);
    static void SUINB_PD_xx(upd7907_state *cpustate);
    static void SUINB_PF_xx(upd7907_state *cpustate);
    static void SUINB_SMH_xx(upd7907_state *cpustate);
    static void SUINB_TMM_xx(upd7907_state *cpustate);
    static void SUINB_V_xx(upd7907_state *cpustate);
    static void SUI_ANM_xx(upd7907_state *cpustate);
    static void SUI_A_xx(upd7907_state *cpustate);
    static void SUI_B_xx(upd7907_state *cpustate);
    static void SUI_C_xx(upd7907_state *cpustate);
    static void SUI_D_xx(upd7907_state *cpustate);
    static void SUI_EOM_xx(upd7907_state *cpustate);
    static void SUI_E_xx(upd7907_state *cpustate);
    static void SUI_H_xx(upd7907_state *cpustate);
    static void SUI_L_xx(upd7907_state *cpustate);
    static void SUI_MKH_xx(upd7907_state *cpustate);
    static void SUI_MKL_xx(upd7907_state *cpustate);
    static void SUI_PA_xx(upd7907_state *cpustate);
    static void SUI_PB_xx(upd7907_state *cpustate);
    static void SUI_PC_xx(upd7907_state *cpustate);
    static void SUI_PD_xx(upd7907_state *cpustate);
    static void SUI_PF_xx(upd7907_state *cpustate);
    static void SUI_SMH_xx(upd7907_state *cpustate);
    static void SUI_TMM_xx(upd7907_state *cpustate);
    static void SUI_V_xx(upd7907_state *cpustate);
    static void TABLE(upd7907_state *cpustate);
    static void XRAW_wa(upd7907_state *cpustate);
    static void XRAX_B(upd7907_state *cpustate);
    static void XRAX_D(upd7907_state *cpustate);
    static void XRAX_Dm(upd7907_state *cpustate);
    static void XRAX_Dp(upd7907_state *cpustate);
    static void XRAX_H(upd7907_state *cpustate);
    static void XRAX_Hm(upd7907_state *cpustate);
    static void XRAX_Hp(upd7907_state *cpustate);
    static void XRA_A_A(upd7907_state *cpustate);
    static void XRA_A_B(upd7907_state *cpustate);
    static void XRA_A_C(upd7907_state *cpustate);
    static void XRA_A_D(upd7907_state *cpustate);
    static void XRA_A_E(upd7907_state *cpustate);
    static void XRA_A_H(upd7907_state *cpustate);
    static void XRA_A_L(upd7907_state *cpustate);
    static void XRA_A_V(upd7907_state *cpustate);
    static void XRA_B_A(upd7907_state *cpustate);
    static void XRA_C_A(upd7907_state *cpustate);
    static void XRA_D_A(upd7907_state *cpustate);
    static void XRA_E_A(upd7907_state *cpustate);
    static void XRA_H_A(upd7907_state *cpustate);
    static void XRA_L_A(upd7907_state *cpustate);
    static void XRA_V_A(upd7907_state *cpustate);
    static void XRI_ANM_xx(upd7907_state *cpustate);
    static void XRI_A_xx(upd7907_state *cpustate);
    static void XRI_B_xx(upd7907_state *cpustate);
    static void XRI_C_xx(upd7907_state *cpustate);
    static void XRI_D_xx(upd7907_state *cpustate);
    static void XRI_EOM_xx(upd7907_state *cpustate);
    static void XRI_E_xx(upd7907_state *cpustate);
    static void XRI_H_xx(upd7907_state *cpustate);
    static void XRI_L_xx(upd7907_state *cpustate);
    static void XRI_MKH_xx(upd7907_state *cpustate);
    static void XRI_MKL_xx(upd7907_state *cpustate);
    static void XRI_PA_xx(upd7907_state *cpustate);
    static void XRI_PB_xx(upd7907_state *cpustate);
    static void XRI_PC_xx(upd7907_state *cpustate);
    static void XRI_PD_xx(upd7907_state *cpustate);
    static void XRI_PF_xx(upd7907_state *cpustate);
    static void XRI_SMH_xx(upd7907_state *cpustate);
    static void XRI_TMM_xx(upd7907_state *cpustate);
    static void XRI_V_xx(upd7907_state *cpustate);
    static void CALT_7801(upd7907_state *cpustate);
    static void DCR_A_7801(upd7907_state *cpustate);
    static void DCR_B_7801(upd7907_state *cpustate);
    static void DCR_C_7801(upd7907_state *cpustate);
    static void DCRW_wa_7801(upd7907_state *cpustate);
    static void INR_A_7801(upd7907_state *cpustate);
    static void INR_B_7801(upd7907_state *cpustate);
    static void INR_C_7801(upd7907_state *cpustate);
    static void INRW_wa_7801(upd7907_state *cpustate);
    static void STM_7801(upd7907_state *cpustate);
    static void MOV_MC_A_7801(upd7907_state *cpustate);
    static UINT8 read_port_byte(upd7907_state *cpustate, offs_t port);
    static void write_port_byte(upd7907_state *cpustate, offs_t port, UINT8 data);



};


#endif /* __UPD7907_H__ */
