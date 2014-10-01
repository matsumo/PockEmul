/*
 $Id$
 Copyright 1995, 2003, 2004, 2005 Eric L. Smith <eric@brouhaha.com>

 Nonpareil is free software; you can redistribute it and/or modify it
 under the terms of the GNU General Public License version 2 as
 published by the Free Software Foundation.  Note that I am not
 granting permission to redistribute or modify Nonpareil under the
 terms of any later version of the General Public License.

 Nonpareil is distributed in the hope that it will be useful, but
 WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program (in the file "COPYING"); if not, write to the
 Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 MA 02111, USA.
 */


#include <QFile>
#include <QDebug>

#include "hpnut.h"
#include "debug.h"
#include "ui/cregsz80widget.h"

//#include "digit_ops.h"
//#include "utils.h"
//#include "voyager_lcd.h"
//#include "proc_nut.h"

#define BIT_SET(val,bit_num)    ((val) |= (1 << (bit_num)))
#define BIT_CLEAR(val,bit_num)  ((val) &= ~(1 << (bit_num)))
#define BIT_TEST(val,bit_num)   ((val) & (1 << (bit_num)))

#define KBD_RELEASE_DEBOUNCE_CYCLES 32

#undef KEYBOARD_DEBUG
#undef ENABLE_RAM_WRITE_8_18

#ifdef KEYBOARD_DEBUG
char *kbd_state_name [KB_STATE_MAX] =
{
    [KB_IDLE]     = "idle",
    [KB_PRESSED]  = "pressed",
    [KB_RELEASED] = "released",
    [KB_WAIT_CHK] = "wait_chk",
    [KB_WAIT_CYC] = "wait_cyc"
};
#endif


#undef WARN_STRAY_WRITE

//#include "voyager_roms.h"

//void print_reg (reg_t reg);

/* map from high opcode bits to register index */
int tmap [16] =
{ 3, 4, 5, 10, 8, 6, 11, -1, 2, 9, 7, 13, 1, 12, 0, -1 };

/* map from register index to high opcode bits */
int itmap [WSIZE] =
{ 0xe, 0xc, 0x8, 0x0, 0x1, 0x2, 0x5, 0xa, 0x4, 0x9, 0x3, 0x6, 0xd, 0xb };



CHPNUT::CHPNUT(CPObject *parent,int ram_size):CCPU(parent)
{
    reg = nut_new_processor(ram_size);

    pDEBUG = new Cdebug_hpnut(pPC);
    regwidget = (CregCPU*) new Cregsz80Widget(0,this);

}

CHPNUT::~CHPNUT()
{

}

bool CHPNUT::init()
{
    nut_init_ops(reg);
    nut_clear_memory(reg);

    return true;
}

bool CHPNUT::exit()
{
    return true;
}

void CHPNUT::step()
{
    nut_execute_instruction(reg);
}

void CHPNUT::Reset(void) {
    nut_reset(reg);
}

void CHPNUT::Load_Internal(QXmlStreamReader *xmlIn)
{
    if (xmlIn->readNextStartElement()) {
        if ( (xmlIn->name()=="cpu") &&
             (xmlIn->attributes().value("model").toString() == "hpnut")) {
            reg_t* _ramPtr = reg->ram;
            bool* _ramExistsPtr = reg->ram_exists;
            QByteArray ba_reg = QByteArray::fromBase64(xmlIn->attributes().value("registers").toString().toLatin1());
            memcpy((char *) reg,ba_reg.data(),sizeof(cpu_t));

            // Restore registers pointers
            reg->ram = _ramPtr;
            reg->ram_exists = _ramExistsPtr;
            reg->pPC = pPC;
            nut_init_ops(reg);

            QByteArray ba_ram = QByteArray::fromBase64(xmlIn->attributes().value("ram").toString().toLatin1());
            memcpy((char *) reg->ram,ba_ram.data(),/*reg->max_ram*/256 * sizeof(reg_t));
            QByteArray ba_ram_exists = QByteArray::fromBase64(xmlIn->attributes().value("ram_exists").toString().toLatin1());
            memcpy((char *) reg->ram_exists,ba_ram_exists.data(),reg->max_ram * sizeof(bool));



        }
        xmlIn->skipCurrentElement();
    }
}

void CHPNUT::save_internal(QXmlStreamWriter *xmlOut)
{

    xmlOut->writeStartElement("cpu");
        xmlOut->writeAttribute("model","hpnut");
        QByteArray ba_reg((char*)reg,sizeof(cpu_t));
        xmlOut->writeAttribute("registers",ba_reg.toBase64());
        QByteArray ba_ram((char*)reg->ram,/*reg->max_ram*/ 256* sizeof(reg_t));
        xmlOut->writeAttribute("ram",ba_ram.toBase64());
        QByteArray ba_ram_exists((char*)reg->ram_exists,reg->max_ram * sizeof(bool));
        xmlOut->writeAttribute("ram_exists",ba_ram_exists.toBase64());
    xmlOut->writeEndElement();
}

UINT32 CHPNUT::get_PC()
{

}

void CHPNUT::Regs_Info(UINT8 Type)
{

}

rom_word_t CHPNUT::nut_get_ucode (cpu_t *nut_reg, rom_addr_t addr)
{
//        return nut_reg->rom[addr];
        return nut_reg->pPC->Get_16r(addr<<1);

}

inline quint8 CHPNUT::arithmetic_base (cpu_t *nut_reg)
{
    return nut_reg->decimal ? 10 : 16;
}


inline quint8 *CHPNUT::pt (cpu_t *nut_reg)
{
    return nut_reg->q_sel ? & nut_reg->q : & nut_reg->p;
}

void CHPNUT::bad_op (cpu_t *nut_reg, int opcode)
{
    //printf ("illegal opcode %03x at %04x\n", opcode, nut_reg->prev_pc);
}


