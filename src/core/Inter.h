/*** PC-XXXX Emulator [INTER.H] ***************************/
/* interrupt emulation & timer emulation class            */
/**********************************************************/
#ifndef _INTER_H_
#define _INTER_H_

#define	STATE_PER_SEC	(frequency)		//execute state/sec.

#define	TIMER_PAR_SEC	1000
#define	FAST_STATE		1536							// cycles/fasttimer (about 2ms)
#define	SLOW_CLK		(TIMER_PAR_SEC/2)				// timer/slowtimer (about 0.5s)
#define	STATE_PAR_TIMER	(STATE_PER_SEC/TIMER_PAR_SEC)	//state/timer(count at CPU speed 100%)

#define	INT_FAST	0x01
#define	INT_SLOW	0x02
#define	INT_KEY		0x04
#define	INT_ONKEY	0x08
#define	INT_SO		0x10
#define	INT_SI		0x20
#define	INT_LOW		0x40
#define	INT_IR		0x80


#include "common.h"
//#include "pobject.h"

class CPObject;
class CpcXXXX;
#include "pcxxxx.h"



//////////////////////////////////////////////////////////////////////
// interrupt & timer emulation   /////////////////////////////////////
//////////////////////////////////////////////////////////////////////
class Ctimer //:public CPObject
{
public:
    CpcXXXX *pPC;
    CPObject *Parent;

	bool init(void);		//initialize
	bool exit(void);		//end
	void check(void);				//check for interrupt
	
    bool CheckSpeed(void);

	float GetCPUspeed(void)
	{			//get CPU speed by %
		return(CPUSpeed);
    }

    int  initTP(int frequency);
    void resetTP(int index);
    bool GetTP(int index);
    void deleteTP(int index);
	void SetCPUspeed(float);
    int     msElapsed(quint64 stateRef);
    quint64  usElapsed(quint64 stateRef);
    quint64  nsElapsed(quint64 stateRef);
    quint64 stElapsedId(int id);
    int     msElapsedId(int id);
    quint64  usElapsedId(int id);
    quint64  nsElapsedId(int id);

    bool    resetTimer(int, quint64 reste=0);
    quint64 currentState(void);

    quint64	state;					//state counter
	long	chkspeed;				//speed counter
	float	CPUSpeed;				//CPU speed(100%=STATE_PAR_TIMER)

	bool	Suspend;

    quint64 last_state,nb_state;
    int deltaStep;
	
    Ctimer(CPObject *parent = 0)//	: CPObject(parent)
	{						//[constructor]
        pPC = (CpcXXXX*) parent;
        Parent	= parent;
		state	= 0;				// state counter
		chkspeed= 1;				// speed checker
		CPUSpeed= 1;
        Suspend = false;

        for (int i=0;i<10;i++)
        {
            previous_state_tp[i]=0;
            timerSate[i] = 0;
            frequency_tp[i]=0;
            tp[i] = false;
        }
    }

	~Ctimer()
	{
    }


    int getFreqTP(int index);
    void setFreqTP(int index, int freq);

private:
    quint64 previous_state_tp[10];
    quint64 timerSate[10];
    int     frequency_tp[10];
    bool    tp[10];

};



#endif	// _INTER_H_
