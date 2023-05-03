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

#include "fluidlauncher.h"
#include "hp41.h"
#include "hp41Cpu.h"
#include "hp41mod.h"
#include "Lcdc_hp41.h"
#include "Keyb.h"
#include "Inter.h"
#include "watchpoint.h"
#include "Connect.h"
#include "ctronics.h"
#include "clink.h"
#include "bus.h"

#include "mainwindowpockemul.h"
extern MainWindowPockemul *mainwindow;

#include "renderView.h"
extern CrenderView* view;

int Chp41::loadedFontID = -1;

/****************************/
// Constructor
/****************************/
void Chp41::addModule(QString item,CPObject *pPC)
{
    qWarning()<<"Add Module:"<< item;

    disconnect(view,SIGNAL(Launched(QString,CPObject *)),this,SLOT(addModule(QString,CPObject *)));


    if ( (currentSlot<0) || (currentSlot>3)) return;

    ModuleHeader *pModuleNew;

    bool customModule = false;

    int _res = 0;
    QString moduleName;
    if (item=="HP82143A") moduleName = P_RES(":/hp41/MOD/PRINTER.MOD");
    if (item=="HP82160A") moduleName = P_RES(":/hp41/MOD/HPIL.MOD");
    if (item=="HP82153A") moduleName = P_RES(":/hp41/MOD/WAND.MOD");
    if (item=="HP41FORTH") moduleName = P_RES(":/hp41/MOD/FORTH.MOD");
    if (item=="HP41GAMES") moduleName = P_RES(":/hp41/MOD/GAMES.MOD");
    if (item=="HP41HEPAX") moduleName = P_RES(":/hp41/MOD/HEPAX.MOD");
    if (item=="HP41MATH") moduleName = P_RES(":/hp41/MOD/MATH.MOD");
    if (item=="HP41MATHSTAT") moduleName = P_RES(":/hp41/MOD/MATHSTAT.MOD");
    if (item=="HP41ADVANTAGE") moduleName = P_RES(":/hp41/MOD/ADVANTAGE.MOD");
    if (item=="HP41CCD") moduleName = P_RES(":/hp41/MOD/CCD.MOD");
    if (item=="HP41PPC") moduleName = P_RES(":/hp41/MOD/PPC.MOD");
    if (item=="HP41MODFILE") {
        moduleName = QFileDialog::getOpenFileName(
                    mainwindow,
                    tr("Choose a MOD file"),
                    ".",
                    tr("Module File (*.mod)"));
        customModule = true;
        qWarning()<<Chp41Mod(moduleName).output_mod_info(1,1);
    }

    if (moduleName.isEmpty()) return;

    _res=LoadMOD(pModuleNew,moduleName.toLatin1().data());

    qWarning()<<"loaded:"<<_res;
    if (_res == 0) {
        slot[currentSlot].used = true;
        slot[currentSlot].id = pModuleNew->szFullFileName;
        slot[currentSlot].label = pModuleNew->szTitle;
        slot[currentSlot].pModule = pModuleNew;
        slot[currentSlot].custom = customModule;
        slotChanged = true;
        qWarning()<<"module:"<<pModuleNew->szPartNumber;
    }
    else {
        qWarning()<<Chp41Mod(moduleName).output_mod_info(1,1);

    }
    if (pPC) {
        // Link  object with main pObject
        mainwindow->pdirectLink->addLink(pConnector[currentSlot],pPC->ConnList.at(0),false);
        pPC->setPosX(this->posx()+this->width()+15);
        pPC->setPosY(this->posy());
    }
    currentSlot = -1;

}

Chp41::Chp41(CPObject *parent):CpcXXXX(parent)
{

    setfrequency( (int) 6262*56);  // 80µs per cycle
    setcfgfname(QString("hp41"));

    SessionHeader	= "HP41PKM";
    Initial_Session_Fname ="hp41.pkm";

    BackGroundFname	= P_RES(":/hp41/hp41.png");

    TopFname = P_RES(":/hp41/top.png");
    BackFname = P_RES(":/hp41/back.png");
    LeftFname = P_RES(":/hp41/left.png");
    RightFname = P_RES(":/hp41/right.png");


    memsize		= 0x2000;
    InitMemValue	= 0x00;

    SlotList.clear();
        SlotList.append(CSlot(8 , 0x0000 ,"", ""	, CSlot::RAM , "RAM"));

    setDXmm(78);
    setDYmm(142);
    setDZmm(31);

    setDX(279);
    setDY(508);

    pLCDC		= new Clcdc_hp41(this,
                                 QRect(30,33,220,40),
                                 QRect(35,55,215,7));
    pTIMER		= new Ctimer(this);
    pCPU		= new Chp41cpu(this);    hp41cpu = (Chp41cpu*)pCPU;
    pKEYB->setMap("hp41.map");

    fPrinter=fCardReader=fTimer=fWand=fHPIL=fInfrared=-1;
    for (int i=0;i<4;i++) slot[i].used=false;
    slotChanged = false;

    // trace
    hLogFile=NULL;
    InstSetIndex=0;
    TEFIndex=0;
    fTrace=false;
    memset(szTraceOut,0,sizeof(szTraceOut));

    if (loadedFontID == -1) {
        loadedFontID = QFontDatabase::addApplicationFont(":/hp41/41chrset.ttf");
        //    QStringList loadedFontFamilies = QFontDatabase::applicationFontFamilies(loadedFontID);
        //  qWarning()<<loadedFontFamilies;
    }
}


