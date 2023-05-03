// *********************************************************************
//    Copyright (c) 2001 Zharkoi Oleg
//    Revised 2004 Warren Furlow
//    Revised 2009 Reinhard Breuer - corrected SHIFT ON problem with flags
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
// *********************************************************************

// *********************************************************************
// HP41Timer.cpp
//   Executes the instructions for the timer chip
// Ref: Zenrom Pg. 126, HEPAX VOL II Pg. 139-141, 1LF6 Timer Chip Detailed Description
//      1LF6 timer Chip Detailed Description
// *********************************************************************

#if 1

#include <QDateTime>
#include <QDebug>

#include "hp41.h"
#include "hp41Cpu.h"

//static CRITICAL_SECTION csTimer;

// TimerSelA ==1 if TIMER=A, ==0 if TIMER=B
// Clock A is usually current time and B is usually stopwatch time (good until Dec 20, 2330)
// Alarm A is usually time of next alarm or cleared. B usually has a constant of 09999999999000h which is checked for timer integrity
// Scratch A is used to hold the last time when corrected.  B bit 5 is set for 24 hour, bit 6 is set for display of both time and date.
// Interval A,B - 56 bits total but only 20 are used: [4-0] sss.hh where sss = seconds, hh = hundredths
// Timer status - 13 bits
//   Bit  Meaning
//   0    ALMA  - Set on valid compare of Alarm A with Clock A
//   1    DTZA  - Set on overflow of clock A (or decrement of 10's complement)
//   2    ALMB  - Set on valid compare of Alarm B with Clock B
//   3    DTZB  - Set on overflow of Clock B
//   4    DTZIT - Set by terminal count state of Interval Timer (it has counted a whole interval)
//   5    PUS   - Power Up Status - set when power first applied or falls below a certain minimum
//   6    CKAEN - Enable Clock A incrementers
//   7    CKBEN - Enable Clock B incrementers
//   8    ALAEN - Enables comparator logic between Alarm A and Clock A - Set if Alarm A is enabled.  Since time alarms are usually enabled, this flag is usually set
//   9    ALBEN - Enables comparator logic between Alarm B and Clock B - Set if Alarm B is enabled.  Always clear since stopwatch alarms are not possible.  Timer alarms occur as a result of bit 3 set.
//  10    ITEN  - Enables Interval Timer incrementing and comparator logic (interval timer is running)
//  11    TESTA - Enables Test A mode
//  12    TESTB - Enables Test B mode
//
// This code does not do any accuracy factor corrections

/****************************/
void Chp41::InitTimer()
  {
  if (fTimer)
    return;
  fTimer=true;
  ResetTimer();
//  InitializeCriticalSection(&csTimer);
//  TimerEvent=timeSetEvent(10,0,TimerProc,0,TIME_PERIODIC|TIME_CALLBACK_FUNCTION);
  }


/****************************/
void Chp41::ResetTimer()
  {
    qWarning()<<"ResetTimer()";
  if (!fTimer)
    return;
  qWarning()<<"ok";

  TimerSelA=1;
  memset(CLK_A,0,14);
  memset(CLK_B,0,14);
  memset(ALM_A,0,14);
  memset(ALM_B,0,14);
  memset(SCR_A,0,14);
  memset(SCR_B,0,14);
  memset(INTV_CNT,0,14);
  memset(INTV_TV,0,14);
  memset(ACC_F,0,14);
  memset(TMR_S,0,14);
  ClockA=0;
  ClockB=0;
  AlarmA=0;
  AlarmB=0;
  IntTimer=0;
  IntTimerEnd=0;

  // set to current time
  ConvertToReg14(ALM_B,9999999999000);                         // anti-corruption constant
  TMR_S[1]&=0x04;                                              // bit 6 - Clock A incrementing
//  COleDateTime CurrentTime=COleDateTime::GetCurrentTime();     // preset the clock with current time
  QDateTime time(QDate(1899,12,31),QTime(24,0));

  qWarning()<<time<<" current:"<<QDateTime::currentDateTime();
  quint64 _t = time.msecsTo(QDateTime::currentDateTime()) / 10;
  qWarning()<<QDateTime::currentDateTime();
//  quint64 _t = QTime::currentTime().hour()*360000+
//          QTime::currentTime().minute()*6000+
//          QTime::currentTime().second()*100+
//          QTime::currentTime().msec()/10+
//          8640000*20;
  qWarning()<<_t<<"cc";
  ClockA=_t;//(UINT64)((_t-2.0)*8640000.0);           // this always overrides any user set value
  }


