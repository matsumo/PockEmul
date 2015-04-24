#ifndef MC6800_H
#define MC6800_H

#include <QQueue>

/*
    Skelton for retropc emulator

    Origin : MAME 0.142
    Author : Takeda.Toshiya
    Date  : 2011.04.23-

    [ Cmc6800 ]
*/
#define HAS_HD6301

// common signal id
#define SIG_CPU_IRQ	101
#define SIG_CPU_FIRQ	102
#define SIG_CPU_NMI	103
#define SIG_CPU_BUSREQ	104
#define SIG_CPU_DEBUG	201

#include "cpu.h"

//#include "vm.h"
//#include "../emu.h"
//#include "device.h"

#if defined(HAS_MC6801) || defined(HAS_HD6301)
#define SIG_MC6801_PORT_1	0
#define SIG_MC6801_PORT_2	1
#define SIG_MC6801_PORT_3	2
#define SIG_MC6801_PORT_4	3
#define SIG_MC6801_PORT_3_SC1	4
#define SIG_MC6801_PORT_3_SC2	5
#define SIG_MC6801_SIO_RECV	6

//class FIFO;
#endif

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

#if defined(HAS_MC6801) || defined(HAS_HD6301)
    // device to device
    typedef struct {
//        DEVICE *device;
        int id;
        quint32 mask;
        int shift;
    } output_t;
#define MAX_OUTPUT	16
    typedef struct {
        int count;
        output_t item[MAX_OUTPUT];
    } outputs_t;


    // data
    typedef struct {
        UINT8 wreg;
        UINT8 rreg;
        UINT8 ddr;
        UINT8 latched_data;
        bool latched;
        // output signals
        outputs_t outputs;
        bool first_write;
    } port_t;

#endif

typedef struct {
    PAIR pc;
    UINT16 prevpc;
    PAIR sp;
    PAIR ix;
    PAIR acc_d;
    PAIR ea;
    UINT8 cc;

    // timer
    PAIR counter;
    PAIR output_compare;
    PAIR timer_over;
    UINT8 tcsr;
    UINT8 pending_tcsr;
    UINT16 input_capture;
#ifdef HAS_HD6301
    UINT16 latch09;
#endif
    UINT32 timer_next;

    port_t port[4];

    UINT8 p3csr;
    bool p3csr_is3_flag_read;
    bool sc1_state;
    bool sc2_state;



    // serial i/o
    outputs_t outputs_sio;
//    FIFO *recv_buffer;

    UINT8 trcsr, rdr, tdr;
    bool trcsr_read_tdre, trcsr_read_orfe, trcsr_read_rdrf;
    UINT8 rmcr;
    int sio_counter;

    // memory controller
    UINT8 ram_ctrl;
} MC6800info;

class Cmc6800 :  public CCPU
{
    Q_OBJECT

public:
    MC6800info regs;

private:
//    DEVICE *d_mem;




    int wai_state;
    int int_state;

    int icount;

    UINT32 RM(UINT32 Addr);
    void WM(UINT32 Addr, UINT32 Value);
    UINT32 RM16(UINT32 Addr);
    void WM16(UINT32 Addr, PAIR *p);

#if defined(HAS_MC6801) || defined(HAS_HD6301)
    QQueue<int> recv_buffer;

    UINT32 mc6801_io_r(UINT32 offset);
    void mc6801_io_w(UINT32 offset, UINT32 data);
    void increment_counter(int amount);
#endif

    void run_one_opecode();
    void enter_interrupt(UINT16 irq_vector);
    void insn(UINT8 code);

