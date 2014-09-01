/*****************************************************************************
 *
 *   tms7000.c
 *   Portable TMS7000 emulator (Texas Instruments 7000)
 *
 *   Copyright tim lindner, all rights reserved.
 *
 *   - This source code is released as freeware for non-commercial purposes.
 *   - You are free to use and redistribute this code in modified or
 *     unmodified form, provided you list me in the credits.
 *   - If you modify this source code, you must add a notice to each modified
 *     source file that it has been changed.  If you're a nice person, you
 *     will clearly mark each change too.  :)
 *   - If you wish to use this for commercial purposes, please contact me at
 *     tlindner@macmess.org
 *   - This entire notice must remain in the source code.
 *
 *****************************************************************************
 *  Misc. improvements were done over the years by team MESS/MAME
 *
 *  Currently this source emulates a TMS70x0, not any of the other variants
 *  Unimplemented is the MC pin which (in conjunection with IOCNT0 bits 7 and 6
 *  control the memory mapping.
 *
 *  This source implements the MC pin at Vss and mode bits in single chip mode.
 *****************************************************************************/


#include "tms7000.h"
#include "pcxxxx.h"
#include "Log.h"
#include "Inter.h"
#include "Debug.h"
#include "ui/cregsz80widget.h"

#define RM(Addr) pPC->Get_8(Addr)
#define WM(Addr,Value) pPC->Set_8(Addr,Value)

#define IMMBYTE(b)  b = pPC->Get_8(ppc); ppc++
#define SKIPBYTE()  pPC->Get_8(ppc); ppc++
#define SIMMBYTE(b) b = ((signed)pPC->Get_8(ppc)); ppc++
#define IMMWORD(w)  w.b.h = pPC->Get_8(ppc++); w.b.l = pPC->Get_8(ppc++)

#define PUSHBYTE(b) pSP++; WM(pSP,b)
#define PUSHWORD(w) pSP++; WM(pSP,w.b.h); pSP++; WM(pSP,w.b.l)
#define PULLBYTE(b) b = RM(pSP); pSP--
#define PULLWORD(w) w.b.l = RM(pSP); pSP--; w.b.h = RM(pSP); pSP--


//const device_type TMS7000 = &device_creator<tms7000>;
//const device_type TMS7020 = &device_creator<tms7020>;
//const device_type TMS7020_EXL = &device_creator<tms7020_exl>;
//const device_type TMS7040 = &device_creator<tms7040>;
//const device_type TMS70C00 = &device_creator<tms70c00>;
//const device_type TMS70C20 = &device_creator<tms70c20>;
//const device_type TMS70C40 = &device_creator<tms70c40>;

//static ADDRESS_MAP_START(tms7000_io, AS_IO, 8, tms7000)
//    AM_RANGE(TMS7000_PORTA, TMS7000_PORTA) AM_WRITENOP
//    AM_RANGE(TMS7000_PORTB, TMS7000_PORTB) AM_READNOP
//ADDRESS_MAP_END

//static ADDRESS_MAP_START(tms7000_mem, AS_PROGRAM, 8, tms7000 )
//    AM_RANGE(0x0000, 0x007f) AM_RAM // 128 bytes internal RAM
//    AM_RANGE(0x0100, 0x010f) AM_READWRITE(tms70x0_pf_r, tms70x0_pf_w)             /* tms7000 internal I/O ports */
//ADDRESS_MAP_END

//static ADDRESS_MAP_START(tms7020_mem, AS_PROGRAM, 8, tms7000 )
//    AM_RANGE(0xf000, 0xffff) AM_ROM // 2kB internal ROM
//    AM_IMPORT_FROM( tms7000_mem )
//ADDRESS_MAP_END

//static ADDRESS_MAP_START(tms7040_mem, AS_PROGRAM, 8, tms7000 )
//    AM_RANGE(0xf000, 0xffff) AM_ROM // 4kB internal ROM
//    AM_IMPORT_FROM( tms7000_mem )
//ADDRESS_MAP_END


Ctms7000::Ctms7000(CPObject *parent, TMS7000_Models mod)
    : CCPU(parent),
    m_opcode(s_opfn)
{

    pDEBUG = new Cdebug_tms7000(parent);
    fn_status="tms7000.sta";
    fn_log="tms7000.log";

    regwidget = (CregCPU*) new Cregsz80Widget(0,this);
}

Ctms7020::Ctms7020(CPObject *parent, TMS7000_Models mod)
    : Ctms7000(parent, mod)
{
}

Ctms7020_exl::Ctms7020_exl(CPObject *parent, TMS7000_Models mod)
    : Ctms7000(parent, mod)
{
}

Ctms7040::Ctms7040(CPObject *parent, TMS7000_Models mod)
    : Ctms7000(parent, mod)
{
}

Ctms70c00::Ctms70c00(CPObject *parent, TMS7000_Models mod)
    : Ctms7000(parent, mod)
{
}

Ctms70c20::Ctms70c20(CPObject *parent, TMS7000_Models mod)
    : Ctms7000(parent, mod)
{
}

Ctms70c40::Ctms70c40(CPObject *parent, TMS7000_Models mod)
    : Ctms7000(parent, mod)
{
}


//offs_t CCtms7000::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
//{
//    extern CPU_DISASSEMBLE( tms7000 );
//    return CPU_DISASSEMBLE_NAME(tms7000)(this, buffer, pc, oprom, opram, options);
//}


#define ppc     m_pc.w.l
#define PC      m_pc
#define pSP     m_sp
#define pSR     m_sr

#define RDA     RM(0x0000)
#define RDB     RM(0x0001)

#define WRA(Value) (WM(0x0000,Value))
#define WRB(Value) (WM(0x0001,Value))

#define SR_C    0x80        /* Carry */
#define SR_N    0x40        /* Negative */
#define SR_Z    0x20        /* Zero */
#define SR_I    0x10        /* Interrupt */

#define CLR_NZC     pSR&=~(SR_N|SR_Z|SR_C)
#define CLR_NZCI    pSR&=~(SR_N|SR_Z|SR_C|SR_I)
#define SET_C8(a)   pSR|=((a&0x0100)>>1)
#define SET_N8(a)   pSR|=((a&0x0080)>>1)
#define SET_Z(a)    if(!a)pSR|=SR_Z
#define SET_Z8(a)   SET_Z((UINT8)a)
#define SET_Z16(a)  SET_Z((UINT8)a>>8)
#define GET_C       (pSR >> 7)

#define SET_N16(a)	pSR|=(((a)&0x008000)>>9)

/* Not working */
#define SET_C16(a)  pSR|=((a&0x010000)>>9)

#define SETC        pSR |= SR_C
#define SETZ        pSR |= SR_Z
#define SETN        pSR |= SR_N


UINT16 Ctms7000::RM16( UINT32 mAddr ) /* Read memory (16-bit) */
{
    UINT32 result = RM(mAddr) << 8;
    return result | RM((mAddr+1)&0xffff);
}

UINT16 Ctms7000::RRF16( UINT32 mAddr ) /* Read register file (16 bit) */
{
    PAIR result;
    result.b.h = RM((mAddr-1)&0xffff);
    result.b.l = RM(mAddr);
    return result.w.l;
}

void Ctms7000::WRF16(UINT32 mAddr, PAIR p ) /* Write register file (16 bit) */
{
    WM( (mAddr-1)&0xffff, p.b.h );
    WM( mAddr, p.b.l );
}


bool Ctms7000::init()
{
//    m_program = &space(AS_PROGRAM);
//    m_direct = &m_program->direct();
//    m_io = &space(AS_IO);

    Check_Log();
    pDEBUG->init();

    memset(m_pf, 0, 0x100);
    m_cycles_per_INT2 = 0;
    m_t1_capture_latch = 0;
    m_t1_prescaler = 0;
    m_t1_decrementer = 0;
    memset(&m_pc, 0x00, sizeof(m_pc));
    m_sp = 0;
    m_sr = 0;
    m_idle_state = 0;

    /* Save register state */
//    save_item(NAME(ppc));
//    save_item(NAME(pSP));
//    save_item(NAME(pSR));

    /* Save Interrupt state */
//    save_item(NAME(m_irq_state));

    /* Save register and perpherial file state */
//    save_item(NAME(m_pf));

    /* Save timer state */
//    save_item(NAME(m_t1_prescaler));
//    save_item(NAME(m_t1_capture_latch));
//    save_item(NAME(m_t1_decrementer));

//    save_item(NAME(m_idle_state));

//    state_add( TMS7000_PC,     "PC",    m_pc.w.l).formatstr("%04X");
//    state_add( TMS7000_SP,     "S",     m_sp).formatstr("%02X");
//    state_add( TMS7000_ST,     "ST",    m_sr).formatstr("%02X");
//    state_add( TMS7000_IDLE,   "Idle",  m_idle_state).formatstr("%02X");
//    state_add( TMS7000_T1_CL,  "T1CL",  m_t1_capture_latch).formatstr("%02X");
//    state_add( TMS7000_T1_PS,  "T1PS",  m_t1_prescaler).mask(0x1f).formatstr("%02X");
//    state_add( TMS7000_T1_DEC, "T1DEC", m_t1_decrementer).mask(0xff).formatstr("%02X");

//    state_add(STATE_GENPC, "GENPC", m_pc.w.l).formatstr("%04X").noshow();
//    state_add(STATE_GENSP, "GENSP", m_sp).formatstr("%02X").noshow();
//    state_add(STATE_GENFLAGS, "GENFLAGS",  m_sr).formatstr("%8s").noshow();

//    m_icountptr = &m_icount;

    return true;
}



void Ctms7000::Reset()
{
//  m_architecture = (int)param;

    m_idle_state = 0;
    m_irq_state[ TMS7000_IRQ1_LINE ] = CLEAR_LINE;
    m_irq_state[ TMS7000_IRQ2_LINE ] = CLEAR_LINE;
    m_irq_state[ TMS7000_IRQ3_LINE ] = CLEAR_LINE;

    WM( 0x100 + 9, 0 );     /* Data direction regs are cleared */
    WM( 0x100 + 11, 0 );

//  if( m_architecture == TMS7000_NMOS )
//  {
        WM( 0x100 + 4, 0xff );      /* Output 0xff on port A */
        WM( 0x100 + 8, 0xff );      /* Output 0xff on port C */
        WM( 0x100 + 10, 0xff );     /* Output 0xff on port D */
//  }
//  else
//  {
//      WM( 0x100 + 4, 0xff );      /* Output 0xff on port A */
//  }

    pSP = 0x01;             /* Set stack pointer to r1 */
    pSR = 0x00;             /* Clear status register (disabling interrupts */
    WM( 0x100 + 0, 0 );     /* Write a zero to IOCNT0 */

    /* On TMS70x2 and TMS70Cx2 IOCNT1 is zero */

    WRA( m_pc.b.h );    /* Write previous PC to A:B */
    WRB( m_pc.b.l );
    ppc = RM16(0xfffe);       /* Load reset vector */

    m_div_by_16_trigger = -16;
}

void Ctms7000::execute_set_input(int irqline, int state)
{
    if (m_irq_state[irqline] != state)
    {   /* check for transition */
        m_irq_state[irqline] = state;

//        LOG(("tms7000: (cpu '%s') set_irq_line (INT%d, state %d)\n", tag(), irqline+1, state));

        if (state == CLEAR_LINE)
        {
            return;
        }

        m_pf[0] |= (0x02 << (irqline * 2)); /* Set INTx iocntl0 flag */

        if( irqline == TMS7000_IRQ3_LINE )
        {
            /* Latch the value in perpherial file register 3 */
            m_t1_capture_latch = m_t1_decrementer & 0x00ff;
        }

        check_IRQ_lines();
    }
}

void Ctms7000::check_IRQ_lines()
{
    if( pSR & SR_I ) /* Check Global Interrupt bit: Status register, bit 4 */
    {
        if ((m_irq_state[TMS7000_IRQ1_LINE] == ASSERT_LINE) || (m_pf[0] & 0x02))
        {
            if( m_pf[0] & 0x01 ) /* INT1 Enable bit */
            {
                do_interrupt( 0xfffc, TMS7000_IRQ1_LINE );
                m_pf[0] &= ~0x02; /* Data Manual, page: 9-41 */
                return;
            }
        }

        if( m_irq_state[ TMS7000_IRQ2_LINE ] == ASSERT_LINE )
        {
            if( m_pf[0] & 0x04 ) /* INT2 Enable bit */
            {
                do_interrupt( 0xfffa, TMS7000_IRQ2_LINE );
                return;
            }
        }

        if ((m_irq_state[TMS7000_IRQ3_LINE] == ASSERT_LINE) || (m_pf[0] & 0x20))
        {
            if( m_pf[0] & 0x10 ) /* INT3 Enable bit */
            {
                do_interrupt( 0xfff8, TMS7000_IRQ3_LINE );
                m_pf[0] &= ~0x20; /* Data Manual, page: 9-41 */
                return;
            }
        }
    }
}

void Ctms7000::do_interrupt( UINT16 address, UINT8 line )
{
    CallSubLevel++;

    PUSHBYTE( pSR );        /* Push Status register */
    PUSHWORD( PC );         /* Push Program Counter */
    pSR = 0;                /* Clear Status register */
    ppc = RM16(address);  /* Load PC with interrupt vector */

    if( m_idle_state == 0 )
        m_icount -= 19;     /* 19 cycles used */
    else
    {
        m_icount -= 17;     /* 17 if idled */
        m_idle_state = 0;
    }

//    standard_irq_callback(line);
}

/*****************************************************************************
 *
 *   tms70op.inc (Op code functions)
 *   Portable TMS7000 emulator (Texas Instruments 7000)
 *
 *   Copyright tim lindner, all rights reserved.
 *
 *   - This source code is released as freeware for non-commercial purposes.
 *   - You are free to use and redistribute this code in modified or
 *     unmodified form, provided you list me in the credits.
 *   - If you modify this source code, you must add a notice to each modified
 *     source file that it has been changed.  If you're a nice person, you
 *     will clearly mark each change too.  :)
 *   - If you wish to use this for commercial purposes, please contact me at
 *     tlindner@macmess.org
 *   - This entire notice must remain in the source code.
 *
 *****************************************************************************/

