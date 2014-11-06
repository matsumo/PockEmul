#ifndef _DEBUG_H_
#define _DEBUG_H_

#include "pobject.h"
#include "pcxxxx.h"

typedef	struct{
	char	len;
    const char	*nim;
    const char	*descr;
}	DisAsmTbl;

class Csymbol {
public:
    Csymbol(QString name) {
        this->name = name;
    }

    QString toLbl();
    QString name;

};

class Parser;

class Cdebug:public QObject
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
    CCPU *pCPU;

    virtual void injectReg(Parser *p) { Q_UNUSED(p) }

    Cdebug(CCPU *parent);
	
	virtual ~Cdebug()
	{
    }

    QMap<UINT32,Csymbol*> symbolMap;

    virtual void loadSymbolMap() {}
    virtual char *toSymbol(quint32 adr, int size=4);
private:
    char tmpSymbolLabel[80];
};


#endif
