// *********************************************************************
//    Copyright (c) 1989-2002  Warren Furlow
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
// HP41.cpp : implementation file
// *********************************************************************

#include <math.h>
#include <QDebug>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QPainter>

#include "hp15c.h"
#include "hpnut.h"


#include "Lcdc_hp15c.h"
#include "Keyb.h"
#include "Inter.h"
#include "init.h"
#include "Connect.h"
#include "clink.h"


#include "mainwindowpockemul.h"
extern MainWindowPockemul *mainwindow;



/****************************/
// Constructor
/****************************/


Chp15c::Chp15c(CPObject *parent):CpcXXXX(parent)
{

    setfrequency( (int) 215000);  // 80µs per cycle
    setcfgfname(QString("hp15c"));

    SessionHeader	= "HP15CPKM";
    Initial_Session_Fname ="hp15c.pkm";

    BackGroundFname	= P_RES(":/hp15c/hp15c.png");
    LcdFname		= P_RES(":/hp15c/hp15clcd.png");
//    SymbFname		= P_RES("");

//    TopFname = P_RES(":/hp15c/top.png");
//    BackFname = P_RES(":/hp15c/back.png");
//    LeftFname = P_RES(":/hp15c/left.png");
//    RightFname = P_RES(":/hp15c/right.png");


    memsize		= 0x10000;
    InitMemValue	= 0x00;

    SlotList.clear();
    SlotList.append(CSlot(24 , 0x0000 ,P_RES(":/hp15c/hp15c.bin"), ""	, CSlot::ROM , "ROM"));
    SlotList.append(CSlot(40 , 0x6000 ,"", ""	, CSlot::RAM , "RAM"));

    setDXmm(130);
    setDYmm(79);
    setDZmm(15);

    setDX(465);
    setDY(283);

    Lcd_X		= 90;
    Lcd_Y		= 35;
    Lcd_DX		= 210;
    Lcd_DY		= 33;
    Lcd_ratio_X	= 1;
    Lcd_ratio_Y	= 1;

    Lcd_Symb_X	= 35;
    Lcd_Symb_Y	= 55;
    Lcd_Symb_DX	= 215;
    Lcd_Symb_DY	= 7;
    Lcd_Symb_ratio_X	= 1;

    pTIMER		= new Ctimer(this);
    pLCDC		= new Clcdc_hp15c(this);
    pCPU		= new CHPNUT(this,80,P_RES(":/hp15c/hp15c.obj"));
    nutcpu = (CHPNUT*)pCPU;
    pKEYB		= new Ckeyb(this,"hp15c.map");

    // trace
    hLogFile=NULL;
    InstSetIndex=0;
    TEFIndex=0;
    fTrace=false;
    memset(szTraceOut,0,sizeof(szTraceOut));
}


/****************************/
// destructor
/****************************/
Chp15c::~Chp15c()
  {

  }


bool Chp15c::init()
{
    //pCPU->logsw = true;



    CpcXXXX::init();

//    pLCDC->updated = true;
//    pLCDC->Refresh = true;
//    pLCDC->disp();

    WatchPoint.remove(this);

    // trace
    hLogFile=NULL;
    InstSetIndex=0;
    TEFIndex=0;
    fTrace=false;
    memset(szTraceOut,0,sizeof(szTraceOut));

    eKeyboard=0;
    eFont=0;
    UseAltPunc=0;

    // activity dot
    Indicator=0;
    RectIndicator = QRect(3,3,7,7);

    // ROM variables
//    for (int page=0;page<=0xf;page++)
//    {
//        for (int bank=1;bank<=4;bank++)
//            PageMatrix[page][bank-1]=NULL;
//        active_bank[page]=1;
//    }
//    pCurPage=NULL;
    CurPage=0;
    NextActualBankGroup=1;


    PC_LAST=0;
    PC_TRACE=0;

    // display registers and variables
    UpdateDisplay=1;
    UpdateAnnun=1;
    DisplayOn=0;
    memset(DIS_C_REG,0,sizeof(DIS_C_REG));
    memset(DIS_B_REG,0,sizeof(DIS_B_REG));
    memset(DIS_A_REG,0,sizeof(DIS_A_REG));
    DIS_ANNUN_REG=0;
    Contrast=DEFAULT_CONTRAST;  // default halfnut value
    //  FontColor=0;                // black

    // instruction delay
    //  QueryPerformanceFrequency(&PCPerf);
    memset(PCCount,0,sizeof(PCCount));
    InstrNSec=0;

    // control and state variables
    MinCLRKEY=0;
    fEnableCLRKEY=false;
    ExecuteEvent=0;
    ProcInterval=0;
    ProcCycles=0;
    MemModules=0;
    XMemModules=0;
    fPrinter=-1;
    fCardReader=-1;
    fTimer=-1;
    fWand=-1;
    fHPIL=-1;
    fInfrared=-1;
//    SoundMode=eSoundNone;
    fRunEnable=false;
    fBreakPtsEnable=false;
    fBreak=false;
    nBreakPts=0;


    pTIMER->resetTimer(0);

   return true;
}