void Ctms7000::illegal()
{
    /* This is a guess */
    m_icount -= 4;
}

void Ctms7000::adc_b2a()
{
    UINT16  t;

    t = RDA + RDB + GET_C;
    WRA(t);

    CLR_NZC;
    SET_C8(t);
    SET_N8(t);
    SET_Z8(t);

    m_icount -= 5;
}

void Ctms7000::adc_r2a()
{
    UINT16  t;
    UINT8   v;

    IMMBYTE(v);

    t = RM(v) + RDA + GET_C;
    WRA(t);

    CLR_NZC;
    SET_C8(t);
    SET_N8(t);
    SET_Z8(t);

    m_icount -= 8;
}

void Ctms7000::adc_r2b()
{
    UINT16  t;
    UINT8   v;

    IMMBYTE(v);

    t = RM(v) + RDB + GET_C;
    WRB(t);

    CLR_NZC;
    SET_C8(t);
    SET_N8(t);
    SET_Z8(t);

    m_icount -= 8;
}

void Ctms7000::adc_r2r()
{
    UINT16  t;
    UINT8   i,j;

    IMMBYTE(i);
    IMMBYTE(j);

    t = RM(i)+RM(j) + GET_C;
    WM(j,t);

    CLR_NZC;
    SET_C8(t);
    SET_N8(t);
    SET_Z8(t);

    m_icount -= 10;
}

void Ctms7000::adc_i2a()
{
    UINT16  t;
    UINT8   v;

    IMMBYTE(v);

    t = v + RDA + GET_C;
    WRA(t);

    CLR_NZC;
    SET_C8(t);
    SET_N8(t);
    SET_Z8(t);

    m_icount -= 7;
}

void Ctms7000::adc_i2b()
{
    UINT16  t;
    UINT8   v;

    IMMBYTE(v);

    t = v + RDB + GET_C;
    WRB(t);

    CLR_NZC;
    SET_C8(t);
    SET_N8(t);
    SET_Z8(t);

    m_icount -= 7;
}

void Ctms7000::adc_i2r()
{
    UINT16  t;
    UINT8   i,j;

    IMMBYTE(i);
    IMMBYTE(j);

    t = i+RM(j) + GET_C;
    WM(j,t);

    CLR_NZC;
    SET_C8(t);
    SET_N8(t);
    SET_Z8(t);

    m_icount -= 9;
}

void Ctms7000::add_b2a()
{
    UINT16  t;

    t = RDA + RDB;
    WRA(t);

    CLR_NZC;
    SET_C8(t);
    SET_N8(t);
    SET_Z8(t);

    m_icount -= 5;
}

void Ctms7000::add_r2a()
{
    UINT16  t;
    UINT8   v;

    IMMBYTE(v);

    t = RM(v) + RDA;
    WRA(t);

    CLR_NZC;
    SET_C8(t);
    SET_N8(t);
    SET_Z8(t);

    m_icount -= 8;
}

void Ctms7000::add_r2b()
{
    UINT16  t;
    UINT8   v;

    IMMBYTE(v);

    t = RM(v) + RDB;
    WRB(t);

    CLR_NZC;
    SET_C8(t);
    SET_N8(t);
    SET_Z8(t);

    m_icount -= 8;
}

void Ctms7000::add_r2r()
{
    UINT16  t;
    UINT8   i,j;

    IMMBYTE(i);
    IMMBYTE(j);

    t = RM(i)+RM(j);
    WM(j,t);

    CLR_NZC;
    SET_C8(t);
    SET_N8(t);
    SET_Z8(t);

    m_icount -= 10;
}

void Ctms7000::add_i2a()
{
    UINT16  t;
    UINT8   v;

    IMMBYTE(v);

    t = v + RDA;
    WRA(t);

    CLR_NZC;
    SET_C8(t);
    SET_N8(t);
    SET_Z8(t);

    m_icount -= 7;
}

void Ctms7000::add_i2b()
{
    UINT16  t;
    UINT8   v;

    IMMBYTE(v);

    t = v + RDB;
    WRB(t);

    CLR_NZC;
    SET_C8(t);
    SET_N8(t);
    SET_Z8(t);

    m_icount -= 7;
}

void Ctms7000::add_i2r()
{
    UINT16  t;
    UINT8   i,j;

    IMMBYTE(i);
    IMMBYTE(j);

    t = i+RM(j);
    WM(j,t);

    CLR_NZC;
    SET_C8(t);
    SET_N8(t);
    SET_Z8(t);

    m_icount -= 9;
}

void Ctms7000::and_b2a()
{
    UINT8   t;

    t = RDA & RDB;
    WRA(t);

    CLR_NZC;
    SET_N8(t);
    SET_Z8(t);

    m_icount -= 5;
}

void Ctms7000::and_r2a()
{
    UINT8   t;
    UINT8   v;

    IMMBYTE(v);

    t = RM(v) & RDA;
    WRA(t);

    CLR_NZC;
    SET_N8(t);
    SET_Z8(t);

    m_icount -= 8;
}

void Ctms7000::and_r2b()
{
    UINT8   t;
    UINT8   v;

    IMMBYTE(v);

    t = RM(v) & RDB;
    WRB(t);

    CLR_NZC;
    SET_N8(t);
    SET_Z8(t);

    m_icount -= 8;
}

void Ctms7000::and_r2r()
{
    UINT8   t;
    UINT8   i,j;

    IMMBYTE(i);
    IMMBYTE(j);

    t = RM(i) & RM(j);
    WM(j,t);

    CLR_NZC;
    SET_N8(t);
    SET_Z8(t);

    m_icount -= 10;
}

void Ctms7000::and_i2a()
{
    UINT8   t;
    UINT8   v;

    IMMBYTE(v);

    t = v & RDA;
    WRA(t);

    CLR_NZC;
    SET_N8(t);
    SET_Z8(t);

    m_icount -= 7;
}

void Ctms7000::and_i2b()
{
    UINT8   t;
    UINT8   v;

    IMMBYTE(v);

    t = v & RDB;
    WRB(t);

    CLR_NZC;
    SET_N8(t);
    SET_Z8(t);

    m_icount -= 7;
}

void Ctms7000::and_i2r()
{
    UINT8   t;
    UINT8   i,j;

    IMMBYTE(i);
    IMMBYTE(j);

    t = i & RM(j);
    WM(j,t);

    CLR_NZC;
    SET_N8(t);
    SET_Z8(t);

    m_icount -= 9;
}

void Ctms7000::andp_a2p()
{
    UINT8   t;
    UINT8   v;

    IMMBYTE(v);
    t = RDA & RM( 0x0100 + v);
    WM( 0x0100+v, t);

    CLR_NZC;
    SET_N8(t);
    SET_Z8(t);

    m_icount -= 10;
}

void Ctms7000::andp_b2p()
{
    UINT8   t;
    UINT8   v;

    IMMBYTE(v);
    t = RDB & RM( 0x0100 + v);
    WM( 0x0100+v, t);

    CLR_NZC;
    SET_N8(t);
    SET_Z8(t);

    m_icount -= 9;
}


void Ctms7000::movp_i2p()
{
    UINT8   i,v;

    IMMBYTE(i);
    IMMBYTE(v);
    WM( 0x0100+v, i);

    CLR_NZC;
    SET_N8(i);
    SET_Z8(i);

    m_icount -= 11;
}

void Ctms7000::andp_i2p()
{
    UINT8   t;
    UINT8   i,v;

    IMMBYTE(i);
    IMMBYTE(v);
    t = i & RM( 0x0100 + v);
    WM( 0x0100+v, t);

    CLR_NZC;
    SET_N8(t);
    SET_Z8(t);

    m_icount -= 11;
}

void Ctms7000::br_dir()
{
    PAIR p;

    IMMWORD( p );
    ppc = p.d;
    m_icount -= 10;
}

void Ctms7000::br_ind()
{
    UINT8   v;

    IMMBYTE( v );
    PC.w.l = RRF16(v);

    m_icount -= 9;
}

void Ctms7000::br_inx()
{
    PAIR p;

    IMMWORD( p );
    ppc = p.w.l + RDB;
    m_icount -= 12;
}

void Ctms7000::btjo_b2a()
{
    UINT8   t;

    t = RDB & RDA;

    CLR_NZC;
    SET_N8(t);
    SET_Z8(t);

    if(t != 0)
    {
        qint8    j;

        SIMMBYTE( j );
        ppc += j;
        m_icount -= 9;
    }
    else
    {
        ppc++;
        m_icount -= 7;
    }
}

void Ctms7000::btjo_r2a()
{
    UINT8   t,r;

    IMMBYTE( r );
    t = RM( r ) & RDA;

    CLR_NZC;
    SET_N8(t);
    SET_Z8(t);

    if(t != 0)
    {
        qint8    j;

        SIMMBYTE( j );
        ppc += j;
        m_icount -= 9;
    }
    else
    {
        ppc++;
        m_icount -= 7;
    }
}

void Ctms7000::btjo_r2b()
{
    UINT8   t,r;

    IMMBYTE(r);
    t = RM(r) & RDB;

    CLR_NZC;
    SET_N8(t);
    SET_Z8(t);

    if(t != 0)
    {
        qint8    j;

        SIMMBYTE(j);
        ppc += j;
        m_icount -= 12;
    }
    else
    {
        ppc++;
        m_icount -= 10;
    }
}

void Ctms7000::btjo_r2r()
{
    UINT8   t,r,s;

    IMMBYTE(r);
    IMMBYTE(s);
    t = RM(r) & RM(s);

    CLR_NZC;
    SET_N8(t);
    SET_Z8(t);

    if(t != 0)
    {
        qint8    j;

        SIMMBYTE(j);
        ppc += j;
        m_icount -= 14;
    }
    else
    {
        ppc++;
        m_icount -= 12;
    }
}

void Ctms7000::btjo_i2a()
{
    UINT8   t,r;

    IMMBYTE(r);
    t = r & RDA;

    CLR_NZC;
    SET_N8(t);
    SET_Z8(t);

    if(t != 0)
    {
        qint8    j;

        SIMMBYTE(j);
        ppc += j;
        m_icount -= 11;
    }
    else
    {
        ppc++;
        m_icount -= 9;
    }
}

void Ctms7000::btjo_i2b()
{
    UINT8   t,i;

    IMMBYTE(i);
    t = i & RDB;

    CLR_NZC;
    SET_N8(t);
    SET_Z8(t);

    if(t != 0)
    {
        qint8    j;

        SIMMBYTE(j);
        ppc += j;
        m_icount -= 11;
    }
    else
    {
        ppc++;
        m_icount -= 9;
    }
}

void Ctms7000::btjo_i2r()
{
    UINT8   t,i,r;

    IMMBYTE(i);
    IMMBYTE(r);
    t = i & RM(r);

    CLR_NZC;
    SET_N8(t);
    SET_Z8(t);

    if(t != 0)
    {
        qint8    j;

        SIMMBYTE(j);
        ppc += j;
        m_icount -= 13;
    }
    else
    {
        ppc++;
        m_icount -= 11;
    }
}

void Ctms7000::btjop_ap()
{
    UINT8   t,p;

    IMMBYTE(p);

    t = RM(0x100+p) & RDA;

    CLR_NZC;
    SET_N8(t);
    SET_Z8(t);

    if(t != 0)
    {
        qint8    j;

        SIMMBYTE(j);
        ppc += j;
        m_icount -= 13;
    }
    else
    {
        ppc++;
        m_icount -= 11;
    }
}

void Ctms7000::btjop_bp()
{
    UINT8   t,p;

    IMMBYTE(p);

    t = RM(0x100+p) & RDB;

    CLR_NZC;
    SET_N8(t);
    SET_Z8(t);

    if(t != 0)
    {
        qint8    j;

        SIMMBYTE(j);
        ppc += j;
        m_icount -= 12;
    }
    else
    {
        ppc++;
        m_icount -= 10;
    }
}

void Ctms7000::btjop_ip()
{
    UINT8   t,p,i;

    IMMBYTE(i);
    IMMBYTE(p);

    t = RM(0x100+p) & i;

    CLR_NZC;
    SET_N8(t);
    SET_Z8(t);

    if(t != 0)
    {
        qint8    j;

        SIMMBYTE(j);
        ppc += j;
        m_icount -= 14;
    }
    else
    {
        ppc++;
        m_icount -= 12;
    }
}

void Ctms7000::btjz_b2a()
{
    UINT8   t;

    t = RDB & (~RDA);

    CLR_NZC;
    SET_N8(t);
    SET_Z8(t);

    if(t != 0)
    {
        qint8    j;

        SIMMBYTE( j );
        ppc += j;
        m_icount -= 9;
    }
    else
    {
        ppc++;
        m_icount -= 7;
    }
}

void Ctms7000::btjz_r2a()
{
    UINT8   t,r;

    IMMBYTE( r );
    t = RM( r ) & (~RDA);

    CLR_NZC;
    SET_N8(t);
    SET_Z8(t);

    if(t != 0)
    {
        qint8    j;

        SIMMBYTE( j );
        ppc += j;
        m_icount -= 9;
    }
    else
    {
        ppc++;
        m_icount -= 7;
    }
}

void Ctms7000::btjz_r2b()
{
    UINT8   t,r;

    IMMBYTE(r);
    t = RM(r) & (~RDB);

    CLR_NZC;
    SET_N8(t);
    SET_Z8(t);

    if(t != 0)
    {
        qint8    j;

        SIMMBYTE(j);
        ppc += j;
        m_icount -= 12;
    }
    else
    {
        ppc++;
        m_icount -= 10;
    }
}

