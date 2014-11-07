#ifndef UPD1007D_H
#define UPD1007D_H
#include "common.h"
#include "debug.h"

enum t_kind {
    NONE,	/* no arguments */
    DROP1,	/* two-byte instructions, no arguments */
    WRIM16,	/* Word Register, 16-bit immediate value */
    IRIM8,	/* Index Register, 8-bit immediate value */
    CCIM16,	/* Condition code, 16-bit immediate value */
    MEMOIM8,	/* (IR), Immediate byte */
    SMEMOIM8,	/* +/-(IR), Immediate byte */
    XREG,
    YREG,
    REGIM8,	/* Rd,Im8 */
    XARY,
    YARY,
    ARYIM6,
    MEMOREG,	/* +/-(IR),Rs */
    REG,	/* Rd */
    OFFSREG,	/* (IR+/-Ri),Rs or (IR+/-nn),Rs */
    REGMEMO,	/* Rd,(IR)+/- */
    REGOFFS,	/* Rd,(IR+/-Ri) or Rd,(IR+/-nn) */
    MEMOARY,	/* +/-(IR),Rsf..Rsl */
    OFFSARY,	/* (IR+/-Ri),Rsf..Rsl or (IR+/-nn),Rsf..Rsl */
    ARY,	/* Rdf..Rdl */
    ARYMTBM,	/* instructions CMPM/MTBM Rdf..Rdl */
    ARYMEMO,	/* Rdf..Rdl,(IR)+/- */
    ARYOFFS,	/* Rdf..Rdl,(IR+/-Ri) or Rdf..Rdl,(IR+/-nn) */
    STREG,	/* status register, data register */
    STIM8,	/* status register, Im8 */
    REGST,	/* data register, status register */
    REGIF,	/* Rd,IF */
    REGKI,	/* Rd,KI */
    CCINDIR,
    DATA1
  };

typedef struct
{
    QString str;
    t_kind kind;
} upd1007_dasm;

class CUPD1007;

class Cdebug_upd1007:public Cdebug{
    Q_OBJECT
public:
    UINT32 DisAsm_1(UINT32 adr);			//disasm 1 line to Buffer

        Cdebug_upd1007(CCPU *parent);
        virtual ~Cdebug_upd1007(){}

        CUPD1007 *pupd1007;

        UINT32 get_dasmflags(UINT8 op);

        static const upd1007_dasm mnem[512];
        static const QString ir_tab[4];
        static const QString wr_tab[8];
        static const QString cc_tab[16];
        static const QString st_tab[8];




        QString CaHexB(BYTE x);
        BYTE Reg1(BYTE x);
        BYTE Reg2(BYTE y);
        BYTE Rl1(BYTE x, BYTE y);
        BYTE Rf2(BYTE x, BYTE y);
        BYTE Im6(BYTE x, BYTE y);
        QString Mnemonic(UINT16 kod);
        QString Arguments(UINT16 kod);
        QString IntToStr(BYTE x);
        QString IntToHex(UINT32 x, BYTE size);
};

#endif // UPD1007D_H
