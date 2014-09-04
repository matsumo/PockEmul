
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
 *  TODO:
 *  - dump CROM and emulate cpu at microinstruction level
 *  - memory modes with IOCNT0, currently ignored
 *  - timer event counter mode (timer control register, bit 6)
 *  - TMS70x1/2 serial port and timer 3
 *  - when they're needed, add TMS70Cx2, TMS7742, TMS77C82, SE70xxx
 *
 *****************************************************************************/
#include <QDebug>

#include "tms7000.h"
#include "pcxxxx.h"
#include "Log.h"
#include "Inter.h"
#include "Debug.h"
#include "ui/cregsz80widget.h"

#define VERBOSE 0

#define LOG(x)
//do { if (VERBOSE) logerror x; } while (0)

// flag helpers
#define SR_C        0x80 /* Carry */
#define SR_N        0x40 /* Negative */
#define SR_Z        0x20 /* Zero */
#define SR_I        0x10 /* Interrupt */

#define GET_C()     (info.m_sr >> 7 & 1)
#define SET_C(x)    info.m_sr = (info.m_sr & 0x7f) | ((x) >> 1 & 0x80)
#define SET_NZ(x)   info.m_sr = (info.m_sr & 0x9f) | ((x) >> 1 & 0x40) | (((x) & 0xff) ? 0 : 0x20)
#define SET_CNZ(x)  info.m_sr = (info.m_sr & 0x1f) | ((x) >> 1 & 0xc0) | (((x) & 0xff) ? 0 : 0x20)


//const device_type TMS7000 = &device_creator<tms7000_device>;
//const device_type TMS7020 = &device_creator<tms7020_device>;
//const device_type TMS7020_EXL = &device_creator<tms7020_exl_device>;
//const device_type TMS7040 = &device_creator<tms7040_device>;
//const device_type TMS70C00 = &device_creator<tms70c00_device>;
//const device_type TMS70C20 = &device_creator<tms70c20_device>;
//const device_type TMS70C40 = &device_creator<tms70c40_device>;
//const device_type TMS7001 = &device_creator<tms7001_device>;
//const device_type TMS7041 = &device_creator<tms7041_device>;
//const device_type TMS7002 = &device_creator<tms7002_device>;
//const device_type TMS7042 = &device_creator<tms7042_device>;


// internal memory maps
//static ADDRESS_MAP_START(tms7000_io, AS_IO, 8, tms7000_device)
//    AM_RANGE(TMS7000_PORTB, TMS7000_PORTB) AM_READNOP
//ADDRESS_MAP_END

//static ADDRESS_MAP_START(tms7000_mem, AS_PROGRAM, 8, tms7000_device )
//    AM_RANGE(0x0000, 0x007f) AM_RAM // 128 bytes internal RAM
//    AM_RANGE(0x0080, 0x00ff) AM_READWRITE(tms7000_unmapped_rf_r, tms7000_unmapped_rf_w)
//    AM_RANGE(0x0104, 0x0105) AM_WRITENOP // no port A write or ddr
//    AM_RANGE(0x0100, 0x010b) AM_READWRITE(tms7000_pf_r, tms7000_pf_w)
//ADDRESS_MAP_END

//static ADDRESS_MAP_START(tms7001_mem, AS_PROGRAM, 8, tms7000_device )
//    AM_RANGE(0x0000, 0x007f) AM_RAM // 128 bytes internal RAM
//    AM_RANGE(0x0080, 0x00ff) AM_READWRITE(tms7000_unmapped_rf_r, tms7000_unmapped_rf_w)
//    AM_RANGE(0x0100, 0x010b) AM_READWRITE(tms7000_pf_r, tms7000_pf_w)
//    AM_RANGE(0x0110, 0x0117) AM_READWRITE(tms7002_pf_r, tms7002_pf_w)
//ADDRESS_MAP_END

//static ADDRESS_MAP_START(tms7002_mem, AS_PROGRAM, 8, tms7000_device )
//    AM_RANGE(0x0000, 0x00ff) AM_RAM // 256 bytes internal RAM
//    AM_RANGE(0x0100, 0x010b) AM_READWRITE(tms7000_pf_r, tms7000_pf_w)
//    AM_RANGE(0x0110, 0x0117) AM_READWRITE(tms7002_pf_r, tms7002_pf_w)
//ADDRESS_MAP_END

//static ADDRESS_MAP_START(tms7020_mem, AS_PROGRAM, 8, tms7000_device )
//    AM_RANGE(0xf000, 0xffff) AM_ROM // 2kB internal ROM
//    AM_IMPORT_FROM( tms7000_mem )
//ADDRESS_MAP_END

//static ADDRESS_MAP_START(tms7040_mem, AS_PROGRAM, 8, tms7000_device )
//    AM_RANGE(0xf000, 0xffff) AM_ROM // 4kB internal ROM
//    AM_IMPORT_FROM( tms7000_mem )
//ADDRESS_MAP_END

//static ADDRESS_MAP_START(tms7041_mem, AS_PROGRAM, 8, tms7000_device )
//    AM_RANGE(0xf000, 0xffff) AM_ROM
//    AM_IMPORT_FROM( tms7001_mem )
//ADDRESS_MAP_END

