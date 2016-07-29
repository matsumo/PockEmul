/*****************************************************************************
 *
 *  Programmable Interval Timer 8253/8254
 *
 *  Three Independent Timers
 *  (gate, clock, out pins)
 *
 *  8254 has an additional readback feature
 *
 *  Revision History
 *      1-Apr-2008 - WFP:   Changed the implementation into a device.
 *      8-Jul-2004 - AJ:    Fixed some bugs. Styx now runs correctly.
 *                          Implemented 8254 features.
 *      1-Mar-2004 - NPW:   Did an almost total rewrite and cleaned out much
 *                          of the ugliness in the previous design.  Bug #430
 *                          seems to be fixed
 *      1-Jul-2000 - PeT:   Split off from PC driver and componentized
 *
 *****************************************************************************/
#include <QDebug>

#include "pit8253.h"
#include "i80L188EB.h"


/***************************************************************************

    Structures & macros

***************************************************************************/

#define MAX_TIMER       3
#define VERBOSE         0


#define CYCLES_NEVER ((UINT32) -1)


#define CTRL_ACCESS(control)        (((control) >> 4) & 0x03)
#define CTRL_MODE(control)          (((control) >> 1) & (((control) & 0x04) ? 0x03 : 0x07))
#define CTRL_BCD(control)           (((control) >> 0) & 0x01)


/***************************************************************************

    Functions

***************************************************************************/




INLINE quint32 C8253PIT::decimal_from_bcd(quint16 val)
{
    /* In BCD mode, a nybble loaded with value A-F counts down the same as in
       binary mode, but wraps around to 9 instead of F after 0, so loading the
       count register with 0xFFFF gives a period of
              0xF  - for the units to count down to 0
       +   10*0xF  - for the tens to count down to 0
       +  100*0xF  - for the hundreds to count down to 0
       + 1000*0xF  - for the thousands to count down to 0
       = 16665 cycles
    */
    return
        ((val>>12) & 0xF) *  1000 +
        ((val>> 8) & 0xF) *   100 +
        ((val>> 4) & 0xF) *    10 +
        ( val      & 0xF);
}





/* This function subtracts 1 from timer->value "cycles" times, taking into
   account binary or BCD operation, and wrapping around from 0 to 0xFFFF or
   0x9999 as necessary. */
void timer::decrease_counter_value(quint64 cycles)
{
    /*
    UINT16 value;
    int units, tens, hundreds, thousands;

    if (CTRL_BCD(timer->control) == 0)
    {
        timer->value -= (cycles & 0xFFFF);
        return;
    }

    value = timer->value;
    units     =  value        & 0xF;
    tens      = (value >>  4) & 0xF;
    hundreds  = (value >>  8) & 0xF;
    thousands = (value >> 12) & 0xF;

    if (cycles <= units)
    {
        units -= cycles;
    }
    else
    {
        cycles -= units;
        units = (10 - cycles%10)%10;

        cycles =(cycles+9)/10; // the +9    is so we get a carry if cycles%10 wasn't 0
        if (cycles <= tens)
        {
            tens -= cycles;
        }
        else
        {
            cycles -= tens;
            tens = (10 - cycles%10) % 10;

            cycles = (cycles+9) / 10;
            if (cycles <= hundreds)
            {
                hundreds -= cycles;
            }
            else
            {
                cycles -= hundreds;
                hundreds = (10 - cycles%10)%10;
                cycles=(cycles+9)/10;
                thousands = (10 + thousands - cycles%10)%10;
            }
        }
    }

    timer->value = (thousands << 12) | (hundreds << 8) | (tens << 4) | units;
*/
}






/* This emulates timer "timer" for "elapsed_cycles" cycles and assumes no
   callbacks occur during that time. */
