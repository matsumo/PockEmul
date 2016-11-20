

#include <qglobal.h>
#if QT_VERSION >= 0x050000
// Qt5 code
#   include <QtWidgets>
#else
// Qt4 code
#   include <QtCore>
#   include <QtGui>
#endif

#include	"common.h"
#include    "fluidlauncher.h"
#include	"pc1600.h"
#include    "z80.h"
#include    "Lcdc_pc1600.h"
#include	"Inter.h"
#include	"Keyb.h"
#include    "Keyb1600.h"
#include	"ce152.h"
#include	"dialoganalog.h"
#include    "buspc1500.h"
#include    "watchpoint.h"


Cpc1600::Cpc1600(CPObject *parent)	: CpcXXXX(parent)
{								//[constructor]
#ifndef QT_NO_DEBUG
    if (!fp_log) fp_log=fopen("pc1600.log","wt");	// Open log file
#endif
if (!fp_log) fp_log=fopen("pc1600.log","wt");	// Open log file

    setfrequency( (int) 3500000);
    ioFreq = 20000;
    setcfgfname(QString("pc1600"));

    SessionHeader	= "PC1600PKM";
    Initial_Session_Fname ="pc1600.pkm";
    BackGroundFname	= P_RES(":/pc1600/pc-1600.png");

    LeftFname   = P_RES(":/pc1600/pc1600Left.png");
    RightFname  = P_RES(":/pc1600/pc1600Right.png");
    BackFname   = P_RES(":/pc1600/pc1600Back.png");
    TopFname    = P_RES(":/pc1600/pc1600Top.png");
    BottomFname = P_RES(":/pc1600/pc1600Bottom.png");

    setDXmm(195);
    setDYmm(86);
    setDZmm(25);

    setDX(679);
    setDY(299);

    SoundOn			= false;
    lh5810_Access	= false;
    ce150_Access	= false;

    pLCDC		= new Clcdc_pc1600(this,
                                   QRect(60,50,312,64),
                                   QRect(60,40,317,5));

    pZ80        = new CZ80(this);
    pLH5803     = new CLH5803(this);

    pCPU		= pZ80;
    masterCPU   = true;
    cpuSwitchPending = false;

    pLU57813P   = new CLU57813P(this);
    pLH5810		= new CLH5810_PC1600(this);
    pTIMER		= new Ctimer(this);
    pKEYB->setMap("pc1600.map");
    pHD61102_1  = new CHD61102(this);
    pHD61102_2  = new CHD61102(this);
    pTC8576P    = new CTC8576P(this,1288800);

    bus = new CbusPc1500();

    Tape_Base_Freq=2500;

    initExtension();
    extensionArray[0] = ext_60pins;
    extensionArray[1] = ext_MemSlot1;
    extensionArray[2] = ext_MemSlot2;

    memsize			= 0x0E0000;
    InitMemValue	= 0x00;

    SlotList.clear();

    // Bank 0
    SlotList.append(CSlot(16, 0x00000 ,	P_RES(":/pc1600/romI-0.bin")	, "" , CSlot::ROM , "ROM"));
    SlotList.append(CSlot(16, 0x04000 ,	P_RES(":/pc1600/romII-0.bin")	, "" , CSlot::ROM , "ROM"));
    SlotList.append(CSlot(16, 0x08000 ,	""								, "" , CSlot::RAM , "RAM"));
    SlotList.append(CSlot(16, 0x0C000 ,	""								, "" , CSlot::RAM , "RAM"));

    // Bank 1
    SlotList.append(CSlot(16, 0x10000 ,	""                              , "" , CSlot::NOT_USED , "NOT USED"));
    SlotList.append(CSlot(16, 0x14000 ,	""                              , "" , CSlot::NOT_USED , "NOT USED"));
    SlotList.append(CSlot(16, 0x18000 ,	""								, "" , CSlot::RAM , "RAM"));
    SlotList.append(CSlot(16, 0x1C000 ,	""								, "" , CSlot::NOT_USED , "NOT USED"));

    // Bank 2
    SlotList.append(CSlot(16, 0x20000 ,	""                              , "" , CSlot::NOT_USED , "NOT USED"));
    SlotList.append(CSlot(16, 0x24000 ,	""                              , "" , CSlot::NOT_USED , "NOT USED"));
    SlotList.append(CSlot(16, 0x28000 ,	""								, "" , CSlot::RAM , "RAM"));
    SlotList.append(CSlot(16, 0x2C000 ,	""								, "" , CSlot::NOT_USED , "NOT USED"));

    // Bank 3
    SlotList.append(CSlot(16, 0x30000 ,	P_RES(":/pc1600/rom3b.bin" )    , "" , CSlot::ROM , "ROM 3b"));
    SlotList.append(CSlot(16, 0x34000 ,	P_RES(":/pc1600/romIII-3.bin")	, "" , CSlot::ROM , "ROM"));
    SlotList.append(CSlot(16, 0x38000 ,	""								, "" , CSlot::RAM , "RAM"));
    SlotList.append(CSlot(16, 0x3C000 ,	""								, "" , CSlot::NOT_USED , "NOT USED"));

    // Bank 4
    SlotList.append(CSlot(16, 0x40000 ,	""                              , "" , CSlot::NOT_USED , "NOT USED"));
    SlotList.append(CSlot(16, 0x44000 , ""                              , "" , CSlot::NOT_USED , "NOT USED"));
    SlotList.append(CSlot(16, 0x48000 ,	""								, "" , CSlot::NOT_USED , "NOT USED"));
    SlotList.append(CSlot(16, 0x4C000 ,	""								, "" , CSlot::NOT_USED , "NOT USED"));

    // Bank 5
    SlotList.append(CSlot(16, 0x50000 ,	""                              , "" , CSlot::NOT_USED , "NOT USED"));
    SlotList.append(CSlot(16, 0x54000 , ""                              , "" , CSlot::NOT_USED , "NOT USED"));
    SlotList.append(CSlot(16, 0x58000 ,	""								, "" , CSlot::NOT_USED , "NOT USED"));
    SlotList.append(CSlot(16, 0x5C000 ,	""								, "" , CSlot::NOT_USED , "NOT USED"));

    // Bank 6
    SlotList.append(CSlot(16, 0x60000 ,	""                              , "" , CSlot::NOT_USED , "NOT USED"));
    SlotList.append(CSlot(16, 0x64000 ,	""                              , "" , CSlot::NOT_USED , "NOT USED"));
    SlotList.append(CSlot(16, 0x68000 ,	P_RES(":/pc1600/romIV-6.bin")	, "" , CSlot::ROM , "ROM"));
    SlotList.append(CSlot(16, 0x6C000 ,	""								, "" , CSlot::NOT_USED , "NOT USED"));

    // Initialize bank nummber
    bank1 = bank2 = bank3 = bank4 = 0;

    // MOD 1 MAP
    SlotList.append(CSlot(8 , 0x70000 ,	""								, "" , CSlot::RAM , "RAM"));
    SlotList.append(CSlot(8 , 0x72000 ,	""								, "" , CSlot::ROM , "ROM"));
    SlotList.append(CSlot(16, 0x74000 ,	""								, "" , CSlot::RAM , "RAM"));
    SlotList.append(CSlot(8 , 0x78000 ,	""								, "" , CSlot::NOT_USED , "NOT USED"));
    SlotList.append(CSlot(8 , 0x7A000 ,	""								, "" , CSlot::ROM , "ROM"));
    SlotList.append(CSlot(16, 0x7C000 ,	P_RES(":/pc1600/rom1500.bin")	, "" , CSlot::ROM , "SYSTEM ROM"));
    SlotList.append(CSlot(64, 0x80000 ,	""								, "" , CSlot::RAM , "RAM"));
    SlotList.append(CSlot(8 , 0x90000 ,	""								, "" , CSlot::ROM , "ROM"));
    SlotList.append(CSlot(8 , 0x92000 ,	""								, "" , CSlot::ROM , "ROM"));
    SlotList.append(CSlot(8 , 0x94000 ,	P_RES(":/pc1500/CE-150.ROM")	, "" , CSlot::ROM , "CE-150 ROM"));

    // RAM BANK
    SlotList.append(CSlot(16, 0xA0000 ,	""								, "" , CSlot::RAM , "RAM"));
    SlotList.append(CSlot(16, 0xA4000 ,	""								, "" , CSlot::RAM , "RAM"));
    SlotList.append(CSlot(16, 0xA8000 ,	""								, "" , CSlot::RAM , "RAM"));
    SlotList.append(CSlot(16, 0xAC000 ,	""								, "" , CSlot::RAM , "RAM"));
    SlotList.append(CSlot(16, 0xB0000 ,	""								, "" , CSlot::RAM , "RAM"));
    SlotList.append(CSlot(16, 0xB4000 ,	""								, "" , CSlot::RAM , "RAM"));
    SlotList.append(CSlot(16, 0xB8000 ,	""								, "" , CSlot::RAM , "RAM"));
    SlotList.append(CSlot(16, 0xBC000 ,	""								, "" , CSlot::RAM , "RAM"));
    SlotList.append(CSlot(16, 0xC0000 ,	""								, "" , CSlot::RAM , "RAM"));
    SlotList.append(CSlot(16, 0xC4000 ,	""								, "" , CSlot::RAM , "RAM"));
    SlotList.append(CSlot(16, 0xC8000 ,	""								, "" , CSlot::RAM , "RAM"));
    SlotList.append(CSlot(16, 0xCC000 ,	""								, "" , CSlot::RAM , "RAM"));
    SlotList.append(CSlot(16, 0xD0000 ,	""								, "" , CSlot::RAM , "RAM"));
    SlotList.append(CSlot(16, 0xD4000 ,	""								, "" , CSlot::RAM , "RAM"));
    SlotList.append(CSlot(16, 0xD8000 ,	""								, "" , CSlot::RAM , "RAM"));
    SlotList.append(CSlot(16, 0xDC000 ,	""								, "" , CSlot::RAM , "RAM"));

}

Cpc1600::~Cpc1600()
{
    delete pLH5810;
    delete pHD61102_1;
    delete pHD61102_2;
    delete pLU57813P;

}