//static ADDRESS_MAP_START(tms7042_mem, AS_PROGRAM, 8, tms7000_device )
//    AM_RANGE(0xf000, 0xffff) AM_ROM
//    AM_IMPORT_FROM( tms7002_mem )
//ADDRESS_MAP_END


// device definitions
//Ctms7000::tms7000_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
//    : cpu_device(mconfig, TMS7000, "TMS7000", tag, owner, clock, "tms7000", __FILE__),
//    m_program_config("program", ENDIANNESS_BIG, 8, 16, 0, ADDRESS_MAP_NAME(tms7000_mem)),
//    m_io_config("io", ENDIANNESS_BIG, 8, 8, 0, ADDRESS_MAP_NAME(tms7000_io)),
//    m_info_flags(0)
//{
//}

Ctms7000::Ctms7000(CPObject *parent, TMS7000_Models mod)
    : CCPU(parent)
{

    pDEBUG = new Cdebug_tms7000(parent);
    fn_status="tms7000.sta";
    fn_log="tms7000.log";

    regwidget = (CregCPU*) new Cregsz80Widget(0,this);
}
//Ctms7000::tms7000_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, address_map_constructor internal, UINT32 info_flags, const char *shortname, const char *source)
//    : cpu_device(mconfig, type, name, tag, owner, clock, shortname, source),
//    m_program_config("program", ENDIANNESS_BIG, 8, 16, 0, internal),
//    m_io_config("io", ENDIANNESS_BIG, 8, 8, 0, ADDRESS_MAP_NAME(tms7000_io)),
//    m_info_flags(info_flags)
//{
//}

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

Ctms70c00::Ctms70c00(CPObject *parent)
    : Ctms7000(parent, TMS70C00)
{
}

Ctms70c20::Ctms70c20(CPObject *parent)
    : Ctms7000(parent, TMS70C20)
{
}

Ctms70c40::Ctms70c40(CPObject *parent, TMS7000_Models mod)
    : Ctms7000(parent, mod)
{
}

Ctms70c46::Ctms70c46(CPObject *parent, TMS7000_Models mod)
    : Ctms7000(parent, mod)
{
}

bool Ctms70c46::init()
{
    Ctms7000::init();
    m_control = 0;
    return true;
}

void Ctms70c46::Reset()
{
    Ctms7000::Reset();
    m_control = 0;
    pPC->mem[0x10C] = 0xff;
}

UINT8 Ctms70c46::pf_read(UINT32 offset)
{
    switch (offset)
    {
    case 0x0c: return e_bus_data_r();
    case 0x0e:    return 0xff;
    case 0x0f:
        // d0: slave _HSK
        // d1: slave _BAV
        // d2: unused?
        // d3: IRQ active
        return (pPC->LastKey>0)? 0x08:00;
    default:
        return Ctms7000::pf_read(offset);
        break;
    }

    return 0;
}

UINT8 Ctms70c46::control_r()
{
    return m_control;
}

void Ctms70c46::control_w(UINT8 data)
{
    // d5: enable external databus
    if (~m_control & data & 0x20)
         e_bus_data_w(0xff); // go into high impedance

    // d4: enable clock divider when accessing slow memory (not emulated)
    // known fast memory areas: internal ROM/RAM, system RAM
    // known slow memory areas: system ROM, cartridge ROM/RAM

    // d0-d3(all bits?): clock divider when d4 is set and addressbus is in slow memory area
    // needs to be measured, i just know that $30 is full speed, and $38 is about 4 times slower
    m_control = data;
}

void Ctms70c46::pf_write(UINT32 offset,UINT8 data)
{
    switch (offset)
    {
    case 0x0c:
        e_bus_data_w(data);
        break;
    case 0x0f:
        // d0: master _HSK (setting it low(write 1) also clears IRQ)
        // d1: master _BAV
        // other bits: unused?
        break;
    default:
        Ctms7000::pf_write(offset,data);
        break;
    }
}

//tms7020_device::tms7020_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
//    : tms7000_device(mconfig, TMS7020, "TMS7020", tag, owner, clock, ADDRESS_MAP_NAME(tms7020_mem), 0, "tms7020", __FILE__)
//{
//}

//tms7020_exl_device::tms7020_exl_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
//    : tms7000_device(mconfig, TMS7020_EXL, "TMS7020 (Exelvision)", tag, owner, clock, ADDRESS_MAP_NAME(tms7020_mem), 0, "tms7020_exl", __FILE__)
//{
//}

//tms7040_device::tms7040_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
//    : tms7000_device(mconfig, TMS7040, "TMS7040", tag, owner, clock, ADDRESS_MAP_NAME(tms7040_mem), 0, "tms7040", __FILE__)
//{
//}

//tms70c00_device::tms70c00_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
//    : tms7000_device(mconfig, TMS70C00, "TMS70C00", tag, owner, clock, ADDRESS_MAP_NAME(tms7000_mem), TMS7000_CHIP_IS_CMOS, "tms70c00", __FILE__)
//{
//}

//tms70c20_device::tms70c20_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
//    : tms7000_device(mconfig, TMS70C20, "TMS70C20", tag, owner, clock, ADDRESS_MAP_NAME(tms7020_mem), TMS7000_CHIP_IS_CMOS, "tms70c20", __FILE__)
//{
//}

//tms70c40_device::tms70c40_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
//    : tms7000_device(mconfig, TMS70C40, "TMS70C40", tag, owner, clock, ADDRESS_MAP_NAME(tms7040_mem), TMS7000_CHIP_IS_CMOS, "tms70c40", __FILE__)
//{
//}

