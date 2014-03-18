#ifndef LU57813P_H
#define LU57813P_H


#include <QDateTime>
#include <QQueue>
#include "pcxxxx.h"

class CLU57813P:public QObject{
Q_OBJECT
    struct datetime_s
{
    int seconds;
    int minutes;
    int hours;
    int days;
    int weekday;
    int month;
};

public:
    const char*	GetClassName(){ return("CLU57813P");}


    bool	init(void);						//initialize
    bool	exit(void);						//end
    void	Reset(void);
    bool	step(void);

    void    command(BYTE cmd);

    void    store(BYTE bank, BYTE value);
    void    action(BYTE value);
    void    request(BYTE value);
    BYTE    get_irq(void);
    void    SetDigital(BYTE val);

    void	increment_day(void);
    void	increment_month(void);

    void	Load_Internal(QXmlStreamReader *);
    void	save_internal(QXmlStreamWriter *);

    void	Regs_Info(UINT8 Type);

    BYTE    Get_reg_out(void);
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

    int     Get_tpIndex64(void) { return tpIndex64;}
    int     Get_tpIndex2(void) { return tpIndex2;}

    bool    Get_Kon(void) {
        return Kon;
    }

    struct	datetime_s datetime;
    bool	c0,c1,c2,stb,cs,data_in,gnd,clk,data_out,tp,out_enable,n_xtal,xtal,vdd;

    CLU57813P(CpcXXXX *parent);
    virtual ~CLU57813P();

    void	addretrace (void);
    //UINT	TP_FREQUENCY;
    bool output_pending;

private:

    BYTE    password[0x10];
//    QByteArray stack;
    BYTE    imem[32];
    BYTE    sp; // Stackpointer
    BYTE    irq;
    BYTE    irq_mask;
    BYTE    digitalvalue;

    int     tpIndex64;
    int     tpIndex2;
    bool    previous_tp64;
    bool    previous_tp2;

    bool    Kon;

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
    CpcXXXX *pPC;

    quint64 previous_state;
    quint64 previous_state_tp;

    BYTE    cmd;
    BYTE    ord;
    BYTE    reg_out;

    quint8 wakeReg[9];
};





#endif // LU57813P_H