void Ctms7000::btjz_r2r()
{
    UINT8   t,r,s;

    IMMBYTE(r);
    IMMBYTE(s);
    t = RM(r) & (~RM(s));

    CLR_NZC;
    SET_N8(t);
    SET_Z8(t);

    if(t != 0)
    {
        qint8    j;

        SIMMBYTE(j);
        ppc += j;
        m_icount -= 14;
    }
    else
    {
        ppc++;
        m_icount -= 12;
    }
}

void Ctms7000::btjz_i2a()
{
    UINT8   t,r;

    IMMBYTE(r);
    t = r & (~RDA);

    CLR_NZC;
    SET_N8(t);
    SET_Z8(t);

    if(t != 0)
    {
        qint8    j;

        SIMMBYTE(j);
        ppc += j;
        m_icount -= 11;
    }
    else
    {
        ppc++;
        m_icount -= 9;
    }
}

void Ctms7000::btjz_i2b()
{
    UINT8   t,i;

    IMMBYTE(i);
    t = i & (~RDB);

    CLR_NZC;
    SET_N8(t);
    SET_Z8(t);

    if(t != 0)
    {
        qint8    j;

        SIMMBYTE(j);
        ppc += j;
        m_icount -= 11;
    }
    else
    {
        ppc++;
        m_icount -= 9;
    }
}

void Ctms7000::btjz_i2r()
{
    UINT8   t,i,r;

    IMMBYTE(i);
    IMMBYTE(r);
    t = i & (~RM(r));

    CLR_NZC;
    SET_N8(t);
    SET_Z8(t);

    if(t != 0)
    {
        qint8    j;

        SIMMBYTE(j);
        ppc += j;
        m_icount -= 13;
    }
    else
    {
        ppc++;
        m_icount -= 11;
    }
}

void Ctms7000::btjzp_ap()
{
    UINT8   t,p;

    IMMBYTE(p);

    t = RDA & (~RM(0x100+p));

    CLR_NZC;
    SET_N8(t);
    SET_Z8(t);

    if(t != 0)
    {
        qint8    j;

        SIMMBYTE(j);
        ppc += j;
        m_icount -= 13;
    }
    else
    {
        ppc++;
        m_icount -= 11;
    }
}

void Ctms7000::btjzp_bp()
{
    UINT8   t,p;

    IMMBYTE(p);

    t = RDB & (~RM(0x100+p));

    CLR_NZC;
    SET_N8(t);
    SET_Z8(t);

    if(t != 0)
    {
        qint8    j;

        SIMMBYTE(j);
        ppc += j;
        m_icount -= 12;
    }
    else
    {
        ppc++;
        m_icount -= 10;
    }
}

void Ctms7000::btjzp_ip()
{
    UINT8   t,p,i;

    IMMBYTE(i);
    IMMBYTE(p);

    t = i & (~RM(0x100+p));

    CLR_NZC;
    SET_N8(t);
    SET_Z8(t);

    if(t != 0)
    {
        qint8    j;

        SIMMBYTE(j);
        ppc += j;
        m_icount -= 14;
    }
    else
    {
        ppc++;
        m_icount -= 12;
    }
}

void Ctms7000::call_dir()
{
    PAIR    tPC;

    CallSubLevel++;
    IMMWORD( tPC );
    PUSHWORD( PC );
    ppc = tPC.d;

    m_icount -= 14;
}

void Ctms7000::call_ind()
{
    UINT8   v;

    CallSubLevel++;
    IMMBYTE( v );
    PUSHWORD( PC );
    PC.w.l = RRF16(v);

    m_icount -= 13;
}

void Ctms7000::call_inx()
{
    PAIR    tPC;

    CallSubLevel++;
    IMMWORD( tPC );
    PUSHWORD( PC );
    ppc = tPC.w.l + RDB;
    m_icount -= 16;
}

void Ctms7000::clr_a()
{
    WRA(0);
    CLR_NZC;
    SETZ;
    m_icount -= 5;
}

void Ctms7000::clr_b()
{
    WRB(0);
    CLR_NZC;
    SETZ;
    m_icount -= 5;
}

void Ctms7000::clr_r()
{
    UINT8   r;

    IMMBYTE(r);
    WM(r,0);
    CLR_NZC;
    SETZ;
    m_icount -= 7;
}

void Ctms7000::clrc()
{
    UINT8   a;

    a = RDA;

    CLR_NZC;
    SET_N8(a);
    SET_Z8(a);

    m_icount -= 6;
}

void Ctms7000::cmp_ba()
{
    UINT16 t;

    t = RDA - RDB;

    CLR_NZC;
    SET_N8(t);
    SET_Z8(t);

    SET_C8( ~t );
    m_icount -= 5;
}

void Ctms7000::cmp_ra()
{
    UINT16  t;
    UINT8   r;

    IMMBYTE(r);
    t = RDA - RM(r);

    CLR_NZC;
    SET_N8(t);
    SET_Z8(t);

    SET_C8( ~t );

    m_icount -= 8;
}

void Ctms7000::cmp_rb()
{
    UINT16  t;
    UINT8   r;

    IMMBYTE(r);
    t = RDB - RM(r);

    CLR_NZC;
    SET_N8(t);
    SET_Z8(t);

    SET_C8( ~t );

    m_icount -= 8;
}

void Ctms7000::cmp_rr()
{
    UINT16  t;
    UINT8   r,s;

    IMMBYTE(r);
    IMMBYTE(s);
    t = RM(s) - RM(r);

    CLR_NZC;
    SET_N8(t);
    SET_Z8(t);

    SET_C8( ~t );

    m_icount -= 10;
}

void Ctms7000::cmp_ia()
{
    UINT16  t;
    UINT8   i;

    IMMBYTE(i);
    t = RDA - i;

    CLR_NZC;
    SET_N8(t);
    SET_Z8(t);

    SET_C8( ~t );

    m_icount -= 7;
}

void Ctms7000::cmp_ib()
{
    UINT16  t;
    UINT8   i;

    IMMBYTE(i);
    t = RDB - i;

    CLR_NZC;
    SET_N8(t);
    SET_Z8(t);
    SET_C8( ~t );

    m_icount -= 7;
}

void Ctms7000::cmp_ir()
{
    UINT16  t;
    UINT8   i,r;

    IMMBYTE(i);
    IMMBYTE(r);
    t = RM(r) - i;

    CLR_NZC;
    SET_N8(t);
    SET_Z8(t);
    SET_C8( ~t );

    m_icount -= 9;
}

void Ctms7000::cmpa_dir()
{
    UINT16  t;
    PAIR    i;

    IMMWORD( i );
    t = RDA - RM(i.w.l);

    CLR_NZC;
    SET_N8(t);
    SET_Z8(t);
    SET_C8( ~t );

    m_icount -= 12;
}

void Ctms7000::cmpa_ind()
{
    UINT16  t;
    PAIR    p;
    qint8    i;

    IMMBYTE(i);
    p.w.l = RRF16(i);
    t = RDA - RM(p.w.l);

    CLR_NZC;
    SET_N8(t);
    SET_Z8(t);
    SET_C8( ~t );

    m_icount -= 11;
}

void Ctms7000::cmpa_inx()
{
    UINT16  t;
    PAIR    i;

    IMMWORD( i );
    t = RDA - RM(i.w.l + RDB);

    CLR_NZC;
    SET_N8(t);
    SET_Z8(t);
    SET_C8( ~t );

    m_icount -= 14;
}

void Ctms7000::dac_b2a()
{
    WRA(bcd_add(RDA, RDB, pSR & SR_C));

    m_icount -= 7;
}

void Ctms7000::dac_r2a()
{
    UINT8 r;
    IMMBYTE(r);

    WRA(bcd_add(RDA, RM(r), pSR & SR_C));

    m_icount -= 10;
}

void Ctms7000::dac_r2b()
{
    UINT8 r;
    IMMBYTE(r);

    WRB(bcd_add(RDB, RM(r), pSR & SR_C));

    m_icount -= 10;
}

void Ctms7000::dac_r2r()
{
    UINT8 s, r;
    IMMBYTE(s);
    IMMBYTE(r);

    WM(r, bcd_add(RM(s), RM(r), pSR & SR_C));

    m_icount -= 12;
}

void Ctms7000::dac_i2a()
{
    UINT8 i;
    IMMBYTE(i);

    WRA(bcd_add(i, RDA, pSR & SR_C));

    m_icount -= 9;
}

void Ctms7000::dac_i2b()
{
    UINT8 i;
    IMMBYTE(i);

    WRB(bcd_add(i, RDB, pSR & SR_C));

    m_icount -= 9;
}

void Ctms7000::dac_i2r()
{
    UINT8 i, r;
    IMMBYTE(i);
    IMMBYTE(r);

    WM(r, bcd_add(i, RM(r), pSR & SR_C));

    m_icount -= 11;
}

void Ctms7000::dec_a()
{
    UINT16 t;

    t = RDA - 1;

    WRA( t );

    CLR_NZC;
    SET_N8(t);
    SET_Z8(t);
    if ((t&0xFF)!=0xFF) SETC;

    m_icount -= 5;
}

void Ctms7000::dec_b()
{
    UINT16 t;

    t = RDB - 1;

    WRB( t );

    CLR_NZC;
    SET_N8(t);
    SET_Z8(t);
    if ((t&0xFF)!=0xFF) SETC;

    m_icount -= 5;
}

void Ctms7000::dec_r()
{
    UINT16  t;
    UINT8   r;

    IMMBYTE(r);

    t = RM(r) - 1;

    WM( r, t );

    CLR_NZC;
    SET_N8(t);
    SET_Z8(t);
    if ((t&0xFF)!=0xFF) SETC;

    m_icount -= 7;
}

void Ctms7000::decd_a()
{
    PAIR    t;

    t.w.h = 0;
    t.w.l = RRF16(0);
    t.d -= 1;
    WRF16(0,t);

    CLR_NZC;

    SET_N16(t.d);

    if ((t.d&0xFF00)==0) SETZ;
    if ((t.d&0xFFFF)!=0xFFFF) SETC;


    m_icount -= 9;
}

void Ctms7000::decd_b()
{
    PAIR    t;

    t.w.h = 0;
    t.w.l = RRF16(1);
    t.d -= 1;
    WRF16(1,t);

    CLR_NZC;
    SET_N16(t.d);

    if ((t.d&0xFF00)==0) SETZ;
    if ((t.d&0xFFFF)!=0xFFFF) SETC;

    m_icount -= 9;
}

void Ctms7000::decd_r()
{
    UINT8   r;
    PAIR    t;

    IMMBYTE(r);
    t.w.h = 0;
    t.w.l = RRF16(r);
    t.d -= 1;
    WRF16(r,t);

    CLR_NZC;
    SET_N16(t.d);

    if ((t.d&0xFF00)==0) SETZ;
    if ((t.d&0xFFFF)!=0xFFFF) SETC;

    m_icount -= 11;
}

void Ctms7000::dint()
{
    CLR_NZCI;
    m_icount -= 5;
}

void Ctms7000::djnz_a()
{
    UINT16 t;

    t = RDA - 1;

    WRA( t );

    if( t != 0 )
    {
        qint8    s;

        SIMMBYTE(s);
        ppc += s;
        m_icount -= 7;
    }
    else
    {
        ppc++;
        m_icount -= 2;
    }
}

void Ctms7000::djnz_b()
{
    UINT16 t;

    t = RDB - 1;

    WRB( t );

    if( t != 0 )
    {
        qint8    s;

        SIMMBYTE(s);
        ppc += s;
        m_icount -= 7;
    }
    else
    {
        ppc++;
        m_icount -= 2;
    }
}

void Ctms7000::djnz_r()
{
    UINT16  t;
    UINT8   r;

    IMMBYTE(r);

    t = RM(r) - 1;

    WM(r,t);
    if( t != 0 )
    {
        qint8    s;

        SIMMBYTE(s);
        ppc += s;
        m_icount -= 9;
    }
    else
    {
        ppc++;
        m_icount -= 3;
    }
}

void Ctms7000::dsb_b2a()
{
    WRA(bcd_sub(RDA, RDB, pSR & SR_C));

    m_icount -= 7;
}

void Ctms7000::dsb_r2a()
{
    UINT8 r;
    IMMBYTE(r);

    WRA(bcd_sub(RDA, RM(r), pSR & SR_C));

    m_icount -= 10;
}

void Ctms7000::dsb_r2b()
{
    UINT8 r;
    IMMBYTE(r);

    WRB(bcd_sub(RDB, RM(r), pSR & SR_C));

    m_icount -= 10;
}

void Ctms7000::dsb_r2r()
{
    UINT8 s, r;
    IMMBYTE(s);
    IMMBYTE(r);

    WM(r, bcd_sub(RM(s), RM(r), pSR & SR_C));

    m_icount -= 12;
}

void Ctms7000::dsb_i2a()
{
    UINT8 i;
    IMMBYTE(i);

    WRA(bcd_sub(RDA, i, pSR & SR_C));

    m_icount -= 9;
}

void Ctms7000::dsb_i2b()
{
    UINT8 i;
    IMMBYTE(i);

    WRB(bcd_sub(RDB, i, pSR & SR_C));

    m_icount -= 9;
}

void Ctms7000::dsb_i2r()
{
    UINT8 i, r;
    IMMBYTE(i);
    IMMBYTE(r);

    WM(r, bcd_sub(RM(r), i, pSR & SR_C));

    m_icount -= 11;
}

void Ctms7000::eint()
{
    pSR |= (SR_N|SR_Z|SR_C|SR_I);
    m_icount -= 5;
    check_IRQ_lines();
}

void Ctms7000::idle()
{
    m_idle_state = 1;
    m_icount -= 6;
}

void Ctms7000::inc_a()
{
    UINT16  t;

    t = RDA + 1;

    WRA( t );

    CLR_NZC;
    SET_C8(t);
    SET_N8(t);
    SET_Z8(t);

    m_icount -= 5;
}

void Ctms7000::inc_b()
{
    UINT16  t;

    t = RDB + 1;

    WRB( t );

    CLR_NZC;
    SET_C8(t);
    SET_N8(t);
    SET_Z8(t);

    m_icount -= 5;
}