void Cpc1600::Reset(void)
{
    bank1 = bank2 = bank3 = bank4 = 0;
    masterCPU = true;
    cpuSwitchPending = false;
    pCPU = pZ80;
    pCPU->halt = false;
    pZ80->Reset();
    masterCPU = false;
    pLH5803->Reset();
    masterCPU = true;
    pLU57813P->Reset();
    pLH5810->Reset();
//    CpcXXXX::Reset();
}

void Cpc1600::TurnON(void)
{
    AddLog(LOG_FUNC,"Cpc1600::TurnOn");
    if (pKEYB->LastKey == 0) pKEYB->LastKey = K_POW_ON;
    qWarning()<<"power="<<getPower();
    if ( (pKEYB->LastKey == K_POW_ON) ||
         (!getPower() && pKEYB->LastKey == K_OF) ||
         (!getPower() && pKEYB->LastKey == K_BRK))
    {
         qWarning()<<"power ON";
        AddLog(LOG_MASTER,"Power ON");
        if (!hardreset) {
//            Initial_Session_Load();
        }
        else hardreset = false;
        off = 0;
        setPower(true);
        PowerSwitch = PS_RUN;
        pHD61102_1->updated = pHD61102_2->updated = true;
//        Reset();
        if (pLCDC) pLCDC->TurnON();
    }
    //remove(Initial_Session_Fname.toStdString().c_str());
    //pCPU->Reset();
}

void Cpc1600::TurnOFF()
{
    mainwindow->saveAll = NO;
    CpcXXXX::TurnOFF();
}

void Cpc1600::initExtension(void)
{
    // initialise ext_MemSlot1  S1
    ext_MemSlot1 = new CExtensionArray("Memory Slot 1 (S1)","Add memory module");
    ext_MemSlot1->setAvailable(ID_CE151,true,false);
    ext_MemSlot1->setAvailable(ID_CE155,true,false);
    ext_MemSlot1->setAvailable(ID_CE159,true,false);
    ext_MemSlot1->setAvailable(ID_CE161,true,false);
    ext_MemSlot1->setAvailable(ID_CE1600M,true,false);
    ext_MemSlot1->setAvailable(ID_CE1601M,true,false);
    addExtMenu(ext_MemSlot1);

    // initialise ext_MemSlot2  S2
    ext_MemSlot2 = new CExtensionArray("Memory Slot 2 (S2)","Add memory module");
    ext_MemSlot2->setAvailable(ID_CE161,true,false);
    ext_MemSlot2->setAvailable(ID_CE1600M,true,false);
    ext_MemSlot2->setAvailable(ID_CE1601M,true,false);
    ext_MemSlot2->setAvailable(ID_CE16096,true,false);
    ext_MemSlot2->setAvailable(ID_CE16128,true,false);
    ext_MemSlot2->setAvailable(ID_CE16160,true,false);
    ext_MemSlot2->setAvailable(ID_CE16192,true,false);
    ext_MemSlot2->setAvailable(ID_CE16224,true,false);
    ext_MemSlot2->setAvailable(ID_CE16256,true,true);

    addExtMenu(ext_MemSlot2);
}

bool Cpc1600::CompleteDisplay(void)
{
    QPainter painter;

//	AddLog(LOG_FUNC,"Cpc1600::CompleteDisplay");

    CpcXXXX::CompleteDisplay();

    return true;
}

bool Cpc1600::InitDisplay(void)
{
    AddLog(LOG_FUNC,"Cpc1600::InitDisplay");

    CpcXXXX::InitDisplay();

    return(1);
}

bool Cpc1600::LoadConfig(QXmlStreamReader *xmlIn)
{
    if (xmlIn->readNextStartElement()) {
        if (xmlIn->name() == "config" && xmlIn->attributes().value("version") == "1.0") {
            if (xmlIn->readNextStartElement() && xmlIn->name() == "internal" ) {
                bank1 = xmlIn->attributes().value("bank1").toString().toInt(0,16);
                bank2 = xmlIn->attributes().value("bank2").toString().toInt(0,16);
                bank3 = xmlIn->attributes().value("bank3").toString().toInt(0,16);
                bank4 = xmlIn->attributes().value("bank4").toString().toInt(0,16);
                cpuSwitchPending = xmlIn->attributes().value("cpuSwitchPending").toString().toInt(0,16);
                masterCPU = xmlIn->attributes().value("masterCPU").toString().toInt(0,16);
                pCPU = (masterCPU ? (CCPU *)pZ80 : (CCPU *)pLH5803);
                xmlIn->skipCurrentElement();
            }
        }
        xmlIn->skipCurrentElement();
    }
    pZ80->Load_Internal(xmlIn);
    pLH5803->Load_Internal(xmlIn);
    pLH5810->Load_Internal(xmlIn);
    pHD61102_1->Load_Internal(xmlIn);
    pHD61102_2->Load_Internal(xmlIn);
    pLU57813P->Load_Internal(xmlIn);
    pTC8576P->Load_Internal(xmlIn);
    return true;
}

bool Cpc1600::SaveConfig(QXmlStreamWriter *xmlOut)
{
    xmlOut->writeStartElement("config");
    xmlOut->writeAttribute("version", "1.0");
        xmlOut->writeStartElement("internal");
            xmlOut->writeAttribute("bank1",QString("%1").arg(bank1,2,16));
            xmlOut->writeAttribute("bank2",QString("%1").arg(bank2,2,16));
            xmlOut->writeAttribute("bank3",QString("%1").arg(bank3,2,16));
            xmlOut->writeAttribute("bank4",QString("%1").arg(bank4,2,16));
            xmlOut->writeAttribute("cpuSwitchPending",QString("%1").arg(cpuSwitchPending,2,16));
            xmlOut->writeAttribute("masterCPU",QString("%1").arg(masterCPU,2,16));
        xmlOut->writeEndElement();
    xmlOut->writeEndElement();

    pZ80->save_internal(xmlOut);
    pLH5803->save_internal(xmlOut);
    pLH5810->save_internal(xmlOut);
    pHD61102_1->save_internal(xmlOut);
    pHD61102_2->save_internal(xmlOut);
    pLU57813P->save_internal(xmlOut);
    pTC8576P->save_internal(xmlOut);

    return true;
}

bool Cpc1600::init(void)				// initialize
{
    fp_CRVA = 0;
    // if DEBUG then log CPU

//    pZ80->logsw = true;
#ifndef QT_NO_DEBUG
//    pZ80->logsw = true;
#endif
    CpcXXXX::init();

    masterCPU = false;
    pCPU = pLH5803;

    // if DEBUG then log CPU
#ifndef QT_NO_DEBUG
//    pCPU->logsw = true;
#endif
//    pCPU->logsw = true;

    pCPU->init();
    masterCPU=true;
    pCPU = pZ80;

    pLU57813P->init();
    pLH5810->init();
    pHD61102_1->init();
    pHD61102_2->init();

    pCONNECTOR	= new Cconnector(this,60,0,Cconnector::Sharp_60,"Connector 60 pins",false,QPoint(0,60));	publish(pCONNECTOR);
    pSIOCONNECTOR=new Cconnector(this,15,1,Cconnector::Sharp_15,"Connector 15 pins",false,QPoint(679,190));	publish(pSIOCONNECTOR);
    pADCONNECTOR= new Cconnector(this,8,2,Cconnector::Jack,"Digital connector 2 pins",false,QPoint(679,20));	publish(pADCONNECTOR);

    WatchPoint.remove(this);
    WatchPoint.add(&pCONNECTOR_value,64,60,this,"Standard 60pins connector");
    WatchPoint.add((qint64 *) &(pLH5810->lh5810.r_opa),8,8,this,"LH5810 Port A");
    WatchPoint.add((qint64 *) &(pLH5810->lh5810.r_opb),8,8,this,"LH5810 Port B");
    WatchPoint.add((qint64 *) &(pLH5810->lh5810.r_opc),8,8,this,"LH5810 Port C");

//    QMessageBox::about(this, tr("Attention"),"PC-1600 Emulation is in alpha stage.");

    return true;
}

bool Cpc1600::run(void)
{
//    UINT32 previous_pc;

//    previous_pc = pCPU->get_PC();

// ---------------------------------------------------------

    CpcXXXX::run();

    if (pCPU->halt) {
        qWarning()<<"halt";
    }
    if (off) {
//        qWarning()<<"off";
        return true;
    }

    if (cpuSwitchPending)
    {
//        qWarning()<<"switching";
        if (masterCPU)
        {
            if (pZ80->z80.r.halt) {
                // Switch CPU
//                qWarning()<<"switch";
                if (pCPU->fp_log) fprintf(pCPU->fp_log,"\nSWITCH CPU\n\n");
                //pLH5803->lh5801.HLT = 0;
                pCPU = pLH5803;
                cpuSwitchPending = false;
                masterCPU = false;
                memcpy((char*) &mem[0x8A30],(char*)&pZ80->imem[0x30],0x10);
            }
        }
        else
        {
            if (pCPU->fp_log) fprintf(pCPU->fp_log,"\nSWITCH CPU\n\n");
            //pLH5803->lh5801.HLT = 1;
            pCPU = pZ80;
            pZ80->z80.r.halt = false;
            cpuSwitchPending = false;
            masterCPU = true;
            // Copy 0x30-0x3f to 1A030-1A03F
            memcpy((char*)&pZ80->imem[0x30],(char*) &mem[0x8A30],0x10);
        }
    }

// ---------------------------------------------------------


//hack(Current_PC);
#ifndef QT_NO_DEBUG
    UINT32 Current_PC = pCPU->get_PC();
//    hack(Current_PC);
#endif

    //----------------------------------
    // SOUND BUFFER (quite simple no?)
    //----------------------------------

    pLH5810->SetRegBit(CLH5810::OPC,6,pLH5810->GetReg(CLH5810::OPC) & 0x80);

    fillSoundBuffer((pLH5810->GetReg(CLH5810::OPC) & 0x40 ? 0x00 : 0xff));
    //----------------------------------

    // 1/64s and 1/2s interrupt
    PUT_BIT(pCPU->imem[0x32],4,pTIMER->GetTP( pLU57813P->Get_tpIndex64()));
    bool tips = pTIMER->GetTP( pLU57813P->Get_tpIndex2());

    if (READ_BIT(pCPU->imem[0x32],6) != tips) {
        PUT_BIT(pCPU->imem[0x32],6,tips);
        pLU57813P->step();
    }
    if (pLU57813P->Get_Kon()) { pLH5810->step(); }

//    pTC8576P->step();

    return true;
}