/****************************/
// destructor
/****************************/
Chp41::~Chp41()
  {
  if (GetTrace())
    StopTrace();
  SetSoundMode(eSoundNone);

  DeInitTimer();
  ExecuteEvent=0;

  // free modules and rom pages
  while (!ModuleList.isEmpty()) {
      ModuleHeader *pModule=ModuleList.takeFirst();
      delete pModule;
  }
  for (int page=0;page<=0xf;page++)
    for (int bank=1;bank<=4;bank++)
      FreePage(page,bank);

//  free(pRAM);

//  SetKeyboard(eKeyboardNone,0,0,0);
  }


bool Chp41::init()
{
    //pCPU->logsw = true;



    CpcXXXX::init();

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

    // Connectors
    pConnector[0] = new Cconnector(this,64,0,Cconnector::hp41,"HP-41 Module 0",false,QPoint(715,50));
    publish(pConnector[0]);
    pConnector[1] = new Cconnector(this,64,1,Cconnector::hp41,"HP-41 Module 1",false,QPoint(715,50));
    publish(pConnector[1]);
    pConnector[2] = new Cconnector(this,64,2,Cconnector::hp41,"HP-41 Module 2",false,QPoint(715,50));
    publish(pConnector[2]);
    pConnector[3] = new Cconnector(this,64,3,Cconnector::hp41,"HP-41 Module 3",false,QPoint(715,50));
    publish(pConnector[3]);

    for (int i=0;i<4;i++) {
        bus[i] = new Cbus();
    }
    WatchPoint.remove(this);
    WatchPoint.add(&pConnector_value[0],64,15,this,"HP-41 Module 0");
    WatchPoint.add(&pConnector_value[1],64,15,this,"HP-41 Module 1");
    WatchPoint.add(&pConnector_value[2],64,15,this,"HP-41 Module 2");
    WatchPoint.add(&pConnector_value[3],64,15,this,"HP-41 Module 3");

    // ROM variables
    for (int page=0;page<=0xf;page++)
    {
        for (int bank=1;bank<=4;bank++)
            PageMatrix[page][bank-1]=NULL;
        active_bank[page]=1;
    }
    pCurPage=NULL;
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
    SoundMode=eSoundNone;
    fRunEnable=false;
    fBreakPtsEnable=false;
    fBreak=false;
    nBreakPts=0;

    ModuleHeader *pModuleNew;
//    int nRes=LoadMOD(pModuleNew,P_RES(":/hp41/MOD/NUT-C.MOD"));
    int nRes=LoadMOD(pModuleNew,P_RES(":/hp41/MOD/NUT-CX.MOD"));

    for (int i=0;i<ModuleList.size();i++){
        stdModule.append(ModuleList.at(i)->szFullFileName);
    }
    hp41cpu->set_PC(0);
//    qWarning()<<Chp41Mod(P_RES(QString(":/hp41/MOD/NUT-CX.MOD"))).output_mod_info(1,1);
//    qWarning()<<"Load Module:"<<nRes;

//    StartTrace();

    pTIMER->resetTimer(0);

   return true;
}

extern int ask(QWidget *parent,QString msg,int nbButton);

