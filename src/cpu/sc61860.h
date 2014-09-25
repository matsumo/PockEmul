/**********************************************************/
/* SC61860 CPU emulation class                            */
/**********************************************************/

#ifndef SC61860_H
#define SC61860_H

#include "cpu.h"
class CPObject;


#define		MAX_IMEM	0x200			/* internal memory size */

#define		IMEM_KOL	0xf0			// Key Output Port(L)
#define		IMEM_KOH	0xff			// Key Output Port(H)

#define		IMEM_IA		0x5c
#define		IMEM_IB		0x5d
#define		IMEM_IC		0x5f
#define		IMEM_FO		0x5e


#define OLD_HEAD_STR	"1350STA"		//status file
#define HEAD_STR		"SC61860"		//status file
#define HEAD_BGN		0
#define HEAD_LEN		7
#define REG_BGN			HEAD_LEN
#define REG_LEN			12
#define IMEM_BGN		(HEAD_LEN+REG_LEN)
#define IMEM_LEN		96


#define	I_REG_I		(imem[0x00])
#define	I_REG_J		(imem[0x01])

#define	I_REG_A		(imem[0x02])
#define	I_REG_B		(imem[0x03])
#define	I_REG_Xl	(imem[0x04])
#define	I_REG_Xh	(imem[0x05])
#define	I_REG_Yl	(imem[0x06])
#define	I_REG_Yh	(imem[0x07])

#define	I_REG_K		(imem[0x08])
#define	I_REG_L		(imem[0x09])
#define	I_REG_M		(imem[0x0A])
#define	I_REG_N		(imem[0x0B])


/*--------------------------------------------------------------------------*/

enum REGNAME{					//register name
	REG_A , REG_B , REG_BA, REG_I, REG_J, REG_X ,
    REG_Y ,  REG_PC, REG_DP, REG_C, REG_Z,REG_P,REG_Q,REG_R
};
extern char	RegName[10];

enum OPRMODE{					//operation mode
	OPR_AND, OPR_OR , OPR_XOR, OPR_ADD, OPR_SUB
};

typedef
union {
    struct
    {
        BYTE	p,q,r,dummy;
#if 0
#ifdef POCKEMUL_BIG_ENDIAN
        BYTE	dph,dpl;
        BYTE	pch,pcl;
#else
        BYTE	dpl,dph;
        BYTE	pcl,pch;
#endif
#else
        WORD	dp,pc;
#endif
        BYTE	c,z;//,IB,FO,CTRL;
    }r;
	struct 
	{
		BYTE	p,q,r,dummy;
		WORD	dp,pc;
		BYTE	c,z;//,IB,FO,CTRL;
	}d;
} SCREG;



class CSC61860:public CCPU{
public:
	bool	init(void);						//initialize
	bool	exit(void);						//end
	void	step(void);						//step SC61860
	void	AddState(BYTE n);

	void	Load_Internal(QFile *file);
	void	save_internal(QFile *file);
    void	Load_Internal(QXmlStreamReader *);
    void	save_internal(QXmlStreamWriter *);
	
	bool	Get_Xin(void);
	void	Set_Xin(bool);
	bool	Get_Xout(void);
	void	Set_Xout(bool);


	UINT32	get_reg(REGNAME regname);			//get register
	void	set_reg(REGNAME regname,UINT32 data);	//set register
	UINT32	get_mem(UINT32 adr,int size);		//get memory
	void	set_mem(UINT32 adr,int size,UINT32 data);	//set memory

	UINT32	Get_r(BYTE);
	void	Set_r(BYTE ,UINT32);


	void	Chk_Flag(UINT32 d,BYTE len);
	void	Chk_Zero(UINT32 d,BYTE len);
	void	Chk_imemAdr(BYTE d,BYTE len);

	void	OpExec(BYTE);

	void	Reset(void);

    UINT32	get_PC(void){return(get_reg(REG_PC));}				//get Program Counter
	void	Regs_Info(UINT8);
    bool    getDisp() { return disp_on;}
    virtual const char*	GetClassName(){ return("CSC61860");}

    CSC61860(CPObject *parent);
    virtual ~CSC61860();

private:
    bool	div500;
    bool	div2;
	bool	Xin,Xout;

	BYTE	Get_i8(BYTE adr);					//get i-mem 8bits
	WORD	Get_i16(BYTE adr);					//get i-mem 16bits

	void	Set_i8(BYTE adr,BYTE d);			//Set i-mem 8bits
	void	Set_i16(BYTE adr,WORD d);			//Set i-mem 16bits