    void aba();
    void abx();
    void adca_di();
    void adca_ex();
    void adca_im();
    void adca_ix();
    void adcb_di();
    void adcb_ex();
    void adcb_im();
    void adcb_ix();
    void adda_di();
    void adda_ex();
    void adda_im();
    void adda_ix();
    void addb_di();
    void addb_ex();
    void addb_im();
    void addb_ix();
    void addd_di();
    void addd_ex();
    void addd_im();
    void addd_ix();
    void adx_ex();
    void adx_im();
    void aim_di();
    void aim_ix();
    void nim_ix();
    void anda_di();
    void anda_ex();
    void anda_im();
    void anda_ix();
    void andb_di();
    void andb_ex();
    void andb_im();
    void andb_ix();
    void asl_ex();
    void asl_ix();
    void asla();
    void aslb();
    void asld();
    void asr_ex();
    void asr_ix();
    void asra();
    void asrb();
    void bcc();
    void bcs();
    void beq();
    void bge();
    void bgt();
    void bhi();
    void bita_di();
    void bita_ex();
    void bita_im();
    void bita_ix();
    void bitb_di();
    void bitb_ex();
    void bitb_im();
    void bitb_ix();
    void ble();
    void bls();
    void blt();
    void bmi();
    void bne();
    void bpl();
    void bra();
    void brn();
    void bsr();
    void bvc();
    void bvs();
    void cba();
    void clc();
    void cli();
    void clr_ex();
    void clr_ix();
    void clra();
    void clrb();
    void clv();
    void cmpa_di();
    void cmpa_ex();
    void cmpa_im();
    void cmpa_ix();
    void cmpb_di();
    void cmpb_ex();
    void cmpb_im();
    void cmpb_ix();
    void cmpx_di();
    void cmpx_ex();
    void cmpx_im();
    void cmpx_ix();
    void com_ex();
    void com_ix();
    void coma();
    void comb();
    void daa();
    void dec_ex();
    void dec_ix();
    void deca();
    void decb();
    void des();
    void dex();
    void eim_di();
    void eim_ix();
    void xim_ix();
    void eora_di();
    void eora_ex();
    void eora_im();
    void eora_ix();
    void eorb_di();
    void eorb_ex();
    void eorb_im();
    void eorb_ix();
    void illegal();
    void inc_ex();
    void inc_ix();
    void inca();
    void incb();
    void ins();
    void inx();
    void jmp_ex();
    void jmp_ix();
    void jsr_di();
    void jsr_ex();
    void jsr_ix();
    void lda_di();
    void lda_ex();
    void lda_im();
    void lda_ix();
    void ldb_di();
    void ldb_ex();
    void ldb_im();
    void ldb_ix();
    void ldd_di();
    void ldd_ex();
    void ldd_im();
    void ldd_ix();
    void lds_di();
    void lds_ex();
    void lds_im();
    void lds_ix();
    void ldx_di();
    void ldx_ex();
    void ldx_im();
    void ldx_ix();
    void lsr_ex();
    void lsr_ix();
    void lsra();
    void lsrb();
    void lsrd();
    void mul();
    void neg_ex();
    void neg_ix();
    void nega();
    void negb();
    void nop();
    void oim_di();
    void oim_ix();
    void oim_ix_mb8861();
    void ora_di();
    void ora_ex();
    void ora_im();
    void ora_ix();
    void orb_di();
    void orb_ex();
    void orb_im();
    void orb_ix();
    void psha();
    void pshb();
    void pshx();
    void pula();
    void pulb();
    void pulx();
    void rol_ex();
    void rol_ix();
    void rola();
    void rolb();
    void ror_ex();
    void ror_ix();
    void rora();
    void rorb();
    void rti();
    void rts();
    void sba();
    void sbca_di();
    void sbca_ex();
    void sbca_im();
    void sbca_ix();
    void sbcb_di();
    void sbcb_ex();
    void sbcb_im();
    void sbcb_ix();
    void sec();
    void sei();
    void sev();
    void slp();
    void sta_di();
    void sta_ex();
    void sta_im();
    void sta_ix();
    void stb_di();
    void stb_ex();
    void stb_im();
    void stb_ix();
    void std_di();
    void std_ex();
    void std_im();
    void std_ix();
    void sts_di();
    void sts_ex();
    void sts_im();
    void sts_ix();
    void stx_di();
    void stx_ex();
    void stx_im();
    void stx_ix();
    void suba_di();
    void suba_ex();
    void suba_im();
    void suba_ix();
    void subb_di();
    void subb_ex();
    void subb_im();
    void subb_ix();
    void subd_di();
    void subd_ex();
    void subd_im();
    void subd_ix();
    void swi();
    void tab();
    void tap();
    void tba();
    void tim_di();
    void tim_ix();
    void tmm_ix();
    void tpa();
    void tst_ex();
    void tst_ix();
    void tsta();
    void tstb();
    void tsx();
    void txs();
    void undoc1();
    void undoc2();
    void wai();
    void xgdx();
    void cpx_di();
    void cpx_ex();
    void cpx_im();
    void cpx_ix();

public:

    virtual const char*	GetClassName(){ return("Cm6502");}

    virtual	bool	init(void);						//initialize
    virtual	bool	exit(void);						//end
    virtual void	step(void);						//step SC61860

    virtual	void	Load_Internal(QXmlStreamReader *);
    virtual	void	save_internal(QXmlStreamWriter *);

//    virtual	DWORD	get_mem(DWORD adr,int size);		//get memory
//    virtual	void	set_mem(DWORD adr,int size,DWORD data);	//set memory

    virtual	bool	Get_Xin(void) { return true;}
    virtual	void	Set_Xin(bool) {}
    virtual	bool	Get_Xout(void) { return true;}
    virtual	void	Set_Xout(bool) {}

    virtual	UINT32	get_PC(void);					//get Program Counter
    virtual void    set_PC(UINT32 val);
    virtual void	Regs_Info(UINT8 Type);


    virtual void	Reset(void);

    Cmc6800( CPObject *parent);
    ~Cmc6800();

    // common functions
    void initialize();
#if defined(HAS_MC6801) || defined(HAS_HD6301)
    void release();
#endif

    int run(int clock);
    void write_signal(int id, UINT32 data, UINT32 mask);
    UINT32 get_pc() {
        return regs.prevpc;
    }

    // unique function
//    void set_context_mem(DEVICE* device) {
//        d_mem = device;
//    }
#if defined(HAS_MC6801) || defined(HAS_HD6301)
//    void set_context_port1(DEVICE* device, int id, UINT32 mask, int shift) {
//        register_output_signal(&port[0].outputs, device, id, mask, shift);
//    }
//    void set_context_port2(DEVICE* device, int id, UINT32 mask, int shift) {
//        register_output_signal(&port[1].outputs, device, id, mask, shift);
//    }
//    void set_context_port3(DEVICE* device, int id, UINT32 mask, int shift) {
//        register_output_signal(&port[2].outputs, device, id, mask, shift);
//    }
//    void set_context_port4(DEVICE* device, int id, UINT32 mask, int shift) {
//        register_output_signal(&port[2].outputs, device, id, mask, shift);
//    }
//    void set_context_sio(DEVICE* device, int id) {
//        register_output_signal(&outputs_sio, device, id, 0xff);
//    }
#endif


    void write_signals(outputs_t *items, UINT32 data);
};





#endif // Cmc6800

