#ifndef TC8576P_H
#define TC8576P_H

// UART used in the PC-1600

#include <QFile>

#include "pobject.h"



typedef struct
{
    /* Serial Input data Register */
    BYTE    sir;

    /* Serial Output data Register */
    BYTE    sor;

    /* Parallel input data Register */
    BYTE    pir;

    /* Parallel output data Register */
    BYTE    por;

    /* Parameter Register */
    BYTE    pr[8];
    // 0 Baud Rate Low
    // 1 Baud Rate High
    // 2 DSTB delay/ACK width
    // 3 DSTB width
    // 4 PRIME length
    // 5 Serial mode
    // 6 Parallel mode
    // 7 Prescaler value

    /* Serial Status Register */
    BYTE    ssr;


    /* Serial Command Register */
    BYTE    scr;

    /* Parallel Command Register */
    BYTE    pcr;

    /* Parameter Address Register */
    BYTE    par;

    /* Parallel Status Register */
    BYTE    psr;

    /* Baud rate compute */
    int baudrate;
    int xclk;
    int fsysclk;

    bool IntF;
    bool XBUSY;
    bool BUSY;
    bool PRIME;
    bool P5V;
    bool PE;
    bool SLCT;
    bool FAULT;
    bool IM1;
    bool IM2;

    // SSR
    bool DSR;
    bool RBRK;
    bool FE;
    bool OE;
    bool PERR;
    bool TxE;
    bool RxRDY;
    bool TxRDY;

    // SCR
    bool TxEN;
    bool DTR;
    bool RxEN;
    bool SBRK;
    bool ERS;
    bool RTS;

    bool SO;
    bool TxINTM;
    int  CL;
    bool PEN;
    bool EP;
    bool ERINTM;
    bool RXINTM;


    BYTE	p,q,r,dummy;

        WORD	dp,pc;

        BYTE	c,z;//,IB,FO,CTRL;

} REG;



class CTC8576P:public QObject{

public:
    const char*	GetClassName(){ return("CTC8576P");}
    CpcXXXX		*pPC;

    bool	init(void);						//initialize
    bool	exit(void);						//end
    void	Reset(void);
    bool	step(void);
    bool    instruction(BYTE cmd);
    bool    in(BYTE data);

    BYTE    get_ssr(void);
    BYTE    get_psr(void);

    void	Load_Internal(QFile *file);
    void	save_internal(QFile *file);
    void	Load_Internal(QXmlStreamReader *);
    void	save_internal(QXmlStreamWriter *);

    CTC8576P(CpcXXXX *parent,int clk);
    virtual ~CTC8576P();

    void    set_xclk(int value) { r.xclk = value;}

private:
    REG r;
    /* Serial Input data Register */
//    BYTE    sir;

    /* Serial Output data Register */
//    BYTE    sor;

    /* Parallel input data Register */
//    BYTE    pir;

    /* Parallel output data Register */
//    BYTE    por;

    /* Parameter Register */
//    BYTE    pr[8];
    // 0 Baud Rate Low
    // 1 Baud Rate High
    // 2 DSTB delay/ACK width
    // 3 DSTB width
    // 4 PRIME length
    // 5 Serial mode
    // 6 Parallel mode
    // 7 Prescaler value
    void SPR(void);

    /* Serial Status Register */
//    BYTE    ssr;


    /* Serial Command Register */
//    BYTE    scr;
    void    SCR(BYTE cmd);

    /* Parallel Command Register */
//    BYTE    pcr;
    void    PCR(BYTE cmd);

    /* Parameter Address Register */
//    BYTE    par;

    /* Parallel Status Register */
//    BYTE    psr;

    /* Baud rate compute */
    void baud(qint16 data);
//    int baudrate;
//    int xclk;
//    int fsysclk;
    void prescaler(qint8 data);

//    bool IntF;
//    bool XBUSY;
//    bool BUSY;
//    bool PRIME;
//    bool P5V;
//    bool PE;
//    bool SLCT;
//    bool FAULT;
//    bool IM1;
//    bool IM2;

    // SSR
//    bool DSR;
//    bool RBRK;
//    bool FE;
//    bool OE;
//    bool PERR;
//    bool TxE;
//    bool RxRDY;
//    bool TxRDY;

    // SCR
//    bool TxEN;
//    bool DTR;
//    bool RxEN;
//    bool SBRK;
//    bool ERS;
//    bool RTS;

//    bool SO;
//    bool TxINTM;
//    int  CL;
//    bool PEN;
//    bool EP;
//    bool ERINTM;
//    bool RXINTM;


};

#endif // TC8576P_H