INLINE void Cpc1600::hack(UINT32 pc)
{
    // HACK Program Counter
    //#define FUNC_CALL(ADDR,LIB) case ADDR: AddLog(LOG_ROM,tr(LIB).arg(((CLH5801 *)pCPU)->get_PC(),5,16,QChar('0')));
    #define FUNC_CALL(BANK,ADDR,LIB) \
        case ADDR: { \
            if ( ((ADDR >= 0x0000) && (ADDR <= 0x3FFF) && (BANK == bank1)) || \
                 ((ADDR >= 0x4000) && (ADDR <= 0x7FFF) && (BANK == bank2)) || \
                 ((ADDR >= 0x8000) && (ADDR <= 0xBFFF) && (BANK == bank3)) || \
                 ((ADDR >= 0xC000) && (ADDR <= 0xFFFF) && (BANK == bank4)) ) {\
            /*if (fp_log) fprintf(fp_log,tr(LIB).arg(((CLH5801 *)pCPU)->get_PC(),5,16,QChar('0')).toStdString().c_str(),pZ80->z80.r.c);*/ \
            /*if (pCPU->fp_log) fprintf(pCPU->fp_log,tr(LIB).arg(((CLH5801 *)pCPU)->get_PC(),5,16,QChar('0')).toStdString().c_str(),pZ80->z80.r.c);*/ \
            }\
        }

    #define FUNC_CALL_C(BANK,ADDR,C,LIB) \
        case C: { \
            if ( ((ADDR >= 0x0000) && (ADDR <= 0x3FFF) && (BANK == bank1)) || \
                 ((ADDR >= 0x4000) && (ADDR <= 0x7FFF) && (BANK == bank2)) || \
                 ((ADDR >= 0x8000) && (ADDR <= 0xBFFF) && (BANK == bank3)) || \
                 ((ADDR >= 0xC000) && (ADDR <= 0xFFFF) && (BANK == bank4)) ) {\
            /*if (fp_log) fprintf(fp_log,tr(LIB).arg(((CLH5801 *)pCPU)->get_PC(),5,16,QChar('0')).toStdString().c_str(),pZ80->z80.r.c);*/ \
            /*if (pCPU->fp_log) fprintf(pCPU->fp_log,tr(LIB).arg(((CLH5801 *)pCPU)->get_PC(),5,16,QChar('0')).toStdString().c_str(),pZ80->z80.r.c);*/ \
            }\
        }

//    if (fp_log == 0) return;

//    if (fp_log) fprintf(fp_log,tr("[%1] - PRTANK  - Display one character\n").arg(((CLH5801 *)pCPU)->get_PC(),5,16,QChar('0')).toStdString().c_str(),pZ80->z80.r.c);
    switch (pc)
    {

    FUNC_CALL(0,0x0100,QT_TR_NOOP("[%1] - PRTANK  - Display one character\n"));             break;
    FUNC_CALL(0,0x00EB,QT_TR_NOOP("[%1] - PRTASTR - Display a string of characters\n"));    break;
    FUNC_CALL(0,0x0115,QT_TR_NOOP("[%1] - CRSRSET - Set the cursor position\n"));           break;
    FUNC_CALL(0,0x0118,QT_TR_NOOP("[%1] - CRSRPOS - Read the current cursor position\n"));  break;
    FUNC_CALL(0,0x011B,QT_TR_NOOP("[%1] - RVSCHR  - Cursor DEreg,Areg* Zeichen invertiere\n"));  break;
    FUNC_CALL(0,0x011E,QT_TR_NOOP("[%1] - CRSRSTAT- Specify the cursor type\n"));			break;
    FUNC_CALL(0,0x012D,QT_TR_NOOP("[%1] - UPSCRL  - scroll up the screen\n"));              break;
    FUNC_CALL(0,0x0130,QT_TR_NOOP("[%1] - DWNSCRL - Srool down the screen\n"));				break;
    FUNC_CALL(0,0x0142,QT_TR_NOOP("[%1] - INS1LN  - Insert a blank line\n"));               break;
    FUNC_CALL(0,0x0145,QT_TR_NOOP("[%1] - ERS1LN  - Erase the contents of a line\n"));      break;
    FUNC_CALL(0,0x013F,QT_TR_NOOP("[%1] - ERSSTR  - Display a specified number of space\n"));break;
    FUNC_CALL(0,0x013C,QT_TR_NOOP("[%1] - SMBLSET - Set the state of the status line symbols\n"));				break;
    FUNC_CALL(0,0x0139,QT_TR_NOOP("[%1] - SMBLREAD-Read the state of the status une symbots.\n"));							break;
    FUNC_CALL(0,0x0109,QT_TR_NOOP("[%1] - SETANK  - Set the display to the character mode.\n"));							break;
    FUNC_CALL(0,0x0127,QT_TR_NOOP("[%1] - DOTSET  - Display a dot in the set/presetlreverse mode.\n"));							break;
    FUNC_CALL(0,0x012A,QT_TR_NOOP("[%1] - DOTREAD - Read the display state of a dot.\n"));							break;
    FUNC_CALL(0,0x0121,QT_TR_NOOP("[%1] - LINE    - Drawa une.\n"));							break;
    FUNC_CALL(0,0x0124,QT_TR_NOOP("[%1] - BOX     - Draw a box.\n"));							break;
    FUNC_CALL(0,0x014B,QT_TR_NOOP("[%1] - GCRSRSET- Set the graphics Cursor position.\n"));							break;
    FUNC_CALL(0,0x0148,QT_TR_NOOP("[%1] - GCRSRPOS- Read the current graphics cursor position.\n"));							break;
    FUNC_CALL(0,0x014E,QT_TR_NOOP("[%1] - PRTGCHR - Display a character at the current graphics cursor position.\n"));							break;
    FUNC_CALL(0,0x00EE,QT_TR_NOOP("[%1] - PRTGSTR - Display a string of characters from the current graphics cursor position.\n"));							break;
    FUNC_CALL(0,0x0154,QT_TR_NOOP("[%1] - PRTGPTN - Display a 1 X 8 dot pattern at the current graphics cursor position.\n"));							break;
    FUNC_CALL(0,0x015A,QT_TR_NOOP("[%1] - GPTNREAD- Read the 1 X 8 dot pattern atthe Current graphics cursor position.\n"));							break;
    FUNC_CALL(0,0x0133,QT_TR_NOOP("[%1] - CGMODE  - Change the character generator mode between the PC-1500 mode and the PC-1600 mode.\n"));							break;
    FUNC_CALL(0,0x0157,QT_TR_NOOP("[%1] - CPY1500LCD- Copy the contents of the fourth Une of the screen to the PC-1500 mode\n"));							break;
    FUNC_CALL(0,0x0112,QT_TR_NOOP("[%1] - CLS     - Clear the screen display.\n"));		break;
    FUNC_CALL(0,0x00E5,QT_TR_NOOP("[%1] - BSPCTR  - Enable/disable the LCD.\n"));							break;
    FUNC_CALL(0,0x015D,QT_TR_NOOP("[%1] - SAVELCD - Save the 156x 8 dot pattern of the specified me to RAM.\n"));						break;
    FUNC_CALL(0,0x0160,QT_TR_NOOP("[%1] - LOADLCD - Load the 156 X 8 dot pattern from RAM to the specified une.\n"));                break;



    FUNC_CALL(6,0x809C,QT_TR_NOOP("[%1] - CRSRCL  - Cursor 1�schen\n"));                break;
    FUNC_CALL(6,0x80A2,QT_TR_NOOP("[%1] - CRSRSTATN-Setze Cursor wie 011E,aber vorher Cursor nicht l�schen\n"));                break;
    FUNC_CALL(6,0x809F,QT_TR_NOOP("[%1] - CHARADR - Pixe1code des Zeichens Aregermitteln,Startadresse>HLreg\n"));                break;




    FUNC_CALL(0,0x0166,QT_TR_NOOP("[%1] - KEYGET     - Read cne character from the key buffer-. If the key buffer- is empty, the routine waits for e key input.\n"));                break;
    FUNC_CALL(0,0x0169,QT_TR_NOOP("[%1] - KEYGETR    - Same function as KEYGET except the routine does net wait for a key input even if the key buffer is empty.\n"));                break;
    FUNC_CALL(0,0x016C,QT_TR_NOOP("[%1] - KBUFSET    - Load data inte the key buffer or clear the key buffer-.\n"));                break;
    FUNC_CALL(0,0x016F,QT_TR_NOOP("[%1] - BREAKCHK   - Read the state of the BREAK key.\n"));                break;
    FUNC_CALL(0,0x0172,QT_TR_NOOP("[%1] - CURUDCHK   - Read the state of the up and down arrow.\n"));                break;
    FUNC_CALL(0,0x0175,QT_TR_NOOP("[%1] - KEYDIRECT  - Scan the keys and read the key code of the key that has been pressed when the routine is called.\n"));                break;
    FUNC_CALL(0,0x0178,QT_TR_NOOP("[%1] - KEYSTRB    - Scan one row of keys te identify a particular ene of more than one key pressed at the same time.\n"));                break;
    FUNC_CALL(0,0x017B,QT_TR_NOOP("[%1] - KEVAUX     - Specify the key input device (main keyboard or RS-232C).\n"));                break;
    FUNC_CALL(0,0x017E,QT_TR_NOOP("[%1] - KEYSTATSET - Specify the key repeat function and the key click function.\n"));                break;
    FUNC_CALL(0,0x0181,QT_TR_NOOP("[%1] - KEYSTATREAD- Read the settings cf the key repeat and click functions and the cur-r-ent key input device.\n"));                break;
    FUNC_CALL(0,0x0184,QT_TR_NOOP("[%1] - OFFCHK     - Read the state cf the OFF key.\n"));                break;
    FUNC_CALL(0,0x0187,QT_TR_NOOP("[%1] - KEYGETND   - Read the first char-acter in the key buffer without changing the contents of the key buffer.\n"));                break;
    FUNC_CALL(0,0x018A,QT_TR_NOOP("[%1] - BREAKRESET - Clear the latch cf the BREAK key.\n"));                break;


    case 0x01D8: switch (pZ80->z80.r.c) {
    FUNC_CALL_C(0,0x01D8,0x01,QT_TR_NOOP("RS232 / SIO - [%1] - CWCOM      - Set communication parameters.\n"));                break;
    FUNC_CALL_C(0,0x01D8,0x02,QT_TR_NOOP("RS232 / SIO - [%1] - CRCOM      - Read the current communication parameters.\n"));                break;
    FUNC_CALL_C(0,0x01D8,0x03,QT_TR_NOOP("RS232 / SIO - [%1] - CSNDA      - Transmit one byte cf data.\n"));                break;
    FUNC_CALL_C(0,0x01D8,0x04,QT_TR_NOOP("RS232 / SIO - [%1] - CRCVA      - Receive one byte cf data (if there is no data te read, wait until data are sent in.)\n"));
                        Hack_CRVA();
                        AddLog(LOG_SIO,"HACK");
                        break;
    FUNC_CALL_C(0,0x01D8,0x07,QT_TR_NOOP("RS232 / SIO - [%1] - CRCV1      - Receive one byte of data (do net wait even if there is no data te read.)\n"));                break;
    FUNC_CALL_C(0,0x01D8,0x0E,QT_TR_NOOP("RS232 / SIO - [%1] - CSETHS     - Set RS and ER signais to high in the auto handshake mode.\n"));                break;
    FUNC_CALL_C(0,0x01D8,0x0F,QT_TR_NOOP("RS232 / SIO - [%1] - CRESHS     - Set RS and ER signais to low in the auto handshake mode.\n"));                break;
    FUNC_CALL_C(0,0x01D8,0x10,QT_TR_NOOP("RS232 / SIO - [%1] - CWOUTS     - Set the state of the outgoing controi signaIs (RS and ER).\n"));                break;
    FUNC_CALL_C(0,0x01D8,0x11,QT_TR_NOOP("RS232 / SIO - [%1] - CRCTRL     - Read the state of the control signals.\n"));                break;
    FUNC_CALL_C(0,0x01D8,0x12,QT_TR_NOOP("RS232 / SIO - [%1] - CWDEV      - Select a channel and set the input and output device selection parameters for that channel.\n"));                break;
    FUNC_CALL_C(0,0x01D8,0x13,QT_TR_NOOP("RS232 / SIO - [%1] - CRDEV      - Read the currently seiected channel number and the input and output device selection parameters set by CWDEV routine.\n"));
//    pCPU->logsw=1;
    pCPU->Check_Log();
    break;
    FUNC_CALL_C(0,0x01D8,0x14,QT_TR_NOOP("RS232 / SIO - [%1] - CESND      - Enable the transmission on(y when the specified incoming control signal (or signaIs) is high.\n"));                break;
    FUNC_CALL_C(0,0x01D8,0x15,QT_TR_NOOP("RS232 / SIO - [%1] - CERCV      - Enabie the reception only when the specified incoming control signal (or- signaIs) is low.\n"));                break;
    FUNC_CALL_C(0,0x01D8,0x16,QT_TR_NOOP("RS232 / SIO - [%1] - CSBRK      - Send the specified number et break characters\n"));                break;
    FUNC_CALL_C(0,0x01D8,0x17,QT_TR_NOOP("RS232 / SIO - [%1] - CSRCVB     - Reserve a receive buffer in memory.\n"));                break;
    FUNC_CALL_C(0,0x01D8,0x18,QT_TR_NOOP("RS232 / SIO - [%1] - CCLRSB     - Clear the work area for transmission.\n"));                break;
    FUNC_CALL_C(0,0x01D8,0x1C,QT_TR_NOOP("RS232 / SIO - [%1] - CCLRRB     - Clearthe receive buffer.\n"));                break;

    }
    break;

    case 0x01D5: switch (pZ80->z80.r.c) {
    FUNC_CALL_C(0,0x01D5,0x00,QT_TR_NOOP("TIMER/ ANALOG PORT - [%1] - SINIT      - Initialize the timers and the analog input port.\n"));                break;
    FUNC_CALL_C(0,0x01D5,0x01,QT_TR_NOOP("TIMER/ ANALOG PORT - [%1] - SBEEP      - Generate a key clicking sound.\n"));                break;
    FUNC_CALL_C(0,0x01D5,0x02,QT_TR_NOOP("TIMER/ ANALOG PORT - [%1] - SWRT       - Set a date and time for the calendar dock.\n"));
//    pCPU->logsw=1;
//    pCPU->Check_Log();
    break;
    FUNC_CALL_C(0,0x01D5,0x03,QT_TR_NOOP("TIMER/ ANALOG PORT - [%1] - SRRT       - Read the current date and time of the calendar dock.\n"));
//                                                    pCPU->logsw=1;
//                                                    pCPU->Check_Log();
//                                                    ((CZ80 *)pCPU)->z80.r.c = 0x23;
    break;
    FUNC_CALL_C(0,0x01D5,0x04,QT_TR_NOOP("TIMER/ ANALOG PORT - [%1] - SWWT       - Set a date and time for the wakeup timer.\n"));
//                                                pCPU->logsw=1;
//                                                pCPU->Check_Log();
                                                break;
    FUNC_CALL_C(0,0x01D5,0x05,QT_TR_NOOP("TIMER/ ANALOG PORT - [%1] - SWRT       - Read the current settings of date and time of the wakeup timer.\n"));
//                                                pCPU->logsw=1;
//                                                pCPU->Check_Log();
                                                break;
    FUNC_CALL_C(0,0x01D5,0x06,QT_TR_NOOP("TIMER/ ANALOG PORT - [%1] - SWA1T      - Set a date and time for the alarm1 timer (same as ON TIME$ command).\n"));                break;
    FUNC_CALL_C(0,0x01D5,0x07,QT_TR_NOOP("TIMER/ ANALOG PORT - [%1] - SRA1T      - Read the current settings of date and time of the alarm timer.\n"));                break;
    FUNC_CALL_C(0,0x01D5,0x08,QT_TR_NOOP("TIMER/ ANALOG PORT - [%1] - SWA2T      - Set a date and time for the alarm2 timer (same as ALARM$ command).\n"));                break;
    FUNC_CALL_C(0,0x01D5,0x09,QT_TR_NOOP("TIMER/ ANALOG PORT - [%1] - SRA2T      - Read the current settings of date and time ofthe alarm2 timer.\n"));                break;
    FUNC_CALL_C(0,0x01D5,0x10,QT_TR_NOOP("TIMER/ ANALOG PORT - [%1] - SWMSK      - Set the interrupt mask for SC-7852.\n"));
//                                                        pCPU->logsw=1;  // 0E, 7C, 5F ????
//                                                        pCPU->Check_Log();
                                                        break;
    FUNC_CALL_C(0,0x01D5,0x11,QT_TR_NOOP("TIMER/ ANALOG PORT - [%1] - SRMSK      - Read the current setting of the interrupt mask for SC-7852.\n"));
//                                                    pCPU->logsw=1;  // 5E
//                                                    pCPU->Check_Log();
                                                        break;
    FUNC_CALL_C(0,0x01D5,0x12,QT_TR_NOOP("TIMER/ ANALOG PORT - [%1] - SRIRQ      - Read the current interrupt cause for SC-7852.\n"));
//                                                    pCPU->logsw=1;    // 5D
//                                                    pCPU->Check_Log();
                                                        break;
    FUNC_CALL_C(0,0x01D5,0x13,QT_TR_NOOP("TIMER/ ANALOG PORT - [%1] - SRINP      - Read the state of the CI signal cf RS-232c port.\n"));
//                                                    pCPU->logsw=1;    // 5C
//                                                    pCPU->Check_Log();
                                                        break;
    FUNC_CALL_C(0,0x01D5,0x14,QT_TR_NOOP("TIMER/ ANALOG PORT - [%1] - SWPON      - Set a mask for the power-on conditions.\n"));                break;

    FUNC_CALL_C(0,0x01D5,0x18,QT_TR_NOOP("TIMER/ ANALOG PORT - [%1] - SRAO       - Read the digital value of the supply voltage of the PC-1600 main unit. \n"));
//                                                        pCPU->logsw=1;    //
//                                                        pCPU->Check_Log();
    break;
    FUNC_CALL_C(0,0x01D5,0x19,QT_TR_NOOP("TIMER/ ANALOG PORT - [%1] - SRA1       - Read the digital value of the voltage input at the analog input port. \n"));
//    pCPU->logsw=1;    //
//    pCPU->Check_Log();
    break;
    FUNC_CALL_C(0,0x01D5,0x1A,QT_TR_NOOP("TIMER/ ANALOG PORT - [%1] - SRA2       - Read the digital value of the CE-1600P battery voltage.\n"));
//    pCPU->logsw=1;    //
//    pCPU->Check_Log();
    break;

    FUNC_CALL_C(0,0x01D5,0x21,QT_TR_NOOP("TIMER/ ANALOG PORT - [%1] - SRPON      - Read the settings of the mask for the power-on conditions. 96 6F \n"));                break;
    FUNC_CALL_C(0,0x01D5,0x22,QT_TR_NOOP("TIMER/ ANALOG PORT - [%1] - SWAB       - Set the conditions for alarm beep generation. \n"));                break;
    FUNC_CALL_C(0,0x01D5,0x23,QT_TR_NOOP("TIMER/ ANALOG PORT - [%1] - SRAB       - Read the current settings of the alarm beep generation conditions set by SWAB routine. 95 6F\n"));                break;
    FUNC_CALL_C(0,0x01D5,0x24,QT_TR_NOOP("TIMER/ ANALOG PORT - [%1] - SWA1A      - Set the software interrupt trigger levels for a digital value of the analog input port.\n"));                break;
        }
    break;


    case 0x01DE: switch (pZ80->z80.r.c) {
    FUNC_CALL_C(0,0x01DE,0x0F,QT_TR_NOOP("FILES - [%1] - OPEN      - Open a file.\n"));                break;
    FUNC_CALL_C(0,0x01DE,0x10,QT_TR_NOOP("FILES - [%1] - CLOSE     - Close a file.\n"));                break;
    FUNC_CALL_C(0,0x01DE,0x11,QT_TR_NOOP("FILES - [%1] - SEARCH FIRST- Search for the first file.\n"));                break;
    FUNC_CALL_C(0,0x01DE,0x12,QT_TR_NOOP("FILES - [%1] - SEARCH NEXT - Search for the next file.\n"));                break;
    FUNC_CALL_C(0,0x01DE,0x13,QT_TR_NOOP("FILES - [%1] - DELETE    - Delete a file.\n"));                break;
    FUNC_CALL_C(0,0x01DE,0x14,QT_TR_NOOP("FILES - [%1] - SEQUENTIAL RD- Read data sequentiaily from a file.\n"));                break;
    FUNC_CALL_C(0,0x01DE,0x15,QT_TR_NOOP("FILES - [%1] - SEQUENTIAL WR- Write data sequentially te o file.\n"));                break;
    FUNC_CALL_C(0,0x01DE,0x16,QT_TR_NOOP("FILES - [%1] - CREATE FILE  - Create a file.\n"));
//                                                        pCPU->logsw=1;
//                                                        pCPU->Check_Log();
    break;
    FUNC_CALL_C(0,0x01DE,0x17,QT_TR_NOOP("FILES - [%1] - RENAME FILE  - Rename e file.\n"));                break;
    FUNC_CALL_C(0,0x01DE,0x1A,QT_TR_NOOP("FILES - [%1] - SET DMA      - Set a transfer- address.\n"));                break;
    FUNC_CALL_C(0,0x01DE,0x1B,QT_TR_NOOP("FILES - [%1] - GET ALLOC    - Get the file device information such as the number cf empty clusters.\n"));                break;
    FUNC_CALL_C(0,0x01DE,0x1E,QT_TR_NOOP("FILES - [%1] - SET ATTRS    - Set file attributes.\n"));                break;
    }
    break;


    FUNC_CALL(0,0x0196,QT_TR_NOOP("[%1] - SLOT1MAP   - \n"));                break;
    FUNC_CALL(0,0x0199,QT_TR_NOOP("[%1] - SLOT2MAP   - \n"));                break;
    FUNC_CALL(0,0x00E8,QT_TR_NOOP("[%1] - SLOTST     - \n"));
//                                                    pCPU->logsw=1;
//                                                    pCPU->Check_Log();
    break;


//    FUNC_CALL(3,0x4051,QT_TR_NOOP("[%1] - KEYSTRBSCAN- \n"));                break;
    FUNC_CALL(3,0x4054,QT_TR_NOOP("[%1] - KEYCODCNV  - \n"));                break;
//    FUNC_CALL(6,0x80B7,QT_TR_NOOP("[%1] - KEYSCAN    - \n"));                break;


    FUNC_CALL(4,0x4020,QT_TR_NOOP("PRINTER - [%1] - PCHEK    - Check whether the printer is ready.\n")); break;
    FUNC_CALL(4,0x4023,QT_TR_NOOP("PRINTER - [%1] - POUT     - Send one byte of character code to the printer\n")); break;
    FUNC_CALL(4,0x4026,QT_TR_NOOP("PRINTER - [%1] - PKOUT    - Send one byte of character code to the printer\n")); break;

    case 0x4008: switch (pZ80->z80.r.c) {
    FUNC_CALL_C(4,0x4008,0x00,QT_TR_NOOP("PRINTER - [%1] - PINIT      - Initialize the printer.\n"));                break;
    FUNC_CALL_C(4,0x4008,0x01,QT_TR_NOOP("PRINTER - [%1] - PTEXT      - Set the printer in the text mode.\n"));                break;
    FUNC_CALL_C(4,0x4008,0x02,QT_TR_NOOP("PRINTER - [%1] - PGRAPH      - Set the printer in the graphics mode.\n"));                break;
    FUNC_CALL_C(4,0x4008,0x03,QT_TR_NOOP("PRINTER - [%1] - PCSIZE      - Set the character size\n"));                break;
    FUNC_CALL_C(4,0x4008,0x04,QT_TR_NOOP("PRINTER - [%1] - PCOLOR      - Set the printer color\n"));                break;
    FUNC_CALL_C(4,0x4008,0x06,QT_TR_NOOP("PRINTER - [%1] - PWIDTH      - Set the une width (characters/line).\n"));                break;
    FUNC_CALL_C(4,0x4008,0x07,QT_TR_NOOP("PRINTER - [%1] - PLEFTM      - Set the ieft marg(n.\n"));                break;
    FUNC_CALL_C(4,0x4008,0x08,QT_TR_NOOP("PRINTER - [%1] - PPITCH      - Set the character pitch and the fine height.\n"));                break;
    FUNC_CALL_C(4,0x4008,0x09,QT_TR_NOOP("PRINTER - [%1] - PPAPER      - Set the paper type.\n"));                break;
    FUNC_CALL_C(4,0x4008,0x0a,QT_TR_NOOP("PRINTER - [%1] - PSCRL      - Sot the printing area on paper in the Y (vertical) direction.\n"));                break;
    FUNC_CALL_C(4,0x4008,0x0b,QT_TR_NOOP("PRINTER - [%1] - PEOL      - Define the printer action for e CR code (ODH).\n"));                break;
    FUNC_CALL_C(4,0x4008,0x0c,QT_TR_NOOP("PRINTER - [%1] - PZONE      - Set the print zone length for LPRINT command.\n"));                break;
    FUNC_CALL_C(4,0x4008,0x0d,QT_TR_NOOP("PRINTER - [%1] - PPENUP      - Lift up or push down the pen.\n"));                break;
    FUNC_CALL_C(4,0x4008,0x0e,QT_TR_NOOP("PRINTER - [%1] - PROTATE      - Set the direction of the print characters.\n"));                break;


    } break;
    }

}

