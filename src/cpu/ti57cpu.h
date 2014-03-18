#ifndef TI57CPU_H
#define TI57CPU_H

/*
 * this code is based on TI-57E emulator
 * http://www.hrastprogrammer.com/ti57e/
 */


#include "cpu.h"

#define MAXROM 0x07FF

typedef UINT8 TRegister[16];

typedef struct {
    UINT8 BASE,COND,MF,KEYR,R5,RAB;
    UINT16 OP,PC;
    bool Power,Run,Trace,EMPTY,SIGMA;
//    Processor: TProcessor;
    TRegister RA,RB,RC,RD;
    TRegister RX[8],RY[8];
    int Speed,Timer;
    WORD ST[3];
} TI57regs;

class Cti57cpu:public CCPU {

public:


    Cti57cpu(CPObject *parent);
    virtual ~Cti57cpu();

    virtual	bool	init(void);						//initialize
    virtual	bool	exit(void);						//end
    virtual void	step(void);
    virtual void	Reset(void);

    virtual	void	Load_Internal(QXmlStreamReader *xmlIn);
    virtual	void	save_internal(QXmlStreamWriter *xmlOut);

    virtual	bool	Get_Xin(void) {return true;}
    virtual	void	Set_Xin(bool) {}
    virtual	bool	Get_Xout(void) {return true;}
    virtual	void	Set_Xout(bool) {}

    virtual	UINT32	get_PC(void) { return r->PC;}					//get Program Counter
    virtual void	Regs_Info(UINT8);


    QString IntToHex(int val, int nb);

    TI57regs *r;
private:

//    Tracer: TextFile;

    void BranchOP();
    void CallOP();
    void FlagOP();
    void Error(QString msg);
    void MaskOP();
    void MiscOP();
    void Emulate();
    void Execute();
};


#endif // TI57CPU_H
