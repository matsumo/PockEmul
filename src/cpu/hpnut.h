#ifndef HPNUT_H
#define HPNUT_H

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

#include "cpu.h"

//
// any changes since 0.77 copyright 2005-2012 Maciej Bartosiak
//

#define WSIZE 14
#define EXPSIZE 3  // two exponent and one exponent sign digit

//typedef digit_t reg_t [WSIZE];


#define SSIZE 14
#define EXT_FLAG_SIZE 14

//#define EF_PRINTER_BUSY     0  // 82143A printer
//#define EF_CARD_READER      1  // 82104A card reader
//#define EF_WAND_DATA_AVAIL  2  // 82153A bar code wand
//#define EF_BLINKY_EDAV      5  // 88242A IR printer module
//#define EF_HPIL_IFCR        6  // 82160A HP-IL module
//#define EF_HPIL_SRQR        7
//#define EF_HPIL_FRAV        8
//#define EF_HPIL_FRNS        9
//#define EF_HPIL_ORAV       10
//#define EF_TIMER           12  // 82182A Time Module (built into 41CX)
//#define EF_SERVICE_REQUEST 13  // shared general service request
                               // Flags 3, 4, and 11 are apparently not used by any standard peripherals


#define STACK_DEPTH 4

//#undef PAGE_SIZE
#define N_PAGE_SIZE 4096
#define MAX_PAGE 16
#define MAX_BANK 4


typedef enum
{
    KB_IDLE,
    KB_PRESSED,
    KB_RELEASED,
    KB_WAIT_CHK,
    KB_WAIT_CYC,
    KB_STATE_MAX  // must be last
} keyboard_state_t;


enum
{
    //event_periph_select = first_arch_event,
    event_ram_select
};


typedef quint16 rom_addr_t;
typedef quint8 digit_t;

typedef digit_t  reg_t [WSIZE];
typedef quint32 addr_t;
typedef quint16 rom_word_t;

typedef enum
{
    norm,
    long_branch,
    cxisa,
    ldi,
    selprf         // "smart" peripheral selected (NPIC, PIL)
} inst_state_t;


struct nut_reg_t;

typedef void ram_access_fn_t (struct nut_reg_t *nut_reg, int addr, reg_t *reg);

typedef struct nut_reg_t
{
    reg_t a;
    reg_t b;
    reg_t c;
    reg_t m;
    reg_t n;
    digit_t g [2];

    digit_t p;
    digit_t q;
    bool q_sel;  // true if q is the selected pointer, false for p

    quint8 fo;  /* flag output regiters, 8 bits, used to drive bender */

    bool decimal;  // true for arithmetic radix 10, false for 16

    bool carry;       // carry being generated in current instruction
    bool prev_carry;  // carry that resulted from previous instruction

    int prev_tef_last;  // last digit of field of previous arith. instruction
    // used to simulate bug in logical or and and

    //bool s [SSIZE];
    quint16 s;

    rom_addr_t pc;
    //rom_addr_t prev_pc;

    rom_addr_t stack [STACK_DEPTH];

    rom_addr_t cxisa_addr;

    inst_state_t inst_state;

    rom_word_t first_word;   /* long branch: remember first word */
    bool long_branch_carry;  /* and carry */

    bool key_down;      /* true while a key is down */
    keyboard_state_t kb_state;
    int kb_debounce_cycle_counter;
    int key_buf;        /* most recently pressed key */

    bool awake;

    void (* op_fcn [1024])(struct nut_reg_t *nut_reg, int opcode);

    rom_word_t *rom;

    // RAM:
    quint16 ram_addr;  // selected RAM address
    bool *ram_exists;
    reg_t *ram;
    //ram_access_fn_t **ram_read_fn;
    //ram_access_fn_t **ram_write_fn;

    quint16 ext_flag;

    //quint8 selprf;  // selected "smart peripheral" number

//voyager_display_reg_t *display_chip;

    //int display_digits;
    //segment_bitmap_t display_segments [MAX_DIGIT_POSITION];

    quint16   max_ram;
    CpcXXXX *pPC;
} cpu_t;

class CHPNUT : public CCPU
{
public:

    CHPNUT(CPObject *parent, int ram_size, QString romFn);
    virtual ~CHPNUT();