extern int ask(QWidget *parent,QString msg,int nbButton);
#define KEY(c)	( pKEYB->keyPressedList.contains(TOUPPER(c)) || pKEYB->keyPressedList.contains(c) || pKEYB->keyPressedList.contains(TOLOWER(c)))


void Chp15c::TurnOFF()
{
    ASKYN _save = mainwindow->saveAll;
    mainwindow->saveAll = YES;
    CpcXXXX::TurnOFF();
    mainwindow->saveAll = _save;
}

bool Chp15c::run()
{

    pLCDC->updated = true;

    if (pKEYB->LastKey!=0) {
       nutcpu->nut_press_key (nutcpu->reg, getKey());
    }
    else {
        nutcpu->nut_release_key(nutcpu->reg);
    }
    if (pTIMER->usElapsedId(0)>=10000) {
        pTIMER->resetTimer(0);
//        TimerProc();
    }
//    SetKeyDown();
//    qWarning()<<"hp41::run";
    //    if (!fRunEnable)
    //        return;
    //    if (ExecuteEvent)
    //        return;
    //    if (!ProcInterval || !ProcCycles)  // if unintialized
    //        return;

    //  // compute delay for 1 HP41 machine cycle - necessary for correct operation of speaker sound & interval timer
    //  InstrNSec=(UINT64)( ((double)PCPerf.QuadPart*(double)DEFAULT_INST_SPEED/1e6));
    //  WaveSound.SetDuration((DWORD)DEFAULT_INST_SPEED);

    //  // compute delay for 1 HP41 machine cycle - necessary for correct operation of speaker sound & interval timer
    //  // scales delay according to ProcInterval, where the default is the same as the real machine
    //  InstrNSec=(UINT64)( ((double)PCPerf.QuadPart*(double)DEFAULT_INST_SPEED/1e6)*((double)ProcInterval/(double)DEFAULT_PROC_INTERVAL) );
    //  WaveSound.SetDuration( (DWORD)( (double)DEFAULT_INST_SPEED*(double)ProcInterval/(double)DEFAULT_PROC_INTERVAL ) );


    // validate PC_REG - important if a ROM is removed from the config while it is being executed
    uint page=(pCPU->get_PC()&0xf000)>>12;
//    if (PageMatrix[page][active_bank[page]-1]==NULL)           // no rom loaded here
//        pCPU->set_PC(0);

    CpcXXXX::run();


    pTIMER->state+=56;
    return true;

}



UINT8 Chp15c::in(UINT8 address)
{
    return 0;
}

UINT8 Chp15c::out(UINT8 address, UINT8 value)
{
    return 0;
}


/*****************************/
void Chp15c::MemoryLost()
{
    //  if (eKeyboard==eKeyboardNone)
    //    return;
    //BASE=16;
    for (int page=1;page<=0xf;page++)
        active_bank[page]=1;
    pCPU->set_PC(0x0232);
//    ResetTimer();
}


/*****************************/
void Chp15c::EnableRun()
{
    fRunEnable=true;
}


/*****************************/
void Chp15c::DisableRun()
{
    fRunEnable=false;
}



/*****************************/
// stops the callback timer
/*****************************/
void Chp15c::Halt()
  {
//  if (ExecuteEvent)
//    KillTimer(NULL,ExecuteEvent);
  ExecuteEvent=0;
  }


/*****************************/
void Chp15c::SetProcParams(
  int ProcIntervalIn,            // time in milliseconds between processor runs
  int ProcCyclesIn)              // number of processor cycles to run each time
  {
  ProcInterval=ProcIntervalIn;
  ProcCycles=ProcCyclesIn;
  if (ExecuteEvent)              // restart processor with new values
    {
    Halt();
    run();
    }
  }


