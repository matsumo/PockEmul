#ifndef _DEBUG_H_
#define _DEBUG_H_

#include "pobject.h"
#include "pcxxxx.h"

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

    virtual void injectReg(Parser *p) { Q_UNUSED(p) }

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

    quint16 Dasm_tms7000(char *buffer, quint16 pc, const UINT8 *oprom, const UINT8 *opram);

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


#endif