void Cpc1600::LoadSIO(void) {
    fclose(fp_CRVA);
    // Display the Open dialog box.
    QString ofn = QFileDialog::getOpenFileName(
                mainwindow,
                "Choose a file",
                ".",
                "Text Files (*.*)");

    if (ofn.isEmpty()) return;

    char * str = qstrdup(ofn.toLocal8Bit());

    if ((fp_CRVA = fopen(str,"r"))==NULL) {
        MSG_ERROR(tr("Failed to open file"));
        return ;
    }
}

void Cpc1600::Hack_CRVA(void){
    // Is a file open ?
    // If no open it
    int c;

    if (fp_CRVA == 0) {
            return;
    }
    else {
        c=fgetc(fp_CRVA);
        if (c==0x0A) c=fgetc(fp_CRVA);
    }
    if (c==EOF) {
        fclose(fp_CRVA);
        fp_CRVA = 0;
        c = 0x1A;
    }
    AddLog(LOG_SIO,tr("data:[%1]=%2").arg(c,2,16,QChar('0')).arg(QChar(c)));
    if (fp_log) fprintf(fp_log,"SEND DATA [%02x]\n",c);
    ((CZ80 *)pCPU)->z80.r.a = c;
    ((CZ80 *)pCPU)->z80retn(&((CZ80 *)pCPU)->z80);

}

