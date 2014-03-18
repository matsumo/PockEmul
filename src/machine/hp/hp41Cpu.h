#ifndef HP41CPU_H
#define HP41CPU_H

// *********************************************************************
//    Copyright (c) 1989-2002  Warren Furlow
//    Sound, Timer, Trace Copyright (c) 2001 Zharkoi Oleg
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
// HP41.h: header file
// *********************************************************************

#include "cpu.h"

#define LOG(x)

typedef unsigned char flag;
typedef unsigned char byte;
typedef unsigned short word;
typedef unsigned long dword;
typedef unsigned short ushort;
typedef unsigned long ulong;
typedef unsigned int uint;
typedef quint64 UINT64;

// ram registers each have 7 bytes
struct RAM_REG
  {
  byte Reg[7];
  };

extern word TypeA[];

#define NORMAL 0                      // for keys - normal position
#define MOVE 1                        // for keys - moved position

// Processor performance characteristics
// ProcCycles / ProcInterval=5.8 for a real machine: 578 cycles / 100 ms per interval
// 5780 inst/sec = 1E6 / 173 ms for a halfnut HP-41CX instruction (older models run at 158 ms)
// time in milliseconds between processor runs:
#define DEFAULT_PROC_INTERVAL 50
#define MAX_PROC_INTERVAL 100
#define MIN_PROC_INTERVAL 10
// number of processor cycles to run each time:
#define DEFAULT_PROC_CYCLES 578

#define DEFAULT_INST_SPEED 173        // milliseconds of actual instruction cycle to simulate for Speaker sound
#define DEFAULT_CONTRAST 5            // contrast value
#define DISPLAY_BOOST 75              // this value is necessary for CAT 2 in SST mode to prevent display glitches
#define MAX_RAM 0x400                 // maximum number of ram registers

//// user code load/save
//struct Cat1Label
//  {
//  char szText[16];
//  int StartReg,StartByte;
//  int EndReg,EndByte;
//  };

/****************************/

class Chp41;

// CPU registers
typedef struct {
    byte A_REG[14],B_REG[14],C_REG[14],M_REG[14],N_REG[14];
    word G_REG,F_REG,ST_REG,Q_REG,P_REG,KEY_REG,XST_REG;
    word PT_PREV;                          // set to value of previous PT
    word FI_REG;                           //14 bits used - there is no actual physical register - this is just an input stream from peripherals
    word CARRY,KEYDOWN,BATTERY;
    word PC_REG,RET_STK0,RET_STK1,RET_STK2,RET_STK3;
    word BASE;
} HP41regs;


/****************************/
// The HP-41 class
/****************************/
class Chp41Cpu:public CCPU{

//  friend class CMcodeDlg;
//  friend class CBreakPointsDlg;
public:
  Chp41Cpu(CPObject *parent);
  ~Chp41Cpu();

  Chp41* hp41;

  virtual bool	init(void);						//initialize
  virtual bool	exit(void);						//end
  virtual void	step(void);						//step SC61860
  virtual void  Reset(void);

  virtual void	Load_Internal(QXmlStreamReader *);
  virtual void	save_internal(QXmlStreamWriter *);

  virtual	bool	Get_Xin(void){}
  virtual	void	Set_Xin(bool){}
  virtual	bool	Get_Xout(void){}
  virtual	void	Set_Xout(bool){}

  virtual	UINT32	get_PC(void);					//get Program Counter
  virtual	void	set_PC(UINT32 );					//set Program Counter
  virtual void	Regs_Info(UINT8){}


  enum {eAwake=0,eLightSleep=1,eDeepSleep=2};       // sleep modes
  void Wakeup(void);
  void Sleep(int eNewMode);
  int IsSleeping(void);



  // CPU registers
  RAM_REG *pRAM;
  HP41regs *r;
  word *PT_REG;                          // set to address of Q_REG or P_REG

  // CPU variables
  unsigned short eSleepMode;
  word perph_in_control,perph_selected,asdf,ram_selected,wqer;    // modes dependant on previous instructions that set modes
  word control_perph;              // number of perph in control
  word Tyte1;                      // current instruction (ten bit byte)
  word Tyte2;                      // valid only if tyte1 is two byte instruction
  word TytePrev;                   // previous instruction code
  word Modifier;                   // instruction modifier
  word FirstTEF,LastTEF;           // starting and ending tef pointers (always starting at least significant digit)
  word Boost;                      // boost the main loop by this many more cycles if it is near completion


  // instruction delay
  UINT64 PCPerf, PCCount[2];
  UINT64 InstrNSec;

  // breakpoint control
  flag fBreakPtsEnable;           // ==1 if breakpoints are enabled
  flag fBreak;                    // ==1 if breakpoint raised
  int nBreakPts;                  // count of breakpoints
  word BreakPts[100];             // ordered list of breakpoints

  word GetNextTyte(void);
  void Execute(void);

  void Class0(void);
  void Subclass0(void);
  void Enbank(int BankSet);
  void Subclass1(void);
  void Subclass2(void);
  void Subclass3(void);
  void Subclass4(void);
  void Subclass5(void);
  void Subclass6(void);
  void Subclass7(void);
  void Subclass8(void);
  void Subclass9(void);
  void SubclassA(void);
  void SubclassB(void);
  void SubclassC(void);
  void SubclassD(void);
  void SubclassE(void);
  void SubclassF(void);
  void wdata(void);
  void rdata(void);

  void Class1(void);
  void PushReturn(word addr);
  word PopReturn(void);
  void LongJump(word addr,flag fPush);

  void Class2(void);
  int ArithCarry(unsigned short tyte);
  void ConvertTEF(void);
  byte Adder(byte nib1,byte nib2);
  byte Subtractor(byte nib1,byte nib2);

  void Class3(void);



  void error_message(short num);
  void Speaker(short Freq, int Duration);

  // user code support
  int DecodeUCByte(int PrevType,byte CurrByte);
  void CalcOffset(int LowReg,int LowByte,int HighReg,int HighByte,int &RegOff,int &ByteOff);
  void PrevGlobal(int CurrReg,int CurrByte,int &PrevReg,int &PrevByte,int &RegOff,int &ByteOff);

};


#endif // HP41CPU_H