//tms7001_device::tms7001_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
//    : tms7000_device(mconfig, TMS7001, "TMS7001", tag, owner, clock, ADDRESS_MAP_NAME(tms7001_mem), TMS7000_CHIP_FAMILY_70X2, "tms7001", __FILE__)
//{
//}

//tms7041_device::tms7041_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
//    : tms7000_device(mconfig, TMS7041, "TMS7041", tag, owner, clock, ADDRESS_MAP_NAME(tms7041_mem), TMS7000_CHIP_FAMILY_70X2, "tms7041", __FILE__)
//{
//}

//tms7002_device::tms7002_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
//    : tms7000_device(mconfig, TMS7002, "TMS7002", tag, owner, clock, ADDRESS_MAP_NAME(tms7002_mem), TMS7000_CHIP_FAMILY_70X2, "tms7002", __FILE__)
//{
//}

//tms7042_device::tms7042_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
//    : tms7000_device(mconfig, TMS7042, "TMS7042", tag, owner, clock, ADDRESS_MAP_NAME(tms7042_mem), TMS7000_CHIP_FAMILY_70X2, "tms7042", __FILE__)
//{
//}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

bool Ctms7000::init()
{
    Check_Log();
    pDEBUG->init();

    // init/zerofill
//    m_program = &space(AS_PROGRAM);
//    m_direct = &m_program->direct();
//    m_io = &space(AS_IO);

//    m_icountptr = &m_icount;

    info.m_irq_state[TMS7000_INT1_LINE] = false;
    info.m_irq_state[TMS7000_INT3_LINE] = false;

    info.m_idle_state = false;
    info.m_idle_halt = false;
    info.m_pc = 0;
    info.m_sp = 0;
    info.m_sr = 0;
    info.m_op = 0;

    memset(info.m_io_control, 0, 3);

    memset(info.m_port_latch, 0, 4);
    memset(info.m_port_ddr, 0, 4);
    info.m_port_ddr[1] = 0xff; // !

    for (int tmr = 0; tmr < 2; tmr++)
    {
//        m_timer_handle[tmr] = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(Ctms7000::simple_timer_cb), this));
//        m_timer_handle[tmr]->adjust(attotime::never, tmr);

        info.m_timer_data[tmr] = 0;
        info.m_timer_control[tmr] = 0;
        info.m_timer_decrementer[tmr] = 0;
        info.m_timer_prescaler[tmr] = 0;
        info.m_timer_capture_latch[tmr] = 0;
    }

    // register for savestates
//    save_item(NAME(m_irq_state));
//    save_item(NAME(m_idle_state));
//    save_item(NAME(m_pc));
//    save_item(NAME(m_sp));
//    save_item(NAME(m_sr));
//    save_item(NAME(m_op));

//    save_item(NAME(m_io_control));
//    save_item(NAME(m_port_latch));
//    save_item(NAME(m_port_ddr));
//    save_item(NAME(m_timer_data));
//    save_item(NAME(m_timer_control));
//    save_item(NAME(m_timer_decrementer));
//    save_item(NAME(m_timer_prescaler));
//    save_item(NAME(m_timer_capture_latch));

//    // register for debugger
//    state_add( TMS7000_PC, "PC", m_pc).formatstr("%02X");
//    state_add( TMS7000_SP, "S", m_sp).formatstr("%02X");
//    state_add( TMS7000_ST, "ST", m_sr).formatstr("%02X");

//    state_add(STATE_GENPC, "GENPC", m_pc).formatstr("%02X").noshow();
//    state_add(STATE_GENSP, "GENSP", m_sp).formatstr("%02X").noshow();
//    state_add(STATE_GENFLAGS, "GENFLAGS", m_sr).formatstr("%8s").noshow();

    return true;
}


//offs_t Ctms7000::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
//{
//    extern CPU_DISASSEMBLE( tms7000 );
//    return CPU_DISASSEMBLE_NAME(tms7000)(this, buffer, pc, oprom, opram, options);
//}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void Ctms7000::Reset()
{
    if (info.m_idle_state)
    {
        info.m_pc++;
        info.m_idle_state = false;
    }

    // while _RESET is asserted:
    // clear ports
    write_p(0x04, 0xff); // port a
    write_p(0x06, 0xff); // port b

    write_p(0x05, 0x00); // ddr a
    write_p(0x09, 0x00); // ddr c
    write_p(0x0b, 0x00); // ddr d

    if (!chip_is_cmos())
    {
        write_p(0x08, 0xff); // port c
        write_p(0x0a, 0xff); // port d
    }

    // when _RESET goes inactive (0 to 1)
    info.m_sr = 0;

    write_p(0x00, 0x00); // IOCNT0
    if (chip_is_family_70x2())
        write_p(0x10, 0x00); // IOCNT1

    write_mem16(0, info.m_pc); // previous PC
    info.m_sp = 0x01;
    info.m_pc = read_mem16(0xfffe);
    info.m_icount -= 17;
}


//-------------------------------------------------
//  interrupts
//-------------------------------------------------