/*****************************/
void Chp15c::GetProcParams(
  int &ProcIntervalOut,          // time in milliseconds between processor runs
  int &ProcCyclesOut)            // number of processor cycles to run each time
  {
  ProcIntervalOut=ProcInterval;
  ProcCyclesOut=ProcCycles;
  }


/****************************/
void Chp15c::SetSoundMode(int val) {
    //  if (GetVersion()&0x80000000)   // high bit set if win95/98
    //    Speaker(0,1);	             // be sure speaker is off

    //  if (val==eSoundSpeaker)
    //    {
    //    if (PCPerf.QuadPart &&           // 0 if performance counter not supported
    //      (GetVersion()&0x80000000) )    // high order bit set if win95/98/win32s - sound not supported on NT
    //      SoundMode=val;
    //    else
    //      SoundMode=eSoundNone;
    //    }
    //  else
    SoundMode=val;
}


/****************************/
int Chp15c::GetSoundMode() {
    return(SoundMode);
}







/****************************/
// return true if RAM exists at the specified RAM address
/****************************/
bool Chp15c::Chk_Adr(UINT32 *d, UINT32 data)
{
    if ((*d>=0x000)&&(*d<=0x00f))      // status registers
        return(true);
    if ((*d>=0x010)&&(*d<=0x03f))      // void
        return(false);
    if ((*d>=0x040)&&(*d<=0x0bf))      // extended functions - 128 regs
        return(XMemModules>=1);
    if ((*d>=0x0c0)&&(*d<=0xff))       // main memory for C
        return(true);
    if ((*d>=0x100)&&(*d<=0x13f))      // memory module 1
        return(MemModules>=1);
    if ((*d>=0x140)&&(*d<=0x17f))      // memory module 2
        return(MemModules>=2);
    if ((*d>=0x180)&&(*d<=0x1bf))      // memory module 3
        return(MemModules>=3);
    if ((*d>=0x1c0)&&(*d<=0x1ff))      // memory module 4
        return(MemModules>=4);
    // void: 200
    if ((*d>=0x201)&&(*d<=0x2ef))      // extended memory 1 - 239 regs
        return(XMemModules>=2);
    // void: 2f0-300
    if ((*d>=0x301)&&(*d<=0x3ef))      // extended memory 2 - 239 regs
        return(XMemModules>=3);
    // void: 3f0-3ff
    // end of memory: 3ff
    return(false);

}

bool Chp15c::Chk_Adr_R(UINT32 *d, UINT32 *data)
{
}



int Chp15c::RamExist(word addr)
  {
  if ((addr>=0x000)&&(addr<=0x00f))      // status registers
    return(1);
  if ((addr>=0x010)&&(addr<=0x03f))      // void
    return(0);
  if ((addr>=0x040)&&(addr<=0x0bf))      // extended functions - 128 regs
    return(XMemModules>=1);
  if ((addr>=0x0c0)&&(addr<=0xff))       // main memory for C
    return(1);
  if ((addr>=0x100)&&(addr<=0x13f))      // memory module 1
    return(MemModules>=1);
  if ((addr>=0x140)&&(addr<=0x17f))      // memory module 2
    return(MemModules>=2);
  if ((addr>=0x180)&&(addr<=0x1bf))      // memory module 3
    return(MemModules>=3);
  if ((addr>=0x1c0)&&(addr<=0x1ff))      // memory module 4
    return(MemModules>=4);
  // void: 200
  if ((addr>=0x201)&&(addr<=0x2ef))      // extended memory 1 - 239 regs
    return(XMemModules>=2);
  // void: 2f0-300
  if ((addr>=0x301)&&(addr<=0x3ef))      // extended memory 2 - 239 regs
    return(XMemModules>=3);
  // void: 3f0-3ff
  // end of memory: 3ff
  return(0);
  }