void timer::simulate2(qint64 elapsed_cycles)
{
#if 0
    quint32 adjusted_value;
    int bcd = CTRL_BCD(tcon);
    int mode = CTRL_MODE(tcon);
    int cycles_to_output = 0;


    switch (mode) {
    case 0:
        /* Mode 0: (Interrupt on Terminal Count)

                  +------------------
                  |
        ----------+
          <- n+1 ->

          ^
          +- counter load

        phase|output|length  |value|next|comment
        -----+------+--------+-----+----+----------------------------------
            0|low   |infinity|     |1   |waiting for count
            1|low   |1       |     |2   |internal delay when counter loaded
            2|low   |n       |n..1 |3   |counting down
            3|high  |infinity|0..1 |3   |counting down

        Gate level sensitive only. Low disables counting, high enables it. */

        if (timer->phase == 0)
        {
            cycles_to_output = CYCLES_NEVER;
        }
        else
        {
            if (elapsed_cycles >= 0 && timer->phase == 1)
            {
                /* Counter load cycle */
                if (elapsed_cycles > 0) {
                    --elapsed_cycles;
                    timer->phase = 2;
                }
                load_counter_value( device, timer );
            }

            if ( pit8253_gate(timer) == 0 )
            {
                cycles_to_output = CYCLES_NEVER;
            }
            else
            {
                if (timer->phase == 2)
                {
                    adjusted_value = adjusted_count(bcd,timer->value);
                    if (elapsed_cycles >= adjusted_value)
                    {
                        /* Counter wrapped, output goes high */
                        elapsed_cycles -= adjusted_value;
                        timer->phase = 3;
                        timer->value = 0;
                        set_output( device, timer, 1 );
                    }
                }

                decrease_counter_value(timer,elapsed_cycles);

                switch( timer->phase )
                {
                case 1:     cycles_to_output = 1; break;
                case 2:     cycles_to_output = adjusted_count( bcd, timer->value ); break;
                case 3:     cycles_to_output = adjusted_count( bcd, timer->value ); break;
                }
            }
        }
        break;


    case 1:
        /* Mode 1: (Hardware Retriggerable One-Shot a.k.a. Programmable One-Shot)

        -----+       +------------------
             |       |
             +-------+
             <-  n  ->

          ^
          +- trigger

        phase|output|length  |value|next|comment
        -----+------+--------+-----+----+----------------------------------
            0|high  |infinity|     |1   |counting down
            1|high  |1       |     |2   |internal delay to load counter
            2|low   |n       |n..1 |3   |counting down
            3|high  |infinity|0..1 |3   |counting down

        Gate rising-edge sensitive only.
        Rising edge initiates counting and resets output after next clock. */

        if ( elapsed_cycles >= 0 && timer->phase == 1 )
        {
            /* Counter load cycle, output goes low */
            if (elapsed_cycles > 0) {
                --elapsed_cycles;
                timer->phase = 2;
            }
            load_counter_value( device, timer );
            set_output( device, timer, 0 );
        }

        if ( timer->phase == 2 )
        {
            adjusted_value = adjusted_count( bcd, timer->value );
            if ( elapsed_cycles >= adjusted_value )
            {
                /* Counter wrapped, output goes high */
                timer->phase = 3;
                set_output( device, timer, 1 );
            }
        }

        decrease_counter_value( timer, elapsed_cycles );

        switch( timer->phase )
        {
        case 1:     cycles_to_output = 1; break;
        case 2:     cycles_to_output = adjusted_count( bcd, timer->value ); break;
        default:    cycles_to_output = CYCLES_NEVER; break;
        }
        break;


    case 2:
        /* Mode 2: (Rate Generator)

        --------------+ +---------+ +----
                      | |         | |
                      +-+         +-+
           <-    n    -X-    n    ->
                      <1>
        ^
        +- counter load or trigger

        phase|output|length  |value|next|comment
        -----+------+--------+-----+----+----------------------------------
            0|high  |infinity|     |1   |waiting for count
            1|high  |1       |     |2   |internal delay to load counter
            2|high  |n       |n..2 |3   |counting down
            3|low   |1       |1    |2   |reload counter

        Counter rewrite has no effect until repeated

        Gate rising-edge and level sensitive.
        Gate low disables counting and sets output immediately high.
        Rising-edge reloads count and initiates counting
        Gate high enables counting. */

        if (pit8253_gate(timer) == 0 || timer->phase == 0)
        {
            /* Gate low or mode control write forces output high */
            set_output(device, timer, 1);
            cycles_to_output = CYCLES_NEVER;
        }
        else
        {
            if ( elapsed_cycles >= 0 && timer->phase == 1 )
            {
                if (elapsed_cycles > 0) {
                    --elapsed_cycles;
                    timer->phase = 2;
                }
                load_counter_value( device, timer );
            }

            adjusted_value = adjusted_count( bcd, timer->value );

            do
            {
                if ( timer->phase == 2 )
                {
                    if ( elapsed_cycles + 1 >= adjusted_value )
                    {
                        /* Coounter hits 1, output goes low */
                        timer->phase = 3;
                        set_output( device, timer, 0 );
                    }
                }

                if ( elapsed_cycles > 0 && timer->phase == 3 )
                {
                    /* Reload counter, output goes high */
                    --elapsed_cycles;
                    timer->phase = 2;
                    load_counter_value( device, timer );
                    adjusted_value = adjusted_count( bcd, timer->value );
                    set_output( device, timer, 1 );
                }
            }
            while( elapsed_cycles >= adjusted_value );

            /* Calculate counter value */
            decrease_counter_value(timer,elapsed_cycles);

            switch( timer->phase )
            {
            case 1:     cycles_to_output = 1; break;
            default:    cycles_to_output = (timer->value == 1 ? 1 : (adjusted_count(bcd,timer->value) - 1));
            }
        }
        break;


    case 3:
        /* Mode 3: (Square Wave Generator)

        ----------------+           +-----------+           +----
                        |           |           |           |
                        +-----------+           +-----------+
            <- (n+1)/2 -X-   n/2   ->
         ^
         +- counter load or trigger

        phase|output|length  |value|next|comment
        -----+------+--------+-----+----+----------------------------------
            0|high  |infinity|     |1   |waiting for count
            1|high  |1       |     |2   |internal delay to load counter
            2|high  |n/2(+1) |n..0 |3   |counting down double speed, reload counter
            3|low   |n/2     |n..0 |2   |counting down double speed, reload counter

        Counter rewrite has no effect until repeated (output falling or rising)

        Gate rising-edge and level sensitive.
        Gate low disables counting and sets output immediately high.
        Rising-edge reloads count and initiates counting
        Gate high enables counting. */

        if (pit8253_gate(timer) == 0 || timer->phase == 0)
        {
            /* Gate low or mode control write forces output high */
            set_output(device, timer, 1);
            cycles_to_output = CYCLES_NEVER;
        }
        else
        {
            if ( elapsed_cycles >= 0 && timer->phase == 1 )
            {
                if (elapsed_cycles > 0) {
                    --elapsed_cycles;
                    timer->phase = 2;
                }
                load_counter_value( device, timer );
            }

            if (elapsed_cycles > 0) {
                adjusted_value = adjusted_count( bcd, timer->value );

                do
                {
                    if ( timer->phase == 2 && elapsed_cycles >= ( ( adjusted_value + 1 ) >> 1 ) )
                    {
                        /* High phase expired, output goes low */
                        elapsed_cycles -= ( ( adjusted_value + 1 ) >> 1 );
                        timer->phase = 3;
                        load_counter_value( device, timer );
                        adjusted_value = adjusted_count( bcd, timer->value );
                        set_output( device, timer, 0 );
                    }

                    if ( timer->phase == 3 && elapsed_cycles >= ( adjusted_value >> 1 ) )
                    {
                        /* Low phase expired, output goes high */
                        elapsed_cycles -= ( adjusted_value >> 1 );
                        timer->phase = 2;
                        load_counter_value( device, timer );
                        adjusted_value = adjusted_count( bcd, timer->value );
                        set_output( device, timer, 1 );
                    }
                }
                while( ( timer->phase == 2 && elapsed_cycles >= ( ( adjusted_value + 1 ) >> 1 ) ) ||
                        ( timer->phase == 3 && elapsed_cycles >= ( adjusted_value >> 1 ) ) );

                decrease_counter_value(timer,elapsed_cycles<<1);
                switch( timer->phase )
                {
                case 1:     cycles_to_output = 1; break;
                case 2:     cycles_to_output = ( adjusted_count( bcd, timer->value ) + 1 ) >> 1; break;
                case 3:     cycles_to_output = adjusted_count( bcd, timer->value ) >> 1; break;
                }
            }
        }
        break;


    case 4:
    case 5:
        /* Mode 4: (Software Trigger Strobe)
           Mode 5: (Hardware Trigger Strobe)

        --------------+ +--------------------
                      | |
                      +-+
            <-  n+1  ->
            ^         <1>
            +- counter load (mode 4) or trigger (mode 5)

        phase|output|length  |value|next|comment
        -----+------+--------+-----+----+----------------------------------
            0|high  |infinity|0..1 |0   |waiting for count/counting down
            1|high  |1       |     |2   |internal delay when counter loaded
            2|high  |n       |n..1 |3   |counting down
            3|low   |1       |0    |0   |strobe

        Mode 4 only: counter rewrite loads new counter
        Mode 5 only: count not reloaded immediately.
        Mode control write doesn't stop count but sets output high

        Mode 4 only: Gate level sensitive only. Low disables counting, high enables it.
        Mode 5 only: Gate rising-edge sensitive only. Rising edge initiates counting */

        if (pit8253_gate(timer) == 0 && mode == 4)
        {
            cycles_to_output = CYCLES_NEVER;
        }
        else
        {
            if (elapsed_cycles >= 0 && timer->phase == 1)
            {
                if (elapsed_cycles > 0) {
                    --elapsed_cycles;
                    timer->phase = 2;
                }
                load_counter_value( device, timer );
            }

            if ( timer->value == 0 && timer->phase == 2 )
                adjusted_value = 0;
            else
                adjusted_value = adjusted_count( bcd, timer->value );

            if ( timer->phase == 2 && elapsed_cycles >= adjusted_value )
            {
                /* Counter has hit zero, set output to low */
                elapsed_cycles -= adjusted_value;
                timer->phase = 3;
                timer->value = 0;
                set_output( device, timer, 0 );
            }

            if (elapsed_cycles > 0 && timer->phase == 3)
            {
                --elapsed_cycles;
                timer->phase = 0;
                decrease_counter_value(timer,1);
                set_output( device, timer, 1 );
            }

            decrease_counter_value(timer,elapsed_cycles);

            switch( timer->phase )
            {
            case 1:     cycles_to_output = 1; break;
            case 2:     cycles_to_output = adjusted_count( bcd, timer->value ); break;
            case 3:     cycles_to_output = 1; break;
            }
        }
        break;
    }

    timer->cycles_to_output = cycles_to_output;
    if (cycles_to_output == CYCLES_NEVER || timer->clockin == 0)
    {
        timer->updatetimer->adjust(attotime::never, timer->index);
    }
    else
    {
        attotime next_fire_time = timer->last_updated + cycles_to_output * attotime::from_hz( timer->clockin );

        timer->updatetimer->adjust(next_fire_time - device->machine().time(), timer->index );
    }

    LOG2(("pit8253: simulate2(): simulating %d cycles for %d in mode %d, bcd = %d, phase = %d, gate = %d, output %d, value = 0x%04x, cycles_to_output = %04x\n",
            (int)elapsed_cycles,timer->index,mode,bcd,timer->phase,pit8253_gate(timer),timer->output,timer->value,cycles_to_output));
#endif
}