/****************************/
void Chp41::DeInitTimer()
  {
//  if (!fTimer)
//    return;
  fTimer=-1;
//  if(TimerEvent)
//    timeKillEvent(TimerEvent);
//  TimerEvent=NULL;
//  DeleteCriticalSection(&csTimer);
  }


/****************************/
// saves values to registers before lod file write
void Chp41::SaveTimer()
  {
  ConvertToReg14(CLK_A,ClockA);
  ConvertToReg14(CLK_B,ClockB);
  ConvertToReg14(INTV_CNT,IntTimer);
  // ALM_A, ALM_B and INTV_TV contain the definitive values, TimerA, TimerB and IntTimerEnd are for reference only
  }


/****************************/
// restores values from registers after lod file read
void Chp41::RestoreTimer()
  {
  // CLK A is not restored because that would overwrite the actual time with the time last saved in file
  ConvertToUINT64(&ClockB,CLK_B);
  ConvertToUINT64(&AlarmA,ALM_A);
  ConvertToUINT64(&AlarmB,ALM_B);
  ConvertToUINT64(&IntTimer,INTV_CNT);
  ConvertToUINT64(&IntTimerEnd,INTV_TV);
  }


/****************************/
// called every .01 sec (10 millisec)
/****************************/
void Chp41::TimerProc()  {
  int fAlert=0;
//  EnterCriticalSection(&csTimer);
  // Clock A
  if (this->TMR_S[1]&0x04)       // bit 6 - Clock A enabled
    {
    this->ClockA++;
    if (this->ClockA>99999999999999 || this->ClockA==0)
      {
      this->ClockA=0;
      this->TMR_S[0]|=0x02;      // set bit 1 - overflow in clock A
      hp41cpu->r->FI_REG|=0x2000;      // set flag 13 - general service request flag
      hp41cpu->r->FI_REG|=0x1000;      // set flag 12 - timer request
      hp41cpu->Wakeup();
      }
    if (this->TMR_S[2]&0x01 && this->ClockA==this->AlarmA)  // if bit 8 set - enable ClockA & AlarmA comparator
      {
      this->TMR_S[0]|=0x01;      // set bit 0 - valid compare
      hp41cpu->r->FI_REG|=0x2000;      // set flag 13
      hp41cpu->r->FI_REG|=0x1000;      // set flag 12
      hp41cpu->Wakeup();
      fAlert=1;
      }
    }
  // Clock B
  if (this->TMR_S[1]&0x08)       // bit 7 - Clock B enabled
    {
    this->ClockB++;
    if (this->ClockB>99999999999999 || this->ClockB==0)
      {
      this->ClockB=0;
      this->TMR_S[0]|=0x08;      // set bit 3 - overflow in clock B
      hp41cpu->r->FI_REG|=0x2000;      // set flag 13
      hp41cpu->r->FI_REG|=0x1000;      // set flag 12
      hp41cpu->Wakeup();
      }
    if (this->TMR_S[2]&0x02 && this->ClockB==this->AlarmB)  // if bit 9 set - enable ClockB & AlarmB comparator
      {
      this->TMR_S[0]|=0x04;      // set bit 2 - valid compare
      hp41cpu->r->FI_REG|=0x2000;      // set flag 13
      hp41cpu->r->FI_REG|=0x1000;      // set flag 12
      hp41cpu->Wakeup();
      fAlert=1;
      }
    }
  // Interval Timer
  if (this->TMR_S[2]&0x04)       // bit 10 - interval timer enabled
    {
    this->IntTimer++;
    if (this->IntTimer==this->IntTimerEnd)
      {
      this->IntTimer=0;          // reset interval timer to zero
      this->TMR_S[1]|=0x01;      // set bit 4 - DTZIT - Decrement Through Zero Interval timer
      hp41cpu->r->FI_REG|=0x2000;      // set flag 13
      hp41cpu->r->FI_REG|=0x1000;      // set flag 12
      hp41cpu->Wakeup();
      }
    }
//  LeaveCriticalSection(&csTimer);
  // alert for an alarm
//  if (fAlert)
//    {
//    if (theApp.m_pMainWnd->IsIconic())
//      theApp.m_pMainWnd->ShowWindow(SW_RESTORE);
//    theApp.m_pMainWnd->SetForegroundWindow();
//    if (this->SoundMode==eSoundNone)
//      for (int i=1;i<5;i++)
//        MessageBeep(0xFFFFFFFF);
//    }
  }