void CHPNUT::op_arith (cpu_t *nut_reg, int opcode)
{
    int op, field;
    int first=0, last=0;

    op = opcode >> 5;
    field = (opcode >> 2) & 7;

    switch (field)
    {
        case 0:  /* p  */  first = *pt (nut_reg);  last = *pt (nut_reg);  break;
        case 1:  /* x  */  first = 0;              last = EXPSIZE - 1;    break;
        case 2:  /* wp */  first = 0;              last = *pt (nut_reg);  break;
        case 3:  /* w  */  first = 0;              last = WSIZE - 1;      break;
        case 4:  /* pq */  first = nut_reg->p;     last = nut_reg->q;
            if (first > last)
                last = WSIZE - 1;
                break;
        case 5:  /* xs */  first = EXPSIZE - 1;    last = EXPSIZE - 1;    break;
        case 6:  /* m  */  first = EXPSIZE;        last = WSIZE - 2;      break;
        case 7:  /* s  */  first = WSIZE - 1;      last = WSIZE - 1;      break;
    }

    nut_reg->prev_tef_last = last;

    switch (op)
    {
        case 0x00:  /* a=0 */
            reg_zero (nut_reg->a, first, last);
            break;

        case 0x01:  /* b=0 */
            reg_zero (nut_reg->b, first, last);
                break;

        case 0x02:  /* c=0 */
            reg_zero (nut_reg->c, first, last);
            break;

        case 0x03:  /* ab ex */
            reg_exch (nut_reg->a, nut_reg->b, first, last);
            break;

        case 0x04:  /* b=a */
            reg_copy (nut_reg->b, nut_reg->a, first, last);
                break;

        case 0x05:  /* ac ex */
            reg_exch (nut_reg->a, nut_reg->c, first, last);
            break;

        case 0x06:  /* c=b */
            reg_copy (nut_reg->c, nut_reg->b, first, last);
            break;

        case 0x07:  /* bc ex */
            reg_exch (nut_reg->b, nut_reg->c, first, last);
            break;

        case 0x08:  /* a=c */
            reg_copy (nut_reg->a, nut_reg->c, first, last);
            break;

        case 0x09:  /* a=a+b */
            reg_add (nut_reg->a, nut_reg->a, nut_reg->b,
                     first, last,
                     & nut_reg->carry, arithmetic_base (nut_reg));
            break;

        case 0x0a:  /* a=a+c */
            reg_add (nut_reg->a, nut_reg->a, nut_reg->c,
                     first, last,
                     & nut_reg->carry, arithmetic_base (nut_reg));
            break;

        case 0x0b:    /* a=a+1 */
            nut_reg->carry = 1;
            reg_add (nut_reg->a, nut_reg->a, NULL,
                     first, last,
                     & nut_reg->carry, arithmetic_base (nut_reg));
            break;

        case 0x0c:  /* a=a-b */
            reg_sub (nut_reg->a, nut_reg->a, nut_reg->b,
                     first, last,
                     & nut_reg->carry, arithmetic_base (nut_reg));
            break;

        case 0x0d:  /* a=a-1 */
            nut_reg->carry = 1;
            reg_sub (nut_reg->a, nut_reg->a, NULL,
                     first, last,
                     & nut_reg->carry, arithmetic_base (nut_reg));
            break;

        case 0x0e:  /* a=a-c */
            reg_sub (nut_reg->a, nut_reg->a, nut_reg->c,
                     first, last,
                     & nut_reg->carry, arithmetic_base (nut_reg));
            break;

        case 0x0f:  /* c=c+c */
            reg_add (nut_reg->c, nut_reg->c, nut_reg->c,
                     first, last,
                     & nut_reg->carry, arithmetic_base (nut_reg));
            break;

        case 0x10:  /* c=a+c */
            reg_add (nut_reg->c, nut_reg->a, nut_reg->c,
                     first, last,
                     & nut_reg->carry, arithmetic_base (nut_reg));
            break;

        case 0x11:  /* c=c+1 */
            nut_reg->carry = 1;
            reg_add (nut_reg->c, nut_reg->c, NULL,
                     first, last,
                     & nut_reg->carry, arithmetic_base (nut_reg));
            break;

        case 0x12:  /* c=a-c */
            reg_sub (nut_reg->c, nut_reg->a, nut_reg->c,
                     first, last,
                     & nut_reg->carry, arithmetic_base (nut_reg));
            break;

        case 0x13:  /* c=c-1 */
            nut_reg->carry = 1;
            reg_sub (nut_reg->c, nut_reg->c, NULL,
                     first, last,
                     & nut_reg->carry, arithmetic_base (nut_reg));
            break;

        case 0x14:  /* c=-c */
            reg_sub (nut_reg->c, NULL, nut_reg->c,
                     first, last,
                     & nut_reg->carry, arithmetic_base (nut_reg));
            break;

        case 0x15:  /* c=-c-1 */
            nut_reg->carry = 1;
            reg_sub (nut_reg->c, NULL, nut_reg->c,
                     first, last,
                     & nut_reg->carry, arithmetic_base (nut_reg));
            break;

        case 0x16:  /* ? b<>0 */
            reg_test_nonequal (nut_reg->b, NULL,
                               first, last,
                               & nut_reg->carry);
            break;

        case 0x17:  /* ? c<>0 */
            reg_test_nonequal (nut_reg->c, NULL,
                               first, last,
                               & nut_reg->carry);
            break;

        case 0x18:  /* ? a<c */
            reg_sub (NULL, nut_reg->a, nut_reg->c,
                     first, last,
                     & nut_reg->carry, arithmetic_base (nut_reg));
            break;

        case 0x19:  /* ? a<b */
            reg_sub (NULL, nut_reg->a, nut_reg->b,
                     first, last,
                     & nut_reg->carry, arithmetic_base (nut_reg));
            break;

        case 0x1a:  /* ? a<>0 */
            reg_test_nonequal (nut_reg->a, NULL,
                               first, last,
                               & nut_reg->carry);
            break;

        case 0x1b:  /* ? a<>c */
            reg_test_nonequal (nut_reg->a, nut_reg->c,
                               first, last,
                               & nut_reg->carry);
            break;

        case 0x1c:  /* a sr */
            reg_shift_right (nut_reg->a, first, last);
            break;

        case 0x1d:  /* b sr */
            reg_shift_right (nut_reg->b, first, last);
            break;

        case 0x1e:  /* c sr */
            reg_shift_right (nut_reg->c, first, last);
            break;

        case 0x1f:  /* a sl */
            reg_shift_left (nut_reg->a, first, last);
            break;
    }
}



rom_addr_t CHPNUT::pop (cpu_t *nut_reg)
{
    int i;
    rom_addr_t ret;

    ret = nut_reg->stack [0];
    for (i = 0; i < STACK_DEPTH - 1; i++)
        nut_reg->stack [i] = nut_reg->stack [i + 1];
    nut_reg->stack [STACK_DEPTH - 1] = 0;
    return (ret);
}

void CHPNUT::push (cpu_t *nut_reg, rom_addr_t a)
{
    int i;

    for (i = STACK_DEPTH - 1; i > 0; i--)
        nut_reg->stack [i] = nut_reg->stack [i - 1];
    nut_reg->stack [0] = a;
}

void CHPNUT::op_return (cpu_t *nut_reg, int opcode)
{
    nut_reg->pc = pop (nut_reg);
}

