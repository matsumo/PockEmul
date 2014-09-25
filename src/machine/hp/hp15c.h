#ifndef HP15C_H
#define HP15C_H


#include "pcxxxx.h"

class CHPNUT;
class CConnector;
class Cbus;

typedef unsigned char flag;
typedef unsigned char byte;
typedef unsigned short word;
typedef unsigned long dword;
typedef unsigned short ushort;
typedef unsigned long ulong;
typedef unsigned int uint;
typedef quint64 UINT64;


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




class Chp15c:public CpcXXXX{

Q_OBJECT

public:
  Chp15c(CPObject *parent=0);
  ~Chp15c();

  CHPNUT *nutcpu;

  bool Chk_Adr(UINT32 *d, UINT32 data);
  bool Chk_Adr_R(UINT32 *d, UINT32 *data);

  virtual bool	init(void);				// initialize
  virtual bool	run(void);					// emulator main
  virtual UINT8 in(UINT8 address);
  virtual UINT8 out(UINT8 address,UINT8 value);
  virtual void TurnOFF();

  enum {eAwake=0,eLightSleep=1,eDeepSleep=2};       // sleep modes

  void MemoryLost(void);
  void EnableRun(void);
  void DisableRun(void);
  void Halt(void);
  void SetProcParams(int ProcIntervalIn,int ProcCyclesIn);
  void GetProcParams(int &ProcIntervalOut,int &ProcCyclesOut);
  void ExecuteProc();
  void TimerProc();


  void SetSoundMode(int val);
  int GetSoundMode(void);
//  CFont *GetFontLCD(void) { return(&CFontLCD); }

  // trace
  FILE *hLogFile;
  uint InstSetIndex;      // instruction set: 0 - HP, 1 - Zencode, 2 - JDA, 3 - Special
  uint TEFIndex;          // TEF format  - corresponds to instruction set above
  flag fTrace;            // trace on/off
  char szTraceLabel[14];  // global label
  char szTraceOut[100];
  word PC_LAST,PC_TRACE;  // PC_REG for last executed instruction and for tracing at any address
  word TraceTyte1;        // same as Tyte1 but for tracing use only

  int eKeyboard;                         // current keyboard
  int eFont;                             // current font
  int UseAltPunc;                        // use alternate punc chars (for ttf wider punc chars only)

  // activity dot
  word Indicator;                        // enables activity indicator
  QRect RectIndicator;

  // ROM variables
  word CurPage;                          // current page number
  byte active_bank[16];                  // array[page] of banks that are enabled
  word NextActualBankGroup;              // counter for loading actual bank groups



  // display registers and variables
  byte DIS_C_REG[12];              // bit 8 for chars 1-12 (lower bit)
  byte DIS_B_REG[12];              // bits 7-4 for chars 1-12 (occupy lower nybbles)
  byte DIS_A_REG[12];              // bits 3-0 for chars 1-12 (occupy lower nybbles)
  word DIS_ANNUN_REG;              // 12 bit annunciator register
  word UpdateDisplay;              // set to 1 when display needs to be updated
  word UpdateAnnun;                // set to 1 when annunciators need to be updated
  word DisplayOn;                  // set to 1 when LCD is turned on
  word Contrast;                   // 0-15 value for contrast (half nut)
//  COLORREF FontColor;              // RBG font color value

  // timer registers
  word TimerSelA;                  // ==1 if TIMER=A, ==0 if TIMER=B
  UINT64 ClockA,ClockB;
  UINT64 AlarmA,AlarmB;
  UINT64 IntTimer,IntTimerEnd;     // interval timer counter and terminal value
  byte CLK_A[14], CLK_B[14];       // clock A&B - 56 bits
  byte ALM_A[14], ALM_B[14];       // alarm A&B - 56 bits
  byte SCR_A[14], SCR_B[14];       // scratch A&B - 56 bits
  byte INTV_CNT[14], INTV_TV[14];  // interval timer - only low 20 bits used - INTV_CNT is counter, INTV_TV is terminal value
  byte ACC_F[14];                  // accuracy factor - only low 13 bits used
  byte TMR_S[14];                  // status bits - only low 13 bits used
//  MMRESULT TimerEvent;             // ==NULL if no timer, higher precision multimedia timers used for callbacks

  // instruction delay
  UINT64 PCPerf, PCCount[2];
  UINT64 InstrNSec;

  // control and state variables
  UINT ExecuteEvent;              // ==0 if no timer, non zero if timer is set - main loop execution timer callback
  int MinCLRKEY;                  // a counter for keeping a key pressed for a minimum number of CLRKEY instructions
  flag fEnableCLRKEY;             // ==1 when CLRKEY instruction is enabled
  int ProcInterval;                  // time in milliseconds between processor runs
  int ProcCycles;                 // number of processor cycles to run each time
  word MemModules;                // number of regular Memory modules (1-4)
  word XMemModules;               // number of Extended Memory modules (1-3)
  int fPrinter,fCardReader,fTimer,fWand,fHPIL,fInfrared;    // 1 if the associated hardware is loaded by a module
  int SoundMode;                  // current sound mode
  flag fRunEnable;                // ==0 if Run() is disabled

  // breakpoint control
  flag fBreakPtsEnable;           // ==1 if breakpoints are enabled
  flag fBreak;                    // ==1 if breakpoint raised
  int nBreakPts;                  // count of breakpoints
  word BreakPts[100];             // ordered list of breakpoints


  int RamExist(word addr);



  void DisplayRead(void);
  void DisplayWrite(void);
  void DisplayRotRight(byte REG[],int Times);
  void DisplayRotLeft(byte REG[],int Times);
  void AnnunRead(void);
  void AnnunWrite(void);
  void HalfnutRead(void);
  void HalfnutWrite(void);

  void InitTimer(void);
  void DeInitTimer(void);
  void ResetTimer(void);
  void SaveTimer(void);
  void RestoreTimer(void);
  void ContinueTimer(void);
  void ConvertToReg14(byte *DEST_REG,UINT64 Src);
  void ConvertToUINT64(UINT64 *Dest,byte *SRC_REG);
  void TimerWrite(void);
  void TimerRead(void);

  UINT8 getKey();

  void setPortChar(int port, UINT8 c);

};



#endif // HP15C_H