void Ctms7000::execute_set_input(int irqline, int state)
{
//    assert(irqline == TMS7000_INT1_LINE || irqline == TMS7000_INT3_LINE);
    bool irqstate = (state == CLEAR_LINE) ? false : true;

    // reverse polarity (70cx2-only)
    if (info.m_io_control[2] & (0x01 << (4 * irqline)))
        irqstate = !irqstate;

    if (info.m_irq_state[irqline] != irqstate)
    {
        info.m_irq_state[irqline] = irqstate;

        // set/clear internal irq flag
        flag_ext_interrupt(irqline);

        if (info.m_irq_state[irqline])
        {
            // latch timer 1 on INT3
            if (irqline == TMS7000_INT3_LINE)
                info.m_timer_capture_latch[0] = info.m_timer_decrementer[0];

            // on 70cx2, latch timer 2 on INT1
            if (irqline == TMS7000_INT1_LINE && chip_is_family_70cx2())
                info.m_timer_capture_latch[1] = info.m_timer_decrementer[1];

            // clear external if it's edge-triggered (70cx2-only)
            if (info.m_io_control[2] & (0x02 << (4 * irqline)))
                info.m_irq_state[irqline] = false;

            check_interrupts();
        }
    }
}

void Ctms7000::flag_ext_interrupt(int irqline)
{
    if (irqline != TMS7000_INT1_LINE && irqline != TMS7000_INT3_LINE)
        return;

    // set/clear for pending external interrupt
    if (info.m_irq_state[irqline])
        info.m_io_control[0] |= (0x02 << (4 * irqline));
    else
        info.m_io_control[0] &= ~(0x02 << (4 * irqline));
}

void Ctms7000::check_interrupts()
{
    // global interrupt bit
    if (!(info.m_sr & SR_I))
        return;


    // check for and handle interrupt
    for (int irqline = 0; irqline < 5; irqline++)
    {
        // INT 1,2,3 are in IOCNT0 d0-d5
        // INT 4,5 are in IOCNT1 d0-d3
        int shift = (irqline > 2) ? irqline * 2 - 6 : irqline * 2;
        if ((info.m_io_control[irqline > 2] >> shift & 3) == 3)
        {
            // ack
            info.m_io_control[irqline > 2] &= ~(0x02 << shift);
qWarning()<<"check_interrupts";
            flag_ext_interrupt(irqline);
            do_interrupt(irqline);
            return;
        }
    }
}

void Ctms7000::do_interrupt(int irqline)
{
    if (info.m_idle_state)
    {
        info.m_icount -= 17;
        info.m_pc++;
        info.m_idle_state = false;
    }
    else
        info.m_icount -= 19;

    push8(info.m_sr);
    push16(info.m_pc);
    CallSubLevel++;
    info.m_sr = 0;
    info.m_pc = read_mem16(0xfffc - irqline * 2);

//    standard_irq_callback(irqline);
}


//-------------------------------------------------
//  timers
//-------------------------------------------------

void Ctms7000::timer_run(int tmr)
{
    info.m_timer_prescaler[tmr] = info.m_timer_control[tmr] & 0x1f;

    // run automatic timer if source is internal
    if ((info.m_timer_control[tmr] & 0xe0) == 0x80)
    {
//        attotime period = attotime::from_hz(clock()) * 16 * (m_timer_prescaler[tmr] + 1); // fOSC/16
//        m_timer_handle[tmr]->adjust(period, tmr);
    }
}

void Ctms7000::timer_reload(int tmr)
{
    // stop possible running timer
//    m_timer_handle[tmr]->adjust(attotime::never, tmr);

    if (info.m_timer_control[tmr] & 0x80)
    {
        info.m_timer_decrementer[tmr] = info.m_timer_data[tmr];
        timer_run(tmr);
    }
}

void Ctms7000::timer_tick_pre(int tmr)
{
    // timer prescaler underflow
    if (--info.m_timer_prescaler[tmr] < 0)
    {
        info.m_timer_prescaler[tmr] = info.m_timer_control[tmr] & 0x1f;
        timer_tick_low(tmr);
    }
}

void Ctms7000::timer_tick_low(int tmr)
{
    // timer decrementer underflow
    if (--info.m_timer_decrementer[tmr] < 0)
    {
        timer_reload(tmr);

        // set INT2/INT5
        info.m_io_control[tmr] |= 0x08;

        // cascaded timer
        if (tmr == 0 && (info.m_timer_control[1] & 0xa0) == 0xa0)
            timer_tick_pre(tmr + 1);
    }
}

//TIMER_CALLBACK_MEMBER(Ctms7000::simple_timer_cb)
//{
//    int tmr = param;

//    // tick and restart timer
//    timer_tick_low(tmr);
//    timer_run(tmr);
//}


//-------------------------------------------------
//  peripheral file - read/write internal ports
//  note: 7000 family is from $00 to $0b, 7002 family adds $10 to $17
//-------------------------------------------------
void Ctms7000::set_input_line(UINT8 line, UINT8 state) {
    info.m_irq_state[line] = state;
}