void CHPNUT::op_return_if_carry (cpu_t *nut_reg, int opcode)
{
    if (nut_reg->prev_carry)
        nut_reg->pc = pop (nut_reg);
}

void CHPNUT::op_return_if_no_carry (cpu_t *nut_reg, int opcode)
{
    if (! nut_reg->prev_carry)
        nut_reg->pc = pop (nut_reg);
}

void CHPNUT::op_pop (cpu_t *nut_reg, int opcode)
{
    (void) pop (nut_reg);
}

void CHPNUT::op_pop_c (cpu_t *nut_reg, int opcode)
{
    rom_addr_t a;

    a = pop (nut_reg);
    nut_reg->c [6] = a >> 12;
    nut_reg->c [5] = (a >> 8) & 0x0f;
    nut_reg->c [4] = (a >> 4) & 0x0f;
    nut_reg->c [3] = a & 0x0f;
}


void CHPNUT::op_push_c (cpu_t *nut_reg, int opcode)
{
    push (nut_reg, ((nut_reg->c [6] << 12) |
                (nut_reg->c [5] << 8) |
                (nut_reg->c [4] << 4) |
                (nut_reg->c [3])));
}


//

void CHPNUT::op_short_branch (cpu_t *nut_reg, int opcode)
{
    int offset;

    offset = (opcode >> 3) & 0x3f;
    if (opcode & 0x200)
        offset -= 64;

    if (((opcode >> 2) & 1) == nut_reg->prev_carry)
        nut_reg->pc = nut_reg->pc + offset - 1;
}


void CHPNUT::op_long_branch (cpu_t *nut_reg, int opcode)
{
    nut_reg->inst_state = long_branch;
    nut_reg->first_word = opcode;
    nut_reg->long_branch_carry = nut_reg->prev_carry;
}


void CHPNUT::op_long_branch_word_2 (cpu_t *nut_reg, int opcode)
{
    rom_addr_t target;

    nut_reg->inst_state = norm;
    target = (nut_reg->first_word >> 2) | ((opcode & 0x3fc) << 6);

    if ((opcode & 0x001) == nut_reg->long_branch_carry)
    {
        if (opcode & 0x002)
            nut_reg->pc = target;
        else
        {
            push (nut_reg, nut_reg->pc);
            nut_reg->pc = target;
            if (nut_get_ucode (nut_reg, nut_reg->pc) == 0)
                nut_reg->pc = pop (nut_reg);
        }
    }
}


void CHPNUT::op_goto_c (cpu_t *nut_reg, int opcode)
{
    nut_reg->pc = ((nut_reg->c [6] << 12) |
                   (nut_reg->c [5] << 8) |
                   (nut_reg->c [4] << 4) |
                   (nut_reg->c [3]));
}

void CHPNUT::op_enbank (cpu_t *nut_reg, int opcode)
{
    //select_bank (nut_reg, nut_reg->prev_pc, ((opcode >> 5) & 2) + ((opcode >> 7) & 1));
    printf("select bank!!!\n");
}



void CHPNUT::op_c_to_m (cpu_t *nut_reg, int opcode)
{
    reg_copy (nut_reg->m, nut_reg->c, 0, WSIZE - 1);
}

void CHPNUT::op_m_to_c (cpu_t *nut_reg, int opcode)
{
    reg_copy (nut_reg->c, nut_reg->m, 0, WSIZE - 1);
}

void CHPNUT::op_c_exch_m (cpu_t *nut_reg, int opcode)
{
    reg_exch (nut_reg->c, nut_reg->m, 0, WSIZE - 1);
}



void CHPNUT::op_c_to_n (cpu_t *nut_reg, int opcode)
{
    reg_copy (nut_reg->n, nut_reg->c, 0, WSIZE - 1);
}

void CHPNUT::op_n_to_c (cpu_t *nut_reg, int opcode)
{
    reg_copy (nut_reg->c, nut_reg->n, 0, WSIZE - 1);
}

void CHPNUT::op_c_exch_n (cpu_t *nut_reg, int opcode)
{
    reg_exch (nut_reg->c, nut_reg->n, 0, WSIZE - 1);
}



void CHPNUT::nut_ram_read_zero (cpu_t *nut_reg, int addr, reg_t *reg)
{
    int i;
    //printf("nut_ram_read_zero\n");
    for (i = 0; i < WSIZE; i++)
        (*reg) [i] = 0;
}

void CHPNUT::nut_ram_write_ignore (cpu_t *nut_reg, int addr, reg_t *reg)
{
    //printf("nut_ram_write_ignore\n");;
}

void CHPNUT::op_c_to_dadd (cpu_t *nut_reg, int opcode)
{
    nut_reg->ram_addr = ((nut_reg->c [2] << 8) |
                         (nut_reg->c [1] << 4) |
                         (nut_reg->c [0])) & 0x3ff;

    //printf("op_c_to_dadd c2=%x c1=%x c0=%x\n",nut_reg->c[2],nut_reg->c [1],nut_reg->c[0]);
    //vooyager_display_event_fn (nut_reg, event_ram_select);
}

void CHPNUT::op_c_to_pfad (cpu_t *nut_reg, int opcode)
{
    //nut_reg->pf_addr = ((nut_reg->c [1] << 4) |
    //					(nut_reg->c [0]));

    //vooyager_display_event_fn (nut_reg, event_periph_select);
    printf("event_periph_select\n");
}

void CHPNUT::op_read_reg_n (cpu_t *nut_reg, int opcode)
{
    quint16 ram_addr;
    int i;

    for (i = 0; i < WSIZE; i++)
        nut_reg->c [i] = 0;

    if ((opcode >> 6) != 0)
        nut_reg->ram_addr = (nut_reg->ram_addr & ~0x0f) | (opcode >> 6);

    ram_addr = nut_reg->ram_addr;

    //printf("op_read_reg_n=%d\n", ram_addr);

    if (nut_reg->ram_exists [ram_addr])
    {
        if ((ram_addr == 0x08)||(ram_addr == 0x18))
            nut_ram_read_zero(nut_reg, ram_addr, & nut_reg->c);
        else
            for (i = 0; i < WSIZE; i++)
                nut_reg->c [i] = nut_reg->ram [ram_addr][i];
    } else
        printf("op_read_reg_n no ram at %x\n",ram_addr);
}