void Chp41::ComputeKey(KEYEVENT ke, int scancode, QMouseEvent *event)
{
    Q_UNUSED(ke)
    Q_UNUSED(scancode)

    int _slot = -1;
    if (KEY(0x240)) _slot = 0;
    if (KEY(0x241)) _slot = 1;
    if (KEY(0x242)) _slot = 2;
    if (KEY(0x243)) _slot = 3;
    qWarning()<<"ComputKey:"<<_slot;
    pKEYB->removeKey(0x240);
    pKEYB->removeKey(0x241);
    pKEYB->removeKey(0x242);
    pKEYB->removeKey(0x243);
    if (_slot == -1) return;
    int _response = 0;
    if (slot[_slot].used)
        _response=ask(this,"The "+slot[_slot].label+ "is already plugged is this slot. Do you want to unplug it ?",2);

    if (_response == 1) {
        UnloadMOD(slot[_slot].pModule);
        slot[_slot].used = false;
        slotChanged = true;
    }
    if (_response==2) return;

    if (_slot>=0) {
        currentSlot = _slot;
#if 1
        connect(view,SIGNAL(Launched(QString,CPObject *)),this,SLOT(addModule(QString,CPObject *)));
        view->pickExtensionConnector("hp41");
#else
        FluidLauncher *launcher = new FluidLauncher(mainwindow,
                                     QStringList()<<P_RES(":/pockemul/configExt.xml"),
                                     FluidLauncher::PictureFlowType,QString(),
                                     "hp41");
        connect(launcher,SIGNAL(Launched(QString,CPObject *)),this,SLOT(addModule(QString,CPObject *)));
        launcher->show();
#endif
    }
}



void Chp41::TurnOFF()
{
    ASKYN _save = mainwindow->saveAll;
    mainwindow->saveAll = YES;
    CpcXXXX::TurnOFF();
    mainwindow->saveAll = _save;
}

bool Chp41::run()
{


    if (pTIMER->usElapsedId(0)>=10000) {
        pTIMER->resetTimer(0);
        TimerProc();
    }
    SetKeyDown();
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

    fillSoundBuffer((quint8)hp41cpu->r->F_REG);

    // validate PC_REG - important if a ROM is removed from the config while it is being executed
    uint page=(pCPU->get_PC()&0xf000)>>12;
    if (PageMatrix[page][active_bank[page]-1]==NULL)           // no rom loaded here
        pCPU->set_PC(0);

    CpcXXXX::run();



    pTIMER->state+=56;
    return true;

}



UINT8 Chp41::in(UINT8 address, QString sender)
{
    return 0;
}

UINT8 Chp41::out(UINT8 address, UINT8 value,QString)
{
    switch (address)
    {
    case 0: case 1: case 2: case 3: case 4: case 5: case 6: case 7:
        // HP-IL
        exec_perph_hpil();
        break;
    case 9:    // printer
        exec_perph_printer();
        break;
    default:
//        error_message(30);
        break;
    }
    return 0;
}


/*****************************/
void Chp41::MemoryLost()
{
    //  if (eKeyboard==eKeyboardNone)
    //    return;
    //BASE=16;
    for (int page=1;page<=0xf;page++)
        active_bank[page]=1;
    pCPU->set_PC(0x0232);
    ResetTimer();
}


/*****************************/
void Chp41::EnableRun()
{
    fRunEnable=true;
}


/*****************************/
void Chp41::DisableRun()
{
    fRunEnable=false;
}



/*****************************/
// stops the callback timer
/*****************************/
void Chp41::Halt()
  {
//  if (ExecuteEvent)
//    KillTimer(NULL,ExecuteEvent);
  ExecuteEvent=0;
  }