	void	compute_xout(void);
    void    backgroundTasks(void);
	qint64	start2khz;
	qint64	start4khz;
	
    bool	wait_loop_running,cup_loop_running,cdn_loop_running;
    int     op_local_counter;
	
	void  Op_00(void);
	void  Op_01(void);
	void  Op_02(void);
	void  Op_03(void);
	void  Op_04(void);
	void  Op_05(void);
	void  Op_06(void);
	void  Op_07(void);
	void  Op_08(void);
	void  Op_09(void);
	void  Op_0a(void);
	void  Op_0b(void);
	void  Op_0c(void);
	void  Op_0d(void);
	void  Op_0e(void);
	void  Op_0f(void);

	void  Op_10(void);
	void  Op_11(void);
	void  Op_12(void);
	void  Op_13(void);
	void  Op_14(void);
	void  Op_15(void);
//	void  Op_16(void);
//	void  Op_17(void);
	void  Op_18(void);
	void  Op_19(void);
	void  Op_1a(void);
	void  Op_1b(void);
	void  Op_1c(void);
	void  Op_1d(void);
	void  Op_1e(void);
	void  Op_1f(void);

	void  Op_20(void);
	void  Op_21(void);
	void  Op_22(void);
	void  Op_23(void);
	void  Op_24(void);
	void  Op_25(void);
	void  Op_26(void);
	void  Op_27(void);
	void  Op_28(void);
	void  Op_29(void);
	void  Op_2a(void);
	void  Op_2b(void);
	void  Op_2c(void);
	void  Op_2d(void);
//	void  Op_2e(void);
	void  Op_2f(void);

	void  Op_30(void);
	void  Op_31(void);
	void  Op_32(void);
//	void  Op_33(void);
	void  Op_34(void);
	void  Op_35(void);
//	void  Op_36(void);
	void  Op_37(void);
	void  Op_38(void);
	void  Op_39(void);
	void  Op_3a(void);
	void  Op_3b(void);

	void  Op_40(BYTE);
	void  Op_41(BYTE);
	void  Op_44(void);
	void  Op_45(void);
	void  Op_46(void);
	void  Op_47(void);


	void  Op_4c(void);
	void  Op_4d(void);
	void  Op_4e(void);
	void  Op_4f(void);

	void  Op_50(void);
	void  Op_51(void);
	void  Op_52(void);
	void  Op_53(void);
	void  Op_54(void);
	void  Op_55(void);
	void  Op_56(void);
	void  Op_57(void);
	void  Op_58(void);
	void  Op_59(void);
	void  Op_5a(void);
	void  Op_5b(void);
//	void  Op_5c(void);
	void  Op_5d(void);
//	void  Op_5e(void);
	void  Op_5f(void);

	void  Op_60(void);
	void  Op_61(void);
	void  Op_62(void);
	void  Op_63(void);
	void  Op_64(void);
	void  Op_65(void);
	void  Op_66(void);
	void  Op_67(void);
//	void  Op_68(void);
//	void  Op_69(void);
//	void  Op_6a(void);
	void  Op_6b(void);
//	void  Op_6c(void);
//	void  Op_6d(void);
//	void  Op_6e(void);
	void  Op_6f(void);

	void  Op_70(void);
	void  Op_71(void);
	void  Op_77(void);
	void  Op_74(void);
	void  Op_75(void);
	void  Op_78(void);
	void  Op_79(void);
	void  Op_7a(void);
//	void  Op_7b(void);
	void  Op_7c(void);
	void  Op_7d(void);
	void  Op_7e(void);
	void  Op_7f(void);

	void  Op_80(BYTE);

	void  Op_c4(void);
	void  Op_c5(void);
	void  Op_c6(void);
	void  Op_c7(void);
	void  Op_cc(void);
//	void  Op_cd(void);
	void  Op_ce(void);
//	void  Op_cf(void);

	void  Op_d0(void);
	void  Op_d1(void);
	void  Op_d2(void);
	void  Op_d4(void);
	void  Op_d5(void);
	void  Op_d6(void);
//	void  Op_d7(void);
	void  Op_d8(void);
//	void  Op_d9(void);
	void  Op_da(void);
	void  Op_db(void);
//	void  Op_dc(void);
	void  Op_dd(void);
//	void  Op_de(void);
	void  Op_df(void);

	void  Op_e0(BYTE);

    SCREG	reg;				// register structure


    qint64 ticks;
    qint64 ticks2;
    qint64 wait2khz;
    qint64 wait4khz;
    qint64 ticksReset;
	BYTE power_on;
	BYTE disp_on;

	BYTE DASMLOG;

private:
    bool first_pass;
};



#endif