void Ctms7000::inc_r()
{
    UINT16  t;
    UINT8   r;

    IMMBYTE(r);

    t = RM(r) + 1;

    WM( r, t );

    CLR_NZC;
    SET_N8(t);
    SET_Z8(t);
    SET_C8(t);

    m_icount -= 7;
}

void Ctms7000::inv_a()
{
    UINT16 t;

    t = ~(RDA);
    WRA(t);

    CLR_NZC;
    SET_N8(t);
    SET_Z8(t);

    m_icount -= 5;
}

void Ctms7000::inv_b()
{
    UINT16 t;

    t = ~(RDB);
    WRB(t);

    CLR_NZC;
    SET_N8(t);
    SET_Z8(t);

    m_icount -= 5;
}

void Ctms7000::inv_r()
{
    UINT16  t;
    UINT8   r;

    IMMBYTE(r);

    t = ~(RM(r));

    WM( r, t );

    CLR_NZC;
    SET_N8(t);
    SET_Z8(t);

    m_icount -= 7;
}

void Ctms7000::jc()
{
    if( pSR & SR_C )
    {
        qint8 s;

        SIMMBYTE( s );
        ppc += s;
        m_icount -= 7;
    }
    else
    {
        ppc++;
        m_icount -= 5;
    }
}

void Ctms7000::jeq()
{
    if( pSR & SR_Z )
    {
        qint8 s;

        SIMMBYTE( s );
        ppc += s;
        m_icount -= 7;
    }
    else
    {
        ppc++;
        m_icount -= 5;
    }
}

void Ctms7000::jl()
{
    if( pSR & SR_C )
    {
        ppc++;
        m_icount -= 5;
    }
    else
    {
        qint8 s;

        SIMMBYTE( s );
        ppc += s;
        m_icount -= 7;
    }
}

void Ctms7000::jmp()
{
    qint8 s;

    SIMMBYTE( s );
    ppc += s;
    m_icount -= 7;
}

void Ctms7000::j_jn()
{
    if( pSR & SR_N )
    {
        qint8 s;

        SIMMBYTE( s );
        ppc += s;
        m_icount -= 7;
    }
    else
    {
        ppc++;
        m_icount -= 5;
    }

}

void Ctms7000::jne()
{
    if( pSR & SR_Z )
    {
        ppc++;
        m_icount -= 5;
    }
    else
    {
        qint8 s;

        SIMMBYTE( s );
        ppc += s;
        m_icount -= 7;
    }
}

void Ctms7000::jp()
{
    if( pSR & (SR_Z|SR_N) )
    {
        ppc++;
        m_icount -= 5;
    }
    else
    {
        qint8 s;

        SIMMBYTE( s );
        ppc += s;
        m_icount -= 7;
    }
}

void Ctms7000::jpz()
{
    // NOTE: JPZ in TI documentation was wrong:
    //     if ((pSR & SR_N) == 0 && (pSR & SR_Z) != 0)
    // should be:
    //     if ((pSR & SR_N) == 0)
    if ((pSR & SR_N) == 0)
    {
        qint8 s;

        SIMMBYTE( s );
        ppc += s;
        m_icount -= 7;
    }
    else
    {
        ppc++;
        m_icount -= 5;
    }
}

void Ctms7000::lda_dir()
{
    UINT16  t;
    PAIR    i;

    IMMWORD( i );
    t = RM(i.w.l);
    WRA(t);

    CLR_NZC;
    SET_N8(t);
    SET_Z8(t);

    m_icount -= 11;
}

void Ctms7000::lda_ind()
{
    UINT16  t;
    PAIR    p;
    qint8    i;

    IMMBYTE(i);
    p.w.l=RRF16(i);
    t = RM(p.w.l);
    WRA(t);

    CLR_NZC;
    SET_N8(t);
    SET_Z8(t);

    m_icount -= 10;
}

void Ctms7000::lda_inx()
{
    UINT16  t;
    PAIR    i;

    IMMWORD( i );
    t = RM(i.w.l + RDB);
    WRA(t);

    CLR_NZC;
    SET_N8(t);
    SET_Z8(t);

    m_icount -= 13;
}

void Ctms7000::ldsp()
{
    pSP = RDB;
    m_icount -= 5;
}

void Ctms7000::mov_a2b()
{
    UINT16  t;

    t = RDA;
    WRB(t);

    CLR_NZC;
    SET_N8(t);
    SET_Z8(t);

    m_icount -= 6;
}

void Ctms7000::mov_b2a()
{
    UINT16  t;

    t = RDB;
    WRA(t);

    CLR_NZC;
    SET_N8(t);
    SET_Z8(t);

    m_icount -= 5;
}


void Ctms7000::mov_a2r()
{
    UINT8   r;
    UINT16  t;

    IMMBYTE(r);

    t = RDA;
    WM(r,t);

    CLR_NZC;
    SET_N8(t);
    SET_Z8(t);

    m_icount -= 8;
}

void Ctms7000::mov_b2r()
{
    UINT8   r;
    UINT16  t;

    IMMBYTE(r);

    t = RDB;
    WM(r,t);

    CLR_NZC;
    SET_N8(t);
    SET_Z8(t);

    m_icount -= 7;
}

void Ctms7000::mov_r2a()
{
    UINT8   r;
    UINT16  t;

    IMMBYTE(r);
    t = RM(r);
    WRA(t);

    CLR_NZC;
    SET_N8(t);
    SET_Z8(t);

    m_icount -= 8;
}

void Ctms7000::mov_r2b()
{
    UINT8   r;
    UINT16  t;

    IMMBYTE(r);
    t = RM(r);
    WRB(t);

    CLR_NZC;
    SET_N8(t);
    SET_Z8(t);

    m_icount -= 8;
}

void Ctms7000::mov_r2r()
{
    UINT8   r,s;
    UINT16  t;

    IMMBYTE(r);
    IMMBYTE(s);
    t = RM(r);
    WM(s,t);

    CLR_NZC;
    SET_N8(t);
    SET_Z8(t);

    m_icount -= 10;
}

void Ctms7000::mov_i2a()
{
    UINT16  t;

    IMMBYTE(t);
    WRA(t);

    CLR_NZC;
    SET_N8(t);
    SET_Z8(t);

    m_icount -= 7;
}

void Ctms7000::mov_i2b()
{
    UINT16  t;

    IMMBYTE(t);
    WRB(t);

    CLR_NZC;
    SET_N8(t);
    SET_Z8(t);

    m_icount -= 7;
}

void Ctms7000::mov_i2r()
{
    UINT16  t;
    UINT8   r;

    IMMBYTE(t);
    IMMBYTE(r);
    WM(r,t);

    CLR_NZC;
    SET_N8(t);
    SET_Z8(t);

    m_icount -= 9;
}

void Ctms7000::movd_imm()
{
    PAIR    t;
    UINT8   r;

    IMMWORD(t);
    IMMBYTE(r);
    WRF16(r,t);

    CLR_NZC;
    SET_N8(t.b.h);
    SET_Z8(t.b.h);

    m_icount -= 15;

}

void Ctms7000::movd_r()
{
    PAIR    t;
    UINT8   r,s;

    IMMBYTE(r);
    IMMBYTE(s);
    t.w.l = RRF16(r);
    WRF16(s,t);

    CLR_NZC;
    SET_N8(t.b.h);
    SET_Z8(t.b.h);

    m_icount -= 14;

}

void Ctms7000::movd_inx()
{
    PAIR    t;
    UINT8   r;

    IMMWORD(t);
    t.w.l = (t.w.l+RDB)&0xFFFF;
    IMMBYTE(r);
    WRF16(r,t);

    CLR_NZC;
    SET_N8(t.b.h);
    SET_Z8(t.b.h);

    m_icount -= 17;
}

void Ctms7000::movp_a2p()
{
    UINT8   p;
    UINT16  t;

    IMMBYTE(p);
    t=RDA;
    WM( 0x0100+p,t);

    CLR_NZC;
    SET_N8(t);
    SET_Z8(t);

    m_icount -= 10;
}

void Ctms7000::movp_b2p()
{
    UINT8   p;
    UINT16  t;

    IMMBYTE(p);
    t=RDB;
    WM( 0x0100+p,t);

    CLR_NZC;
    SET_N8(t);
    SET_Z8(t);

    m_icount -= 10;
}

#if 0
/* this appears to be unused */
void Ctms7000::movp_r2p()
{
    UINT8   p,r;
    UINT16  t;

    IMMBYTE(r);
    IMMBYTE(p);
    t=RM(r);
    WM( 0x0100+p,t);

    CLR_NZC;
    SET_N8(t);
    SET_Z8(t);

    m_icount -= 11;
}
#endif

void Ctms7000::movp_p2a()
{
    UINT8   p;
    UINT16  t;

    IMMBYTE(p);
    t=RM(0x0100+p);
    WRA(t);

    CLR_NZC;
    SET_N8(t);
    SET_Z8(t);

    m_icount -= 9;
}

void Ctms7000::movp_p2b()
{
    UINT8   p;
    UINT16  t;

    IMMBYTE(p);
    t=RM(0x0100+p);
    WRB(t);

    CLR_NZC;
    SET_N8(t);
    SET_Z8(t);

    m_icount -= 8;
}

void Ctms7000::mpy_ba()
{
    PAIR t;

    t.w.l = RDA * RDB;

    WRF16(0x01,t);

    CLR_NZC;
    SET_N8(t.b.h);
    SET_Z8(t.b.h);

    m_icount -= 43;

}

void Ctms7000::mpy_ra()
{
    PAIR    t;
    UINT8   r;

    IMMBYTE(r);

    t.w.l = RDA * RM(r);

    WRF16(0x01,t);

    CLR_NZC;
    SET_N8(t.b.h);
    SET_Z8(t.b.h);

    m_icount -= 46;

}

void Ctms7000::mpy_rb()
{
    PAIR    t;
    UINT8   r;

    IMMBYTE(r);

    t.w.l = RDB * RM(r);

    WRF16(0x01,t);

    CLR_NZC;
    SET_N8(t.b.h);
    SET_Z8(t.b.h);

    m_icount -= 46;

}

void Ctms7000::mpy_rr()
{
    PAIR    t;
    UINT8   r,s;

    IMMBYTE(r);
    IMMBYTE(s);

    t.w.l = RM(s) * RM(r);

    WRF16(0x01,t);

    CLR_NZC;
    SET_N8(t.b.h);
    SET_Z8(t.b.h);

    m_icount -= 48;

}

void Ctms7000::mpy_ia()
{
    PAIR    t;
    UINT8   i;

    IMMBYTE(i);

    t.w.l = RDA * i;

    WRF16(0x01,t);

    CLR_NZC;
    SET_N8(t.b.h);
    SET_Z8(t.b.h);

    m_icount -= 45;

}

void Ctms7000::mpy_ib()
{
    PAIR    t;
    UINT8   i;

    IMMBYTE(i);

    t.w.l = RDB * i;

    WRF16(0x01,t);

    CLR_NZC;
    SET_N8(t.b.h);
    SET_Z8(t.b.h);

    m_icount -= 45;

}

void Ctms7000::mpy_ir()
{
    PAIR    t;
    UINT8   i,r;

    IMMBYTE(i);
    IMMBYTE(r);

    t.w.l = RM(r) * i;

    WRF16(0x01,t);

    CLR_NZC;
    SET_N8(t.b.h);
    SET_Z8(t.b.h);

    m_icount -= 47;

}

void Ctms7000::nop()
{
    m_icount -= 4;
}

void Ctms7000::or_b2a()
{
    UINT8   t;

    t = RDA | RDB;
    WRA(t);

    CLR_NZC;
    SET_N8(t);
    SET_Z8(t);

    m_icount -= 5;
}

void Ctms7000::or_r2a()
{
    UINT8   t;
    UINT8   v;

    IMMBYTE(v);

    t = RM(v) | RDA;
    WRA(t);

    CLR_NZC;
    SET_N8(t);
    SET_Z8(t);

    m_icount -= 8;
}

void Ctms7000::or_r2b()
{
    UINT8   t;
    UINT8   v;

    IMMBYTE(v);

    t = RM(v) | RDB;
    WRB(t);

    CLR_NZC;
    SET_N8(t);
    SET_Z8(t);

    m_icount -= 8;
}

void Ctms7000::or_r2r()
{
    UINT8   t;
    UINT8   i,j;

    IMMBYTE(i);
    IMMBYTE(j);

    t = RM(i) | RM(j);
    WM(j,t);

    CLR_NZC;
    SET_N8(t);
    SET_Z8(t);

    m_icount -= 10;
}

void Ctms7000::or_i2a()
{
    UINT8   t;
    UINT8   v;

    IMMBYTE(v);

    t = v | RDA;
    WRA(t);

    CLR_NZC;
    SET_N8(t);
    SET_Z8(t);

    m_icount -= 7;
}

void Ctms7000::or_i2b()
{
    UINT8   t;
    UINT8   v;

    IMMBYTE(v);

    t = v | RDB;
    WRB(t);

    CLR_NZC;
    SET_N8(t);
    SET_Z8(t);

    m_icount -= 7;
}

void Ctms7000::or_i2r()
{
    UINT8   t;
    UINT8   i,j;

    IMMBYTE(i);
    IMMBYTE(j);

    t = i | RM(j);
    WM(j,t);

    CLR_NZC;
    SET_N8(t);
    SET_Z8(t);

    m_icount -= 9;
}

void Ctms7000::orp_a2p()
{
    UINT8   t;
    UINT8   v;

    IMMBYTE(v);
    t = RDA | RM( 0x0100 + v);
    WM( 0x0100+v, t);

    CLR_NZC;
    SET_N8(t);
    SET_Z8(t);

    m_icount -= 10;
}

void Ctms7000::orp_b2p()
{
    UINT8   t;
    UINT8   v;

    IMMBYTE(v);
    t = RDB | RM( 0x0100 + v);
    WM( 0x0100+v, t);

    CLR_NZC;
    SET_N8(t);
    SET_Z8(t);

    m_icount -= 9;
}