void CHPNUT::op_write_reg_n (cpu_t *nut_reg, int opcode)
{
    quint16 ram_addr;
    int i;

    nut_reg->ram_addr = (nut_reg->ram_addr & ~0x0f) | (opcode >> 6);
    ram_addr = nut_reg->ram_addr;

    //printf("op_write_reg_n ram_addr=%d\n", ram_addr);

    if (nut_reg->ram_exists [ram_addr])
    {
        //if (nut_reg->ram_write_fn [ram_addr])
        //	nut_reg->ram_write_fn [ram_addr] (nut_reg, ram_addr, & nut_reg->c);
        if ((ram_addr == 0x08)||(ram_addr == 0x18))
            nut_ram_write_ignore(nut_reg, ram_addr, & nut_reg->c);
        else
        //if ((ram_addr != 0x08)&&(ram_addr != 0x18)) //TO DO: check this.
            for (i = 0; i < WSIZE; i++)
                nut_reg->ram [ram_addr][i] = nut_reg->c [i];
        //else
        //    printf("op_write_reg_n not written ram_addr=%d\n", ram_addr);
    } else
        printf("op_write_reg_n no ram at %x\n",ram_addr);
}

void CHPNUT::op_c_to_data (cpu_t *nut_reg, int opcode)
{
    quint16 ram_addr;
    int i;

    ram_addr = nut_reg->ram_addr;
    //printf("op_write_reg_n ram_addr=%d\n", ram_addr);

    if (nut_reg->ram_exists [ram_addr])
    {
        //if (nut_reg->ram_write_fn [ram_addr])
        //	nut_reg->ram_write_fn [ram_addr] (nut_reg, ram_addr, & nut_reg->c);
        if ((ram_addr == 0x08)||(ram_addr == 0x18))
            nut_ram_write_ignore(nut_reg, ram_addr, & nut_reg->c);
        else
        //if ((ram_addr != 0x08)&&(ram_addr != 0x18))
            for (i = 0; i < WSIZE; i++)
                nut_reg->ram [ram_addr][i] = nut_reg->c [i];
        //else
        //    printf("op_c_to_data not written ram_addr=%d\n", ram_addr);
    } else
        printf("op_c_to_data no ram at %x\n",ram_addr);
}


void CHPNUT::op_test_ext_flag (cpu_t *nut_reg, int opcode)
{
    //nut_reg->carry = nut_reg->ext_flag [tmap [opcode >> 6]];
    nut_reg->carry = BIT_TEST(nut_reg->ext_flag, (tmap [opcode >> 6]));
}


/*void op_selprf (cpu_t *nut_reg, int opcode)
{
    nut_reg->selprf = opcode >> 6;
    nut_reg->inst_state = selprf;
    printf("selprt?\n");
}*/


// This "opcode" handles all instructions following a selprf (AKA
// PERTCT or SELP) instruction until a return of control.
/*void op_smart_periph (cpu_t *nut_reg, int opcode)
{
    bool flag = false;

    if (nut_reg->selprf_fcn [nut_reg->selprf])
        flag = nut_reg->selprf_fcn [nut_reg->selprf] (nut_reg, opcode);
    if ((opcode & 0x03f) == 0x003)
        nut_reg->carry = flag;
    if (opcode & 1)
        nut_reg->inst_state = norm;
}*/



void CHPNUT::op_set_s (cpu_t *nut_reg, int opcode)
{
    //nut_reg->s [tmap [opcode >> 6]] = 1;
    BIT_SET(nut_reg->s, (tmap [opcode >> 6]));
}

void CHPNUT::op_clr_s (cpu_t *nut_reg, int opcode)
{
    //nut_reg->s [tmap [opcode >> 6]] = 0;
    BIT_CLEAR(nut_reg->s, (tmap [opcode >> 6]));
}

void CHPNUT::op_test_s (cpu_t *nut_reg, int opcode)
{
    //nut_reg->carry = nut_reg->s [tmap [opcode >> 6]];
    nut_reg->carry = BIT_TEST(nut_reg->s, (tmap [opcode >> 6]));
}

int CHPNUT::get_s_bits (cpu_t *nut_reg, int first, int count)
{
    int i;
    int mask = 1;
    int r = 0;
    for (i = first; i < first + count; i++)
    {
        //if (nut_reg->s [i])
        if (BIT_TEST(nut_reg->s, i))
            r = r + mask;
        mask <<= 1;
    }
    return (r);
}

void CHPNUT::set_s_bits (cpu_t *nut_reg, int first, int count, int a)
{
    int i;
    int mask = 1;

    for (i = first; i < first + count; i++)
    {
        //nut_reg->s [i] = (a & mask) != 0;
        if ((a & mask) != 0) {
            BIT_SET(nut_reg->s, i);
        } else {
            BIT_CLEAR(nut_reg->s, i);
        }
        mask <<= 1;
    }
}

void CHPNUT::op_clear_all_s (cpu_t *nut_reg, int opcode)
{
    set_s_bits (nut_reg, 0, 8, 0);
}

void CHPNUT::op_c_to_s (cpu_t *nut_reg, int opcode)
{
    set_s_bits (nut_reg, 0, 4, nut_reg->c [0]);
    set_s_bits (nut_reg, 4, 4, nut_reg->c [1]);
}

void CHPNUT::op_s_to_c (cpu_t *nut_reg, int opcode)
{
    nut_reg->c [0] = get_s_bits (nut_reg, 0, 4);
    nut_reg->c [1] = get_s_bits (nut_reg, 4, 4);
}

void CHPNUT::op_c_exch_s (cpu_t *nut_reg, int opcode)
{
    int t;

    t = get_s_bits (nut_reg, 0, 4);
    set_s_bits (nut_reg, 0, 4, nut_reg->c [0]);
    nut_reg->c [0] = t;
    t = get_s_bits (nut_reg, 4, 4);
    set_s_bits (nut_reg, 4, 4, nut_reg->c [1]);
    nut_reg->c [1] = t;
}

void CHPNUT::op_sb_to_f (cpu_t *nut_reg, int opcode)
{
    nut_reg->fo = get_s_bits (nut_reg, 0, 8);
}

void CHPNUT::op_f_to_sb (cpu_t *nut_reg, int opcode)
{
    set_s_bits (nut_reg, 0, 8, nut_reg->fo);
}

void CHPNUT::op_f_exch_sb (cpu_t *nut_reg, int opcode)
{
    int t;

    t = get_s_bits (nut_reg, 0, 8);
    set_s_bits (nut_reg, 0, 8, nut_reg->fo);
    nut_reg->fo = t;
}


void CHPNUT::op_dec_pt (cpu_t *nut_reg, int opcode)
{
    (*pt (nut_reg))--;
    if ((*pt (nut_reg)) >= WSIZE)  // can't be negative because it is unsigned
        (*pt (nut_reg)) = WSIZE - 1;
}

