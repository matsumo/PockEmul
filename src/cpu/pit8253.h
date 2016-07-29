#ifndef __PIT8253_H__
#define __PIT8253_H__

#include "common.h"

class Ci80L188EB;
class C8253PIT;
/*
 TCON Control register
 15                                                                     0
 _______________   _______________   ___________________   _______________
| E | I | I | R | |   |   |   |   | |   |   |   | M | R | | P | E | A | C |
| N | N | N | I | |   |   |   |   | |   |   |   | C | T | |   | X | L | O |
|   | H | T | U | |   |   |   |   | |   |   |   |   | G | |   | T | T | N |
|   |   |   |   | |   |   |   |   | |   |   |   |   |   | |   |   |   | T |
 ---------------   ---------------   -------------------   ---------------
Mnemonic    Bit Name    Reset           State Function
EN          Enable      0       Set to enable the timer. This bit can be written only
                                when the INH bit is set.
INH         Inhibit     X       Set to enable writes to the EN bit. Clear to ignore
                                writes to the EN bit. The INH bit is not stored; it
                                always reads as zero.
INT         Interrupt   X       Set to generate an interrupt request when the Count
                                register equals a Maximum Count register. Clear to
                                disable interrupt requests.
RIU         Register    X       Indicates which compare register is in use. When set,
            In Use              the current compare register is Maxcount Compare B;
                                when clear, it is Maxcount Compare A.
MC          Max Count   X       This bit is set when the counter reaches a maximum
                                count. The MC bit must be cleared by writing to the
                                Timer Control register. This is not done automatically.
                                If MC is clear, the counter has not reached a
                                maximum count.
RTG         Retrigger   X       This bit specifies the action caused by a low-to-high
                                transition on the TMR INx input. Set RTG to reset the
                                count; clear RTG to enable counting. This bit is
                                ignored with external clocking (EXT=1).
P           Prescaler   X       Set to increment the timer when Timer 2 reaches its
                                maximum count. Clear to increment the timer at 1/4
                                CLKOUT. This bit is ignored with external clocking
                                (EXT=1).
EXT         External    X       Set to use external clock; clear to use internal clock.
            Clock               The RTG and P bits are ignored with external clocking
                                (EXT set).
ALT         Alternate   X       This bit controls whether the timer runs in single or
            Compare             dual maximum count mode (see Figure 9-4 on page
            Register            9-6). Set to specify dual maximum count mode; clear
                                to specify single maximum count mode.
CONT        Continuous  X       Set to cause the timer to run continuously. Clear to
            Mode                disable the counter (clear the EN bit) after each
                                counting sequence.
*/


class timer {

public:
    timer(C8253PIT *parent,quint8 intnb);

    quint16 EN() { return (tcon&0x8000); }
    quint16 INH(){ return (tcon&0x4000); }
    quint16 INT(){ return (tcon&0x2000); }
    quint16 RIU(){ return (tcon&0x1000); }
    quint16 MC() { return (tcon&0x0020); }
    quint16 RTG(){ return (tcon&0x0010); }
    quint16 P()  { return (tcon&0x0008); }
    quint16 EXT(){ return (tcon&0x0004); }
    quint16 ALT(){ return (tcon&0x0002); }
    quint16 CONT(){return (tcon&0x0001); }

    // Timer 0 and 1 Clock Sources
    // EXT P Clock Source
    //  0  0 Timer clocked internally at 1/4 CLKOUT frequency.
    //  0  1 Timer clocked internally, prescaled by Timer 2.
    //  1  X Timer clocked externally at up to 1/4 CLKOUT frequency.
    quint8 clockSource() { return ((tcon >> 3)&0x01) | ((tcon >> 1)&0x02); }


    //EXT RTG Timer Operation
    // 0   0  Timer counts internal events, if input pin remains high.
    // 0   1  Timer counts internal events; count resets to zero on every low-to-high transition on
    //        the input pin.
    // 1   X Timer input acts as clock source.
    quint8 Retriggering() {return ((tcon >> 4)&0x01) | ((tcon>>1)&0x02); }

    void step();
    void simulate2(qint64 elapsed_cycles);
    void decrease_counter_value(quint64 cycles);

    quint8  tinp;           // timer input
    quint16 tcon;
    quint16 tcnt;
    quint16 tcmpA,tcmpB;    // Contains the maximum value a timer will count
                            // to before resetting its Count register to zero.
    C8253PIT *p8253pit;

    quint8 intNb;

};


class C8253PIT
{
public:

    C8253PIT(Ci80L188EB *parent);
    ~C8253PIT();

    void step(quint64 nbstates);
    quint32 decimal_from_bcd(quint16 val);
    quint32 adjusted_count(int bcd, UINT16 val);
    void throwint(int intNo);

    timer *t0,*t1,*t2;

    Ci80L188EB *i80l188ebcpu;
private:

    quint8 count;
};






#endif  /* __PIT8253_H__ */