/****************************/
// Converts UINT64 to Reg14 (BCD)
void Chp41::ConvertToReg14(
  My_byte *DEST_REG,
  UINT64 Src)
  {
  for (int i=0;i<14;i++)
    {
    DEST_REG[i]=(My_byte)(Src%10);
    Src/=10;
    }
  }


/****************************/
// Converts Reg14 (BCD) to UINT64
void Chp41::ConvertToUINT64(
  UINT64 *Dest,
  My_byte *SRC_REG)
  {
  UINT64 temp=0;
  for (int i=13;i>=0;i--)
    {
    temp*=10;
    temp+=SRC_REG[i]%10;
    }
  *Dest=temp;
  }


/****************************/
void Chp41::TimerWrite()
  {
  switch (hp41cpu->Modifier)
    {
    case 0:              // WTIME
      {
      if (TimerSelA)
        {
        memcpy(CLK_A,hp41cpu->r->C_REG,14);
        ConvertToUINT64(&ClockA,CLK_A);
        }
      else
        {
        memcpy(CLK_B,hp41cpu->r->C_REG,14);
        ConvertToUINT64(&ClockB,CLK_B);
        }
      break;
      }
    case 1:              // WTIME-
      {
      if (TimerSelA)
        {
        memcpy(CLK_A,hp41cpu->r->C_REG,14);
        ConvertToUINT64(&ClockA,CLK_A);
        }
      else
        {
        memcpy(CLK_B,hp41cpu->r->C_REG,14);
        ConvertToUINT64(&ClockB,CLK_B);
        }
      break;
      }
    case 2:              // WALM
      {
      if (TimerSelA)
        {
        memcpy(ALM_A,hp41cpu->r->C_REG,14);
        ConvertToUINT64(&AlarmA,ALM_A);
        }
      else
        {
        memcpy(ALM_B,hp41cpu->r->C_REG,14);
        ConvertToUINT64(&AlarmB,ALM_B);
        }
      break;
      }
    case 3:              // WSTS
      {
      if (TimerSelA)     // first 6 status bits may only be cleared
        {
        TMR_S[0]&=hp41cpu->r->C_REG[0];
        TMR_S[1]&=(0x0C|(hp41cpu->r->C_REG[1]&0x03));
        //RB++ check for bits 0 to 5 to be zero and clear flag 12 & 13
        // all flags should be buffered for each peripheral and ORed at the beginning of each instruction.
        // flag 12 should only indicate 1, if flag 13 is set AND Perph Addr=FB
        // see: 1LF6 detailed description -> pg. 6 -> 2.7 FLAG and System Interrupt
        if ((TMR_S[0]||(TMR_S[1]&0x03))==0)
          hp41cpu->r->FI_REG&=0xcfff;    // clear flag 12, 13
        //RB--
        }
      else
        {
        ACC_F[3]=hp41cpu->r->C_REG[4]&0x1;
        ACC_F[2]=hp41cpu->r->C_REG[3];
        ACC_F[1]=hp41cpu->r->C_REG[2];
        ACC_F[0]=hp41cpu->r->C_REG[1];
        }
      break;
      }
    case 4:              // WSCR
      {
      if (TimerSelA)
        memcpy(SCR_A,hp41cpu->r->C_REG,14);
      else
        memcpy(SCR_B,hp41cpu->r->C_REG,14);
      break;
      }
    case 5:              // WINTST - set and start interval time
      {
      memcpy(INTV_TV,hp41cpu->r->C_REG,5);            // set terminal count value
      ConvertToUINT64((UINT64*)&IntTimerEnd,INTV_TV);
      IntTimer=0;
      TMR_S[2]|=0x04;					// set bit 10- ITEN - Interval Timer Enable
      break;
      }
    case 7:              // STPINT - stop interval timer
      {
      TMR_S[2]&=0x0B;	 // clear bit 10
      break;
      }
    case 8:              // WKUPOFF - clear test mode
      {
      if (TimerSelA)
        TMR_S[2]&=0x07;  // clear bit 11 - Test A mode
      else
        TMR_S[3]&=0x0E;  // clear bit 12 - Test B mode
      break;
      }
    case 9:              // WKUPON - set test mode
      {
      if (TimerSelA)
        TMR_S[2]|=0x08;  // set bit 11
      else
        TMR_S[3]|=0x01;  // set bit 12
      break;
      }
    case 0xA:            // ALMOFF
      {
      if (TimerSelA)
        TMR_S[2]&=0x0E;  // clear bit 8
      else
        TMR_S[2]&=0x0D;  // clear bit 9
      break;
      }
    case 0xB:            // ALMON
      {
      if (TimerSelA)
        TMR_S[2]|=0x01;  // set bit 8
      else
        TMR_S[2]|=0x02;  // set bit 9
      break;
      }
    case 0xC:            // STOPC
      {
      if (TimerSelA)     // should never stop Clock A or time will get messed up!
        TMR_S[1]&=0x0B;	 // clear bit 6 - Clock A incrementer
      else
        TMR_S[1]&=0x07;	 // clear bit 7 - Clock B incrementer
      break;
      }
    case 0xD:            // STARTC
      {
      if (TimerSelA)
        TMR_S[1]|=0x04;	 // set bit 6
      else
        TMR_S[1]|=0x08;	 // set bit 7
      break;
      }
    case 0xE:            // TIMER=B
      {
      TimerSelA=0;
      break;
      }
    case 0xF:            // TIMER=A
      {
      TimerSelA=1;
      break;
      }
    }
  }


