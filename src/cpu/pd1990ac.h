#ifndef PD1990AC_H
#define PD1990AC_H

#include <QDateTime>
#include "pobject.h"

class CPD1990AC:public QObject{

	struct pd1990ac_s
{
	int seconds;
	int minutes;
	int hours;
	int days;
	int weekday;
	int month;
};

public:
    const char*	GetClassName(){ return("CPD1990AC");}


	bool	init(void);						//initialize
	bool	exit(void);						//end
	void	Reset(void);
	bool	step(void);

	void	increment_day(void);
	void	increment_month(void);

	void	Load_Internal(FILE *ffile);
	void	save_internal(FILE *file);

	void	Regs_Info(UINT8 Type);

	bool	Get_data(void);
	bool	Get_tp(void);

	void	Set_c0(bool);
	void	Set_c1(bool);
	void	Set_c2(bool);
	void	Set_stb(bool);
	void	Set_cs(bool);
	void	Set_data(bool);
	void	Set_gnd(bool);
	void	Set_clk(bool);
    void	Set_out_enable(bool);
	void	Set_n_xtal(bool);
	void	Set_xtal(bool);
	void	Set_vdd(bool);
	
	struct	pd1990ac_s pd1990ac;
	bool	c0,c1,c2,stb,cs,data_in,gnd,clk,data_out,tp,out_enable,n_xtal,xtal,vdd;

	CPD1990AC(CPObject *parent);
    virtual ~CPD1990AC();
	
	void	addretrace (void);
    //UINT	TP_FREQUENCY;

private:

	char	Regs_String[255];

	UINT	nTimerId;
	UINT	nTPTimerId;

	UINT8	mode;
	UINT	bitno,Current_Bit;
	bool	New_Mode;
	bool	New_clk;
	UINT8	prev_mode;
	UINT8	prev_clk;
	bool	flip_clk;
    UINT	TP_FREQUENCY;
	
	QDateTime	lastDateTime;
	CPObject *pPC;

    quint64 previous_state;
    quint64 previous_state_tp;
};

#endif