/*****************************/
// timer for execution loop
/****************************/
void Chp15c::ExecuteProc() {
#if 0
    for (int i=0;i<pThis->ProcCycles;i++)
    {
        if (pThis->GetSoundMode()==eSoundWave)                              // if wave mode, put must be called continuously to minimize clicks
        {
            static int SkipSound=0;
            if (SkipSound)
                SkipSound--;
            else if (WaveSound.Put(pThis->F_REG)==-1)                         // returns -1 if failed to open
                SkipSound=10000;                                                // skip sound for a while because Put() slows way down while waiting for sound board to free up
        }
        else if (pThis->IsSleeping())
            break;
        if (pThis->GetSoundMode()==eSoundSpeaker || (pThis->TMR_S[2]&0x04)) // override ProcInterval to slow down execution to match real hardware
        {
            QueryPerformanceCounter(&pThis->PCCount[0]);	                    // get current count
            pThis->Execute();                                                 // execute one instruction
            pThis->PCCount[0].QuadPart+=pThis->InstrNSec;
            do
                QueryPerformanceCounter(&pThis->PCCount[1]);
            while (pThis->PCCount[1].QuadPart < pThis->PCCount[0].QuadPart);  // delay until exact amount of time for real machine cycle has passed
        }
        else if (!pThis->IsSleeping())                                      // dont call execute while sleeping because it will execute the next instruction anyway
            pThis->Execute();                                                 // execute one instruction
        if (pThis->Boost && i+pThis->Boost>=pThis->ProcCycles)              // boost this loop for a few more cycles
        {                                                                 // to prevent it from exiting in the middle of heavy display operations
            i-=pThis->Boost;
            pThis->Boost=0;
        }
        if (pThis->fBreak)                                                  // breakpoint has been raised
        {
            theApp.pMainWnd->OpenConsole();
            theApp.pMainWnd->OpenBreakpoints();
            pThis->fBreak=false;
            break;
        }
    }
    if (pThis->Indicator)
        theApp.m_pMainWnd->InvalidateRect(&pThis->RectIndicator,false);     // activity indicator
    if (pThis->UpdateDisplay)
        theApp.m_pMainWnd->InvalidateRect(pThis->pRectLCD,false);           // refresh LCD
    if (pThis->UpdateAnnun)
        theApp.m_pMainWnd->InvalidateRect(pThis->pRectAnnun,false);         // refresh Annunciator
#endif
}





#define KEY(c)	( pKEYB->keyPressedList.contains(TOUPPER(c)) || pKEYB->keyPressedList.contains(c) || pKEYB->keyPressedList.contains(TOLOWER(c)))

UINT8 Chp15c::getKey()
{
    UINT8 code = 0;

    if ( (currentView==FRONTview) && (pKEYB->LastKey))
    {

        if (KEY('A'))			code = 0x13;    // A
        if (KEY('B'))			code = 0x33;    // B
        if (KEY('C'))			code = 0x73;    // C
        if (KEY('D'))			code = 0xC3;    // D
        if (KEY('E'))			code = 0x83;    // E
        if (KEY(K_SIGN))		code = 0x82;    // CHS
        if (KEY('7'))			code = 0xC2;    // 7
        if (KEY('8'))			code = 0x72;    // 8
        if (KEY('9'))			code = 0x32;    // 9
        if (KEY('/'))			code = 0x12;    // /

        if (KEY(K_SST))			code = 0x10;    // SST
        if (KEY(K_GTO))			code = 0x30;    // GTO
        if (KEY(K_SIN))			code = 0x70;    // SIN
        if (KEY(K_COS))			code = 0xC0;    // COS
        if (KEY(K_TAN))			code = 0x80;    // TAN
        if (KEY(K_EE))			code = 0x87;    // EEX
        if (KEY('4'))			code = 0xC7;    // 4
        if (KEY('5'))			code = 0x77;    // 5
        if (KEY('6'))			code = 0x37;    // 6
        if (KEY('*'))			code = 0x17;    // *

        if (KEY(K_RS))			code = 0x11;    // R/S
//        if (KEY(K_GSB))			code = 0x31;    // GSB
//        if (KEY(K_RDA))			code = 0x71;    // R Down
//        if (KEY(K_XY))			code = 0xC1;    // XY
        if (KEY(K_LA))			code = 0x81;    // left arrow
        if (KEY(K_RET))			code = 0x84;    // ENTER
        if (KEY('1'))			code = 0xC4;    // 1
        if (KEY('2'))			code = 0x74;    // 2
        if (KEY('3'))			code = 0x34;    // 3
        if (KEY('-'))			code = 0x14;    // -

        if (KEY(K_SST))			code = 0x18;    // ON
        if (KEY('F'))			code = 0x38;    // f
        if (KEY('G'))			code = 0x78;    // g
        if (KEY(K_STO))			code = 0xC8;    // STO
        if (KEY(K_RCL))			code = 0x88;    // RCL
        if (KEY(K_RET))			code = 0x85;    // ENTER
        if (KEY('0'))			code = 0xC5;    // 0
        if (KEY('.'))			code = 0x75;    // .
        if (KEY(K_SUM))			code = 0x35;    // sigma
        if (KEY('+'))			code = 0x15;    // +

    }

    return code;

}