/* This emulates timer "timer" for "elapsed_cycles" cycles, broken down into
   sections punctuated by callbacks.

   The loop technically should never execute even once. It's here to eliminate
   the following potential bug:

   1) The mame timer isn't perfectly accurate.
   2) The output callback is executed too late, after an update which
      brings the timer's local time past the callback time.
   3) A short pulse is skipped.
   4) That short pulse would have triggered an interrupt. The interrupt is
      skipped.

   This is a loop instead of an "if" statement in case the mame timer is
   inaccurate by more than one cycle, and the output changed multiple
   times during the discrepancy. In practice updates should still be O(1).
*/








#if 0
static void common_start( device_t *device, int device_type ) {
    pit8253_t   *pit8253 = get_safe_token(device);
    int         timerno;

    pit8253->config = (const struct pit8253_config *)device->static_config();
    pit8253->device_type = device_type;

    /* register for state saving */
    for (timerno = 0; timerno < MAX_TIMER; timerno++)
    {
        pit8253_timer *timer = get_timer(pit8253, timerno);

        /* initialize timer */
        timer->clockin = pit8253->config->timer[timerno].clockin;
        timer->updatetimer = device->machine().scheduler().timer_alloc(FUNC(update_timer_cb), (void *)device);
        timer->updatetimer->adjust(attotime::never, timerno);

        /* resolve callbacks */
        timer->in_gate_func.resolve(pit8253->config->timer[timerno].in_gate_func, *device);
        timer->out_out_func.resolve(pit8253->config->timer[timerno].out_out_func, *device);

        /* set up state save values */
        device->save_item(NAME(timer->clockin), timerno);
        device->save_item(NAME(timer->control), timerno);
        device->save_item(NAME(timer->status), timerno);
        device->save_item(NAME(timer->lowcount), timerno);
        device->save_item(NAME(timer->latch), timerno);
        device->save_item(NAME(timer->count), timerno);
        device->save_item(NAME(timer->value), timerno);
        device->save_item(NAME(timer->wmsb), timerno);
        device->save_item(NAME(timer->rmsb), timerno);
        device->save_item(NAME(timer->output), timerno);
        device->save_item(NAME(timer->gate), timerno);
        device->save_item(NAME(timer->latched_count), timerno);
        device->save_item(NAME(timer->latched_status), timerno);
        device->save_item(NAME(timer->null_count), timerno);
        device->save_item(NAME(timer->phase), timerno);
        device->save_item(NAME(timer->cycles_to_output), timerno);
        device->save_item(NAME(timer->last_updated), timerno);
        device->save_item(NAME(timer->clock), timerno);
    }
}