UINT8 Ctms7000::pf_read(UINT32 offset)
{
    switch (offset)
    {
        // i/o control
        case 0x00: case 0x10:
            return info.m_io_control[offset >> 4];

        // timer 1/2 data
        case 0x02: case 0x12:
            // current decrementer value
            return info.m_timer_decrementer[offset >> 4];

        // timer 1 control
        case 0x03:
            // timer capture (latched by INT3)
            return info.m_timer_capture_latch[0];

        // port data
        case 0x04: case 0x06: case 0x08: case 0x0a:
        {
            // note: port B is write-only, reading it returns the output value as if ddr is 0xff
            int port = offset / 2 - 2;
            return (pPC->in( port ) & ~info.m_port_ddr[port]) | (info.m_port_latch[port] & info.m_port_ddr[port]);
        }

        // port direction (note: 7000 doesn't support it for port A)
        case 0x05: case 0x09: case 0x0b:
            return info.m_port_ddr[offset / 2 - 2];

        default:
//            logerror("%s: tms7000_pf_r @ $%04x\n", tag(), offset);
            break;
    }

    return 0;
}

void Ctms7000::pf_write(UINT32 offset,UINT8 data)
{
    switch (offset)
    {
        // i/o control (IOCNT0)
        case 0x00:
            // d0,d2,d4: INT1,2,3 enable
            // d1,d3,d5: INT1,2,3 flag (write 1 to clear flag)
            // d6-d7: memory mode (currently not implemented)
            info.m_io_control[0] = (info.m_io_control[0] & (~data & 0x2a)) | (data & 0xd5);

            // possibly need to reactivate flags
            if (data & 0x02)
                flag_ext_interrupt(TMS7000_INT1_LINE);
            if (data & 0x20)
                flag_ext_interrupt(TMS7000_INT3_LINE);

            check_interrupts();
            break;

        // i/o control (IOCNT1)
        case 0x10:
            // d0,d2: INT4,5 enable
            // d1,d3: INT4,5 flag (write 1 to clear flag)
            info.m_io_control[1] = (info.m_io_control[1] & (~data & 0x0a)) | (data & 0x05);
            check_interrupts();
            break;

        // timer 1/2 data
        case 0x02: case 0x12:
            // decrementer reload value
            info.m_timer_data[offset >> 4] = data;
            break;

        // timer 1/2 control
        case 0x03:
            // d5: t1: cmos low-power mode when IDLE opcode is used (not emulated)
            // 0(normal), or 1(halt) - indicating it can only wake up with RESET or external interrupt
            if (chip_is_cmos())
            {
                info.m_idle_halt = (data & 0x20) ? true : false;
                if (info.m_idle_halt)
                    qWarning()<<"CMOS low-power halt mode enabled";
            }
            data &= ~0x20;

        case 0x13:
            // d0-d4: prescaler reload value
            // d5: t2: cascade from t1
            // d6: source (internal/external)
            // d7: stop/start timer
            info.m_timer_control[offset >> 4] = data;
            timer_reload(offset >> 4);

            // on cmos chip, clear INT2/INT5 as well
            if (~data & 0x80 && chip_is_cmos())
                info.m_io_control[offset >> 4] &= ~0x08;

            break;

        // port data (note: 7000 doesn't support it for port A)
        case 0x04: case 0x06: case 0x08: case 0x0a:
        {
            // note: in memory expansion modes, some port output pins are used for memory strobes.
            // this is currently ignored, since port writes will always be visible externally on peripheral expansion anyway.
            int port = offset / 2 - 2;
            pPC->out(port, data & info.m_port_ddr[port]);
            info.m_port_latch[port] = data;
            break;
        }

        // port direction (note: 7000 doesn't support it for port A)
        case 0x05: case 0x09: case 0x0b:
            // note: changing port direction does not change(refresh) the output pins
            info.m_port_ddr[offset / 2 - 2] = data;
            break;

        default:
//            logerror("%s: tms7000_pf_w @ $%04x = $%02x\n", tag(), offset, data);
            break;
    }
}


//-------------------------------------------------
//  execute
//-------------------------------------------------

#include "tms70op.inc"

void Ctms7000::execute_run()
{
    check_interrupts();

    do
    {
//        debugger_instruction_hook(this, info.m_pc);

        info.m_op = pPC->Get_8(info.m_pc++);
        execute_one(info.m_op);
    } while (info.m_icount > 0);
}