/*****************************/
void Chp41::SetProcParams(
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
void Chp41::GetProcParams(
  int &ProcIntervalOut,          // time in milliseconds between processor runs
  int &ProcCyclesOut)            // number of processor cycles to run each time
  {
  ProcIntervalOut=ProcInterval;
  ProcCyclesOut=ProcCycles;
  }


/****************************/
void Chp41::SetSoundMode(int val) {
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
int Chp41::GetSoundMode() {
    return(SoundMode);
}







/****************************/
// return true if RAM exists at the specified RAM address
/****************************/
bool Chp41::Chk_Adr(UINT32 *d, UINT32 data)
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

bool Chp41::Chk_Adr_R(UINT32 *d, UINT32 *data)
{
    return true;
}



int Chp41::RamExist(word addr)
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
void Chp41::ExecuteProc() {
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

void Chp41::exec_perph_hpil(void) {

}

/****************************/
// executes intelligent printer instructions
/****************************/


void Chp41::exec_perph_printer(void)
{


    hp41cpu->r->CARRY=0;
//      qWarning()<<"exec_perph_printer:"<<fPrinter;
    if (PageMatrix[6][0]==NULL)  // if no printer ROM
        return;
    //  qWarning()<<QString("exec_perph_printer:%1").arg(hp41cpu->Tyte1,2,16,QChar('0'));

    switch(hp41cpu->Tyte1)
    {
    case 0x003:       /* ?XF 0 or BUSY? */
    {
        hp41cpu->r->CARRY = 0;
        break;
    }
    case 0x083:       /* ?XF 2 or ERROR? */
    {
        hp41cpu->r->CARRY = 1;
        break;
    }
    case 0x043:       /* ?XF 1 or POWON? */
    {
        hp41cpu->r->CARRY = 1;
        break;
    }
    case 0x007:       /* PRshort or BUF=BUF+C */
    {
        if ( (fPrinter<0) || (fPrinter >3)) break;
              qWarning() << QString("print:%1").arg(((hp41cpu->r->C_REG[1] << 4) | hp41cpu->r->C_REG[0] ),4,16,QChar('0'));
        quint8 c= (hp41cpu->r->C_REG[1] << 4) | hp41cpu->r->C_REG[0];
//        qWarning()<<"print:"<<((hp41cpu->r->C_REG[1] << 4) | hp41cpu->r->C_REG[0]);
        bus[fPrinter]->setWrite(true);
        bus[fPrinter]->setData(c);
        bus[fPrinter]->setEnable(true);
        manageBus();
        bus[fPrinter]->setEnable(false);
        break;
    }
    case 0x03a:       /* RPREG 0 or C=STATUS */
    {
        if ( (fPrinter<0) || (fPrinter >3)) break;

        bus[fPrinter]->setWrite(false);
        bus[fPrinter]->setAddr(0x3a);
        bus[fPrinter]->setEnable(true);
        manageBus();
        quint8 ret1 = bus[fPrinter]->getData();
//        qWarning()<<"Get Printer Status1:"<<ret1;
        bus[fPrinter]->setWrite(false);
        bus[fPrinter]->setAddr(0x3b);
        manageBus();
        quint8 ret2 = bus[fPrinter]->getData();
        bus[fPrinter]->setEnable(false);
//        qWarning()<<"Get Printer Status2:"<<ret2;

        UINT16 PRINT_STATUS= (ret1<<8) | ret2;
//        qWarning()<<"Get Printer Status:"<<PRINT_STATUS;

        memset(hp41cpu->r->C_REG,0,sizeof(hp41cpu->r->C_REG));
        hp41cpu->r->C_REG[13]=(My_byte)((PRINT_STATUS>>12)&0x0f);
        hp41cpu->r->C_REG[12]=(My_byte)((PRINT_STATUS>>8)&0x0f);
        hp41cpu->r->C_REG[11]=(My_byte)((PRINT_STATUS>>4)&0x0f);
        hp41cpu->r->C_REG[10]=(My_byte)( PRINT_STATUS&0x0f);
        break;
    }
    case 0x005:       /* RTNCPU or WPREG 01, RTN */
    {
        break;
    }
    default:
    {
        LOG(30);
        break;
    }
    }
}





/****************************/
// _inp() and _outp() do not work on WinNT
// by Zharkoi Oleg
/****************************/
void Chp41::Speaker(short Freq, int Duration)
{
#if 0
    ulong Count;
    My_byte off,on;
    My_byte spk;

    spk=_inp(0x61);      // get current status
    off=spk&0xFC;
    on=spk|0x03;         // turn on bits 0 and 1

    if (Freq!=0)
    {
        Count=(unsigned long)(PCPerf.QuadPart/Freq);  // determine the timer frequency
        if (!(spk&0x03))
        {
            _outp(0x61,on);          // turn on the speaker
            _outp(0x43,0xB6);        // set up the timer
        }
        _outp(0x42,Count&0xff);
        _outp(0x42,Count>>8);
    }
    else
    {
        _outp(0x61,off);           // turn off the speaker
    }
#endif
}


void Chp41::SetKeyDown(My_byte KeyCode) {

    if (pKEYB->LastKey)
        fEnableCLRKEY=false;           // disable CLRKEY instruction
    else {
        fEnableCLRKEY=true;
        MinCLRKEY=3;                   // the key will be held down for this minimum number of CLRKEY instructions to avoid debounce code
    }
  hp41cpu->r->KEY_REG= KeyCode ? KeyCode : getKey();
  if (hp41cpu->r->KEY_REG) {
      hp41cpu->r->KEYDOWN=true;
  }
//  if ( (eLightSleep==IsSleeping()) ||                 // light sleep and any key pressed or
//    ((eDeepSleep==IsSleeping())&&(KeyCode==0x18)) )   // deep sleep and ON key pressed
//    Wakeup();


  }

/**********************************/
// Simulates a key release
/**********************************/
void Chp41::SetKeyUp()
  {
//  if (eKeyboard==eKeyboardNone)
//    return;
  fEnableCLRKEY=true;
  }

UINT8 Chp41::getKey()
{
    UINT8 code = 0;

    if ( (currentView==FRONTview) && (pKEYB->LastKey))
    {
        if (KEY(K_OF))			code = 0x18;
        if (KEY(K_F1))			code = 0x18;
        if (KEY(K_F2))			code = 0xC6;
        if (KEY(K_F3))			code = 0xC5;
        if (KEY(K_F4))			code = 0xC4;

        if (KEY('A'))			code = 0x10;
        if (KEY('B'))			code = 0x30;
        if (KEY('C'))			code = 0x70;
        if (KEY('D'))			code = 0x80;
        if (KEY('E'))			code = 0xC0;
        if (KEY('F'))			code = 0x11;
        if (KEY('G'))			code = 0x31;
        if (KEY('H'))			code = 0x71;
        if (KEY('I'))			code = 0x81;
        if (KEY('J'))			code = 0xC1;
        if (KEY(K_SHT))			code = 0x12;
        if (KEY('K'))			code = 0x32;
        if (KEY('L'))			code = 0x72;
        if (KEY('M'))			code = 0x82;
        if (KEY(K_SST))			code = 0xC2;
        if (KEY('N'))			code = 0x13;
        if (KEY('O'))			code = 0x73;
        if (KEY('P'))			code = 0x83;
        if (KEY(K_LA))			code = 0xC3;
        if (KEY('Q'))			code = 0x14;
        if (KEY('R'))			code = 0x34;
        if (KEY('S'))			code = 0x74;
        if (KEY('T'))			code = 0x84;
        if (KEY('U'))			code = 0x15;
        if (KEY('V'))			code = 0x35;
        if (KEY('W'))			code = 0x75;
        if (KEY('X'))			code = 0x85;
        if (KEY('Y'))			code = 0x16;
        if (KEY('Z'))			code = 0x36;
        if (KEY('='))			code = 0x76;
        if (KEY('?'))			code = 0x86;

        if (KEY(':'))			code = 0x17;
        if (KEY(' '))			code = 0x37;
        if (KEY('.'))			code = 0x77;
        if (KEY(K_RS))			code = 0x87;

    }

    return code;

}

bool Chp41::Get_Connector(Cbus *_bus) {
    for (int i=0;i<4;i++) {
        bus[i]->fromUInt64(pConnector[i]->Get_values());
    }
    return true;
}

bool Chp41::Set_Connector(Cbus *_bus) {

    for (int i=0;i<4;i++) {
        pConnector[i]->Set_values(bus[i]->toUInt64());
    }
    return true;
}

bool Chp41::InitDisplay(void)
{

    CpcXXXX::InitDisplay();
    slotChanged = true;

    return(1);
}

bool Chp41::UpdateFinalImage(void) {

    CpcXXXX::UpdateFinalImage();


    // on TOP view, draw installed modules
    if ((currentView == TOPview) && slotChanged) {
        InitDisplay();
        slotChanged = false;
        QPainter painter;
        painter.begin(TopImage);
//        qWarning()<<"UpdateFinalImage:";

        QRect slotLabelPos[4];
        slotLabelPos[0] = QRect(25,8,100,10);
        slotLabelPos[1] = QRect(146,8,100,10);
        slotLabelPos[2] = QRect(25,35,100,10);
        slotLabelPos[3] = QRect(146,35,100,10);

        QFont font;
//        font.setFamily("HP41 Character Set");
        font.setPixelSize(10);
        painter.setFont(font);
        painter.setPen(QColor("white"));
        for (int i=0;i<4;i++) {
            if (slot[i].used) {
                // draw label
//                qWarning()<<"UpdateFinalImage:"<<i<<"="<<slot[i].label;
//                painter.setPen(QColor("white"));
                QImage *lbl = new QImage(slotLabelPos[0].size(),QImage::Format_ARGB32);
                lbl->fill(QColor(0,0,0,0));
                QPainter paintLbl(lbl);
                paintLbl.setPen(QColor("white"));
                paintLbl.drawText(QRect(QPoint(0,0),slotLabelPos[0].size()),Qt::AlignCenter,slot[i].label);
                paintLbl.end();
                painter.drawImage(slotLabelPos[i].x(),slotLabelPos[i].y(),lbl->mirrored(true));
            }
        }
        painter.end();
    }

    emit updatedPObject(this);
    return true;
}