    virtual	bool	init(void);						//initialize
    virtual	bool	exit(void);						//end
    virtual void	step(void);
    virtual void	Reset(void);


    virtual	void	Load_Internal(QXmlStreamReader *);
    virtual	void	save_internal(QXmlStreamWriter *);

    virtual	bool	Get_Xin(void){ return true;}
    virtual	void	Set_Xin(bool){}
    virtual	bool	Get_Xout(void){ return true;}
    virtual	void	Set_Xout(bool){}

    virtual	UINT32	get_PC(void);					//get Program Counter
    virtual void	Regs_Info(UINT8 Type);

    cpu_t * reg;
    cpu_t * nut_new_processor (int ram_size);
    bool nut_read_object_file (cpu_t *nut_reg, QString fn);
    void nut_press_key (cpu_t *nut_reg, int keycode);
    void nut_release_key (cpu_t *nut_reg);
    bool nut_execute_instruction (cpu_t *nut_reg);

    char* reg2str (char *str, reg_t reg);
    void str2reg(reg_t reg, const char *str);
private:
    static quint8 arithmetic_base(cpu_t *nut_reg);
    static quint8 *pt(cpu_t *nut_reg);
    static void bad_op(cpu_t *nut_reg, int opcode);
    static void op_arith(cpu_t *nut_reg, int opcode);
    static rom_addr_t pop(cpu_t *nut_reg);
    static void push(cpu_t *nut_reg, rom_addr_t a);
    static void op_return(cpu_t *nut_reg, int opcode);
    static void op_return_if_carry(cpu_t *nut_reg, int opcode);
    static void op_return_if_no_carry(cpu_t *nut_reg, int opcode);
    static void op_pop(cpu_t *nut_reg, int opcode);
    static void op_pop_c(cpu_t *nut_reg, int opcode);
    static void op_push_c(cpu_t *nut_reg, int opcode);
    static void op_short_branch(cpu_t *nut_reg, int opcode);
    static void op_long_branch(cpu_t *nut_reg, int opcode);
    static void op_long_branch_word_2(cpu_t *nut_reg, int opcode);
    static void op_goto_c(cpu_t *nut_reg, int opcode);
    static void op_enbank(cpu_t *nut_reg, int opcode);
    static void op_c_to_m(cpu_t *nut_reg, int opcode);
    static void op_m_to_c(cpu_t *nut_reg, int opcode);
    static void op_c_exch_m(cpu_t *nut_reg, int opcode);
    static void op_c_to_n(cpu_t *nut_reg, int opcode);
    static void op_n_to_c(cpu_t *nut_reg, int opcode);
    static void op_c_exch_n(cpu_t *nut_reg, int opcode);
    static void nut_ram_read_zero(cpu_t *nut_reg, int addr, reg_t *reg);
    static void nut_ram_write_ignore(cpu_t *nut_reg, int addr, reg_t *reg);
    static void op_c_to_dadd(cpu_t *nut_reg, int opcode);
    static void op_c_to_pfad(cpu_t *nut_reg, int opcode);
    static void op_read_reg_n(cpu_t *nut_reg, int opcode);
    static void op_write_reg_n(cpu_t *nut_reg, int opcode);
    static void op_c_to_data(cpu_t *nut_reg, int opcode);
    static void op_test_ext_flag(cpu_t *nut_reg, int opcode);
    static void op_set_s(cpu_t *nut_reg, int opcode);
    static void op_clr_s(cpu_t *nut_reg, int opcode);
    static void op_test_s(cpu_t *nut_reg, int opcode);
    static int get_s_bits(cpu_t *nut_reg, int first, int count);
    static void set_s_bits(cpu_t *nut_reg, int first, int count, int a);
    static void op_clear_all_s(cpu_t *nut_reg, int opcode);
    static void op_c_to_s(cpu_t *nut_reg, int opcode);
    static void op_s_to_c(cpu_t *nut_reg, int opcode);
    static void op_c_exch_s(cpu_t *nut_reg, int opcode);
    static void op_sb_to_f(cpu_t *nut_reg, int opcode);
    static void op_f_to_sb(cpu_t *nut_reg, int opcode);
    static void op_f_exch_sb(cpu_t *nut_reg, int opcode);
    static void op_dec_pt(cpu_t *nut_reg, int opcode);
    static void op_inc_pt(cpu_t *nut_reg, int opcode);
    static void op_set_pt(cpu_t *nut_reg, int opcode);
    static void op_test_pt(cpu_t *nut_reg, int opcode);
    static void op_sel_p(cpu_t *nut_reg, int opcode);
    static void op_sel_q(cpu_t *nut_reg, int opcode);
    static void op_test_pq(cpu_t *nut_reg, int opcode);
    static void op_lc(cpu_t *nut_reg, int opcode);
    static void op_c_to_g(cpu_t *nut_reg, int opcode);
    static void op_g_to_c(cpu_t *nut_reg, int opcode);
    static void op_c_exch_g(cpu_t *nut_reg, int opcode);
    static void op_keys_to_rom_addr(cpu_t *nut_reg, int opcode);
    static void op_keys_to_c(cpu_t *nut_reg, int opcode);
    static void op_test_kb(cpu_t *nut_reg, int opcode);
    static void op_reset_kb(cpu_t *nut_reg, int opcode);
    static void op_nop(cpu_t *nut_reg, int opcode);
    static void op_set_hex(cpu_t *nut_reg, int opcode);
    static void op_set_dec(cpu_t *nut_reg, int opcode);
    static void op_rom_to_c(cpu_t *nut_reg, int opcode);
    static void op_rom_to_c_cycle_2(cpu_t *nut_reg, int opcode);
    static void op_clear_abc(cpu_t *nut_reg, int opcode);
    static void op_ldi(cpu_t *nut_reg, int opcode);
    static void op_ldi_cycle_2(cpu_t *nut_reg, int opcode);
    static void op_or(cpu_t *nut_reg, int opcode);
    static void op_and(cpu_t *nut_reg, int opcode);
    static void op_rcr(cpu_t *nut_reg, int opcode);
    static void op_lld(cpu_t *nut_reg, int opcode);
    static void nut_init_ops(cpu_t *nut_reg);
    static bool nut_execute_cycle(cpu_t *nut_reg);
    static void reg_zero(digit_t *dest, int first, int last);
    static void reg_copy(digit_t *dest, const digit_t *src, int first, int last);
    static void reg_exch(digit_t *dest, digit_t *src, int first, int last);
    static digit_t do_add(digit_t x, digit_t y, bool *carry, quint8 base);
    static digit_t do_sub(digit_t x, digit_t y, bool *carry, quint8 base);
    static void reg_add(digit_t *dest, const digit_t *src1, const digit_t *src2, int first, int last, bool *carry, quint8 base);
    static void reg_sub(digit_t *dest, const digit_t *src1, const digit_t *src2, int first, int last, bool *carry, quint8 base);
    static void reg_test_equal(const digit_t *src1, const digit_t *src2, int first, int last, bool *carry);
    static void reg_test_nonequal(const digit_t *src1, const digit_t *src2, int first, int last, bool *carry);
    static void reg_shift_right(digit_t *reg, int first, int last);
    static void reg_shift_left(digit_t *reg, int first, int last);
    static quint64 bcd_reg_to_binary(digit_t *reg, int digits);
    static void binary_to_bcd_reg(quint64 val, digit_t *reg, int digits);
    static void nut_kbd_scanner_cycle(cpu_t *nut_reg);
    static void nut_kbd_scanner_sleep(cpu_t *nut_reg);
    static void op_powoff(cpu_t *nut_reg, int opcode);
    static void nut_set_ext_flag(cpu_t *nut_reg, int flag, bool state);
    static void nut_reset(cpu_t *nut_reg);
    static void nut_clear_memory(cpu_t *nut_reg);
    static void nut_new_ram_addr_space(cpu_t *nut_reg, int max_ram);
    static void nut_new_rom_addr_space(cpu_t *nut_reg, int rom_size);
    static void nut_new_ram(cpu_t *nut_reg, int base_addr, int count);
    static bool nut_parse_object_line(QByteArray buf, int *addr, rom_word_t *opcode);
    static bool parse_hex(char *hex, int digits, int *val);
    static void *alloc(size_t size);
    static void trim_trailing_whitespace(char *s);
    static size_t fread_bytes(FILE *stream, void *ptr, size_t byte_count, bool *eof, bool *error);

    static rom_word_t nut_get_ucode(cpu_t *nut_reg, rom_addr_t addr);
};



#endif // HPNUT_H