void C8253PIT::Reset(void) {


    for (int i = 0; i < MAX_TIMER; i++)
    {
        pit8253_timer *timer = get_timer(i);
        /* According to Intel's 8254 docs, the state of a timer is undefined
           until the first mode control word is written. Here we define this
           undefined behaviour */
        timer->index = i;
        timer->control = timer->status = 0x30;
        timer->rmsb = timer->wmsb = 0;
        timer->count = timer->value = timer->latch = 0;
        timer->lowcount = 0;

        if (!timer->in_gate_func.isnull())
            timer->gate = timer->in_gate_func();
        else
            timer->gate = 1;

        timer->output = 2;  /* output is undetermined */
        timer->latched_count = 0;
        timer->latched_status = 0;
        timer->null_count = 1;
        timer->cycles_to_output = CYCLES_NEVER;

        timer->last_updated = device->machine().time();

//        update(device, timer);
    }
}


//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void pit8253_device::device_config_complete()
{
}
#endif

C8253PIT::C8253PIT(Ci80L188EB *parent)
{
    i80l188ebcpu = parent;

    t0 = new timer(this,0x08);
    t1 = new timer(this,0x12);
    t2 = new timer(this,0x13);

    count = 0;
}

C8253PIT::~C8253PIT()
{
}

void C8253PIT::step(quint64 nbstates)
{
    for (quint64 i=0; i< nbstates;i++) {
        count++;
        if (count == 2) t0->step();
        else if (count == 4) t1->step();
        else if (count == 6) t2->step();
        else if (count == 8) count = 0;
    }
}