void Ctms7000::orp_i2p()
{
    UINT8   t;
    UINT8   i,v;

    IMMBYTE(i);
    IMMBYTE(v);
    t = i | RM( 0x0100 + v);
    WM( 0x0100+v, t);

    CLR_NZC;
    SET_N8(t);
    SET_Z8(t);

    m_icount -= 11;
}

void Ctms7000::pop_a()
{
    UINT16  t;

    PULLBYTE(t);
    WRA(t);

    CLR_NZC;
    SET_N8(t);
    SET_Z8(t);

    m_icount -= 6;
}

void Ctms7000::pop_b()
{
    UINT16  t;

    PULLBYTE(t);
    WRB(t);

    CLR_NZC;
    SET_N8(t);
    SET_Z8(t);

    m_icount -= 6;
}

void Ctms7000::pop_r()
{
    UINT16  t;
    UINT8   r;

    IMMBYTE(r);
    PULLBYTE(t);
    WM(r,t);

    CLR_NZC;
    SET_N8(t);
    SET_Z8(t);

    m_icount -= 8;
}

void Ctms7000::pop_st()
{
    UINT16  t;

    PULLBYTE(t);
    pSR = t;

    m_icount -= 6;
}

void Ctms7000::push_a()
{
    UINT16  t;

    t = RDA;
    PUSHBYTE(t);

    CLR_NZC;
    SET_N8(t);
    SET_Z8(t);

    m_icount -= 6;
}

void Ctms7000::push_b()
{
    UINT16  t;

    t = RDB;
    PUSHBYTE(t);

    CLR_NZC;
    SET_N8(t);
    SET_Z8(t);

    m_icount -= 6;
}

void Ctms7000::push_r()
{
    UINT16  t;
    qint8    r;

    IMMBYTE(r);
    t = RM(r);
    PUSHBYTE(t);

    CLR_NZC;
    SET_N8(t);
    SET_Z8(t);

    m_icount -= 8;
}

void Ctms7000::push_st()
{
    UINT16  t;
    t = pSR;
    PUSHBYTE(t);

    m_icount -= 6;
}

void Ctms7000::reti()
{
    CallSubLevel--;
    PULLWORD( PC );
    PULLBYTE( pSR );

    m_icount -= 9;
    check_IRQ_lines();
}

void Ctms7000::rets()
{
    CallSubLevel--;
    PULLWORD( PC );
    m_icount -= 7;
}

void Ctms7000::rl_a()
{
    UINT16  t;

    t = RDA << 1;

    CLR_NZC;
    SET_C8(t);

    if( pSR & SR_C )
        t |= 0x01;

    SET_N8(t);
    SET_Z8(t);
    WRA(t);

    m_icount -= 5;
}

void Ctms7000::rl_b()
{
    UINT16  t;

    t = RDB << 1;

    CLR_NZC;
    SET_C8(t);

    if( pSR & SR_C )
        t |= 0x01;

    SET_N8(t);
    SET_Z8(t);
    WRB(t);

    m_icount -= 5;
}

void Ctms7000::rl_r()
{
    UINT16  t;
    UINT8   r;

    IMMBYTE(r);
    t = RM(r) << 1;

    CLR_NZC;
    SET_C8(t);

    if( pSR & SR_C )
        t |= 0x01;

    SET_N8(t);
    SET_Z8(t);
    WM(r,t);

    m_icount -= 7;
}

void Ctms7000::rlc_a()
{
    UINT16  t;
    int     old_carry;

    old_carry = (pSR & SR_C);

    t = RDA << 1;

    CLR_NZC;
    SET_C8(t);

    if( old_carry )
        t |= 0x01;

    SET_N8(t);
    SET_Z8(t);
    WRA(t);

    m_icount -= 5;
}

void Ctms7000::rlc_b()
{
    UINT16  t;
    int     old_carry;

    old_carry = (pSR & SR_C);

    t = RDB << 1;

    CLR_NZC;
    SET_C8(t);

    if( old_carry )
        t |= 0x01;

    SET_N8(t);
    SET_Z8(t);
    WRB(t);

    m_icount -= 5;
}

void Ctms7000::rlc_r()
{
    UINT16  t;
    UINT8   r;
    int     old_carry;

    old_carry = (pSR & SR_C);

    IMMBYTE(r);
    t = RM(r) << 1;

    CLR_NZC;
    SET_C8(t);

    if( old_carry )
        t |= 0x01;

    SET_N8(t);
    SET_Z8(t);
    WM(r,t);

    m_icount -= 7;
}

void Ctms7000::rr_a()
{
    UINT16  t;
    int     old_bit0;

    t = RDA;

    old_bit0 = t & 0x0001;
    t = t >> 1;

    CLR_NZC;

    if( old_bit0 )
    {
        SETC;
        t |= 0x80;
    }

    SET_N8(t);
    SET_Z8(t);

    WRA(t);

    m_icount -= 5;
}

void Ctms7000::rr_b()
{
    UINT16  t;
    int     old_bit0;

    t = RDB;

    old_bit0 = t & 0x0001;
    t = t >> 1;

    CLR_NZC;

    if( old_bit0 )
    {
        SETC;
        t |= 0x80;
    }

    SET_N8(t);
    SET_Z8(t);

    WRB(t);

    m_icount -= 5;
}

void Ctms7000::rr_r()
{
    UINT16  t;
    UINT8   r;

    int     old_bit0;

    IMMBYTE(r);
    t = RM(r);

    old_bit0 = t & 0x0001;
    t = t >> 1;

    CLR_NZC;

    if( old_bit0 )
    {
        SETC;
        t |= 0x80;
    }

    SET_N8(t);
    SET_Z8(t);

    WM(r,t);

    m_icount -= 7;
}

void Ctms7000::rrc_a()
{
    UINT16  t;
    int     old_bit0;

    t = RDA;

    old_bit0 = t & 0x0001;
    /* Place carry bit in 9th position */
    t |= ((pSR & SR_C) << 1);
    t = t >> 1;

    CLR_NZC;

    if( old_bit0 )
        SETC;
    SET_N8(t);
    SET_Z8(t);

    WRA(t);

    m_icount -= 5;
}

void Ctms7000::rrc_b()
{
    UINT16  t;
    int     old_bit0;

    t = RDB;

    old_bit0 = t & 0x0001;
    /* Place carry bit in 9th position */
    t |= ((pSR & SR_C) << 1);
    t = t >> 1;

    CLR_NZC;

    if( old_bit0 )
        SETC;
    SET_N8(t);
    SET_Z8(t);

    WRB(t);

    m_icount -= 5;
}

void Ctms7000::rrc_r()
{
    UINT16  t;
    UINT8   r;
    int     old_bit0;

    IMMBYTE(r);
    t = RM(r);

    old_bit0 = t & 0x0001;
    /* Place carry bit in 9th position */
    t |= ((pSR & SR_C) << 1);
    t = t >> 1;

    CLR_NZC;

    if( old_bit0 )
        SETC;
    SET_N8(t);
    SET_Z8(t);

    WM(r,t);

    m_icount -= 7;
}

void Ctms7000::sbb_ba()
{
    UINT16  t;

    t = RDA - RDB - ((pSR & SR_C) ? 0 : 1);
    WRA(t);

    CLR_NZC;
    SET_C8(~t);
    SET_N8(t);
    SET_Z8(t);

    m_icount -= 5;
}

void Ctms7000::sbb_ra()
{
    UINT16  t;
    UINT8   r;

    IMMBYTE(r);
    t = RDA - RM(r) - ((pSR & SR_C) ? 0 : 1);
    WRA(t);

    CLR_NZC;
    SET_C8(~t);
    SET_N8(t);
    SET_Z8(t);

    m_icount -= 8;
}

void Ctms7000::sbb_rb()
{
    UINT16  t;
    UINT8   r;

    IMMBYTE(r);
    t = RDB - RM(r) - ((pSR & SR_C) ? 0 : 1);
    WRB(t);

    CLR_NZC;
    SET_C8(~t);
    SET_N8(t);
    SET_Z8(t);

    m_icount -= 8;
}

void Ctms7000::sbb_rr()
{
    UINT16  t;
    UINT8   r,s;

    IMMBYTE(s);
    IMMBYTE(r);
    t = RM(r) - RM(s) - ((pSR & SR_C) ? 0 : 1);
    WM(r,t);

    CLR_NZC;
    SET_C8(~t);
    SET_N8(t);
    SET_Z8(t);

    m_icount -= 10;
}

void Ctms7000::sbb_ia()
{
    UINT16  t;
    UINT8   i;

    IMMBYTE(i);
    t = RDA - i - ((pSR & SR_C) ? 0 : 1);
    WRA(t);

    CLR_NZC;
    SET_C8(~t);
    SET_N8(t);
    SET_Z8(t);

    m_icount -= 7;
}

void Ctms7000::sbb_ib()
{
    UINT16  t;
    UINT8   i;

    IMMBYTE(i);
    t = RDB - i - ((pSR & SR_C) ? 0 : 1);
    WRB(t);

    CLR_NZC;
    SET_C8(~t);
    SET_N8(t);
    SET_Z8(t);

    m_icount -= 7;
}

void Ctms7000::sbb_ir()
{
    UINT16  t;
    UINT8   r,i;

    IMMBYTE(i);
    IMMBYTE(r);
    t = RM(r) - i - ((pSR & SR_C) ? 0 : 1);
    WM(r,t);

    CLR_NZC;
    SET_C8(~t);
    SET_N8(t);
    SET_Z8(t);

    m_icount -= 9;
}

void Ctms7000::setc()
{
    CLR_NZC;
    pSR |= (SR_C|SR_Z);

    m_icount -= 5;
}

void Ctms7000::sta_dir()
{
    UINT16  t;
    PAIR    i;

    t = RDA;
    IMMWORD( i );

    WM(i.w.l,t);

    CLR_NZC;
    SET_N8(t);
    SET_Z8(t);

    m_icount -= 11;
}

void Ctms7000::sta_ind()
{
    UINT16  t;
    PAIR    p;
    qint8    r;

    IMMBYTE(r);
    p.w.l = RRF16(r);
    t = RDA;
    WM(p.w.l,t);

    CLR_NZC;
    SET_N8(t);
    SET_Z8(t);

    m_icount -= 10;
}

void Ctms7000::sta_inx()
{
    UINT16  t;
    PAIR    i;

    IMMWORD( i );
    t = RDA;
    WM(i.w.l+RDB,t);

    CLR_NZC;
    SET_N8(t);
    SET_Z8(t);

    m_icount -= 13;
}

void Ctms7000::stsp()
{
    WRB(pSP);

    m_icount -= 6;
}

void Ctms7000::sub_ba()
{
    UINT16  t;

    t = RDA - RDB;
    WRA(t);

    CLR_NZC;
    SET_C8(~t);
    SET_N8(t);
    SET_Z8(t);

    m_icount -= 5;
}

void Ctms7000::sub_ra()
{
    UINT16  t;
    UINT8   r;

    IMMBYTE(r);
    t = RDA - RM(r);
    WRA(t);

    CLR_NZC;
    SET_C8(~t);
    SET_N8(t);
    SET_Z8(t);

    m_icount -= 8;
}

void Ctms7000::sub_rb()
{
    UINT16  t;
    UINT8   r;

    IMMBYTE(r);
    t = RDB - RM(r);
    WRB(t);

    CLR_NZC;
    SET_C8(~t);
    SET_N8(t);
    SET_Z8(t);

    m_icount -= 8;
}

void Ctms7000::sub_rr()
{
    UINT16  t;
    UINT8   r,s;

    IMMBYTE(s);
    IMMBYTE(r);
    t = RM(r) - RM(s);
    WM(r,t);

    CLR_NZC;
    SET_C8(~t);
    SET_N8(t);
    SET_Z8(t);

    m_icount -= 10;
}

void Ctms7000::sub_ia()
{
    UINT16  t;
    UINT8   i;

    IMMBYTE(i);
    t = RDA - i;
    WRA(t);

    CLR_NZC;
    SET_C8(~t);
    SET_N8(t);
    SET_Z8(t);

    m_icount -= 7;
}

void Ctms7000::sub_ib()
{
    UINT16  t;
    UINT8   i;

    IMMBYTE(i);
    t = RDB - i;
    WRB(t);

    CLR_NZC;
    SET_C8(~t);
    SET_N8(t);
    SET_Z8(t);

    m_icount -= 7;
}

void Ctms7000::sub_ir()
{
    UINT16  t;
    UINT8   r,i;

    IMMBYTE(i);
    IMMBYTE(r);
    t = RM(r) - i;
    WM(r,t);

    CLR_NZC;
    SET_C8(~t);
    SET_N8(t);
    SET_Z8(t);

    m_icount -= 9;
}

void Ctms7000::trap_0()
{
    CallSubLevel++;
    PUSHWORD( PC );
    ppc = RM16(0xfffe);
    m_icount -= 14;
}

void Ctms7000::trap_1()
{
    CallSubLevel++;
    PUSHWORD( PC );
    ppc = RM16(0xfffc);
    m_icount -= 14;
}

void Ctms7000::trap_2()
{
    CallSubLevel++;
    PUSHWORD( PC );
    ppc = RM16(0xfffa);
    m_icount -= 14;
}

void Ctms7000::trap_3()
{
    CallSubLevel++;
    PUSHWORD( PC );
    ppc = RM16(0xfff8);
    m_icount -= 14;
}

void Ctms7000::trap_4()
{
    CallSubLevel++;
    PUSHWORD( PC );
    ppc = RM16(0xfff6);
    m_icount -= 14;
}

void Ctms7000::trap_5()
{
    CallSubLevel++;
    PUSHWORD( PC );
    ppc = RM16(0xfff4);
    m_icount -= 14;
}

void Ctms7000::trap_6()
{
    CallSubLevel++;
    PUSHWORD( PC );
    ppc = RM16(0xfff2);
    m_icount -= 14;
}