void CHPNUT::op_inc_pt (cpu_t *nut_reg, int opcode)
{
    (*pt (nut_reg))++;
    if ((*pt (nut_reg)) >= WSIZE)
        (*pt (nut_reg)) = 0;
}

void CHPNUT::op_set_pt (cpu_t *nut_reg, int opcode)
{
    (*pt (nut_reg)) = tmap [opcode >> 6];
}

void CHPNUT::op_test_pt (cpu_t *nut_reg, int opcode)
{
    nut_reg->carry = ((*pt (nut_reg)) == tmap [opcode >> 6]);
}

void CHPNUT::op_sel_p (cpu_t *nut_reg, int opcode)
{
    nut_reg->q_sel = false;
}

void CHPNUT::op_sel_q (cpu_t *nut_reg, int opcode)
{
    nut_reg->q_sel = true;
}

void CHPNUT::op_test_pq (cpu_t *nut_reg, int opcode)
{
    if (nut_reg->p == nut_reg->q)
        nut_reg->carry = 1;
}

void CHPNUT::op_lc (cpu_t *nut_reg, int opcode)
{
    nut_reg->c [(*pt (nut_reg))--] = opcode >> 6;
    if ((*pt (nut_reg)) >= WSIZE)  /* unsigned, can't be negative */
        *pt (nut_reg) = WSIZE - 1;
}

void CHPNUT::op_c_to_g (cpu_t *nut_reg, int opcode)
{
    nut_reg->g [0] = nut_reg->c [*pt (nut_reg)];
    if ((*pt (nut_reg)) == (WSIZE - 1))
    {
        nut_reg->g [1] = 0;
#ifdef WARNING_G
        fprintf (stderr, "warning: c to g transfer with pt=13\n");
#endif
    }
    else
        nut_reg->g [1] = nut_reg->c [(*pt (nut_reg)) + 1];
}

void CHPNUT::op_g_to_c (cpu_t *nut_reg, int opcode)
{
    nut_reg->c [(*pt (nut_reg))] = nut_reg->g [0];
    if ((*pt (nut_reg)) == (WSIZE - 1))
    {
        ;
#ifdef WARNING_G
        fprintf (stderr, "warning: g to c transfer with pt=13\n");
#endif
    }
    else
    {
        nut_reg->c [(*pt (nut_reg)) + 1] = nut_reg->g [1];
    }

}

void CHPNUT::op_c_exch_g (cpu_t *nut_reg, int opcode)
{
    int t;

    t = nut_reg->g [0];
    nut_reg->g [0] = nut_reg->c [*pt (nut_reg)];
    nut_reg->c [*pt (nut_reg)] = t;
    if ((*pt (nut_reg)) == (WSIZE - 1))
    {
        nut_reg->g [1] = 0;
#ifdef WARNING_G
        fprintf (stderr, "warning: c exchange g with pt=13\n");
#endif
    }
    else
    {
        t = nut_reg->g [1];
        nut_reg->g [1] = nut_reg->c [(*pt (nut_reg)) + 1];
        nut_reg->c [(*pt (nut_reg)) + 1] = t;
    }
}



void CHPNUT::op_keys_to_rom_addr (cpu_t *nut_reg, int opcode)
{
    nut_reg->pc = (nut_reg->pc & 0xff00) | nut_reg->key_buf;
}


void CHPNUT::op_keys_to_c (cpu_t *nut_reg, int opcode)
{
    nut_reg->c [4] = nut_reg->key_buf >> 4;
    nut_reg->c [3] = nut_reg->key_buf & 0x0f;
}


void CHPNUT::op_test_kb (cpu_t *nut_reg, int opcode)
{
#ifdef KEYBOARD_DEBUG
    printf ("kb test, addr %04x, state %s\n", nut_reg->prev_pc, kbd_state_name [nut_reg->kb_state]);
#endif

    nut_reg->carry = ((nut_reg->kb_state == KB_PRESSED) ||
                      (nut_reg->kb_state == KB_RELEASED));
    if (nut_reg->kb_state == KB_WAIT_CHK)
    {
        nut_reg->kb_state = KB_WAIT_CYC;
        nut_reg->kb_debounce_cycle_counter = KBD_RELEASE_DEBOUNCE_CYCLES;
    }
}


void CHPNUT::op_reset_kb (cpu_t *nut_reg, int opcode)
{
#ifdef KEYBOARD_DEBUG
    printf ("kb reset, addr %04x, state %s\n", nut_reg->prev_pc, kbd_state_name [nut_reg->kb_state]);
#endif

    if (nut_reg->kb_state == KB_RELEASED)
        nut_reg->kb_state = KB_WAIT_CHK;
}

void CHPNUT::nut_kbd_scanner_cycle (cpu_t *nut_reg)
{
    if ((nut_reg->kb_state == KB_WAIT_CYC) &&
        (--nut_reg->kb_debounce_cycle_counter == 0))
    {
        if (nut_reg->key_down)
        {
            nut_reg->kb_state = KB_PRESSED;
#ifdef SLEEP_DEBUG
            if (! nut_reg->awake)
                printf ("waking up!\n");
#endif
            nut_reg->awake = true;
        }
        else
            nut_reg->kb_state = KB_IDLE;
    }
}


void CHPNUT::nut_kbd_scanner_sleep (cpu_t *nut_reg)
{
#ifdef KEYBOARD_DEBUG
    printf ("nut_kbd_scanner_sleep, state=%s\n", kbd_state_name [nut_reg->kb_state]);
#endif

    if (nut_reg->kb_state == KB_PRESSED)
    {
        // $$$ This shouldn't happen, should it?
#if defined(KEYBOARD_DEBUG) || defined(SLEEP_DEBUG)
        if (! nut_reg->awake)
            printf ("waking up!\n");
#endif
        nut_reg->awake = true;
    }
    if (nut_reg->kb_state == KB_WAIT_CYC)
    {
        if (nut_reg->key_down)
        {
            nut_reg->kb_state = KB_PRESSED;
#if defined(KEYBOARD_DEBUG) || defined(SLEEP_DEBUG)
            if (! nut_reg->awake)
                printf ("waking up!\n");
#endif
            nut_reg->awake = true;
        }
        else
            nut_reg->kb_state = KB_IDLE;
    }
}



void CHPNUT::op_nop (cpu_t *nut_reg, int opcode)
{
}

void CHPNUT::op_set_hex (cpu_t *nut_reg, int opcode)
{
    nut_reg->decimal = false;
}

void CHPNUT::op_set_dec (cpu_t *nut_reg, int opcode)
{
    nut_reg->decimal = true;
}

