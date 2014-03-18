
#include "common.h"
#include "Log.h"
#include "pd1990ac.h"
#include "Inter.h"

bool	CPD1990AC::exit(void)						//end
{
	return true;
}

INLINE WORD HEX2BCD(BYTE d)
{
	BYTE	a,b,c;
	a=d/100;
	b=d-(a*100);
	c=b/10;
	return((a<<8)+(c<<4)+b-(c*10));
}

bool	CPD1990AC::init(void)
{
	
	lastDateTime = QDateTime::currentDateTime();
	pd1990ac.seconds	= HEX2BCD(lastDateTime.time().second());	/* seconds BCD */
	pd1990ac.minutes	= HEX2BCD(lastDateTime.time().minute());	/* minutes BCD */
	pd1990ac.hours		= HEX2BCD(lastDateTime.time().hour());		/* hours   BCD */
	pd1990ac.days		= HEX2BCD(lastDateTime.date().day());		/* days    BCD */
	pd1990ac.weekday	= 0;										/* weekday BCD */
	pd1990ac.month		= lastDateTime.date().month();				/* month   Hexadecimal form */

	bitno = 0;
	prev_mode = 0x10;
	tp = 0;
    TP_FREQUENCY=1;
	previous_state = 0;
	previous_state_tp = 0;

	return(1);
}

				//initialize
void	CPD1990AC::Reset(void){}

void CPD1990AC::addretrace (void)
{
	pd1990ac.seconds++;
	if ( (pd1990ac.seconds & 0x0f) < 10 )
		return;
	pd1990ac.seconds &= 0xf0;
	pd1990ac.seconds += 0x10;
	if (pd1990ac.seconds < 0x60)
		return;
	pd1990ac.seconds = 0;
	pd1990ac.minutes++;
	if ( (pd1990ac.minutes & 0x0f) < 10 )
		return;
	pd1990ac.minutes &= 0xf0;
	pd1990ac.minutes += 0x10;
	if (pd1990ac.minutes < 0x60)
		return;
	pd1990ac.minutes = 0;
	pd1990ac.hours++;
	if ( (pd1990ac.hours & 0x0f) < 10 )
		return;
	pd1990ac.hours &= 0xf0;
	pd1990ac.hours += 0x10;
	if (pd1990ac.hours < 0x24)
		return;
	pd1990ac.hours = 0;
	increment_day();
}

void CPD1990AC::increment_day(void)
{
	pd1990ac.days++;
	if ((pd1990ac.days & 0x0f) >= 10)
	{
		pd1990ac.days &= 0xf0;
		pd1990ac.days += 0x10;
	}

	pd1990ac.weekday++;
	if (pd1990ac.weekday == 7)
		pd1990ac.weekday=0;

	switch(pd1990ac.month)
	{
	case 1: case 3: case 5: case 7: case 8: case 10: case 12:
		if (pd1990ac.days == 0x32)
		{
			pd1990ac.days = 1;
			increment_month();
		}
		break;
	case 2:	case 4: case 6: case 9: case 11:
		if (pd1990ac.days == 0x31)
		{
			pd1990ac.days = 1;
			increment_month();
		}
		break;
	}
}

void CPD1990AC::increment_month(void)
{
	pd1990ac.month++;
	if (pd1990ac.month == 13)
	{
		pd1990ac.month = 1;
	}
}

