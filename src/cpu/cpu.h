#ifndef CPU_H
#define CPU_H

#include <QtCore/QFile>
#include "common.h"

class CPObject;
class CpcXXXX;
class Cdebug;
class QXmlStreamReader;
class QXmlStreamWriter;
class CregCPU;


#define		MASK_4		0xf				/*  4bit data mask */
#define		MASK_8		0xff			/*  8bit data mask */
#define		MASK_16		0xffff			/* 16bit data mask */
#define		MASK_20		0xfffff			/* 20bit data mask */
#define		MASK_24		0xffffff		/* 24bit data mask */
#define		SIZE_8		1				/*  8bit data size(by byte) */
#define		SIZE_16		2				/* 16bit data size(by byte) */
#define		SIZE_20		3				/* 20bit data size(by byte) */
#define		SIZE_24		SIZE_20+1		/* 24bit dummy data size */


typedef union {
#ifdef POCKEMUL_BIG_ENDIAN
    struct {
        quint8 h3, h2, h, l;
    } b;
    struct {
        qint8 h3, h2, h, l;
    } sb;
    struct {
        quint16 h, l;
    } w;
    struct {
        qint16 h, l;
    } sw;
#else
    struct {
        quint8 l, h, h2, h3;
    } b;
    struct {
        quint16 l, h;
    } w;
    struct {
        qint8 l, h, h2, h3;
    } sb;
    struct {
        qint16 l, h;
    } sw;
#endif
    quint32 d;
    qint32 sd;
} DPAIR;

// I/O line states
enum line_state
{
    CLEAR_LINE = 0,				// clear (a fired or held) line
    ASSERT_LINE,				// assert an interrupt immediately
    HOLD_LINE,					// hold interrupt line until acknowledged
    PULSE_LINE					// pulse interrupt line instantaneously (only for NMI, RESET)
};

class CCPU:public QObject{
Q_OBJECT
public:
	Cdebug	*pDEBUG;
	CpcXXXX *pPC;

	virtual	bool	init(void) = 0;						//initialize
	virtual	bool	exit(void);						//end
	virtual void	step(void) =0;						//step SC61860

    virtual	void	Load_Internal(QFile *){}
    virtual	void	Load_Internal(QXmlStreamReader *) =0;
    virtual	void	save_internal(QFile *){}
    virtual	void	save_internal(QXmlStreamWriter *) =0;

    virtual	UINT32	get_mem(UINT32 adr,int size);		//get memory
    virtual	void	set_mem(UINT32 adr,int size,UINT32 data);	//set memory
	
	virtual	bool	Get_Xin(void) =0;
	virtual	void	Set_Xin(bool) =0;
	virtual	bool	Get_Xout(void) =0;
	virtual	void	Set_Xout(bool) =0;

	virtual	UINT32	get_PC(void) =0;					//get Program Counter
    virtual	void	set_PC(UINT32 ) {}					//set Program Counter
	virtual void	Regs_Info(UINT8) =0;



	bool	halt;					//halt,off flag
    bool	end,savesw, cpulog,logsw;	//etc.flag
    bool    resetFlag;
    char	Regs_String[1024];

	FILE	*fp_log;			// file pointer to log file
    const char    *fn_log;

    BYTE	imem[0x200];			// Internal memory
    int   imemsize;

    int		CallSubLevel,prevCallSubLevel;

	void	Check_Log(void);

    virtual const char*	GetClassName(){ return("CCPU");}

	virtual void	Reset(void) = 0;

    CCPU(CPObject *parent);
    virtual ~CCPU();

    virtual QByteArray getimem();

    CregCPU * regwidget;
    void setImemBit(WORD adr, int bit, BYTE data);

signals:
    void msgError(QString);
    void showDasm();

};


#endif