void CHPNUT::op_rom_to_c (cpu_t *nut_reg, int opcode)
{
    nut_reg->cxisa_addr = ((nut_reg->c [6] << 12) |
                           (nut_reg->c [5] << 8) |
                           (nut_reg->c [4] << 4) |
                           (nut_reg->c [3]));
    nut_reg->inst_state = cxisa;
}

void CHPNUT::op_rom_to_c_cycle_2 (cpu_t *nut_reg, int opcode)
{
    nut_reg->c [2] = opcode >> 8;
    nut_reg->c [1] = (opcode >> 4) & 0x0f;
    nut_reg->c [0] = opcode & 0x0f;

    nut_reg->inst_state = norm;
}

void CHPNUT::op_clear_abc (cpu_t *nut_reg, int opcode)
{
    reg_zero (nut_reg->a, 0, WSIZE - 1);
    reg_zero (nut_reg->b, 0, WSIZE - 1);
    reg_zero (nut_reg->c, 0, WSIZE - 1);
}

void CHPNUT::op_ldi (cpu_t *nut_reg, int opcode)
{
    nut_reg->inst_state = ldi;
}

void CHPNUT::op_ldi_cycle_2 (cpu_t *nut_reg, int opcode)
{
    nut_reg->c [2] = opcode >> 8;
    nut_reg->c [1] = (opcode >> 4) & 0x0f;
    nut_reg->c [0] = opcode & 0x00f;

    nut_reg->inst_state = norm;
}

void CHPNUT::op_or (cpu_t *nut_reg, int opcode)
{
    int i;

    for (i = 0; i < WSIZE; i++)
        nut_reg->c [i] |= nut_reg->a [i];
    if (nut_reg->prev_carry && (nut_reg->prev_tef_last == (WSIZE - 1)))
    {
        nut_reg->c [WSIZE - 1] = nut_reg->c [0];
        nut_reg->a [WSIZE - 1] = nut_reg->c [0];
    }
}

void CHPNUT::op_and (cpu_t *nut_reg, int opcode)
{
    int i;

    for (i = 0; i < WSIZE; i++)
        nut_reg->c [i] &= nut_reg->a [i];
    if (nut_reg->prev_carry && (nut_reg->prev_tef_last == (WSIZE - 1)))
    {
        nut_reg->c [WSIZE - 1] = nut_reg->c [0];
        nut_reg->a [WSIZE - 1] = nut_reg->c [0];
    }
}

void CHPNUT::op_rcr (cpu_t *nut_reg, int opcode)
{
    int i, j;
    reg_t t;

    j = tmap [opcode >> 6];
    for (i = 0; i < WSIZE; i++)
    {
        t [i] = nut_reg->c [j++];
        if (j >= WSIZE)
            j = 0;
    }
    for (i = 0; i < WSIZE; i++)
        nut_reg->c [i] = t [i];
}

void CHPNUT::op_lld (cpu_t *nut_reg, int opcode)
{
    nut_reg->carry = 0;  /* "batteries" are fine */
}

void CHPNUT::op_powoff (cpu_t *nut_reg, int opcode)
{
#ifdef SLEEP_DEBUG
    printf ("going to sleep!\n");
#endif
    nut_reg->awake = false;
    nut_reg->pc = 0;

    nut_kbd_scanner_sleep (nut_reg);
//    voyager_display_sleep(nut_reg);
}


void CHPNUT::nut_init_ops (cpu_t *nut_reg)
{
    int i;

    for (i = 0; i < 1024; i += 4)
    {
        nut_reg->op_fcn [i + 0] = bad_op;
        nut_reg->op_fcn [i + 1] = op_long_branch;
        nut_reg->op_fcn [i + 2] = op_arith;  /* type 2: ooooowww10 */
        nut_reg->op_fcn [i + 3] = op_short_branch;
    }

    nut_reg->op_fcn [0x000] = op_nop;

    // nut_reg->op_fcn [0x040] = op_write_mldl;

    for (i = 0; i < 4; i++)
        nut_reg->op_fcn [0x100 + i * 0x040] = op_enbank;

    // for (i = 0; i < 8; i++)
    //   op_fcn [0x200 + (i << 6)] = op_write_pil;

    for (i = 0; i < WSIZE; i ++)
    {
        nut_reg->op_fcn [0x004 + (itmap [i] << 6)] = op_clr_s;
        nut_reg->op_fcn [0x008 + (itmap [i] << 6)] = op_set_s;
        nut_reg->op_fcn [0x00c + (itmap [i] << 6)] = op_test_s;
        nut_reg->op_fcn [0x014 + (itmap [i] << 6)] = op_test_pt;
        nut_reg->op_fcn [0x01c + (itmap [i] << 6)] = op_set_pt;
        nut_reg->op_fcn [0x02c + (itmap [i] << 6)] = op_test_ext_flag;
        nut_reg->op_fcn [0x03c + (itmap [i] << 6)] = op_rcr;
    }
    nut_reg->op_fcn [0x3c4] = op_clear_all_s;
    nut_reg->op_fcn [0x3c8] = op_reset_kb;
    nut_reg->op_fcn [0x3cc] = op_test_kb;
    nut_reg->op_fcn [0x3d4] = op_dec_pt;
    nut_reg->op_fcn [0x3dc] = op_inc_pt;
    // 0x3fc = LCD compensation

    for (i = 0; i < 16; i++)
    {
        nut_reg->op_fcn [0x010 + (i << 6)] = op_lc;
        //nut_reg->op_fcn [0x024 + (i << 6)] = op_selprf;
        nut_reg->op_fcn [0x028 + (i << 6)] = op_write_reg_n;
        nut_reg->op_fcn [0x038 + (i << 6)] = op_read_reg_n;
    }

    nut_reg->op_fcn [0x058] = op_c_to_g;
    nut_reg->op_fcn [0x098] = op_g_to_c;
    nut_reg->op_fcn [0x0d8] = op_c_exch_g;

    nut_reg->op_fcn [0x158] = op_c_to_m;
    nut_reg->op_fcn [0x198] = op_m_to_c;
    nut_reg->op_fcn [0x1d8] = op_c_exch_m;

    nut_reg->op_fcn [0x258] = op_sb_to_f;
    nut_reg->op_fcn [0x298] = op_f_to_sb;
    nut_reg->op_fcn [0x2d8] = op_f_exch_sb;

    nut_reg->op_fcn [0x358] = op_c_to_s;
    nut_reg->op_fcn [0x398] = op_s_to_c;
    nut_reg->op_fcn [0x3d8] = op_c_exch_s;

    nut_reg->op_fcn [0x020] = op_pop;
    nut_reg->op_fcn [0x060] = op_powoff;
    nut_reg->op_fcn [0x0a0] = op_sel_p;
    nut_reg->op_fcn [0x0e0] = op_sel_q;
    nut_reg->op_fcn [0x120] = op_test_pq;
    nut_reg->op_fcn [0x160] = op_lld;
    nut_reg->op_fcn [0x1a0] = op_clear_abc;
    nut_reg->op_fcn [0x1e0] = op_goto_c;
    nut_reg->op_fcn [0x220] = op_keys_to_c;
    nut_reg->op_fcn [0x260] = op_set_hex;
    nut_reg->op_fcn [0x2a0] = op_set_dec;
    // 0x2e0 = display off (Nut, Voyager)
    // 0x320 = display toggle (Nut, Voyager)
    nut_reg->op_fcn [0x360] = op_return_if_carry;
    nut_reg->op_fcn [0x3a0] = op_return_if_no_carry;
    nut_reg->op_fcn [0x3e0] = op_return;

    // 0x030 = display blink (Voyager)
    // 0x030 = ROMBLK (Hepax)
    nut_reg->op_fcn [0x070] = op_c_to_n;
    nut_reg->op_fcn [0x0b0] = op_n_to_c;
    nut_reg->op_fcn [0x0f0] = op_c_exch_n;
    nut_reg->op_fcn [0x130] = op_ldi;
    nut_reg->op_fcn [0x170] = op_push_c;
    nut_reg->op_fcn [0x1b0] = op_pop_c;
    // 0x1f0 = WPTOG (Hepax)
    nut_reg->op_fcn [0x230] = op_keys_to_rom_addr;
    nut_reg->op_fcn [0x270] = op_c_to_dadd;
    // nut_reg->op_fcn [0x2b0] = op_clear_regs;
    nut_reg->op_fcn [0x2f0] = op_c_to_data;
    nut_reg->op_fcn [0x330] = op_rom_to_c;
    nut_reg->op_fcn [0x370] = op_or;
    nut_reg->op_fcn [0x3b0] = op_and;
    nut_reg->op_fcn [0x3f0] = op_c_to_pfad;

}