FILE *fp_tmp2;
void Cpc1600::ReadQuarterTape(void){}

INLINE bool Cpc1600::lh5810_write(UINT32 d, UINT32 data)
{
//	AddLog(LOG_FUNC,"Cpc1500::lh5810_write");

    switch (d) {
    case 0x14: pLH5810->SetReg(CLH5810::RESET,	data); break;
    case 0x15: pLH5810->SetReg(CLH5810::U,		data); break;
    case 0x16: pLH5810->SetReg(CLH5810::L,		data); break;
    case 0x17: pLH5810->SetReg(CLH5810::F,		data); break;
    case 0x18: pLH5810->SetReg(CLH5810::OPC,     data); break;
    case 0x19: pLH5810->SetReg(CLH5810::G  ,     data); break;
    case 0x1A: pLH5810->SetReg(CLH5810::MSK,     data); break;
    case 0x1B: pLH5810->SetReg(CLH5810::IF ,     data); break;
    case 0x1C: pLH5810->SetReg(CLH5810::DDA,     data); break;
    case 0x1D: pLH5810->SetReg(CLH5810::DDB,     data); break;
    case 0x1E: pLH5810->SetReg(CLH5810::OPA,     data); break;
    case 0x1F: pLH5810->SetReg(CLH5810::OPB,     data); break;
    default: break;
    }

    return true;
}

INLINE quint8 Cpc1600::lh5810_read(UINT32 d)
{
    switch (d) {
    case 0x15: return (pLH5810->GetReg(CLH5810::U)); break;
    case 0x16: return (pLH5810->GetReg(CLH5810::L)); break;
    case 0x17: return (pLH5810->GetReg(CLH5810::F)); break;
    case 0x18: return (pLH5810->GetReg(CLH5810::OPC)); break;
    case 0x19: return (pLH5810->GetReg(CLH5810::G)); break;
    case 0x1A: return (pLH5810->GetReg(CLH5810::MSK)); break;
    case 0x1B: return (pLH5810->GetReg(CLH5810::IF)); break;
    case 0x1C: return (pLH5810->GetReg(CLH5810::DDA)); break;
    case 0x1D: return (pLH5810->GetReg(CLH5810::DDB)); break;
    case 0x1E: return (pLH5810->GetReg(CLH5810::OPA)); break;
    case 0x1F: return (pLH5810->GetReg(CLH5810::OPB)); break;
    default: break;
    }

    return 0;
}


bool Cpc1600::Mem_Mirror(UINT32 *d)
{
    // LH5803 mirror
    if (!masterCPU) {
        if ( (*d>=0x7400) && (*d<=0x744F) )	{ *d+=0x200; return(1); }
        if ( (*d>=0x7500) && (*d<=0x754F) )	{ *d+=0x200; return(1); }
    }

    return(1);
}