void C8253PIT::throwint(int intNo) {
//    qWarning()<<"C8253PIT::throwint";
    if (i80l188ebcpu->eoi & 0x8000) {
        if (i80l188ebcpu->i86int(&(i80l188ebcpu->i86stat), intNo)) {
            i80l188ebcpu->eoi = 0;
            if(i80l188ebcpu->fp_log) fprintf(i80l188ebcpu->fp_log,"C8253PIT::throwint");
        }
    }
}

timer::timer(C8253PIT *parent, quint8 intnb)
{
    p8253pit = parent;
    this->intNb = intnb;
    tcon=0;
    tinp=1;           // timer input
    tcnt=0;
    tcmpA=tcmpB=0;
}

void timer::step()
{
    if (!EN()) return; // timer disabled

    if (!EXT()) {
        if (RTG()) {
            qWarning("RTG");
        }
        else { // RTG=0
            if (!tinp) {
//                qWarning("! TINP");
                return;
            }
        }
        if (P()) {
            // check t2 reach count
            if (p8253pit->t2->tcnt != p8253pit->t2->tcmpA) {
                qWarning("check t2 reach count");
                return;
            }
        }
    }


    // increment counter
    tcnt++;

    if (ALT()) {
        if(RIU()) { // use B
            if (tcnt != tcmpB) return;

            // set tout hight ????

            if (!CONT()) { // Clear EN bit
                tcon &= 0x7fff;
            }
        }
        else { // use A
            if (tcnt != tcmpA) return;
            // set tout hight ????
            // set RIU bit
            tcon |= 0x1000;

        }

    }
    else {
        if (tcnt != tcmpA) return;

        // pulse tout low for 1 clock ????

        if (!CONT()) { // Clear EN bit
            tcon &= 0x7fff;
        }

    }

    if (INT()) {
        // request interrupt
//        qWarning()<<"request interrupt";
        p8253pit->throwint(intNb);
    }

    // clear counter
    tcnt = 0;

}