void Ctms7000::trap_7()
{
    CallSubLevel++;
    PUSHWORD( PC );
    ppc = RM16(0xfff0);
    m_icount -= 14;
}

void Ctms7000::trap_8()
{
    CallSubLevel++;
    PUSHWORD( PC );
    ppc = RM16(0xffee);
    m_icount -= 14;
}

void Ctms7000::trap_9()
{
    CallSubLevel++;
    PUSHWORD( PC );
    ppc = RM16(0xffec);
    m_icount -= 14;
}

void Ctms7000::trap_10()
{
    CallSubLevel++;
    PUSHWORD( PC );
    ppc = RM16(0xffea);
    m_icount -= 14;
}

void Ctms7000::trap_11()
{
    CallSubLevel++;
    PUSHWORD( PC );
    ppc = RM16(0xffe8);
    m_icount -= 14;
}

void Ctms7000::trap_12()
{
    CallSubLevel++;
    PUSHWORD( PC );
    ppc = RM16(0xffe6);
    m_icount -= 14;
}

void Ctms7000::trap_13()
{
    CallSubLevel++;
    PUSHWORD( PC );
    ppc = RM16(0xffe4);
    m_icount -= 14;
}

void Ctms7000::trap_14()
{
    CallSubLevel++;
    PUSHWORD( PC );
    ppc = RM16(0xffe2);
    m_icount -= 14;
}

void Ctms7000::trap_15()
{
    CallSubLevel++;
    PUSHWORD( PC );
    ppc = RM16(0xffe0);
    m_icount -= 14;
}

void Ctms7000::trap_16()
{
    CallSubLevel++;
    PUSHWORD( PC );
    ppc = RM16(0xffde);
    m_icount -= 14;
}

void Ctms7000::trap_17()
{
    CallSubLevel++;
    PUSHWORD( PC );
    ppc = RM16(0xffdc);
    m_icount -= 14;
}

void Ctms7000::trap_18()
{
    CallSubLevel++;
    PUSHWORD( PC );
    ppc = RM16(0xffda);
    m_icount -= 14;
}

void Ctms7000::trap_19()
{
    CallSubLevel++;
    PUSHWORD( PC );
    ppc = RM16(0xffd8);
    m_icount -= 14;
}

void Ctms7000::trap_20()
{
    CallSubLevel++;
    PUSHWORD( PC );
    ppc = RM16(0xffd6);
    m_icount -= 14;
}

void Ctms7000::trap_21()
{
    CallSubLevel++;
    PUSHWORD( PC );
    ppc = RM16(0xffd4);
    m_icount -= 14;
}

void Ctms7000::trap_22()
{
    CallSubLevel++;
    PUSHWORD( PC );
    ppc = RM16(0xffd2);
    m_icount -= 14;
}

void Ctms7000::trap_23()
{
    CallSubLevel++;
    PUSHWORD( PC );
    ppc = RM16(0xffd0);
    m_icount -= 14;
}

void Ctms7000::swap_a()
{
    UINT8   a,b;
    UINT16  t;

    a = b = RDA;

    a <<= 4;
    b >>= 4;
    t = a+b;

    WRA(t);

    CLR_NZC;

    pSR|=((t&0x0001)<<7);
    SET_N8(t);
    SET_Z8(t);

    m_icount -=8;
}

void Ctms7000::swap_b()
{
    UINT8   a,b;
    UINT16  t;

    a = b = RDB;

    a <<= 4;
    b >>= 4;
    t = a+b;

    WRB(t);

    CLR_NZC;

    pSR|=((t&0x0001)<<7);
    SET_N8(t);
    SET_Z8(t);

    m_icount -=8;
}

void Ctms7000::swap_r()
{
    UINT8   a,b,r;
    UINT16  t;

    IMMBYTE(r);
    a = b = RM(r);

    a <<= 4;
    b >>= 4;
    t = a+b;

    WM(r,t);

    CLR_NZC;

    pSR|=((t&0x0001)<<7);
    SET_N8(t);
    SET_Z8(t);

    m_icount -=8;
}

void Ctms7000::swap_r_exl()
{
    UINT16	t;

    SKIPBYTE();

    /* opcode D7 ?? (LVDP) mostly equivalent to
     * MOVP P40,xx
     * MOVP P36,A
     */
    RM(0x0128);
    t=RM(0x0124);
    WM(0,t);

    CLR_NZC;
    SET_N8(t);
    SET_Z8(t);

    m_icount -= 9;	/* FIXME : check real timing */
}

void Ctms7000::tstb()
{
    UINT16  t;

    t=RDB;

    CLR_NZC;
    SET_N8(t);
    SET_Z8(t);

    m_icount -= 6;
}

void Ctms7000::xchb_a()
{
    UINT16  t,u;

    t = RDB;
    u = RDA;

    WRA(t);
    WRB(u);

    CLR_NZC;
    SET_N8(t);
    SET_Z8(t);

    m_icount -= 6;
}

void Ctms7000::xchb_b()
{
    UINT16  t;

    t = RDB;

    CLR_NZC;
    SET_N8(t);
    SET_Z8(t);

    m_icount -= 6;
}

void Ctms7000::xchb_r()
{
    UINT16  t,u;
    UINT8   r;

    IMMBYTE(r);

    t = RDB;
    u = RM(r);

    WM(r,t);
    WRB(u);

    CLR_NZC;
    SET_N8(t);
    SET_Z8(t);

    m_icount -= 8;
}

void Ctms7000::xor_b2a()
{
    UINT8   t;

    t = RDA ^ RDB;
    WRA(t);

    CLR_NZC;
    SET_N8(t);
    SET_Z8(t);

    m_icount -= 5;
}

void Ctms7000::xor_r2a()
{
    UINT8   t;
    UINT8   v;

    IMMBYTE(v);

    t = RM(v) ^ RDA;
    WRA(t);

    CLR_NZC;
    SET_N8(t);
    SET_Z8(t);

    m_icount -= 8;
}

void Ctms7000::xor_r2b()
{
    UINT8   t;
    UINT8   v;

    IMMBYTE(v);

    t = RM(v) ^ RDB;
    WRB(t);

    CLR_NZC;
    SET_N8(t);
    SET_Z8(t);

    m_icount -= 8;
}

void Ctms7000::xor_r2r()
{
    UINT8   t;
    UINT8   i,j;

    IMMBYTE(i);
    IMMBYTE(j);

    t = RM(i) ^ RM(j);
    WM(j,t);

    CLR_NZC;
    SET_N8(t);
    SET_Z8(t);

    m_icount -= 10;
}

void Ctms7000::xor_i2a()
{
    UINT8   t;
    UINT8   v;

    IMMBYTE(v);

    t = v ^ RDA;
    WRA(t);

    CLR_NZC;
    SET_N8(t);
    SET_Z8(t);

    m_icount -= 7;
}

void Ctms7000::xor_i2b()
{
    UINT8   t;
    UINT8   v;

    IMMBYTE(v);

    t = v ^ RDB;
    WRB(t);

    CLR_NZC;
    SET_N8(t);
    SET_Z8(t);

    m_icount -= 7;
}

void Ctms7000::xor_i2r()
{
    UINT8   t;
    UINT8   i,j;

    IMMBYTE(i);
    IMMBYTE(j);

    t = i ^ RM(j);
    WM(j,t);

    CLR_NZC;
    SET_N8(t);
    SET_Z8(t);

    m_icount -= 9;
}

void Ctms7000::xorp_a2p()
{
    UINT8   t;
    UINT8   v;

    IMMBYTE(v);
    t = RDA ^ RM( 0x0100 + v);
    WM( 0x0100+v, t);

    CLR_NZC;
    SET_N8(t);
    SET_Z8(t);

    m_icount -= 10;
}

void Ctms7000::xorp_b2p()
{
    UINT8   t;
    UINT8   v;

    IMMBYTE(v);
    t = RDB ^ RM( 0x0100 + v);
    WM( 0x0100+v, t);

    CLR_NZC;
    SET_N8(t);
    SET_Z8(t);

    m_icount -= 9;
}

void Ctms7000::xorp_i2p()
{
    UINT8   t;
    UINT8   i,v;

    IMMBYTE(i);
    IMMBYTE(v);
    t = i ^ RM( 0x0100 + v);
    WM( 0x0100+v, t);

    CLR_NZC;
    SET_N8(t);
    SET_Z8(t);

    m_icount -= 11;
}



/*****************************************************************************
 *
 *   tms70tb.inc (function table)
 *   Portable TMS7000 emulator (Texas Instruments 7000)
 *
 *   Copyright tim lindner, all rights reserved.
 *
 *   - This source code is released as freeware for non-commercial purposes.
 *   - You are free to use and redistribute this code in modified or
 *     unmodified form, provided you list me in the credits.
 *   - If you modify this source code, you must add a notice to each modified
 *     source file that it has been changed.  If you're a nice person, you
 *     will clearly mark each change too.  :)
 *   - If you wish to use this for commercial purposes, please contact me at
 *     tlindner@macmess.org
 *   - This entire notice must remain in the source code.
 *
 *****************************************************************************/

const Ctms7000::opcode_func Ctms7000::s_opfn[0x100] = {
/*          0xX0,   0xX1,     0xX2,    0xX3,    0xX4,    0xX5,    0xX6,    0xX7,
            0xX8,   0xX9,     0xXA,    0xXB,    0xXC,    0xXD,    0xXE,    0xXF   */

/* 0x0X */  &Ctms7000::nop,     &Ctms7000::idle,    &Ctms7000::illegal, &Ctms7000::illegal, &Ctms7000::illegal, &Ctms7000::eint,    &Ctms7000::dint,    &Ctms7000::setc,
            &Ctms7000::pop_st,  &Ctms7000::stsp,    &Ctms7000::rets,    &Ctms7000::reti,    &Ctms7000::illegal, &Ctms7000::ldsp,    &Ctms7000::push_st, &Ctms7000::illegal,

/* 0x1X */  &Ctms7000::illegal, &Ctms7000::illegal, &Ctms7000::mov_r2a, &Ctms7000::and_r2a, &Ctms7000::or_r2a,  &Ctms7000::xor_r2a, &Ctms7000::btjo_r2a,&Ctms7000::btjz_r2a,
            &Ctms7000::add_r2a, &Ctms7000::adc_r2a, &Ctms7000::sub_ra,  &Ctms7000::sbb_ra,  &Ctms7000::mpy_ra,  &Ctms7000::cmp_ra,  &Ctms7000::dac_r2a, &Ctms7000::dsb_r2a,

/* 0x2X */  &Ctms7000::illegal, &Ctms7000::illegal, &Ctms7000::mov_i2a, &Ctms7000::and_i2a, &Ctms7000::or_i2a,  &Ctms7000::xor_i2a, &Ctms7000::btjo_i2a,&Ctms7000::btjz_i2a,
            &Ctms7000::add_i2a, &Ctms7000::adc_i2a, &Ctms7000::sub_ia,  &Ctms7000::sbb_ia,  &Ctms7000::mpy_ia,  &Ctms7000::cmp_ia,  &Ctms7000::dac_i2a, &Ctms7000::dsb_i2a,

/* 0x3X */  &Ctms7000::illegal, &Ctms7000::illegal, &Ctms7000::mov_r2b, &Ctms7000::and_r2b, &Ctms7000::or_r2b,  &Ctms7000::xor_r2b, &Ctms7000::btjo_r2b,&Ctms7000::btjz_r2b,
            &Ctms7000::add_r2b, &Ctms7000::adc_r2b, &Ctms7000::sub_rb,  &Ctms7000::sbb_rb,  &Ctms7000::mpy_rb,  &Ctms7000::cmp_rb,  &Ctms7000::dac_r2b, &Ctms7000::dsb_r2b,

/* 0x4X */  &Ctms7000::illegal, &Ctms7000::illegal, &Ctms7000::mov_r2r, &Ctms7000::and_r2r, &Ctms7000::or_r2r,  &Ctms7000::xor_r2r, &Ctms7000::btjo_r2r,&Ctms7000::btjz_r2r,
            &Ctms7000::add_r2r, &Ctms7000::adc_r2r, &Ctms7000::sub_rr,  &Ctms7000::sbb_rr,  &Ctms7000::mpy_rr,  &Ctms7000::cmp_rr,  &Ctms7000::dac_r2r, &Ctms7000::dsb_r2r,

/* 0x5X */  &Ctms7000::illegal, &Ctms7000::illegal, &Ctms7000::mov_i2b, &Ctms7000::and_i2b, &Ctms7000::or_i2b,  &Ctms7000::xor_i2b, &Ctms7000::btjo_i2b,&Ctms7000::btjz_i2b,
            &Ctms7000::add_i2b, &Ctms7000::adc_i2b, &Ctms7000::sub_ib,  &Ctms7000::sbb_ib,  &Ctms7000::mpy_ib,  &Ctms7000::cmp_ib,  &Ctms7000::dac_i2b, &Ctms7000::dsb_i2b,

/* 0x6X */  &Ctms7000::illegal, &Ctms7000::illegal, &Ctms7000::mov_b2a, &Ctms7000::and_b2a, &Ctms7000::or_b2a,  &Ctms7000::xor_b2a, &Ctms7000::btjo_b2a,&Ctms7000::btjz_b2a,
            &Ctms7000::add_b2a, &Ctms7000::adc_b2a, &Ctms7000::sub_ba,  &Ctms7000::sbb_ba,  &Ctms7000::mpy_ba,  &Ctms7000::cmp_ba,  &Ctms7000::dac_b2a, &Ctms7000::dsb_b2a,

/* 0x7X */  &Ctms7000::illegal, &Ctms7000::illegal, &Ctms7000::mov_i2r, &Ctms7000::and_i2r, &Ctms7000::or_i2r,  &Ctms7000::xor_i2r, &Ctms7000::btjo_i2r,&Ctms7000::btjz_i2r,
            &Ctms7000::add_i2r, &Ctms7000::adc_i2r, &Ctms7000::sub_ir,  &Ctms7000::sbb_ir,  &Ctms7000::mpy_ir,  &Ctms7000::cmp_ir,  &Ctms7000::dac_i2r, &Ctms7000::dsb_i2r,

/* 0x8X */  &Ctms7000::movp_p2a,&Ctms7000::illegal, &Ctms7000::movp_a2p,&Ctms7000::andp_a2p,&Ctms7000::orp_a2p, &Ctms7000::xorp_a2p,&Ctms7000::btjop_ap,&Ctms7000::btjzp_ap,
            &Ctms7000::movd_imm,&Ctms7000::illegal, &Ctms7000::lda_dir, &Ctms7000::sta_dir, &Ctms7000::br_dir,  &Ctms7000::cmpa_dir,&Ctms7000::call_dir,&Ctms7000::illegal,

/* 0x9X */  &Ctms7000::illegal, &Ctms7000::movp_p2b,&Ctms7000::movp_b2p,&Ctms7000::andp_b2p,&Ctms7000::orp_b2p, &Ctms7000::xorp_b2p,&Ctms7000::btjop_bp,&Ctms7000::btjzp_bp,
            &Ctms7000::movd_r,  &Ctms7000::illegal, &Ctms7000::lda_ind, &Ctms7000::sta_ind, &Ctms7000::br_ind,  &Ctms7000::cmpa_ind,&Ctms7000::call_ind,&Ctms7000::illegal,

/* 0xAX */  &Ctms7000::illegal, &Ctms7000::illegal, &Ctms7000::movp_i2p,&Ctms7000::andp_i2p,&Ctms7000::orp_i2p, &Ctms7000::xorp_i2p,&Ctms7000::btjop_ip,&Ctms7000::btjzp_ip,
            &Ctms7000::movd_inx,&Ctms7000::illegal, &Ctms7000::lda_inx, &Ctms7000::sta_inx, &Ctms7000::br_inx,  &Ctms7000::cmpa_inx,&Ctms7000::call_inx,&Ctms7000::illegal,

/* 0xBX */  &Ctms7000::clrc,    &Ctms7000::illegal, &Ctms7000::dec_a,   &Ctms7000::inc_a,   &Ctms7000::inv_a,   &Ctms7000::clr_a,   &Ctms7000::xchb_a,  &Ctms7000::swap_a,
            &Ctms7000::push_a,  &Ctms7000::pop_a,   &Ctms7000::djnz_a,  &Ctms7000::decd_a,  &Ctms7000::rr_a,    &Ctms7000::rrc_a,   &Ctms7000::rl_a,    &Ctms7000::rlc_a,

/* 0xCX */  &Ctms7000::mov_a2b, &Ctms7000::tstb,    &Ctms7000::dec_b,   &Ctms7000::inc_b,   &Ctms7000::inv_b,   &Ctms7000::clr_b,   &Ctms7000::xchb_b,  &Ctms7000::swap_b,
            &Ctms7000::push_b,  &Ctms7000::pop_b,   &Ctms7000::djnz_b,  &Ctms7000::decd_b,  &Ctms7000::rr_b,    &Ctms7000::rrc_b,   &Ctms7000::rl_b,    &Ctms7000::rlc_b,

/* 0xDX */  &Ctms7000::mov_a2r, &Ctms7000::mov_b2r, &Ctms7000::dec_r,   &Ctms7000::inc_r,   &Ctms7000::inv_r,   &Ctms7000::clr_r,   &Ctms7000::xchb_r,  &Ctms7000::swap_r,
            &Ctms7000::push_r,  &Ctms7000::pop_r,   &Ctms7000::djnz_r,  &Ctms7000::decd_r,  &Ctms7000::rr_r,    &Ctms7000::rrc_r,   &Ctms7000::rl_r,    &Ctms7000::rlc_r,

/* 0xEX */  &Ctms7000::jmp,     &Ctms7000::j_jn,    &Ctms7000::jeq,     &Ctms7000::jc,      &Ctms7000::jp,      &Ctms7000::jpz,     &Ctms7000::jne,     &Ctms7000::jl,
            &Ctms7000::trap_23, &Ctms7000::trap_22, &Ctms7000::trap_21, &Ctms7000::trap_20, &Ctms7000::trap_19, &Ctms7000::trap_18, &Ctms7000::trap_17, &Ctms7000::trap_16,

/* 0xFX */  &Ctms7000::trap_15, &Ctms7000::trap_14, &Ctms7000::trap_13, &Ctms7000::trap_12, &Ctms7000::trap_11, &Ctms7000::trap_10, &Ctms7000::trap_9,  &Ctms7000::trap_8,
            &Ctms7000::trap_7,  &Ctms7000::trap_6,  &Ctms7000::trap_5,  &Ctms7000::trap_4,  &Ctms7000::trap_3,  &Ctms7000::trap_2,  &Ctms7000::trap_1,  &Ctms7000::trap_0
};