bool Cpc1600::Chk_Adr(UINT32 *d,UINT32 data)
{
    Q_UNUSED(data)
    Mem_Mirror(d);


    if (masterCPU)
    {
        if (                 (*d<=0x3FFF) ) {
            if (bank1) {
                writeBus(bus,d,data);
                return false;
            }
            return false;
        }
        if ( (*d>=0x4000) && (*d<=0x7FFF) )
        {
            switch (bank2) {
            case 0:
            case 1:
            case 3: return false;
            default: writeBus(bus,d,data); return false;
            }
        }
        if ( (*d>=0x8000) && (*d<=0xBFFF) )
        {
            bool ret = false;
            int rambank = pCPU->imem[0x28];

            int subBank = 0;
            switch (bank3)
            {
            case 0:
                if ( (*d>=0xB000) && (*d<=0xBFFF)) subBank = 0;
                if ( (*d>=0xA000) && (*d<=0xAFFF)) subBank = 1;
                if ( (*d>=0x8000) && (*d<=0x9FFF)) subBank = 2;
                switch (subBank)
                {
                case 0: ret = (S1_EXTENSION_CE151_CHECK | S1_EXTENSION_CE155_CHECK | S1_EXTENSION_CE159_CHECK | S1_EXTENSION_CE161_CHECK | S1_EXTENSION_CE1600M_CHECK | S1_EXTENSION_CE1601M_CHECK); break;
                case 1: ret = (S1_EXTENSION_CE155_CHECK | S1_EXTENSION_CE159_CHECK | S1_EXTENSION_CE161_CHECK | S1_EXTENSION_CE1600M_CHECK | S1_EXTENSION_CE1601M_CHECK); break;
                case 2: ret = (S1_EXTENSION_CE161_CHECK | S1_EXTENSION_CE1600M_CHECK | S1_EXTENSION_CE1601M_CHECK); break;
                }
                return ret;
            case 1: ret =  (S1_EXTENSION_CE1600M_CHECK | S1_EXTENSION_CE1601M_CHECK);
                *d += 0x10000;
                return ret;

            case 2:
            case 3: switch (rambank)
                    {
                    case 0: switch (bank3) {
                            case 2: ret =  (S2_EXTENSION_CE161_CHECK | S2_EXTENSION_CE1600M_CHECK | S2_EXTENSION_CE1601M_CHECK | S2_EXTENSION_CE16096_CHECK | S2_EXTENSION_CE16128_CHECK |S2_EXTENSION_CE16160_CHECK|S2_EXTENSION_CE16192_CHECK|S2_EXTENSION_CE16224_CHECK | S2_EXTENSION_CE16256_CHECK); break;
                            case 3: ret =  (S2_EXTENSION_CE1600M_CHECK | S2_EXTENSION_CE1601M_CHECK | S2_EXTENSION_CE16096_CHECK | S2_EXTENSION_CE16128_CHECK |S2_EXTENSION_CE16160_CHECK|S2_EXTENSION_CE16192_CHECK|S2_EXTENSION_CE16224_CHECK | S2_EXTENSION_CE16256_CHECK);               break;
                            }
                        *d += bank3 * 0x10000;
                        return ret;
                        break;
                    case 1: ret = (S2_EXTENSION_CE1601M_CHECK | S2_EXTENSION_CE16096_CHECK | S2_EXTENSION_CE16128_CHECK |S2_EXTENSION_CE16160_CHECK|S2_EXTENSION_CE16192_CHECK|S2_EXTENSION_CE16224_CHECK | S2_EXTENSION_CE16256_CHECK ); break;
                    case 2: ret = (S2_EXTENSION_CE16096_CHECK | S2_EXTENSION_CE16128_CHECK |S2_EXTENSION_CE16160_CHECK|S2_EXTENSION_CE16192_CHECK|S2_EXTENSION_CE16224_CHECK | S2_EXTENSION_CE16256_CHECK ); break;
                    case 3: ret = (S2_EXTENSION_CE16128_CHECK | S2_EXTENSION_CE16160_CHECK|S2_EXTENSION_CE16192_CHECK|S2_EXTENSION_CE16224_CHECK | S2_EXTENSION_CE16256_CHECK ); break;
                    case 4: ret = (S2_EXTENSION_CE16160_CHECK | S2_EXTENSION_CE16192_CHECK|S2_EXTENSION_CE16224_CHECK | S2_EXTENSION_CE16256_CHECK ); break;
                    case 5: ret = (S2_EXTENSION_CE16192_CHECK | S2_EXTENSION_CE16224_CHECK | S2_EXTENSION_CE16256_CHECK ); break;
                    case 6: ret = (S2_EXTENSION_CE16224_CHECK | S2_EXTENSION_CE16256_CHECK ); break;
                    case 7: ret = (S2_EXTENSION_CE16256_CHECK ); break;
                    }

                    *d += 0xA0000 + (rambank-1) * 0x8000 + (bank3 - 2) * 0x4000 - 0x8000;
                    return ret;
                    break;
            case 6: return false;   // Internal ROM
            default: writeBus(bus,d,data);
                    return false;
                    break;
            }
        }
        if ( (*d>=0xC000) && (*d<=0xFFFF) ) {
            if (bank4) {
                writeBus(bus,d,data);
                return false;
            }
            return true;    // Internal RAM 3
        }
    }
    else
    {
        //if (pCPU->fp_log) fprintf(pCPU->fp_log,"Check adr [%05x]",*d);

        if (                 (*d<=0x1FFF) )	{ *d+=0x8000;return(EXTENSION_CE161_CHECK); }						// ROM area(0000-3FFF) 16K
        if ( (*d>=0x2000) && (*d<=0x37FF) )	{ *d+=0x8000;return(EXTENSION_CE161_CHECK | EXTENSION_CE159_CHECK); }	// ROM area(0000-3FFF) 16K
        if ( (*d>=0x3800) && (*d<=0x3FFF) )	{ *d+=0x8000;return(EXTENSION_CE161_CHECK | EXTENSION_CE159_CHECK | EXTENSION_CE155_CHECK); }		// ROM area(0000-3FFF) 16K
        if ( (*d>=0x4000) && (*d<=0x57FF) )	{ *d+=0x8000;return(1); }										// RAM area(0000-3FFF) 16K
        if ( (*d>=0x5800) && (*d<=0x67FF) )	{ *d+=0x8000;return(EXTENSION_CE155_CHECK | EXTENSION_CE151_CHECK); }	// RAM area(0000-3FFF) 16K
        if ( (*d>=0x6800) && (*d<=0x6FFF) )	{ *d+=0x8000;return(EXTENSION_CE155_CHECK); }						// RAM area(0000-3FFF) 16K
        if ( (*d>=0x7000) && (*d<=0x7FFF) ) { *d+=0x8000;return(1);}										// RAM area(7800-7BFF)
        if ( (*d>=0x8000) && (*d<=0x9FFF) ) { *d+=0x70000;return(0); }										// RAM area(4000-7FFFF)
        if ( (*d>=0xA000) && (*d<=0xBFFF) ) { *d+=0x70000;return(0); }										// RAM area(4000-7FFFF)
        if ( (*d>=0xC000) && (*d<=0xFFFF) ) { *d+=0x70000;return(0); }										// RAM area(4000-7FFFF)
        if ( (*d>=0x1A030)&&(*d<=0x1A03F) )
        {
            if (*d == 0x1A038) cpuSwitchPending = true;
            *d+=0x70000; return (1);
        }
        if ( (*d>=0x1B000)&&(*d<=0x1B00F) ) { ce150_Access = true;*d+=0x70000;	return(1); }
        if ( (*d>=0x1F000)&&(*d<=0x1F00F) )	{ //lh5810_Access = true;
                                              //if (*d==0x1F006) pLH5810->New_L=true;
                                              *d+=0x70000;return(1);}										// I/O area(LH5810)

        if ( (*d>=0x1D000)&&(*d<=0x1D00F) ) { *d+=0x70000;return(1); }
        if ( (*d>=0x18000)&&(*d<=0x1800F) ) { *d+=0x70000;return(1); }
        *d+=0x70000;return(1);
    }

    // else it is ROM
    return (false);
}

#define CS24 (pCPU->imem[0x3D] & 0x04)

bool Cpc1600::Chk_Adr_R(UINT32 *d,UINT32 *data)
{
    Q_UNUSED(data)
    Mem_Mirror(d);

    if (masterCPU)
    {
        if (                 (*d<=0x3FFF) ) {
            if (bank1) {
                readBus(bus,d,data);
                return false;
            }
            return true;
        }
        if ( (*d>=0x4000) && (*d<=0x7FFF) )
        {
            switch (bank2) {
            case 0: return true;    // internal ROM
            case 1: *d += bank2 * 0x10000; return true;
            case 3: *d += bank2 * 0x10000 - (CS24 ? 0 : 0x4000); return true;
            default:
                readBus(bus,d,data);
                return false;
            }

            return true;
        }

        if ( (*d>=0x8000) && (*d<=0xBFFF) ) {
            int rambank = pCPU->imem[0x28];
            switch (bank3)
            {
            case 0: break;
            case 1: *d += 0x10000; break;
            case 2:
            case 3:
                if (rambank == 0)
                {
                    *d += bank3 * 0x10000;
                }
                else
                {
                    *d += 0xA0000 + (rambank -1 ) * 0x8000 + (bank3 - 2) * 0x4000 - 0x8000;
                }
                break;
            case 6: *d += bank3 * 0x10000; break;
            default: readBus(bus,d,data); return false;
            }
            return true;
        }
        if ( (*d>=0xC000) && (*d<=0xFFFF) ) {
            if (bank4) {
                readBus(bus,d,data);
                return false;
            }
            return true;
        }
    }
    else
    {
        if (                 (*d<=0x7FFF) )	{ *d+=0x8000;return(1); }						// ROM area(0000-3FFF) 16K

        if ( (*d>=0x8000) && (*d<=0x9FFF) ) { *d+=0x70000;return(1); }										// RAM area(4000-7FFFF)
        if ( (*d>=0xA000) && (*d<=0xBFFF) ) { *d+=0x70000;return(1); }										// RAM area(4000-7FFFF)
        if ( (*d>=0xC000) && (*d<=0xFFFF) ) { *d+=0x70000;return(1); }
        //if (pCPU->fp_log) fprintf(pCPU->fp_log,"Check adr_R [%05x] = %02x",*d,mem[*d+0x70000]);
        if ( (*d>=0x1A000) && (*d<=0x1A00F) ) {	*d+=0x70000;	return(1);	}

        if ( (*d>=0x1B000) && (*d<=0x1B00F) ) {	*d+=0x70000;	return(1);	}
        if ( (*d>=0x1D000) && (*d<=0x1D00F) ) {	*d+=0x70000;	return(1);	}
        if ( (*d>=0x1F000) && (*d<=0x1F00F) ) {	*d+=0x70000;	return(1);	}

        if ( (ce150_connected) && (*d>=0xA000) && (*d<=0xBFFF) &&  (pLH5803->lh5801.pv==0) )
        {
            //*d+=0x1A000;
            *d+=0x8A000;
            return(1);
        }
        *d+=0x70000;
    }

    return(1);
}

