#ifndef _DEBUG_H_
#define _DEBUG_H_

#include "pobject.h"
#include "pcxxxx.h"
#include "i80x86.h"
#include "i8085.h"

typedef	struct{
	char	len;
    const char	*nim;
    const char	*descr;
}	DisAsmTbl;

extern 	DisAsmTbl	AsmTbl_sc61860[];
extern 	DisAsmTbl	AsmTbl_lh5801[];


class Csymbol {
public:
    Csymbol(QString name) {
        this->name = name;
    }

    QString toLbl();
    QString name;

};

class Parser;

class Cdebug:public CPObject
{
    Q_OBJECT
public:
	bool	isdebug, breakf,debugged;			//debug,break point flag
	UINT32	breakadr;					//break point address
    char	Buffer[1024];
	UINT32	DasmAdr,NextDasmAdr;
	bool init(void);					//initialize
	bool exit(void);					//end
	UINT32 DisAsm_File(UINT32 adr,FILE *fp);	//disasm 1 line to File

    void header(UINT32 adr,int l);
	virtual	UINT32	DisAsm_1(UINT32 adr) = 0;			//disasm 1 line to Buffer

	DisAsmTbl	*AsmTbl;

    virtual void injectReg(Parser *p) {}

	Cdebug(CPObject *parent)	: CPObject(parent)
	{							//[constructor]
		breakf		= 0;
		breakadr	= 0;
		debugged	= 0;	//break point(0:disable, 1:enable)
		isdebug		= 0;			//debug?(0:none, 1:debugging)

        loadSymbolMap();
    }
	
	virtual ~Cdebug()
	{
    }

    QMap<UINT32,Csymbol*> symbolMap;

    virtual void loadSymbolMap() {}
    virtual char *toSymbol(quint32 adr, int size=4);
private:
    char tmpSymbolLabel[80];
};

class Cdebug_sc61860:public Cdebug{
    Q_OBJECT
public:
	UINT32 DisAsm_1(UINT32 adr);			//disasm 1 line to Buffer
    virtual void injectReg(Parser *p);

		Cdebug_sc61860(CPObject *parent)	: Cdebug(parent)
		{
			AsmTbl = AsmTbl_sc61860;
		}
		virtual ~Cdebug_sc61860(){}

};

class Cdebug_sc62015:public Cdebug{
    Q_OBJECT
public:
    UINT32 DisAsm_1(UINT32 adr);			//disasm 1 line to Buffer
    virtual void injectReg(Parser *p);

        Cdebug_sc62015(CPObject *parent)	: Cdebug(parent)
        {
            AsmTbl = AsmTbl_sc61860;
        }
        virtual ~Cdebug_sc62015(){}

};

class Cdebug_lh5801:public Cdebug{
    Q_OBJECT
public:
	UINT32 DisAsm_1(UINT32 adr);			//disasm 1 line to Buffer
    virtual void loadSymbolMap();

    virtual void injectReg(Parser *p);
    Cdebug_lh5801(CPObject *parent);


};

class Cdebug_upd7810:public Cdebug{
    Q_OBJECT
public:
    struct dasm_s {
        UINT8 token;
        const void *args;
    };

    UINT32 DisAsm_1(UINT32 adr);			//disasm 1 line to Buffer

        Cdebug_upd7810(CPObject *parent)	: Cdebug(parent)
        {
            AsmTbl = AsmTbl_sc61860;
        }

        quint16 Dasm_upd7810(char *buffer, quint16 pc, const dasm_s *dasmXX, const UINT8 *oprom, const UINT8 *opram, int is_7810);
};

class Cdebug_tms7000:public Cdebug{
    Q_OBJECT
public:

    UINT32 DisAsm_1(UINT32 adr);			//disasm 1 line to Buffer

    Cdebug_tms7000(CPObject *parent)	: Cdebug(parent)
    {
    }

};

class Cdebug_z80:public Cdebug{
    Q_OBJECT
public:
    UINT32 DisAsm_1(UINT32 adr);			//disasm 1 line to Buffer

        Cdebug_z80(CPObject *parent)	: Cdebug(parent)
        {
            AsmTbl = AsmTbl_sc61860;
        }

};
#if 1
class Cdebug_i80x86:public Cdebug{
    Q_OBJECT
public:
    UINT32 DisAsm_1(UINT32 adr);			//disasm 1 line to Buffer