const Ctms7000::opcode_func Ctms7000::s_opfn_exl[0x100] = {
/*          0xX0,   0xX1,     0xX2,    0xX3,    0xX4,    0xX5,    0xX6,    0xX7,
            0xX8,   0xX9,     0xXA,    0xXB,    0xXC,    0xXD,    0xXE,    0xXF   */

/* 0x0X */  &Ctms7000::nop,     &Ctms7000::idle,    &Ctms7000::illegal, &Ctms7000::illegal, &Ctms7000::illegal, &Ctms7000::eint,    &Ctms7000::dint,    &Ctms7000::setc,
            &Ctms7000::pop_st,  &Ctms7000::stsp,    &Ctms7000::rets,    &Ctms7000::reti,    &Ctms7000::illegal, &Ctms7000::ldsp,    &Ctms7000::push_st, &Ctms7000::illegal,

/* 0x1X */  &Ctms7000::illegal, &Ctms7000::illegal, &Ctms7000::mov_r2a, &Ctms7000::and_r2a, &Ctms7000::or_r2a,  &Ctms7000::xor_r2a, &Ctms7000::btjo_r2a,&Ctms7000::btjz_r2a,
            &Ctms7000::add_r2a, &Ctms7000::adc_r2a, &Ctms7000::sub_ra,  &Ctms7000::sbb_ra,  &Ctms7000::mpy_ra,  &Ctms7000::cmp_ra,  &Ctms7000::dac_r2a, &Ctms7000::dsb_r2a,

/* 0x2X */  &Ctms7000::illegal, &Ctms7000::illegal, &Ctms7000::mov_i2a, &Ctms7000::and_i2a, &Ctms7000::or_i2a,  &Ctms7000::xor_i2a, &Ctms7000::btjo_i2a,&Ctms7000::btjz_i2a,
            &Ctms7000::add_i2a, &Ctms7000::adc_i2a, &Ctms7000::sub_ia,  &Ctms7000::sbb_ia,  &Ctms7000::mpy_ia,  &Ctms7000::cmp_ia,  &Ctms7000::dac_i2a, &Ctms7000::dsb_i2a,

/* 0x3X */  &Ctms7000::illegal, &Ctms7000::illegal, &Ctms7000::mov_r2b, &Ctms7000::and_r2b, &Ctms7000::or_r2b,  &Ctms7000::xor_r2b, &Ctms7000::btjo_r2b,&Ctms7000::btjz_r2b,
            &Ctms7000::add_r2b, &Ctms7000::adc_r2b, &Ctms7000::sub_rb,  &Ctms7000::sbb_rb,  &Ctms7000::mpy_rb,  &Ctms7000::cmp_rb,  &Ctms7000::dac_r2b, &Ctms7000::dsb_r2b,

/* 0x4X */  &Ctms7000::illegal, &Ctms7000::illegal, &Ctms7000::mov_r2r, &Ctms7000::and_r2r, &Ctms7000::or_r2r,  &Ctms7000::xor_r2r, &Ctms7000::btjo_r2r,&Ctms7000::btjz_r2r,
            &Ctms7000::add_r2r, &Ctms7000::adc_r2r, &Ctms7000::sub_rr,  &Ctms7000::sbb_rr,  &Ctms7000::mpy_rr,  &Ctms7000::cmp_rr,  &Ctms7000::dac_r2r, &Ctms7000::dsb_r2r,

/* 0x5X */  &Ctms7000::illegal, &Ctms7000::illegal, &Ctms7000::mov_i2b, &Ctms7000::and_i2b, &Ctms7000::or_i2b,  &Ctms7000::xor_i2b, &Ctms7000::btjo_i2b,&Ctms7000::btjz_i2b,
            &Ctms7000::add_i2b, &Ctms7000::adc_i2b, &Ctms7000::sub_ib,  &Ctms7000::sbb_ib,  &Ctms7000::mpy_ib,  &Ctms7000::cmp_ib,  &Ctms7000::dac_i2b, &Ctms7000::dsb_i2b,

/* 0x6X */  &Ctms7000::illegal, &Ctms7000::illegal, &Ctms7000::mov_b2a, &Ctms7000::and_b2a, &Ctms7000::or_b2a,  &Ctms7000::xor_b2a, &Ctms7000::btjo_b2a,&Ctms7000::btjz_b2a,
            &Ctms7000::add_b2a, &Ctms7000::adc_b2a, &Ctms7000::sub_ba,  &Ctms7000::sbb_ba,  &Ctms7000::mpy_ba,  &Ctms7000::cmp_ba,  &Ctms7000::dac_b2a, &Ctms7000::dsb_b2a,

/* 0x7X */  &Ctms7000::illegal, &Ctms7000::illegal, &Ctms7000::mov_i2r, &Ctms7000::and_i2r, &Ctms7000::or_i2r,  &Ctms7000::xor_i2r, &Ctms7000::btjo_i2r,&Ctms7000::btjz_i2r,
            &Ctms7000::add_i2r, &Ctms7000::adc_i2r, &Ctms7000::sub_ir,  &Ctms7000::sbb_ir,  &Ctms7000::mpy_ir,  &Ctms7000::cmp_ir,  &Ctms7000::dac_i2r, &Ctms7000::dsb_i2r,

/* 0x8X */  &Ctms7000::movp_p2a,&Ctms7000::illegal, &Ctms7000::movp_a2p,&Ctms7000::andp_a2p,&Ctms7000::orp_a2p, &Ctms7000::xorp_a2p,&Ctms7000::btjop_ap,&Ctms7000::btjzp_ap,
            &Ctms7000::movd_imm,&Ctms7000::illegal, &Ctms7000::lda_dir, &Ctms7000::sta_dir, &Ctms7000::br_dir,  &Ctms7000::cmpa_dir,&Ctms7000::call_dir,&Ctms7000::illegal,

/* 0x9X */  &Ctms7000::illegal, &Ctms7000::movp_p2b,&Ctms7000::movp_b2p,&Ctms7000::andp_b2p,&Ctms7000::orp_b2p, &Ctms7000::xorp_b2p,&Ctms7000::btjop_bp,&Ctms7000::btjzp_bp,
            &Ctms7000::movd_r,  &Ctms7000::illegal, &Ctms7000::lda_ind, &Ctms7000::sta_ind, &Ctms7000::br_ind,  &Ctms7000::cmpa_ind,&Ctms7000::call_ind,&Ctms7000::illegal,

/* 0xAX */  &Ctms7000::illegal, &Ctms7000::illegal, &Ctms7000::movp_i2p,&Ctms7000::andp_i2p,&Ctms7000::orp_i2p, &Ctms7000::xorp_i2p,&Ctms7000::btjop_ip,&Ctms7000::btjzp_ip,
            &Ctms7000::movd_inx,&Ctms7000::illegal, &Ctms7000::lda_inx, &Ctms7000::sta_inx, &Ctms7000::br_inx,  &Ctms7000::cmpa_inx,&Ctms7000::call_inx,&Ctms7000::illegal,

/* 0xBX */  &Ctms7000::clrc,    &Ctms7000::illegal, &Ctms7000::dec_a,   &Ctms7000::inc_a,   &Ctms7000::inv_a,   &Ctms7000::clr_a,   &Ctms7000::xchb_a,  &Ctms7000::swap_a,
            &Ctms7000::push_a,  &Ctms7000::pop_a,   &Ctms7000::djnz_a,  &Ctms7000::decd_a,  &Ctms7000::rr_a,    &Ctms7000::rrc_a,   &Ctms7000::rl_a,    &Ctms7000::rlc_a,

/* 0xCX */  &Ctms7000::mov_a2b, &Ctms7000::tstb,    &Ctms7000::dec_b,   &Ctms7000::inc_b,   &Ctms7000::inv_b,   &Ctms7000::clr_b,   &Ctms7000::xchb_b,  &Ctms7000::swap_b,
            &Ctms7000::push_b,  &Ctms7000::pop_b,   &Ctms7000::djnz_b,  &Ctms7000::decd_b,  &Ctms7000::rr_b,    &Ctms7000::rrc_b,   &Ctms7000::rl_b,    &Ctms7000::rlc_b,

/* 0xDX */  &Ctms7000::mov_a2r, &Ctms7000::mov_b2r, &Ctms7000::dec_r,   &Ctms7000::inc_r,   &Ctms7000::inv_r,   &Ctms7000::clr_r,   &Ctms7000::xchb_r,  &Ctms7000::swap_r_exl,
            &Ctms7000::push_r,  &Ctms7000::pop_r,   &Ctms7000::djnz_r,  &Ctms7000::decd_r,  &Ctms7000::rr_r,    &Ctms7000::rrc_r,   &Ctms7000::rl_r,    &Ctms7000::rlc_r,

/* 0xEX */  &Ctms7000::jmp,     &Ctms7000::j_jn,    &Ctms7000::jeq,     &Ctms7000::jc,      &Ctms7000::jp,      &Ctms7000::jpz,     &Ctms7000::jne,     &Ctms7000::jl,
            &Ctms7000::trap_23, &Ctms7000::trap_22, &Ctms7000::trap_21, &Ctms7000::trap_20, &Ctms7000::trap_19, &Ctms7000::trap_18, &Ctms7000::trap_17, &Ctms7000::trap_16,

/* 0xFX */  &Ctms7000::trap_15, &Ctms7000::trap_14, &Ctms7000::trap_13, &Ctms7000::trap_12, &Ctms7000::trap_11, &Ctms7000::trap_10, &Ctms7000::trap_9,  &Ctms7000::trap_8,
            &Ctms7000::trap_7,  &Ctms7000::trap_6,  &Ctms7000::trap_5,  &Ctms7000::trap_4,  &Ctms7000::trap_3,  &Ctms7000::trap_2,  &Ctms7000::trap_1,  &Ctms7000::trap_0
};