bool CHPNUT::nut_execute_cycle (cpu_t *nut_reg)
{
    int opcode;
    rom_addr_t prev_pc;

    nut_kbd_scanner_cycle (nut_reg);
    //voyager_display_cycle(nut_reg);

    if (! nut_reg->awake)
        return (false);

    if (nut_reg->inst_state == cxisa)
        prev_pc = nut_reg->cxisa_addr;
    else
        prev_pc = nut_reg->pc;

    opcode = nut_get_ucode (nut_reg, prev_pc);

//    qWarning()<<QString("EXEC [%1] = %2").arg(prev_pc,4,16,QChar('0')).arg(opcode,3,16,QChar('0'));
    nut_reg->prev_carry = nut_reg->carry;
    nut_reg->carry = 0;

    switch (nut_reg->inst_state)
    {
        case norm:
            nut_reg->pc++;
            (* nut_reg->op_fcn [opcode]) (nut_reg, opcode);
            break;
        case long_branch:
            nut_reg->pc++;
            op_long_branch_word_2 (nut_reg, opcode);
            break;
        case cxisa:
            op_rom_to_c_cycle_2 (nut_reg, opcode);
            break;
        case ldi:
            nut_reg->pc++;
            op_ldi_cycle_2 (nut_reg, opcode);
            break;
        //case selprf:
        //	nut_reg->pc++;
        //	op_smart_periph (nut_reg, opcode);
        //	break;
        default:
            printf ("nut: bad inst_state %d!\n", nut_reg->inst_state);
            nut_reg->inst_state = norm;
            break;
    }

    return (true);
}


bool CHPNUT::nut_execute_instruction (cpu_t *nut_reg)
{
    do
    {
#if 1
        (void) nut_execute_cycle (nut_reg);
#else
        if (! nut_execute_cycle (nut_reg))
            return false;
#endif
    }
    while (nut_reg->inst_state != norm);
    return true;
}

void CHPNUT::nut_press_key (cpu_t *nut_reg, int keycode)
{
#ifdef KEYBOARD_DEBUG
    printf ("key %o press, addr %04x, state %s\n", keycode, nut_reg->prev_pc, kbd_state_name [nut_reg->kb_state]);
#endif

#if 0
    if ((! nut_reg->awake) && (! nut_reg->display_enable) && (keycode != 0x18))
        return;
#endif
    nut_reg->key_buf = keycode;
    nut_reg->key_down = true;
    if (nut_reg->kb_state == KB_IDLE)
        nut_reg->kb_state = KB_PRESSED;
#ifdef SLEEP_DEBUG
    if (! nut_reg->awake)
        printf ("waking up!\n");
#endif
    nut_reg->awake = true;
    //voyager_display_wake(nut_reg);
}

void CHPNUT::nut_release_key (cpu_t *nut_reg)
{
#ifdef KEYBOARD_DEBUG
    printf ("key release, addr %04x, state %s\n", nut_reg->prev_pc, kbd_state_name [nut_reg->kb_state]);
#endif

    nut_reg->key_down = false;
    if (nut_reg->kb_state == KB_PRESSED)
        nut_reg->kb_state = KB_RELEASED;
}

void CHPNUT::nut_set_ext_flag (cpu_t *nut_reg, int flag, bool state)
{
    ;  // not yet implemented
}






void CHPNUT::nut_reset (cpu_t *nut_reg)
{
    int i;

    for (i = 0; i < WSIZE; i++)
    {
        nut_reg->a [i] = 0;
        nut_reg->b [i] = 0;
        nut_reg->c [i] = 0;
        nut_reg->m [i] = 0;
        nut_reg->n [i] = 0;
    }

    for (i = 0; i < SSIZE; i++)
        //nut_reg->s [i] =0;
        BIT_CLEAR(nut_reg->s, i);

    nut_reg->p = 0;
    nut_reg->q = 0;
    nut_reg->q_sel = false;

    //for (i = 0; i < MAX_PAGE; i++)
    //	nut_reg->active_bank [i] = 0;

    /* wake from deep sleep */
    nut_reg->awake = true;
    nut_reg->pc = 0;
    nut_reg->inst_state = norm;
    nut_reg->carry = 1;

    nut_reg->kb_state = KB_IDLE;
}


void CHPNUT::nut_clear_memory (cpu_t *nut_reg)
{
    int addr;

    for (addr = 0; addr < nut_reg->max_ram; addr++)
        if (nut_reg->ram_exists [addr])
            reg_zero ((digit_t *)& nut_reg->ram [addr], 0, WSIZE - 1);
}


