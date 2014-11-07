#ifndef _DEBUG_H_
#define _DEBUG_H_

#include <QObject>
#include <QMap>

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
class CCPU;

class Cdebug:public QObject
{
    Q_OBJECT
public:
	bool	isdebug, breakf,debugged;			//debug,break point flag
    quint32	breakadr;					//break point address
    char	Buffer[1024];
    quint32	DasmAdr,NextDasmAdr;
	bool init(void);					//initialize
	bool exit(void);					//end
    quint32 DisAsm_File(quint32 adr,FILE *fp);	//disasm 1 line to File

    void header(quint32 adr,int l);
    virtual	quint32	DisAsm_1(quint32 adr) = 0;			//disasm 1 line to Buffer

	DisAsmTbl	*AsmTbl;
    CCPU *pCPU;

    virtual void injectReg(Parser *p) { Q_UNUSED(p) }

    Cdebug(CCPU *parent);
	
	virtual ~Cdebug()
	{
    }

    QMap<quint32,Csymbol*> symbolMap;

    virtual void loadSymbolMap() {}
    virtual char *toSymbol(quint32 adr, int size=4);
private:
    char tmpSymbolLabel[80];
};


#endif