void Cpc1600::Set_Port(PORTS Port,BYTE data){
    Q_UNUSED(Port)
    Q_UNUSED(data)
}

BYTE Cpc1600::Get_Port(PORTS Port){
    Q_UNUSED(Port)
    return(0);
}

UINT8 Cpc1600::out(UINT8 address, UINT8 value, QString sender)
{
    if (pCPU->fp_log) {
        pCPU->Regs_Info(1);
        fprintf(pCPU->fp_log,"%-40s   iff=%i OUT [%02X]=%02X\t\t\t%s\n"," ",((CZ80 *)pCPU)->z80.r.iff,address,value,pCPU->Regs_String);
    }    // Manage CPU port modification

    switch(address) {
    case 0x10:    case 0x11:    case 0x12:    case 0x13:
    case 0x14:    case 0x15:    case 0x16:    case 0x17:
    case 0x18:    case 0x19:    case 0x1a:    case 0x1b:
    case 0x1c:    case 0x1d:    case 0x1e:    case 0x1f:
//        if (fp_log) fprintf(fp_log,"OUT [%02X]=%02X\n",address,value);
        lh5810_write(address,value);
        pLH5810->step();
        break;
    case 0x20: if (fp_log) fprintf(fp_log,"OUT [%02X]=%02X\n",address,value);
        break;
    case 0x21:  // Send a command to the sub-CPU ( the sub-CPU answer on port 33h)
        // In fact it send the command via he UART parallel port !!!!
//        if (fp_log) fprintf(fp_log,"OUT [%02X]=%02X   - SEND //\n",address,value);
        pLU57813P->command(value);
        break;
    case 0x22: pTC8576P->in(value);
        if (fp_log) fprintf(fp_log,"OUT [%02X]=%02X\n",address,value);
        break;
    case 0x23: pTC8576P->instruction(value);
        if (fp_log) fprintf(fp_log,"OUT [%02X]=%02X\n",address,value);
        break;
    case 0x28:
        if (fp_log) fprintf(fp_log,"OUT [%02X]=%02X\n",address,value);
        break;

    case 0x30: break;
        // RomBank = port 31h
    case 0x31:
        bank1 = value & 0x01;
        bank2 = (value >> 1) & 0x07;
        bank3 = (value >> 4) & 0x07;
        bank4 = (value >> 7) & 0x01;
        if (pCPU->fp_log) fprintf(pCPU->fp_log," r1=%i   r2=%i   r3=%i   r4=%i  [3d]=%02x\n",bank1,bank2,bank3,bank4,pCPU->imem[0x3d]);
        break;

    case 0x32:
    case 0x33:
    case 0x34:
    case 0x35:
    case 0x36: break;

    case 0x37:  break;

    case 0x38: // switch control between CPUs
        cpuSwitchPending = true;
        break;
    case 0x39:
    case 0x3a:
    case 0x3b: break;
    case 0x3c: if (fp_log) fprintf(fp_log,"OUT [%02X]=%02X\n",address,value);
        break;

    case 0x3d:
    case 0x3e:
    case 0x3f: break;



        //////////////////////////////////////////////////////////
       // DISPLAY CONTROL
      //////////////////////////////////////////////////////////
    case 0x50:  pHD61102_1->instruction(value);
                pHD61102_2->instruction(value);
                pLCDC->Refresh = true;
                break;
    case 0x54:  pHD61102_2->instruction(value);
                pLCDC->Refresh = true;
                break;
    case 0x58:  pHD61102_1->instruction(value);
                pLCDC->Refresh = true;
                break;
    case 0x52:  pHD61102_1->instruction(0x100|value);
                pHD61102_2->instruction(0x100|value);
                pLCDC->Refresh = true;
                break;
    case 0x56:  pHD61102_2->instruction(0x100|value);
                pLCDC->Refresh = true;
                break;
    case 0x5a:  pHD61102_1->instruction(0x100|value);
                pLCDC->Refresh = true;
                break;
//////////////////////////////////////////////////////////
    case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x76: case 0x77:
    case 0x78: case 0x79: case 0x7a: case 0x7b: case 0x7c: case 0x7d: case 0x7e: case 0x7f:
//        qWarning()<<QString("Write[%1]=%2").arg(address,2,16,QChar('0')).arg(value,2,16,QChar('0'));
        // FDD
    {
        if (fp_log) fprintf(fp_log,"PRINTER - OUT [%02X]=%02X\n",address,value);
        UINT32 _adr = address | 0x10000;
        writeBus(bus,&_adr,value);
    }
        break;

    case 0x80:
    case 0x81:
    case 0x82:
    case 0x83: {
        if (fp_log) fprintf(fp_log,"PRINTER - OUT [%02X]=%02X\n",address,value);
        UINT32 _adr = address | 0x10000;
        writeBus(bus,&_adr,value);
    }
        break;
    }

    return 1;
}



UINT8 Cpc1600::in(UINT8 address,QString)
{
    if (pCPU->fp_log) {
        pCPU->Regs_Info(0);
        fprintf(pCPU->fp_log,"%-40s   IN [%02X]\t\t\t%s\n"," ",address,pCPU->Regs_String);
    }
    switch (address) {
    case 0x10:    case 0x11:    case 0x12:    case 0x13:
    case 0x14:    case 0x15:    case 0x16:    case 0x17:
    case 0x18:    case 0x19:    case 0x1a:    case 0x1b:
    case 0x1c:    case 0x1d:    case 0x1e:    case 0x1f:
//        if (fp_log) fprintf(fp_log,"IN [%02X]=%02X\n",address,pCPU->imem[address]);
        pLH5810->step();
        pCPU->imem[address] = lh5810_read(address);
        break;


    case 0x20: if (fp_log) fprintf(fp_log,"IN [%02X]=%02X\n",address,pCPU->imem[address]);
        break;
    case 0x21: if (fp_log) fprintf(fp_log,"IN [%02X]=%02X\n",address,pCPU->imem[address]);
        break;
    case 0x22:
        pCPU->imem[address] = pTC8576P->get_ssr();
//        if (fp_log) fprintf(fp_log,"IN [%02X]=%02X\n",address,pCPU->imem[address]);
        break;
    case 0x23:
        pCPU->imem[address] = pTC8576P->get_psr();
//        if (fp_log) fprintf(fp_log,"IN [%02X]=%02X\n",address,pCPU->imem[address]);
        break;// bit 5 to 0
    case 0x28:
        if (fp_log) fprintf(fp_log,"IN [%02X]=%02X\n",address,pCPU->imem[address]);
        break;


    case 0x33: if (pLU57813P->output_pending) pCPU->imem[address] = pLU57813P->Get_reg_out();
        break;

    case 0x37:  {
            BYTE c = getKey();
                pCPU->imem[address] = c;
//                if (pCPU->fp_log) fprintf(pCPU->fp_log,"Last Key = %02x   Get key = %02x\n",c,pKEYB->LastKey);
            }
                break;


      //////////////////////////////////////////////////////////
     // DISPLAY CONTROL
    //////////////////////////////////////////////////////////
    case 0x55: pCPU->imem[address] = pHD61102_2->instruction(0x200); break;
    case 0x59: pCPU->imem[address] = pHD61102_1->instruction(0x200); break;
    case 0x57: pCPU->imem[address] = pHD61102_2->instruction(0x300); break;
    case 0x5b: pCPU->imem[address] = pHD61102_1->instruction(0x300); break;

        //////////////////////////////////////////////////////////
       // FDD CONTROL
      //////////////////////////////////////////////////////////
/*
 *70-77: Floppy II, otherwise known as 78-7F
78-7F; floppy I
78w: command (40 = Read, Write = 60, A0 = format)
78r: Motor and disk status
        b7: Engine not started
        b6: no write protection
        b3: disk in the drive
79w: define Sector
79r: Read current Sector

7Aw: Motor b7: motor on
7Ar: Status
        b7: busy ????
        b6: changed Disk
        b1: Ready
        b0: Error (inverted)
7B: read / write data
7C-7F: unused
*/
    case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x76: case 0x77:
    case 0x78: case 0x79: case 0x7a: case 0x7b: case 0x7c: case 0x7d: case 0x7e: case 0x7f:
        pCPU->imem[0x78] |= 0x48;  // bit 3 and 6 set to 1. Disk loaded and not write protected
        pCPU->imem[0x7A] &= 0xFE;   // bit 0 (ERR) to 0
        pCPU->imem[0x7A] |= 0x42;   //   b7:ack ???  b6: changed Disk    b1: Ready  b0: Error (inverted)
        if (fp_log) fprintf(fp_log,"IN [%02X]=%02X\n",address,pCPU->imem[address]);
//        qWarning()<<QString("Read[%1]   pc=%2").arg(address,2,16,QChar('0')).arg(pCPU->get_PC(),4,16,QChar('0'));
    {
            UINT32 _adr = address | 0x10000;
            UINT32 _data=0;
            readBus(bus,&_adr,&_data);
            pCPU->imem[address] = _data;
            if (fp_log) fprintf(fp_log,"FDD - IN [%02X]=%02X\n",address,_data);
        }
        break;
    case 0x80:
    case 0x81:
    case 0x82:
    case 0x83: {
        UINT32 _adr = address | 0x10000;
        UINT32 _data=0;
        readBus(bus,&_adr,&_data);
        pCPU->imem[address] = _data;
        if (fp_log) fprintf(fp_log,"PRINTER - IN [%02X]=%02X\n",address,_data);
    }
        break;
    }
    return 1;

}