void Ctms7000::execute_one(UINT8 op)
{
    switch (op)
    {
        case 0x00: nop(); break;
        case 0x01: idle(); break;
        case 0x05: eint(); break;
        case 0x06: dint(); break;
        case 0x07: setc(); break;
        case 0x08: pop_st(); break;
        case 0x09: stsp(); break;
        case 0x0a: rets(); break;
        case 0x0b: reti(); break;
        case 0x0d: ldsp(); break;
        case 0x0e: push_st(); break;

        case 0x12: am_r2a(&Ctms7000::op_mov); break;
        case 0x13: am_r2a(&Ctms7000::op_and); break;
        case 0x14: am_r2a(&Ctms7000::op_or); break;
        case 0x15: am_r2a(&Ctms7000::op_xor); break;
        case 0x16: am_r2a(&Ctms7000::op_btjo); break;
        case 0x17: am_r2a(&Ctms7000::op_btjz); break;
        case 0x18: am_r2a(&Ctms7000::op_add); break;
        case 0x19: am_r2a(&Ctms7000::op_adc); break;
        case 0x1a: am_r2a(&Ctms7000::op_sub); break;
        case 0x1b: am_r2a(&Ctms7000::op_sbb); break;
        case 0x1c: am_r2a(&Ctms7000::op_mpy); break;
        case 0x1d: am_r2a(&Ctms7000::op_cmp); break;
        case 0x1e: am_r2a(&Ctms7000::op_dac); break;
        case 0x1f: am_r2a(&Ctms7000::op_dsb); break;

        case 0x22: am_i2a(&Ctms7000::op_mov); break;
        case 0x23: am_i2a(&Ctms7000::op_and); break;
        case 0x24: am_i2a(&Ctms7000::op_or); break;
        case 0x25: am_i2a(&Ctms7000::op_xor); break;
        case 0x26: am_i2a(&Ctms7000::op_btjo); break;
        case 0x27: am_i2a(&Ctms7000::op_btjz); break;
        case 0x28: am_i2a(&Ctms7000::op_add); break;
        case 0x29: am_i2a(&Ctms7000::op_adc); break;
        case 0x2a: am_i2a(&Ctms7000::op_sub); break;
        case 0x2b: am_i2a(&Ctms7000::op_sbb); break;
        case 0x2c: am_i2a(&Ctms7000::op_mpy); break;
        case 0x2d: am_i2a(&Ctms7000::op_cmp); break;
        case 0x2e: am_i2a(&Ctms7000::op_dac); break;
        case 0x2f: am_i2a(&Ctms7000::op_dsb); break;

        case 0x32: am_r2b(&Ctms7000::op_mov); break;
        case 0x33: am_r2b(&Ctms7000::op_and); break;
        case 0x34: am_r2b(&Ctms7000::op_or); break;
        case 0x35: am_r2b(&Ctms7000::op_xor); break;
        case 0x36: am_r2b(&Ctms7000::op_btjo); break;
        case 0x37: am_r2b(&Ctms7000::op_btjz); break;
        case 0x38: am_r2b(&Ctms7000::op_add); break;
        case 0x39: am_r2b(&Ctms7000::op_adc); break;
        case 0x3a: am_r2b(&Ctms7000::op_sub); break;
        case 0x3b: am_r2b(&Ctms7000::op_sbb); break;
        case 0x3c: am_r2b(&Ctms7000::op_mpy); break;
        case 0x3d: am_r2b(&Ctms7000::op_cmp); break;
        case 0x3e: am_r2b(&Ctms7000::op_dac); break;
        case 0x3f: am_r2b(&Ctms7000::op_dsb); break;

        case 0x42: am_r2r(&Ctms7000::op_mov); break;
        case 0x43: am_r2r(&Ctms7000::op_and); break;
        case 0x44: am_r2r(&Ctms7000::op_or); break;
        case 0x45: am_r2r(&Ctms7000::op_xor); break;
        case 0x46: am_r2r(&Ctms7000::op_btjo); break;
        case 0x47: am_r2r(&Ctms7000::op_btjz); break;
        case 0x48: am_r2r(&Ctms7000::op_add); break;
        case 0x49: am_r2r(&Ctms7000::op_adc); break;
        case 0x4a: am_r2r(&Ctms7000::op_sub); break;
        case 0x4b: am_r2r(&Ctms7000::op_sbb); break;
        case 0x4c: am_r2r(&Ctms7000::op_mpy); break;
        case 0x4d: am_r2r(&Ctms7000::op_cmp); break;
        case 0x4e: am_r2r(&Ctms7000::op_dac); break;
        case 0x4f: am_r2r(&Ctms7000::op_dsb); break;

        case 0x52: am_i2b(&Ctms7000::op_mov); break;
        case 0x53: am_i2b(&Ctms7000::op_and); break;
        case 0x54: am_i2b(&Ctms7000::op_or); break;
        case 0x55: am_i2b(&Ctms7000::op_xor); break;
        case 0x56: am_i2b(&Ctms7000::op_btjo); break;
        case 0x57: am_i2b(&Ctms7000::op_btjz); break;
        case 0x58: am_i2b(&Ctms7000::op_add); break;
        case 0x59: am_i2b(&Ctms7000::op_adc); break;
        case 0x5a: am_i2b(&Ctms7000::op_sub); break;
        case 0x5b: am_i2b(&Ctms7000::op_sbb); break;
        case 0x5c: am_i2b(&Ctms7000::op_mpy); break;
        case 0x5d: am_i2b(&Ctms7000::op_cmp); break;
        case 0x5e: am_i2b(&Ctms7000::op_dac); break;
        case 0x5f: am_i2b(&Ctms7000::op_dsb); break;

        case 0x62: am_b2a(&Ctms7000::op_mov); break;
        case 0x63: am_b2a(&Ctms7000::op_and); break;
        case 0x64: am_b2a(&Ctms7000::op_or); break;
        case 0x65: am_b2a(&Ctms7000::op_xor); break;
        case 0x66: am_b2a(&Ctms7000::op_btjo); break;
        case 0x67: am_b2a(&Ctms7000::op_btjz); break;
        case 0x68: am_b2a(&Ctms7000::op_add); break;
        case 0x69: am_b2a(&Ctms7000::op_adc); break;
        case 0x6a: am_b2a(&Ctms7000::op_sub); break;
        case 0x6b: am_b2a(&Ctms7000::op_sbb); break;
        case 0x6c: am_b2a(&Ctms7000::op_mpy); break;
        case 0x6d: am_b2a(&Ctms7000::op_cmp); break;
        case 0x6e: am_b2a(&Ctms7000::op_dac); break;
        case 0x6f: am_b2a(&Ctms7000::op_dsb); break;

        case 0x72: am_i2r(&Ctms7000::op_mov); break;
        case 0x73: am_i2r(&Ctms7000::op_and); break;
        case 0x74: am_i2r(&Ctms7000::op_or); break;
        case 0x75: am_i2r(&Ctms7000::op_xor); break;
        case 0x76: am_i2r(&Ctms7000::op_btjo); break;
        case 0x77: am_i2r(&Ctms7000::op_btjz); break;
        case 0x78: am_i2r(&Ctms7000::op_add); break;
        case 0x79: am_i2r(&Ctms7000::op_adc); break;
        case 0x7a: am_i2r(&Ctms7000::op_sub); break;
        case 0x7b: am_i2r(&Ctms7000::op_sbb); break;
        case 0x7c: am_i2r(&Ctms7000::op_mpy); break;
        case 0x7d: am_i2r(&Ctms7000::op_cmp); break;
        case 0x7e: am_i2r(&Ctms7000::op_dac); break;
        case 0x7f: am_i2r(&Ctms7000::op_dsb); break;

        case 0x80: am_p2a(&Ctms7000::op_mov); break;
        case 0x82: am_a2p(&Ctms7000::op_mov); break;
        case 0x83: am_a2p(&Ctms7000::op_and); break;
        case 0x84: am_a2p(&Ctms7000::op_or); break;
        case 0x85: am_a2p(&Ctms7000::op_xor); break;
        case 0x86: am_a2p(&Ctms7000::op_btjo); break;
        case 0x87: am_a2p(&Ctms7000::op_btjz); break;
        case 0x88: movd_dir(); break;
        case 0x8a: lda_dir(); break;
        case 0x8b: sta_dir(); break;
        case 0x8c: br_dir(); break;
        case 0x8d: cmpa_dir(); break;
        case 0x8e: call_dir(); break;

        case 0x91: am_p2b(&Ctms7000::op_mov); break;
        case 0x92: am_b2p(&Ctms7000::op_mov); break;
        case 0x93: am_b2p(&Ctms7000::op_and); break;
        case 0x94: am_b2p(&Ctms7000::op_or); break;
        case 0x95: am_b2p(&Ctms7000::op_xor); break;
        case 0x96: am_b2p(&Ctms7000::op_btjo); break;
        case 0x97: am_b2p(&Ctms7000::op_btjz); break;
        case 0x98: movd_ind(); break;
        case 0x9a: lda_ind(); break;
        case 0x9b: sta_ind(); break;
        case 0x9c: br_ind(); break;
        case 0x9d: cmpa_ind(); break;
        case 0x9e: call_ind(); break;

        case 0xa2: am_i2p(&Ctms7000::op_mov); break;
        case 0xa3: am_i2p(&Ctms7000::op_and); break;
        case 0xa4: am_i2p(&Ctms7000::op_or); break;
        case 0xa5: am_i2p(&Ctms7000::op_xor); break;
        case 0xa6: am_i2p(&Ctms7000::op_btjo); break;
        case 0xa7: am_i2p(&Ctms7000::op_btjz); break;
        case 0xa8: movd_inx(); break;
        case 0xaa: lda_inx(); break;
        case 0xab: sta_inx(); break;
        case 0xac: br_inx(); break;
        case 0xad: cmpa_inx(); break;
        case 0xae: call_inx(); break;

        case 0xb0: am_a2a(&Ctms7000::op_mov); break; // aka clrc/tsta
        case 0xb1: am_b2a(&Ctms7000::op_mov); break; // undocumented
        case 0xb2: am_a(&Ctms7000::op_dec); break;
        case 0xb3: am_a(&Ctms7000::op_inc); break;
        case 0xb4: am_a(&Ctms7000::op_inv); break;
        case 0xb5: am_a(&Ctms7000::op_clr); break;
        case 0xb6: am_a(&Ctms7000::op_xchb); break;
        case 0xb7: am_a(&Ctms7000::op_swap); break;
        case 0xb8: push_a(); break;
        case 0xb9: pop_a(); break;
        case 0xba: am_a(&Ctms7000::op_djnz); break;
        case 0xbb: decd_a(); break;
        case 0xbc: am_a(&Ctms7000::op_rr); break;
        case 0xbd: am_a(&Ctms7000::op_rrc); break;
        case 0xbe: am_a(&Ctms7000::op_rl); break;
        case 0xbf: am_a(&Ctms7000::op_rlc); break;

        case 0xc0: am_a2b(&Ctms7000::op_mov); break;
        case 0xc1: am_b2b(&Ctms7000::op_mov); break; // aka tstb
        case 0xc2: am_b(&Ctms7000::op_dec); break;
        case 0xc3: am_b(&Ctms7000::op_inc); break;
        case 0xc4: am_b(&Ctms7000::op_inv); break;
        case 0xc5: am_b(&Ctms7000::op_clr); break;
        case 0xc6: am_b(&Ctms7000::op_xchb); break; // result equivalent to tstb
        case 0xc7: am_b(&Ctms7000::op_swap); break;
        case 0xc8: push_b(); break;
        case 0xc9: pop_b(); break;
        case 0xca: am_b(&Ctms7000::op_djnz); break;
        case 0xcb: decd_b(); break;
        case 0xcc: am_b(&Ctms7000::op_rr); break;
        case 0xcd: am_b(&Ctms7000::op_rrc); break;
        case 0xce: am_b(&Ctms7000::op_rl); break;
        case 0xcf: am_b(&Ctms7000::op_rlc); break;

        case 0xd0: am_a2r(&Ctms7000::op_mov); break;
        case 0xd1: am_b2r(&Ctms7000::op_mov); break;
        case 0xd2: am_r(&Ctms7000::op_dec); break;
        case 0xd3: am_r(&Ctms7000::op_inc); break;
        case 0xd4: am_r(&Ctms7000::op_inv); break;
        case 0xd5: am_r(&Ctms7000::op_clr); break;
        case 0xd6: am_r(&Ctms7000::op_xchb); break;
        case 0xd7: am_r(&Ctms7000::op_swap); break;
        case 0xd8: push_r(); break;
        case 0xd9: pop_r(); break;
        case 0xda: am_r(&Ctms7000::op_djnz); break;
        case 0xdb: decd_r(); break;
        case 0xdc: am_r(&Ctms7000::op_rr); break;
        case 0xdd: am_r(&Ctms7000::op_rrc); break;
        case 0xde: am_r(&Ctms7000::op_rl); break;
        case 0xdf: am_r(&Ctms7000::op_rlc); break;

        case 0xe0: jmp(true); break;
        case 0xe1: jmp(info.m_sr & SR_N); break; // jn/jlt
        case 0xe2: jmp(info.m_sr & SR_Z); break; // jz/jeq
        case 0xe3: jmp(info.m_sr & SR_C); break; // jc/jhs
        case 0xe4: jmp(!(info.m_sr & (SR_Z | SR_N))); break; // jp/jgt
        case 0xe5: jmp(!(info.m_sr & SR_N)); break; // jpz/jge - note: error in TI official documentation
        case 0xe6: jmp(!(info.m_sr & SR_Z)); break; // jnz/jne
        case 0xe7: jmp(!(info.m_sr & SR_C)); break; // jnc/jl

        case 0xe8: case 0xe9: case 0xea: case 0xeb: case 0xec: case 0xed: case 0xee: case 0xef:
        case 0xf0: case 0xf1: case 0xf2: case 0xf3: case 0xf4: case 0xf5: case 0xf6: case 0xf7:
        case 0xf8: case 0xf9: case 0xfa: case 0xfb: case 0xfc: case 0xfd: case 0xfe: case 0xff:
            trap(op << 1); break;

        default: illegal(op); break;
    }
}

