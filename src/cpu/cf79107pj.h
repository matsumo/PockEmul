#ifndef CF79107PJ_H
#define CF79107PJ_H

#include <QObject>
#include "common.h"
class CpcXXXX;
class QXmlStreamReader;
class QXmlStreamWriter;



class CCF79107PJ:public QObject{

public:

    const char*	GetClassName(){ return("CCF79107PJ");}
    CpcXXXX		*pPC;

    bool	init(void);						//initialize
    bool	exit(void);						//end
    void	Reset(void);
    bool	step(void);
    bool    instruction1(UINT8 cmd);
    bool    instruction2(UINT8 cmd);
    UINT8    get_status(void);

    void	Load_Internal(QXmlStreamReader *);
    void	save_internal(QXmlStreamWriter *);

    CCF79107PJ(CpcXXXX *parent);
    virtual ~CCF79107PJ();

    UINT16 make_bcd_sub(UINT8 arg1, UINT8 arg2);
    UINT16 make_bcd_add(UINT8 arg1, UINT8 arg2);
    void cmd_add_mantisse(void);
    void dumpXYW();
    void cmd_add_exp(void);
    void cmd_inc_exp(void);
    void cmd_sub_exp(void);
    void cmd_dec_exp(void);
    void cmd_0e();
    void cmd_sub_mantisseXY(void);
    void cmd_shiftL_mantisse(void);
    void cmd_shiftR_mantisse(void);
    void Read_Reg(UINT16 adr);
    void Write_Reg(UINT16 adr);
    void Exch_Reg(UINT16 adr);
    void cmd_sub_mantisseYX(void);
    void push(UINT16 adr);
    void pop(UINT16 adr);
private:


    UINT8 BCDret;
    bool BCDz,BCDc,BCDaf;
    UINT8 reg[0x10];

};


#endif // CF79107PJ_H

#if 0

/***************************************/
/* ??:comet.h ????              */
/* ??:comet?????               */
/* ??:????                      */
/* ??:1.0 ?                        */
/* ??:???                        */
/* ??:chai2010@2002.cug.edu.cn      */
/* ??:2005-5-3                      */
/***************************************/

#ifndef COMET_H
#define COMET_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ???????? */

#define		NELEMS(x)	(sizeof(x) / sizeof((x)[0]))

#define		MEMSIZE		0x10000		/* ???? */
#define		pc_max		0xFC00		/* ???? */
#define		sp_start	0xFC00		/* ???	*/

#define		IO_ADDR		0xFD10		/* IO??	*/
#define		IO_FLAG		0xFD11		/* IO??	*/

#define		IO_FIO		0x0100		/* ??		*/
#define		IO_TYPE		0x1C00		/* ??		*/
#define		IO_MAX		0x00FF		/* ????	*/
#define		IO_ERROR	0x0200		/* ??		*/

#define		IO_IN		0x0000		/* ??		*/
#define		IO_OUT		0x0100		/* ??		*/
#define		IO_CHR		0x0400		/* ??		*/
#define		IO_OCT		0x0800		/* ???	*/
#define		IO_DEC		0x0C00		/* ???	*/
#define		IO_HEX		0x1000		/* ????	*/

/* comet??????? */

#define		off_t		unsigned short

/* comet???? */

typedef enum {
    HALT, LD, ST, LEA,
    ADD, SUB, MUL, DIV, MOD,
    AND, OR, EOR,
    SLA, SRA, SLL, SRL,
    CPA, CPL,
    JMP, JPZ, JMI, JNZ, JZE,
    PUSH, POP, CALL, RET,
} OpType;

/* comet???? */

struct { OpType op; char *str; int len; } opTab[] = {
    {HALT, "HALT", 1},

    {LD, "LD", 2}, {ST, "ST", 2}, {LEA, "LEA", 2},

    {ADD, "ADD", 2}, {SUB, "SUB", 2},
    {MUL, "MUL", 2}, {DIV, "DIV", 2}, {MOD, "MOD", 2},
    {AND, "AND", 2}, {OR, "OR", 2}, {EOR, "EOR", 2},

    {SLA, "SLA", 2}, {SRA, "SRA", 2}, {SLL, "SLL", 2}, {SRL, "SRL", 2},

    {CPA, "CPA", 2}, {CPL, "CPL", 2},

    {JMP, "JMP", 2},
    {JPZ, "JPZ", 2}, {JMI, "JMI", 2}, {JNZ, "JNZ", 2}, {JZE, "JZE", 2},

    {PUSH, "PUSH", 2}, {POP, "POP", 1},
    {CALL, "CALL", 2}, {RET, "RET", 1}
};

/* ???? */

typedef enum {
    HELP, GO, STEP, JUMP, REGS, IMEM, DMEM,
    ALTER, TRACE, PRINT, CLEAR, QUIT } DbType;

/* ???? */

struct { DbType db; char *s1, *s2; } dbTab[] = {
    {HELP , "help" , "h"},
    {GO   , "go"   , "g"},
    {STEP , "step" , "s"},
    {JUMP , "jump" , "j"},
    {REGS , "regs" , "r"},
    {IMEM , "iMem" , "i"},
    {DMEM , "dMem" , "d"},
    {ALTER, "alter", "a"},
    {TRACE, "trace", "t"},
    {PRINT, "print", "p"},
    {CLEAR, "clear", "c"},
    {QUIT , "quit" , "q"}
};

/* comet????? */

struct comet {
    off_t pc;
    short fr;
    short gr[5];
    short mem[MEMSIZE];
} cmt;

/* ???? */

char pgmName[32];

/* ?????? */

FILE * source;

/* ???? */

int debug = 0;

#endif

#endif