bool CPD1990AC::step(void)
{
//	Mode :
//			0	-	Register Hold		DATA OUT = 1 Hz
//			1	-	Register Shift		DATA OUT = [LSB] = 0 or 1
//			2	-	Time Set			DATA OUT = [LSB] = 0 or 1
//			3	-	Time Read			DATA OUT = 1 Hz

	if (previous_state == 0) previous_state = pPC->pTIMER->state;

    while ( (pPC->pTIMER->state - previous_state) >= pPC->getfrequency() )
	{
		addretrace();
        previous_state += pPC->getfrequency();
	};



	if (stb)
	{
		// Mode can change
		mode = c0+(c1<<1)+(c2<<2);
        AddLog(LOG_TIME,tr("Mode:%1 m=%2 clk=%3").arg(mode).arg(pd1990ac.minutes).arg(clk));
        if (mode !=prev_mode) { New_Mode = true; prev_mode=mode; }
        else					New_Mode = false;
	}

    if (clk != prev_clk) {	flip_clk=true; prev_clk=clk;	}
    else					flip_clk=false;

    if (mode == 4)	{
//        if (( (CpcXXXX *)pPC)->pCPU->fp_log) fprintf(( (CpcXXXX *)pPC)->pCPU->fp_log,"TP64\n");
        TP_FREQUENCY=64;
    }
    if (mode == 0)	{ clk = true; flip_clk=true; bitno=0; }

    if (clk && flip_clk)
	{
	
	
		switch (mode)
		{
		case 0x00:	/* Start afresh with shifting */
		case 0x01:	/* Load Register */
			switch(bitno)
			{
			case 0x00: case 0x01: case 0x02: case 0x03:
				data_out=READ_BIT(pd1990ac.seconds , bitno);			// Read seconds 1
				break;
			case 0x04: case 0x05: case 0x06: case 0x07:
				data_out=READ_BIT(pd1990ac.seconds , bitno);			// Read seconds 10
				break;
			case 0x08: case 0x09: case 0x0a: case 0x0b:
				data_out=READ_BIT(pd1990ac.minutes , (bitno-0x08));		// Read minutes 1
				break;
			case 0x0c: case 0x0d: case 0x0e: case 0x0f:
				data_out=READ_BIT(pd1990ac.minutes , (bitno-0x08));		// Read minutes 10
				break;
			case 0x10: case 0x11: case 0x12: case 0x13:
				data_out=READ_BIT(pd1990ac.hours , (bitno-0x10));		// Read hours 1
				break;
			case 0x14: case 0x15: case 0x16: case 0x17:
				data_out=READ_BIT(pd1990ac.hours , (bitno-0x10));		// Read hours 10
				break;
			case 0x18: case 0x19: case 0x1a: case 0x1b:
				data_out=READ_BIT(pd1990ac.days , (bitno-0x18));		// Read day 1
				break;
			case 0x1c: case 0x1d: case 0x1e: case 0x1f:
				data_out=READ_BIT(pd1990ac.days , (bitno-0x18));		// Read day 10
				break;
			case 0x20: case 0x21: case 0x22: case 0x23:
				data_out=READ_BIT(pd1990ac.weekday , (bitno-0x20));		// Read weekday
				break;
			case 0x24: case 0x25: case 0x26: case 0x27:
				data_out=READ_BIT(pd1990ac.month , (bitno-0x24));		// Read month
				break;
			}
			bitno++;
			break;
	
		case 0x02:	/* Set Register */
            AddLog(LOG_TIME,"SET TIME");
			switch(bitno)
			{
			case 0x00: case 0x01: case 0x02: case 0x03:
			case 0x04: case 0x05: case 0x06: case 0x07:
				PUT_BIT(pd1990ac.seconds, bitno, data_in);
				break;
			case 0x08: case 0x09: case 0x0a: case 0x0b:
			case 0x0c: case 0x0d: case 0x0e: case 0x0f:
				PUT_BIT(pd1990ac.minutes, bitno-0x08, data_in);
				break;
			case 0x10: case 0x11: case 0x12: case 0x13:
			case 0x14: case 0x15: case 0x16: case 0x17:
				PUT_BIT(pd1990ac.hours, bitno-0x10, data_in);
				break;
			case 0x18: case 0x19: case 0x1a: case 0x1b:
			case 0x1c: case 0x1d: case 0x1e: case 0x1f:
				PUT_BIT(pd1990ac.days, bitno-0x18,data_in);
				break;
			case 0x20: case 0x21: case 0x22: case 0x23:
				PUT_BIT(pd1990ac.weekday, bitno-0x20,data_in);
				break;
			case 0x24: case 0x25: case 0x26: case 0x27:
				PUT_BIT(pd1990ac.month, bitno-0x24, data_in);
				break;
			}
			bitno++;
			break;
	
		default:	/* Unhandled value */
            //AddLog(LOG_TIME,"MODE %02X (Unhandled) - bitno=%02X, D_in=%s",mode,bitno,data_in?"1":"0");
			break;
		}
	}	

	return(1);
}

bool	CPD1990AC::Get_data(void)
{
	return( data_out);
}

bool	CPD1990AC::Get_tp(void)
{
// generate tp signal. Used by tape functionnality
#define TP_STATE		(pPC->getfrequency() / TP_FREQUENCY)
qint64 delta_state;

	if (previous_state_tp == 0) previous_state_tp = pPC->pTIMER->state;
    while ( ((pPC->pTIMER->state - previous_state_tp)) >= (TP_STATE/2) )
	{
		tp ^= 1;
		previous_state_tp += (TP_STATE / 2);
	};

	return(tp);
}


void	CPD1990AC::Set_c0(bool bit)			{ c0			= bit;	}
void	CPD1990AC::Set_c1(bool bit)			{ c1			= bit;	}
void	CPD1990AC::Set_c2(bool bit)			{ c2			= bit;	}
void	CPD1990AC::Set_stb(bool bit)		{ stb			= bit;	}
void	CPD1990AC::Set_cs(bool bit)			{ cs			= bit;	}
void	CPD1990AC::Set_data(bool bit)		{ data_in		= bit;	}
void	CPD1990AC::Set_gnd(bool bit)		{ gnd			= bit;	}
void	CPD1990AC::Set_clk(bool bit)		{ clk			= bit;	}
void	CPD1990AC::Set_out_enable(bool bit)	{ out_enable	= bit;	}
void	CPD1990AC::Set_n_xtal(bool bit)		{ n_xtal		= bit;	}
void	CPD1990AC::Set_xtal(bool bit)		{ xtal			= bit;	}
void	CPD1990AC::Set_vdd(bool bit)		{ vdd			= bit;	}

CPD1990AC::CPD1990AC(CPObject *parent)
{
        c0=c1=c2=stb=cs=data_in=gnd=clk=data_out=tp=out_enable=n_xtal=xtal=vdd=false;
		Current_Bit = 0;
		pPC = parent;
		init();
}

CPD1990AC::~CPD1990AC(){}

void CPD1990AC::Regs_Info(UINT8 Type)
{
#if 1
	switch(Type)
	{
	case 0:			// Monitor Registers Dialog
	case 1:			// For Log File
		sprintf(Regs_String,	"C0:%s C1:%s C2:%s STB:%s CS:%s D_In:%s CLK:%s D_Ou:%s TP:%s Out_en:%s",
							c0?"1":"0",c1?"1":"0",c2?"1":"0",stb?"1":"0",cs?"1":"0",
							data_in?"1":"0",clk?"1":"0",data_out?"1":"0",tp?"1":"0",
							out_enable?"1":"0");
		break;
	}
#endif
}