void Ctms7020_exl::execute_one(UINT8 op)
{
    // TMS7020 Exelvision EXL 100 custom opcode(s)
    if (op == 0xd7)
        lvdp();
    else
        Ctms7000::execute_one(op);
}

UINT32 Ctms7000::get_PC()
{
//    upd7907_state *cpustate = &upd7907stat;
    return info.m_pc;
}

void Ctms7000::Load_Internal(QXmlStreamReader *xmlIn)
{
    if (xmlIn->readNextStartElement()) {
        if ( (xmlIn->name()=="cpu") &&
             (xmlIn->attributes().value("model").toString() == "tms7000")) {
            QByteArray ba_reg = QByteArray::fromBase64(xmlIn->attributes().value("registers").toString().toLatin1());
            memcpy((char *) &info,ba_reg.data(),sizeof(TMS7000info));

        }
        xmlIn->skipCurrentElement();
    }
}

void Ctms7000::save_internal(QXmlStreamWriter *xmlOut)
{

    xmlOut->writeStartElement("cpu");
        xmlOut->writeAttribute("model","tms7000");
        QByteArray ba_reg((char*)&info,sizeof(TMS7000info));
        xmlOut->writeAttribute("registers",ba_reg.toBase64());
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
                                info.m_sr & 0x80 ? 'C':'c',
                                info.m_sr & 0x40 ? 'N':'n',
                                info.m_sr & 0x20 ? 'Z':'z',
                                info.m_sr & 0x10 ? 'I':'i',
                                info.m_sr & 0x08 ? '?':'.',
                                info.m_sr & 0x04 ? '?':'.',
                                info.m_sr & 0x02 ? '?':'.',
                                info.m_sr & 0x01 ? '?':'.'
                    );
        break;
    case 1:			// Log File
        sprintf(
                    Regs_String,
                    "%c%c%c%c%c%c%c%c",
                                info.m_sr & 0x80 ? 'C':'c',
                                info.m_sr & 0x40 ? 'N':'n',
                                info.m_sr & 0x20 ? 'Z':'z',
                                info.m_sr & 0x10 ? 'I':'i',
                                info.m_sr & 0x08 ? '?':'.',
                                info.m_sr & 0x04 ? '?':'.',
                                info.m_sr & 0x02 ? '?':'.',
                                info.m_sr & 0x01 ? '?':'.'
                    );

        for (int i=0;i<0x20;i++)
            sprintf(Regs_String,"%s%02X ",Regs_String,read_r8(i));
        sprintf(Regs_String,"%s    ",Regs_String);
        for (int i=0;i<0x10;i++)
            sprintf(Regs_String,"%s%02X ",Regs_String,pf_read(i));
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
        info.m_icount = 0;
        execute_run();
        pPC->pTIMER->state -= info.m_icount;
    }


}


Ctms7000::~Ctms7000()
{
}