        Cdebug_i80x86(CPObject *parent)	: Cdebug(parent)
        {
            AsmTbl = AsmTbl_sc61860;
            i80x86 = (Ci80x86*)(pPC->pCPU);
        }
        Ci80x86 *i80x86;

        int i86disasm(char *buf, const I86stat *i86, uint16 seg, uint16 off);
        char *s8(int8 val);
        char *s16(int16 val);
        void getsegoff(const I86stat *i86, uint8 rm, uint16 *seg, uint16 *off);
        char *rm8val(const I86stat *i86, uint8 rm);
        char *rm16val(const I86stat *i86, uint8 rm);
        char *i86regs(char *buf, const I86stat *i86);
};
#endif

class Cdebug_i8085:public Cdebug{
    Q_OBJECT
public:
    UINT32 DisAsm_1(UINT32 adr);			//disasm 1 line to Buffer

        Cdebug_i8085(CPObject *parent)	: Cdebug(parent)
        {
            AsmTbl = AsmTbl_sc61860;
            i8085 = (Ci8085*)(pPC->pCPU);
        }
        Ci8085 *i8085;

        char *i85regs(char *buf, const I85stat *i85);

        static const char   *Op_Code[256];
        static char         Op_Code_Size[256];
};

typedef quint32 UINT32;
typedef struct
{
    const char *str;
    UINT8		arg1;
    UINT8		arg2;
    bool		optjr;
} hd61700_dasm;

class Cdebug_hd61700:public Cdebug{
    Q_OBJECT
public:
    UINT32 DisAsm_1(UINT32 adr);			//disasm 1 line to Buffer

        Cdebug_hd61700(CPObject *parent)	: Cdebug(parent)
        {
            AsmTbl = AsmTbl_sc61860;
        }
        virtual ~Cdebug_hd61700(){}

        UINT8 getMem(int);
        UINT32 get_dasmflags(UINT8 op);
        int dasm_arg(char *buffer, UINT8 op, UINT16 pc, int arg, const UINT8 *oprom, int &pos);
        int dasm_im8(char *buffer, UINT16 pc, int arg, int arg1, const UINT8 *oprom, int &pos);
        int dasm_im8(char *buffer, UINT16 pc, int arg, const UINT8 *oprom, int &pos, int type);

        static const hd61700_dasm hd61700_ops[256];
        static const char *const reg_5b[4];
        static const char *const reg_8b[8];
        static const char *const reg_16b[8];
        static const char *const jp_cond[8];


};

#include "ti57cpu.h"

class Cdebug_m6502:public Cdebug{
    Q_OBJECT
public:
    UINT32 DisAsm_1(UINT32 adr);			//disasm 1 line to Buffer

        Cdebug_m6502(CPObject *parent)	: Cdebug(parent)
        {
        }
        virtual ~Cdebug_m6502(){}

        int DasmOpe(char *S,BYTE *A,unsigned long PC);
};

class Cdebug_ti57cpu:public Cdebug{
    Q_OBJECT
public:
    UINT32 DisAsm_1(UINT32 adr);			//disasm 1 line to Buffer

        Cdebug_ti57cpu(CPObject *parent)	: Cdebug(parent)
        {
        }
        virtual ~Cdebug_ti57cpu(){}


        static QString IntToHex(int val, int nb = 3);
        QString BranchOP(TI57regs *r);
        QString CallOP(TI57regs *r);
        QString FlagOP(TI57regs *r);
        QString MaskOP(TI57regs *r);
        QString MiscOP(TI57regs *r);
        QString Decode(TI57regs *r);
        static QString Reg(TRegister R);
        void Analyze(TI57regs *r);
        void Disassemble(TI57regs *r);
        QString Debugging(TI57regs *r);
        QString Tracing(TI57regs *r);
};

class Cdebug_hp41cpu:public Cdebug{
    Q_OBJECT
public:
    UINT32 DisAsm_1(UINT32 adr);			//disasm 1 line to Buffer

        Cdebug_hp41cpu(CPObject *parent)	: Cdebug(parent)
        {
        }
        virtual ~Cdebug_hp41cpu(){}
};
#endif