/****************************/
void Chp41::TimerRead()
  {
//    qWarning()<<"TimerRead()";

  switch (hp41cpu->Modifier)
    {
    case 0:             // RTIME
      {
      if (TimerSelA)
        {
        ConvertToReg14(CLK_A,ClockA);
        memcpy(hp41cpu->r->C_REG,CLK_A,14);
        }
      else
        {
        ConvertToReg14(CLK_B,ClockB);
        memcpy(hp41cpu->r->C_REG,CLK_B,14);
        }
      break;
      }
    case 1:             // RTIMEST
      {
      if (TimerSelA)
        {
        ConvertToReg14(CLK_A,ClockA);
        memcpy(hp41cpu->r->C_REG,CLK_A,14);
        }
      else
        {
        ConvertToReg14(CLK_B,ClockB);
        memcpy(hp41cpu->r->C_REG,CLK_B,14);
        }
      break;
      }
    case 2:             // RALM
      {
      if (TimerSelA)
        memcpy(hp41cpu->r->C_REG,ALM_A,14);
      else
        memcpy(hp41cpu->r->C_REG,ALM_B,14);
      break;
      }
    case 3:             // RSTS
      {
      if (TimerSelA)
        memcpy(hp41cpu->r->C_REG,TMR_S,14);
      else
        {
        memset(hp41cpu->r->C_REG,0,14);
        for (int i=0;i<4;i++)
          hp41cpu->r->C_REG[i+1]=ACC_F[i];
        }
      break;
      }
    case 4:             // RSCR
      {
      if (TimerSelA)
        memcpy(hp41cpu->r->C_REG,SCR_A,14);
      else
        memcpy(hp41cpu->r->C_REG,SCR_B,14);
      break;
      }
    case 5:             // RINT
      {
      memset(hp41cpu->r->C_REG,0,14);
      memcpy(hp41cpu->r->C_REG,INTV_TV,5);
      break;
      }
    default:
      {
      break;
      }
    }
  }

#endif