void CHPNUT::nut_new_ram_addr_space (cpu_t *nut_reg, int max_ram)
{
    nut_reg->max_ram = max_ram;
    nut_reg->ram_exists   = (bool *)			alloc (max_ram * sizeof (bool));
    nut_reg->ram          = (reg_t *)			alloc (max_ram * sizeof (reg_t));
}

void CHPNUT::nut_new_rom_addr_space (cpu_t *nut_reg, int rom_size)
{
//    nut_reg->rom          = (rom_word_t *)			alloc (rom_size * sizeof (rom_word_t));
}

void CHPNUT::nut_new_ram (cpu_t *nut_reg, int base_addr, int count)
{
    int i;

    for (i = base_addr; i < (base_addr + count); i++) {
        nut_reg->ram_exists [i] = true;
    }
}

cpu_t * CHPNUT::nut_new_processor (int ram_size)
{
    cpu_t *nut_reg;

    nut_reg = (cpu_t *)alloc (sizeof (cpu_t));
    nut_reg->pPC = pPC;
    nut_init_ops (nut_reg);

    nut_new_ram_addr_space (nut_reg, 256);
//#ifdef NONPAREIL_15C
    nut_new_rom_addr_space(nut_reg, 14336);
//#else
//    nut_new_rom_addr_space(nut_reg, 6144);
//#endif

    nut_new_ram (nut_reg, 0x000, 11);
    ram_size -= 8;

    //nut_new_ram (nut_reg, 0x008, 3);  // I/O registers

    if (ram_size > 40)
    {
        // 15-C
        //printf("here we are\n");
        //15-C needs adresses 11-15?
        nut_new_ram (nut_reg, 0x00b, 6);
        nut_new_ram (nut_reg, 0x010, 11);
        ram_size -= 8;

        //nut_new_ram (nut_reg, 0x018, 3);  // I/O registers
    }

    nut_new_ram (nut_reg, 0x100 - ram_size, ram_size);


    //int i;

    /*for(i=0;i<256;i++) {
        //if(nut_reg->ram_exists [i]);
        printf("ram@%d=%d, read=%lx write=%lx\n", i, nut_reg->ram_exists [i], (unsigned long)nut_reg->ram_read_fn[i],(unsigned long)nut_reg->ram_write_fn [i]);
    }*/
//    voyager_display_init (nut_reg);

    nut_reset (nut_reg);
//    voyager_display_reset (nut_reg);

    //nut_reg->rom = rom_data;

    return nut_reg;
}


void CHPNUT::reg_zero (digit_t *dest, int first, int last)
{
    int i;
    for (i = first; i <= last; i++)
        dest [i] = 0;
}


void CHPNUT::reg_copy (digit_t *dest, const digit_t *src, int first, int last)
{
    int i;
    for (i = first; i <= last; i++)
        dest [i] = src [i];
}


void CHPNUT::reg_exch (digit_t *dest, digit_t *src, int first, int last)
{
    int i, t;
    for (i = first; i <= last; i++)
    {
        t = dest [i];
        dest [i] = src [i];
        src [i] = t;
    }
}


digit_t CHPNUT::do_add (digit_t x, digit_t y, bool *carry, quint8 base)
{
    int res;

    res = x + y + *carry;
    if (res >= base)
    {
        res -= base;
        *carry = 1;
    }
    else
        *carry = 0;
    return (res);
}


digit_t CHPNUT::do_sub (digit_t x, digit_t y, bool *carry, quint8 base)
{
    int res;

    res = (x - y) - *carry;
    if (res < 0)
    {
        res += base;
        *carry = 1;
    }
    else
        *carry = 0;
    return (res);
}


void CHPNUT::reg_add (digit_t *dest, const digit_t *src1, const digit_t *src2,
              int first, int last,
              bool *carry, quint8 base)
{
    int i;

    for (i = first; i <= last; i++)
    {
        int s2 = src2 ? src2 [i] : 0;
        dest [i] = do_add (src1 [i], s2, carry, base);
    }
}


void CHPNUT::reg_sub (digit_t *dest, const digit_t *src1, const digit_t *src2,
              int first, int last,
              bool *carry, quint8 base)
{
    int i;

    for (i = first; i <= last; i++)
    {
        int s1 = src1 ? src1 [i] : 0;
        int s2 = src2 ? src2 [i] : 0;
        int d = do_sub (s1, s2, carry, base);
        if (dest)
            dest [i] = d;
    }
}


// $$$ if in decimal mode, do illegal digits get normalized?
void CHPNUT::reg_test_equal    (const digit_t *src1, const digit_t *src2,
                        int first, int last,
                        bool *carry)
{
    int i;

    *carry = true;
    for (i = first; i <= last; i++)
    {
        int s2 = src2 ? src2 [i] : 0;
        (*carry) &= (src1 [i] == s2);
    }
}


// $$$ if in decimal mode, do illegal digits get normalized?
void CHPNUT::reg_test_nonequal (const digit_t *src1, const digit_t *src2,
                        int first, int last,
                        bool *carry)
{
    int i;

    *carry = false;
    for (i = first; i <= last; i++)
    {
        int s2 = src2 ? src2 [i] : 0;
        (*carry) |= (src1 [i] != s2);
    }
}


void CHPNUT::reg_shift_right (digit_t *reg, int first, int last)
{
    int i;

    for (i = first; i <= last; i++)
        reg [i] = (i == last) ? 0 : reg [i+1];
}


void CHPNUT::reg_shift_left (digit_t *reg, int first, int last)
{
    int i;

    for (i = last; i >= first; i--)
        reg [i] = (i == first) ? 0 : reg [i-1];
}



void *CHPNUT::alloc (size_t size)
{
    void *addr;

    addr = malloc (size);
    memset(addr, 0, size);
    return addr;
}

char* CHPNUT::reg2str (char *str, reg_t reg)
{
    static char hexmap[] = "0123456789abcdef";
    int i = WSIZE - 1;
    char *tmp = str;

    do {
        *tmp++ = hexmap[reg[i]];
    } while (i--);
    *tmp = 0;
    return str;
}


void CHPNUT::str2reg(reg_t reg, const char *str)
{
    int cchr;
    int i = WSIZE - 1;

    do {
        cchr = *str++;

        if((cchr >= 'a') && (cchr <= 'f')) // a,b...f
            reg[i] = cchr - 'a' + 10;
        else if((cchr >= 'A') && (cchr <= 'F')) // a,b...f
            reg[i] = cchr - 'A' + 10;
        else if(cchr >= 0x30) //1, 2...
            reg[i] = cchr - '0';
        else {
            printf("warning: wrong code\n");
            reg[i] = 0;
        }
    } while (i--);
}