void Cpc1600::Regs_Info(UINT8 Type)
{
    Q_UNUSED(Type)

//    sprintf(Log_String,"%s b1:%d b2:%d b3:%d b4:%d ",Log_String,bank1,bank2,bank3,bank4);

    strcat(Regs_String,	"");
    pCPU->Regs_Info(1);
}

#define LOGPC() \
        if (fp_log) { \
            pPC->pCPU->Regs_Info(0); \
            fprintf(fp_log,"IN [%02X]\t%s\n",address,pPC->pCPU->Regs_Info); \
                }

#define SIO_GNDP	1
#define SIO_SD 		2   // TXD
#define SIO_RD		3   // RXD
#define SIO_RS		4   // RTS
#define SIO_CS		5   // CTS
#define SIO_DS		6   // DSR
#define SIO_GND		7   // SG
#define SIO_CD		8
#define SIO_CI		9
#define SIO_VC1		10
#define SIO_RR		11  // NC
#define SIO_PAK		12  // NC
#define SIO_VC2		13  // NC
#define SIO_ER		14  // DTR
#define SIO_PRQ		15  // NC

void Cpc1600::setPUPVPT(CbusPc1500 *bus, UINT32 adr) {
    if (bus->isME1()) return;

    if (adr <= 0x3FFF) {
        bus->setPV(bank1 & 0x01);
    }
    else if ( (adr >= 0x4000) && (adr <= 0x7FFF) ) {
        bus->setPV(bank2 & 0x01);
        bus->setPU((bank2>>1) & 0x01);
        bus->setPT((bank2>>2) & 0x01);
    }
    else if ( (adr >= 0x8000) && (adr <= 0xBFFF) ) {
        bus->setPV(bank3 & 0x01);
        bus->setPU((bank3>>1) & 0x01);
        bus->setPT((bank3>>2) & 0x01);
    }
    else if ( (adr >= 0xC000) && (adr <= 0xFFFF) ) {
        bus->setPV(bank4 & 0x01);
    }
}

bool Cpc1600::Set_Connector(Cbus *_bus)
{
    Q_UNUSED(_bus)

    // MANAGE SERIAL CONNECTOR
    // TO DO
//    pSIOCONNECTOR->Set_pin(SIO_SD	,READ_BIT(v,b));

    setPUPVPT((CbusPc1500*)bus,bus->getAddr());

    pCONNECTOR->Set_values(bus->toUInt64());
    return true;
}

bool Cpc1600::Get_Connector(Cbus *_bus)
{
    Q_UNUSED(_bus)

    bus->fromUInt64(pCONNECTOR->Get_values());
    bus->setEnable(false);

    // MANAGE SERIAL CONNECTOR


    PUT_BIT(pCPU->imem[0x23],2,pSIOCONNECTOR->Get_pin(SIO_DS));
    PUT_BIT(pCPU->imem[0x23],1,pSIOCONNECTOR->Get_pin(SIO_CD));
    PUT_BIT(pCPU->imem[0x23],0,!pSIOCONNECTOR->Get_pin(SIO_CS));

    PUT_BIT(pCPU->imem[0x23],5,!pSIOCONNECTOR->Get_pin(SIO_CI));

    // A/D Connector
    pLU57813P->SetDigital(pADCONNECTOR->Get_values());

    return true;
}


void Cpc1600::contextMenuEvent ( QContextMenuEvent * event )
{
    QMenu *menu= new QMenu(this);

    BuildContextMenu(menu);

        menu->popup(event->globalPos () );
        event->accept();
}

bool	CLH5810_PC1600::init(void)
{
    return(1);
}


BYTE Cpc1600::getKey()
{

    UINT8 data=0;

    if (pKEYB->LastKey)
    {
        BYTE ks1 = pLH5810->GetReg(CLH5810::OPB);
        if (!( ks1 & 0x40)) {
            if (KEY(K_CTRL))		data|=0x01;
            if (KEY(K_KBII))		data|=0x02;
            if (KEY(K_BS))			data|=0x04;
        }
    }
    BYTE ks = pKEYB->Get_KS() ^ 0xff;
    if ((pKEYB->LastKey) && ks )
    {     
        if (ks&1) {
            if (KEY('2'))			data|=0x01;
            if (KEY('5'))			data|=0x02;
            if (KEY('8'))			data|=0x04;
            if (KEY('H'))			data|=0x08;
            if (KEY(K_SHT))			data|=0x10;
            if (KEY('Y'))			data|=0x20;
            if (KEY('N'))			data|=0x40;
            if (KEY(K_UA))			data|=0x80;			// UP ARROW
        }
        if (ks&2) {
            if (KEY('.'))			data|=0x01;
            if (KEY('-'))			data|=0x02;
            if (KEY(K_OF))			data|=0x04;			// OFF
            if (KEY('S'))			data|=0x08;
            if (KEY(K_F1))			data|=0x10;
            if (KEY('W'))			data|=0x20;
            if (KEY('X'))			data|=0x40;
            if (KEY(K_RSV))			data|=0x80;
        }
        if (ks&4) {
            if (KEY('1'))			data|=0x01;
            if (KEY('4'))			data|=0x02;
            if (KEY('7'))			data|=0x04;
            if (KEY('J'))			data|=0x08;
            if (KEY(K_F5))			data|=0x10;
            if (KEY('U'))			data|=0x20;
            if (KEY('M'))			data|=0x40;
            if (KEY('0'))			data|=0x80;
        }
        if (ks&8) {
            if (KEY(')'))			data|=0x01;
            if (KEY('L'))			data|=0x02;
            if (KEY('O'))			data|=0x04;
            if (KEY('K'))			data|=0x08;
            if (KEY(K_F6))			data|=0x10;
            if (KEY('I'))			data|=0x20;
            if (KEY('('))			data|=0x40;
            if (KEY(K_RET))			data|=0x80;
        }
        if (ks&0x10) {
            if (KEY('+'))			data|=0x01;			// +
            if (KEY('*'))			data|=0x02;			// *
            if (KEY('/'))			data|=0x04;			// /
            if (KEY('D'))			data|=0x08;
            if (KEY(K_F2))			data|=0x10;			// Key F2
            if (KEY('E'))			data|=0x20;
            if (KEY('C'))			data|=0x40;
            if (KEY(K_RCL))			data|=0x80;
        }
        if (ks&0x20) {
            if (KEY('='))			data|=0x01;			// =
            if (KEY(K_LA))			data|=0x02;			// LEFT ARROW
            if (KEY('P'))			data|=0x04;
            if (KEY('F'))			data|=0x08;
            if (KEY(K_F3))			data|=0x10;
            if (KEY('R'))			data|=0x20;
            if (KEY('V'))			data|=0x40;
            if (KEY(' '))			data|=0x80;
        }
        if (ks&0x40) {
            if (KEY(K_RA))			data|=0x01;			// R ARROW
            if (KEY(K_MOD))			data|=0x02;			// MODE
            if (KEY(K_CLR))			data|=0x04;			// CLS
            if (KEY('A'))			data|=0x08;
            if (KEY(K_DEF))			data|=0x10;
            if (KEY('Q'))			data|=0x20;
            if (KEY('Z'))			data|=0x40;
            if (KEY(K_SML))			data|=0x80;
        }
        if (ks&0x80) {
            if (KEY('3'))			data|=0x01;
            if (KEY('6'))			data|=0x02;
            if (KEY('9'))			data|=0x04;
            if (KEY('G'))			data|=0x08;
            if (KEY(K_F4))			data|=0x10;			// Key F4
            if (KEY('T'))			data|=0x20;
            if (KEY('B'))			data|=0x40;
            if (KEY(K_DA))			data|=0x80;			// DOWN ARROW
        }

//        if (fp_log) fprintf(fp_log,"Read key [%02x]: strobe=%02x result=%02x\n",pKEYB->LastKey,ks,data^0xff);
        //SetReg(CLH5810::OPA,data^0xff);
    }
    return data^0xff;

}


bool CLH5810_PC1600::step()
{
    ////////////////////////////////////////////////////////////////////
    //	INT FROM connector to IRQ
    ////////////////////////////////////////////////////////////////////
//    IRQ = pPC->pCONNECTOR->Get_pin(30);

    ////////////////////////////////////////////////////////////////////
    //	Timer pulse to PB5
    ////////////////////////////////////////////////////////////////////
    SetRegBit(OPB,5,pPC->pTIMER->GetTP( ((Cpc1600 *)pPC)->pLU57813P->Get_tpIndex64()));    // 1/64s signal le timer doit etre en mode 4

    ////////////////////////////////////////////////////////////////////
    //	ON/Break
    ////////////////////////////////////////////////////////////////////
    SetRegBit(OPB,7,((Cpc1600 *)pPC)->pLU57813P->Get_Kon());

      //----------------------//
     // Standard LH5810 STEP //
    //----------------------//
    CLH5810::step();

    pPC->pKEYB->Set_KS(GetReg(OPA));
//    if (pPC->fp_log) fprintf(pPC->fp_log,"Set_KS( %02x )\n",GetReg(CLH5810::OPA));

    return(1);
}


void Cpc1600::ComputeKey(KEYEVENT ke, int scancode, QMouseEvent *event)
{
    Q_UNUSED(ke)
    Q_UNUSED(scancode)

    if (KEY(K_F7)) {
        qWarning()<<"LOG";
        pCPU->logsw = true;
        pCPU->Check_Log();
        if (!fp_log) fp_log=fopen("pc1600.log","wt");	// Open log file
    }

    // Manage left connector click
    if (KEY(0x240) && (currentView==LEFTview)) {
        pKEYB->removeKey(0x240);
        FluidLauncher *launcher = new FluidLauncher(mainwindow,
                                     QStringList()<<P_RES(":/pockemul/configExt.xml"),
                                     FluidLauncher::PictureFlowType,QString(),
                                     "Sharp_60");
        launcher->show();
    }
}