void Ctms7000::execute_run()
{
    int op;

    m_div_by_16_trigger += m_icount;

    check_IRQ_lines();

    do
    {
//        debugger_instruction_hook(this, ppc);

        if( m_idle_state == 0 )
        {
            op = pPC->Get_PC(ppc++);

            (this->*m_opcode[op])();
        }
        else
            m_icount -= 16;

        /* Internal timer system */

        while( m_icount < m_div_by_16_trigger )
        {
            m_div_by_16_trigger -= 16;

            if( (m_pf[0x03] & 0x80) == 0x80 ) /* Is timer system active? */
            {
                if( (m_pf[0x03] & 0x40) != 0x40) /* Is system clock (divided by 16) the timer source? */
                    service_timer1();
            }
        }

    } while( m_icount > 0 );

    m_div_by_16_trigger -= m_icount;
}


/****************************************************************************
 * Trigger the event counter
 ****************************************************************************/

void Ctms7000::service_timer1()
{
    if( --m_t1_prescaler < 0 ) /* Decrement prescaler and check for underflow */
    {
        m_t1_prescaler = m_pf[3] & 0x1f; /* Reload prescaler (5 bit) */

        if( --m_t1_decrementer < 0 ) /* Decrement timer1 register and check for underflow */
        {
            m_t1_decrementer = m_pf[2]; /* Reload decrementer (8 bit) */
//            set_input_line(TMS7000_IRQ2_LINE, HOLD_LINE);
            //LOG( ("tms7000: trigger int2 (cycles: %d)\t%d\tdelta %d\n", total_cycles(), total_cycles() - tick, m_cycles_per_INT2-(total_cycles() - tick) );
            //tick = total_cycles() );
            /* Also, cascade out to timer 2 - timer 2 unimplemented */
        }
    }
//  LOG( ( "tms7000: service timer1. 0x%2.2x 0x%2.2x (cycles %d)\t%d\t\n", m_t1_prescaler, m_t1_decrementer, total_cycles(), total_cycles() - tick2 ) );
//  tick2 = total_cycles();
}


//WRITE8_MEMBER( Ctms7000::tms70x0_pf_w )   /* Perpherial file write */
void Ctms7000::pf_write(UINT32 offset,UINT8 data)
{
    UINT8   temp1, temp2, temp3;

    switch( offset )
    {
        case 0x00:  /* IOCNT0, Input/Ouput control */
            temp1 = data & 0x2a;                            /* Record which bits to clear */
            temp2 = m_pf[0x00] & 0x2a;              /* Get copy of current bits */
            temp3 = (~temp1) & temp2;                       /* Clear the requested bits */
            m_pf[0x00] = temp3 | (data & (~0x2a) ); /* OR in the remaining data */
            break;
        case 0x02:
            m_t1_decrementer = m_pf[0x02] = data;
            m_cycles_per_INT2 = 0x10*((m_pf[3] & 0x1f)+1)*(m_pf[0x02]+1);
//            LOG( ( "tms7000: Timer adjusted. Decrementer: 0x%2.2x (Cycles per interrupt: %d)\n", m_t1_decrementer, m_cycles_per_INT2 ) );
            break;
        case 0x03:  /* T1CTL, timer 1 control */
            if( ((m_pf[0x03] & 0x80) == 0) && ((data & 0x80) == 0x80 ) )   /* Start timer? */
            {
                m_pf[0x03] = data;
                m_t1_prescaler = m_pf[3] & 0x1f; /* Reload prescaler (5 bit) */
                m_cycles_per_INT2 = 0x10*((m_pf[3] & 0x1f)+1)*(m_pf[0x02]+1);
//                LOG( ( "tms7000: Timer started. Prescaler: 0x%2.2x (Cycles per interrupt: %d)\n", m_pf[3] & 0x1f, m_cycles_per_INT2 ) );
            }
            else if( ((data & 0x80) == 0x80 ) && ((m_pf[0x03] & 0x80) == 0) )   /* Timer Stopped? */
            {
                m_pf[0x03] = data;
                m_t1_prescaler = m_pf[3] & 0x1f; /* Reload prescaler (5 bit) */
                m_cycles_per_INT2 = 0x10*((m_pf[3] & 0x1f)+1)*(m_pf[0x02]+1);
//                LOG( ( "tms7000: Timer stopped. Prescaler: 0x%2.2x (Cycles per interrupt: %d)\n", m_pf[3] & 0x1f, m_cycles_per_INT2 ) );
            }
            else /* Don't modify timer state, but still store data */
            {
                m_pf[0x03] = data;
                m_cycles_per_INT2 = 0x10*((m_pf[3] & 0x1f)+1)*(m_pf[0x02]+1);
//                LOG( ( "tms7000: Timer adjusted. Prescaler: 0x%2.2x (Cycles per interrupt: %d)\n", m_pf[3] & 0x1f, m_cycles_per_INT2 ) );
            }
            break;

        case 0x04: /* Port A write */
            /* Port A is read only so this is a NOP */
            break;

        case 0x06: /* Port B write */
            pPC->out(TMS7000_PORTB,data);
//            m_io->write_byte( TMS7000_PORTB, data );
            m_pf[ 0x06 ] = data;
            break;

        case 0x08: /* Port C write */
            temp1 = data & m_pf[ 0x09 ];    /* Mask off input bits */
//            m_io->write_byte( TMS7000_PORTC, temp1 );
            pPC->out(TMS7000_PORTC,temp1);
            m_pf[ 0x08 ] = temp1;
            break;

        case 0x0a: /* Port D write */
            temp1 = data & m_pf[ 0x0b ];    /* Mask off input bits */
//            m_io->write_byte( TMS7000_PORTD, temp1 );
            pPC->out(TMS7000_PORTD,temp1);
            m_pf[ 0x0a ] = temp1;
            break;

        default:
            /* Just stuff the other registers */
            m_pf[ offset ] = data;
            break;
    }
}

//READ8_MEMBER( Ctms7000::tms70x0_pf_r )    /* Perpherial file read */
UINT8 Ctms7000::pf_read(UINT32 offset)
{
    UINT8 result;
    UINT8   temp1, temp2, temp3;

    switch( offset )
    {
        case 0x00:  /* IOCNT0, Input/Ouput control */
            result = m_pf[0x00];
            if (m_irq_state[TMS7000_IRQ1_LINE] == ASSERT_LINE)
                result |= 0x02;
            if (m_irq_state[TMS7000_IRQ3_LINE] == ASSERT_LINE)
                result |= 0x20;
            break;

        case 0x02:  /* T1DATA, timer 1 8-bit decrementer */
            result = (m_t1_decrementer & 0x00ff);
            break;

        case 0x03:  /* T1CTL, timer 1 capture (latched by INT3) */
            result = m_t1_capture_latch;
            break;

        case 0x04: /* Port A read */
            result = pPC->in( TMS7000_PORTA );
            break;


        case 0x06: /* Port B read */
            /* Port B is write only, return a previous written value */
            result = m_pf[ 0x06 ];
            break;

        case 0x08: /* Port C read */
            temp1 = m_pf[ 0x08 ] & m_pf[ 0x09 ];    /* Get previous output bits */
            temp2 = pPC->in( TMS7000_PORTC );           /* Read port */
            temp3 = temp2 & (~m_pf[ 0x09 ]);                /* Mask off output bits */
            result = temp1 | temp3;                             /* OR together */
            break;

        case 0x0a: /* Port D read */
            temp1 = m_pf[ 0x0a ] & m_pf[ 0x0b ];    /* Get previous output bits */
            temp2 = pPC->in( TMS7000_PORTD );           /* Read port */
            temp3 = temp2 & (~m_pf[ 0x0b ]);                /* Mask off output bits */
            result = temp1 | temp3;                             /* OR together */
            break;

        default:
            /* Just unstuff the other registers */
            result = m_pf[ offset ];
            break;
    }

    return result;
}


// BCD arthrimetic handling
static const UINT8 lut_bcd_out[6] = { 0x00, 0x06, 0x00, 0x66, 0x60, 0x66 };

inline UINT8 Ctms7000::bcd_add( UINT8 a, UINT8 b, UINT8 c )
{
    c = (c != 0) ? 1 : 0;

    UINT8 h1 = a >> 4 & 0xf;
    UINT8 l1 = a >> 0 & 0xf;
    UINT8 h2 = b >> 4 & 0xf;
    UINT8 l2 = b >> 0 & 0xf;

    // compute bcd constant
    UINT8 d = ((l1 + l2 + c) < 10) ? 0 : 1;
    if ((h1 + h2) == 9)
        d |= 2;
    else if ((h1 + h2) > 9)
        d |= 4;

    UINT8 ret = a + b + c + lut_bcd_out[d];

    CLR_NZC;
    SET_N8(ret);
    SET_Z8(ret);

    if (d > 2)
        pSR |= SR_C;

    return ret;
}

inline UINT8 Ctms7000::bcd_sub( UINT8 a, UINT8 b, UINT8 c )
{
    c = (c != 0) ? 0 : 1;

    UINT8 h1 = a >> 4 & 0xf;
    UINT8 l1 = a >> 0 & 0xf;
    UINT8 h2 = b >> 4 & 0xf;
    UINT8 l2 = b >> 0 & 0xf;

    // compute bcd constant
    UINT8 d = ((l1 - c) >= l2) ? 0 : 1;
    if (h1 == h2)
        d |= 2;
    else if (h1 < h2)
        d |= 4;

    UINT8 ret = a - b - c - lut_bcd_out[d];

    CLR_NZC;
    SET_N8(ret);
    SET_Z8(ret);

    if (d > 2)
        pSR |= SR_C;

    return ret;
}


UINT32 Ctms7000::get_PC()
{
//    upd7907_state *cpustate = &upd7907stat;
    return ppc;
}

void Ctms7000::Load_Internal(QXmlStreamReader *xmlIn)
{
    if (xmlIn->readNextStartElement()) {
        if ( (xmlIn->name()=="cpu") &&
             (xmlIn->attributes().value("model").toString() == "tms7000")) {
            QByteArray ba_reg = QByteArray::fromBase64(xmlIn->attributes().value("registers").toString().toLatin1());
//            memcpy((char *) &upd7907stat,ba_reg.data(),sizeof(upd7907stat));
//            upd7907stat.pPC = pPC;
//            upd7907stat.opXX = opXX_7907;
//            upd7907stat.op48 = op48_7907;
//            upd7907stat.op4C = op4C_7907;
//            upd7907stat.op4D = op4D_7907;
//            upd7907stat.op60 = op60_7907;
//            upd7907stat.op64 = op64_7907;
//            upd7907stat.op70 = op70_7907;
//            upd7907stat.op74 = op74_7907;
//            upd7907stat.handle_timers = upd78c05_timers;
//            QByteArray ba_imem = QByteArray::fromBase64(xmlIn->attributes().value("iMem").toString().toLatin1());
//            memcpy((char *) &(upd7907stat.imem),ba_imem.data(),IMEM_LEN);

        }
        xmlIn->skipCurrentElement();
    }
}

void Ctms7000::save_internal(QXmlStreamWriter *xmlOut)
{

    xmlOut->writeStartElement("cpu");
        xmlOut->writeAttribute("model","tms7000");
//        QByteArray ba_reg((char*)&upd7907stat,sizeof(upd7907stat));
//        xmlOut->writeAttribute("registers",ba_reg.toBase64());
    xmlOut->writeEndElement();
}

void Ctms7000::Regs_Info(UINT8 Type)
{
    sprintf(Regs_String,"EMPTY");

#if 1
//    upd7907_state *cpustate = &upd7907stat;
    char buf[32];
    switch(Type)
    {
    case 0:			// Monitor Registers Dialog
        sprintf(
                    Regs_String,
                    "%c%c%c%c%c%c%c%c",
                                m_sr & 0x80 ? 'C':'c',
                                m_sr & 0x40 ? 'N':'n',
                                m_sr & 0x20 ? 'Z':'z',
                                m_sr & 0x10 ? 'I':'i',
                                m_sr & 0x08 ? '?':'.',
                                m_sr & 0x04 ? '?':'.',
                                m_sr & 0x02 ? '?':'.',
                                m_sr & 0x01 ? '?':'.'
                    );
        break;
    case 1:			// Log File
        sprintf(
                    Regs_String,
                    "%c%c%c%c%c%c%c%c",
                                m_sr & 0x80 ? 'C':'c',
                                m_sr & 0x40 ? 'N':'n',
                                m_sr & 0x20 ? 'Z':'z',
                                m_sr & 0x10 ? 'I':'i',
                                m_sr & 0x08 ? '?':'.',
                                m_sr & 0x04 ? '?':'.',
                                m_sr & 0x02 ? '?':'.',
                                m_sr & 0x01 ? '?':'.'
                    );

        break;
    }

//    char linebuf[100];
//    strcat(Regs_String," ,");
//    for (int i=0;i < 0x05;i++)
//    {
//        sprintf(linebuf,"%02x:[",i*0x10);
//        strcat(Regs_String,linebuf);
//        for (int j=0;j<0x10;j++)
//        {
//            sprintf(linebuf,"%02X ",imem[i*0x10 + j]);
//            strcat(Regs_String,linebuf);
//        }
//
//        strcat(Regs_String,"],");
//    }
#endif
}


bool Ctms7000::exit()
{
    return true;
}

void Ctms7000::step()
{
    {
        m_icount = 0;
        execute_run();
        pPC->pTIMER->state -= m_icount;
    }


}


Ctms7000::~Ctms7000()
{
}
