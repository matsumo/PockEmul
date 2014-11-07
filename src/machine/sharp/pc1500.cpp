/*** POCKEMUL [PC1500.CPP] ********************************/
/* PC1500 emulator main class                             */
/**********************************************************/
//#include	<string.h>
//#include	<stdlib.h>

#define TEST_BUS

#include <QtGui> 

#include	"common.h"
#include "fluidlauncher.h"
#include "ui/dialogdasm.h"

#include	"pc1500.h"
#include    "cextension.h"
#include    "Lcdc_pc1500.h"
#include	"Inter.h"
#include	"Keyb.h"
#include	"dialoganalog.h"
#include    "buspc1500.h"
#include    "Connect.h"

#include "breakpoint.h"
#include "watchpoint.h"


extern int	g_DasmStep;
extern bool	UpdateDisplayRunning;
 

Cpc15XX::Cpc15XX(CPObject *parent)	: CpcXXXX(parent)
{								//[constructor]
	setfrequency( (int) 2600000/2);
    ioFreq = 20000;
	setcfgfname(QString("pc1500"));

    SessionHeader	= "PC1500PKM";
	Initial_Session_Fname ="pc1500.pkm";
    BackGroundFname	= P_RES(":/pc1500/pc1500.png");

    memsize			= 0x10000;
	InitMemValue	= 0xFF;

    LeftFname   = P_RES(":/pc1600/pc1600Left.png");

	SlotList.clear();

    setDXmm(195);
    setDYmm(86);
    setDZmm(25);

    setDX(679);
    setDY(299);

    SoundOn			= false;

    pLCDC		= new Clcdc_pc1500(this,
                                   QRect(93,61,156*2*1.18,7*2*1.18),
                                   QRect(95,53,316*1.18,5*1.18));
    pCPU		= new CLH5801(this); pCPU->logsw=false;
	pLH5810		= new CLH5810_PC1500(this);
	pTIMER		= new Ctimer(this);
    pKEYB		= new Ckeyb(this,"pc1500.map");
	
    bus = new CbusPc1500();
	
	Tape_Base_Freq=2500;
	initExtension();
	extensionArray[0] = ext_60pins;
	extensionArray[1] = ext_MemSlot1;
}

Cpc15XX::~Cpc15XX()
{
    delete pLH5810;
}

Cpc1500A::Cpc1500A(CPObject *parent)	: Cpc15XX(parent)
{								//[constructor]
    Q_UNUSED(parent)

    setfrequency( (int) 2600000/2);
    setcfgfname("pc1500a");

    SessionHeader	= "PC1500APKM";
    Initial_Session_Fname ="pc1500A.pkm";
    BackGroundFname	= P_RES(":/pc1500A/pc1500A.png");

    memsize			= 0x10000;

    SlotList.clear();
    SlotList.append(CSlot(8 , 0x0000 ,	""								, "" , CSlot::RAM , "RAM"));
    SlotList.append(CSlot(8 , 0x2000 ,	""								, "" , CSlot::ROM , "ROM"));
    SlotList.append(CSlot(16, 0x4000 ,	""								, "" , CSlot::RAM , "RAM"));
    SlotList.append(CSlot(8 , 0x8000 ,	""								, "" , CSlot::NOT_USED , "NOT USED"));
    SlotList.append(CSlot(8 , 0xA000 ,	""								, "" , CSlot::ROM , "ROM"));
    SlotList.append(CSlot(16, 0xC000 ,	P_RES(":/pc1500A/SYS1500A.ROM"), "" , CSlot::ROM , "SYSTEM ROM"));

}

Ctrspc2::Ctrspc2(CPObject *parent)	: Cpc1500(parent)
{								//[constructor]
    Q_UNUSED(parent)

    setcfgfname("trspc2");

    SessionHeader	= "TRSPC-2PKM";
    Initial_Session_Fname ="trspc2.pkm";
    BackGroundFname	= P_RES(":/pc1500/trspc2.png");

    pLCDC->rect.moveTo(181,62);

    pLCDC->symbRect.moveTo(181,53);

    pKEYB->fn_KeyMap = "trspc2.map";

}

void Cpc15XX::TurnON(void)
{
    qWarning()<<"turnon pcxx="<<Power;
//    if (!Power && pKEYB->LastKey == K_BRK)
    {
	AddLog(LOG_FUNC,"Cpc1500::TurnOn");

    manageBus();
//--	remove(Initial_Session_Fname);
	pCPU->Reset();
	CpcXXXX::TurnON();
    }
}

void	Cpc15XX::initExtension(void)
{
	// initialise ext_MemSlot1
	ext_MemSlot1 = new CExtensionArray("Memory Slot","Add memory module");
	ext_MemSlot1->setAvailable(ID_CE151,true);		ext_MemSlot1->setChecked(ID_CE151,false);
	ext_MemSlot1->setAvailable(ID_CE155,true);		ext_MemSlot1->setChecked(ID_CE155,true);
	ext_MemSlot1->setAvailable(ID_CE159,true);		ext_MemSlot1->setChecked(ID_CE159,false);
	ext_MemSlot1->setAvailable(ID_CE160,true);		ext_MemSlot1->setChecked(ID_CE160,false);
	ext_MemSlot1->setAvailable(ID_CE161,true);		ext_MemSlot1->setChecked(ID_CE161,false);

	addExtMenu(ext_MemSlot1);
}

bool Cpc15XX::CompleteDisplay(void)
{
	QPainter painter;

//	AddLog(LOG_FUNC,"Cpc1500::CompleteDisplay");
	
	CpcXXXX::CompleteDisplay();
	
    return true;
}


bool Cpc15XX::InitDisplay(void)
{
	AddLog(LOG_FUNC,"Cpc1500::InitDisplay");

	CpcXXXX::InitDisplay();

	return(1);
}

bool Cpc15XX::LoadConfig(QXmlStreamReader *xmlIn)
{
    Q_UNUSED(xmlIn)
    pLH5810->Load_Internal(xmlIn);
    pBreakpointManager->unserialize(xmlIn);
    return true;
}

bool Cpc15XX::SaveConfig(QXmlStreamWriter *xmlOut)
{

    pLH5810->save_internal(xmlOut);
    pBreakpointManager->serialize(xmlOut);
    return true;
}

bool Cpc15XX::init(void)				// initialize
{
//    pCPU->logsw = true;
//        if (!fp_log) fp_log=fopen("pc1500.log","wt");	// Open log file

	CpcXXXX::init();

    pCONNECTOR	= new Cconnector(this,
                                 60,
                                 0,
                                 Cconnector::Sharp_60,
                                 "Connector 60 pins",
                                 false,
                                 QPoint(0,72),
                                 Cconnector::WEST);
    publish(pCONNECTOR);

	WatchPoint.remove(this);
	
	WatchPoint.add(&pCONNECTOR_value,64,60,this,"Standard 60pins connector");
	WatchPoint.add((qint64 *) &(pLH5810->lh5810.r_opa),8,8,this,"LH5810 Port A");
	WatchPoint.add((qint64 *) &(pLH5810->lh5810.r_opb),8,8,this,"LH5810 Port B");
	WatchPoint.add((qint64 *) &(pLH5810->lh5810.r_opc),8,8,this,"LH5810 Port C");


#if 1
        // Init I/O LH5810 memory
        Set_8(0x1F000 , 0);
        Set_8(0x1F001 , 0);
        Set_8(0x1F002 , 0);
        Set_8(0x1F003 , 0);
        Set_8(0x1F004 , 0);
        Set_8(0x1F005 , 0);
        Set_8(0x1F006 , 0);
        Set_8(0x1F007 , 0);
        Set_8(0x1F008 , 0);
        Set_8(0x1F009 , 0);
        Set_8(0x1F00A , 0);
        Set_8(0x1F00B , 0);
        Set_8(0x1F00C , 0);
        Set_8(0x1F00D , 0);
        Set_8(0x1F00E , 0);
        Set_8(0x1F00F , 0);
#endif

	return true;
}

bool Cpc15XX::run(void) 
{

    //TODO NEED TO USE IMEMSIZE instead
    if (dialogdasm)
        dialogdasm->imem=false;

	UINT32 Current_PC;	


// ---------------------------------------------------------	
    CpcXXXX::run();
// ---------------------------------------------------------

    if (off) return true;

	Current_PC = pCPU->get_PC();

    //TODO pLCDC->On = ((CLH5801 *)pCPU)->lh5801.dp;

	if (pKEYB->CheckKon()) 
		((CLH5801 *)pCPU)->lh5801.bf=1;

    pLH5810->step();
    pKEYB->Set_KS(lh5810_read(0x1F00C));
    lh5810_Access = false;


    if (pLH5810->INT==true)
        ((CLH5801 *)pCPU)->lh5801.IR2=true;

#if 0
	if (((CLH5801 *)pCPU)->lh5801.bf)
	{
		TurnON();
	}
	else
	{
//		TurnOFF();
	}
#endif

#if 0
// HACK Program Counter
#define FUNC_CALL(ADDR,LIB) case ADDR: AddLog(LOG_ROM,tr(LIB).arg(((CLH5801 *)pCPU)->get_PC(),5,16,QChar('0')))

	switch (Current_PC)
	{
	case 0xBCE8:	// 2eme

//	AddLog(LOG_ROM,"Function Call [%05X]",((CLH5801 *)pCPU)->get_PC());
		//sprintf((char *) &(pPC->mem[0x7B91]),"MICRO TENNIS");
//	g_DasmStep=1;
		break;


//    FUNC_CALL(0xA519,QT_TR_NOOP("Function Call [%1] - Change printer pen color"));				break;
//	FUNC_CALL(0xA769,QT_TR_NOOP("Function Call [%1] - Printer motor OFF"));						break;
//	FUNC_CALL(0xA781,QT_TR_NOOP("Function Call [%1] - Send ASCCI char to printer. No LF"));		break;
//	FUNC_CALL(0xA8DD,QT_TR_NOOP("Function Call [%1] - Move PEN"));								break;
//	FUNC_CALL(0xA9F1,QT_TR_NOOP("Function Call [%1] - Send Line Feed to printer"));				break;
//	FUNC_CALL(0xAA04,QT_TR_NOOP("Function Call [%1] - Send (n) Line Feeds to printer"));		break;
//	FUNC_CALL(0xAAE3,QT_TR_NOOP("Function Call [%1] - Pen UP/DOWN"));							break;
//	FUNC_CALL(0xABCB,QT_TR_NOOP("Function Call [%1] - Switch Printer Graphic to Text mode"));	break;
//	FUNC_CALL(0xABEF,QT_TR_NOOP("Function Call [%1] - Switch Printer Text to Graphic mode"));	break;
//	FUNC_CALL(0xB8A6,QT_TR_NOOP("Function Call [%1] - CSAVE"));									break;
//	FUNC_CALL(0xBF11,QT_TR_NOOP("Function Call [%1] - REMOTE ON  (Open/Close)"));				break;
//	FUNC_CALL(0xBF43,QT_TR_NOOP("Function Call [%1] - REMOTE OFF"));							break;
//	FUNC_CALL(0xBBF5,QT_TR_NOOP("Function Call [%1] - Termination of CMT I/O Control"));		break;
//	FUNC_CALL(0xBD3C,QT_TR_NOOP("Function Call [%1] - File Transfer"));							break;
//	FUNC_CALL(0xBDF3,QT_TR_NOOP("Function Call [%1] - Sortie Load Char"));						break;
//	FUNC_CALL(0xBE02,QT_TR_NOOP("Function Call [%1] - Load Quarter"));
//											pCPU->logsw=1;
//											pCPU->Check_Log();
//											ReadQuarterTape();
																								break;
//	FUNC_CALL(0xBDF0,QT_TR_NOOP("Function Call [%1] - Load Char"));								break;
//	FUNC_CALL(0xBDCC,QT_TR_NOOP("Function Call [%1] - CE150 Save one character"));				break;
//	FUNC_CALL(0xBBD6,QT_TR_NOOP("Function Call [%1] - Creation of Header"));					break;
	}
#endif

	//----------------------------------
	// SOUND BUFFER (quite simple no?) 
	//----------------------------------
    fillSoundBuffer(
                (
                    (pLH5810->lh5810.r_opc & 0x40 ? false:true) ||
                    pLH5810->SDO ||
                    ((CbusPc1500*)bus)->isCMTIN()
                    ) ? 0xff:0x00);
	//----------------------------------

	return(1); 
}


INLINE bool Cpc15XX::lh5810_write(UINT32 d, UINT32 data)
{
//	AddLog(LOG_FUNC,"Cpc1500::lh5810_write");

    switch (d) {
    case 0x1F004: pLH5810->SetReg(CLH5810::RESET,	data); break;
    case 0x1F005: pLH5810->SetReg(CLH5810::U,		data); break;
    case 0x1F006: pLH5810->SetReg(CLH5810::L,		data); break;
    case 0x1F007: pLH5810->SetReg(CLH5810::F,		data); break;
    case 0x1F008: pLH5810->SetReg(CLH5810::OPC,     data); break;
    case 0x1F009: pLH5810->SetReg(CLH5810::G  ,     data); break;
    case 0x1F00A: pLH5810->SetReg(CLH5810::MSK,     data); break;
    case 0x1F00B: pLH5810->SetReg(CLH5810::IF ,     data); break;
    case 0x1F00C: pLH5810->SetReg(CLH5810::DDA,     data); break;
    case 0x1F00D: pLH5810->SetReg(CLH5810::DDB,     data); break;
    case 0x1F00E: pLH5810->SetReg(CLH5810::OPA,     data); break;
    case 0x1F00F: pLH5810->SetReg(CLH5810::OPB,     data); break;
    default: break;
    }

    return true;
}

INLINE quint8 Cpc15XX::lh5810_read(UINT32 d)
{
    switch (d) {
    case 0x1F005: return (pLH5810->GetReg(CLH5810::U)); break;
    case 0x1F006: return (pLH5810->GetReg(CLH5810::L)); break;
    case 0x1F007: return (pLH5810->GetReg(CLH5810::F)); break;
    case 0x1F008: return (pLH5810->GetReg(CLH5810::OPC)); break;
    case 0x1F009: return (pLH5810->GetReg(CLH5810::G)); break;
    case 0x1F00A: return (pLH5810->GetReg(CLH5810::MSK)); break;
    case 0x1F00B: return (pLH5810->GetReg(CLH5810::IF)); break;
    case 0x1F00C: return (pLH5810->GetReg(CLH5810::DDA)); break;
    case 0x1F00D: return (pLH5810->GetReg(CLH5810::DDB)); break;
    case 0x1F00E: return (pLH5810->GetReg(CLH5810::OPA)); break;
    case 0x1F00F: return (pLH5810->GetReg(CLH5810::OPB)); break;
    default: break;
    }
	
    return 0;
}

bool Cpc15XX::Mem_Mirror(UINT32 *d) 
{
#if 1
	if ( (*d>=0x7000) && (*d<=0x71FF) )	{ *d+=0x600; return(1); }
	if ( (*d>=0x7200) && (*d<=0x73FF) )	{ *d+=0x400; return(1); }
	if ( (*d>=0x7400) && (*d<=0x75FF) )	{ *d+=0x200; return(1); }
#else
	if ( (*d>=0x7000) && (*d<=0x75FF) )	{ *d+=0x600; return(1); }
#endif
	if ( (*d>=0x7C00) && (*d<=0x7FFF) ) { *d-=0x400; return(1); }
	
	return(1);
} 

inline bool Cpc1500A::Mem_Mirror(UINT32 *d)
{
	if ( (*d>=0x7000) && (*d<=0x71FF) )	{ *d+=0x600; return(1); }
	if ( (*d>=0x7200) && (*d<=0x73FF) )	{ *d+=0x400; return(1); }
	if ( (*d>=0x7400) && (*d<=0x75FF) )	{ *d+=0x200; return(1); }
	
	return(1);
}


bool Cpc15XX::Chk_Adr(UINT32 *d,UINT32 data) 
{
    Q_UNUSED(data)

	Mem_Mirror(d);

    if (                 (*d<=0x1FFF) )	{ return(EXTENSION_CE161_CHECK); }						// ROM area(0000-3FFF) 16K
	if ( (*d>=0x2000) && (*d<=0x37FF) )	{ return(EXTENSION_CE161_CHECK | EXTENSION_CE159_CHECK); }	// ROM area(0000-3FFF) 16K
	if ( (*d>=0x3800) && (*d<=0x3FFF) )	{ return(EXTENSION_CE161_CHECK | EXTENSION_CE159_CHECK| EXTENSION_CE155_CHECK); }		// ROM area(0000-3FFF) 16K
	if ( (*d>=0x4000) && (*d<=0x47FF) )	{ return(1); }										// RAM area(0000-3FFF) 16K
	if ( (*d>=0x4800) && (*d<=0x57FF) )	{ return(EXTENSION_CE155_CHECK | EXTENSION_CE151_CHECK); }	// RAM area(0000-3FFF) 16K
	if ( (*d>=0x5800) && (*d<=0x5FFF) )	{ return(EXTENSION_CE155_CHECK); }						// RAM area(0000-3FFF) 16K
	if ( (*d>=0x6000) && (*d<=0x6FFF) )	{ return(0); }										// ROM area(0000-3FFF) 16K
	if ( (*d>=0x7000) && (*d<=0x75FF) ) { return(0); }										// INHIBITED MIRRORING
	if ( (*d>=0x7600) && (*d<=0x77FF) ) { pLCDC->SetDirtyBuf(*d-0x7600);return(1);}
	if ( (*d>=0x7800) && (*d<=0x7BFF) ) { return(1); }										// RAM area(7800-7BFF)
	if ( (*d>=0x7C00) && (*d<=0x7FFF) ) { return(0); }										// INHIBITED MIRRORING
    if ( (*d>=0x8000) && (*d<=0xBFFF) ) { writeBus(bus,d,data); return false; }										// RAM area(4000-7FFFF)

    if ( (*d>=0xC000) && (*d<=0xFFFF) ) {
        if (((CbusPc1500*)bus)->isINHIBIT()) writeBus(bus,d,data);
        return false;
    }
    if ( (*d>=0x1F000) && (*d<=0x1F00F) ) { lh5810_write(*d,data); return false; }	// I/O area(LH5810)

    if ( (*d>=0x10000) && (*d<=0x1FFFF) ) { writeBus(bus,d,data); return false; }

	// else it's ROM
    return false;
}

bool Cpc1500A::Chk_Adr(UINT32 *d,UINT32 data) 
{
    Q_UNUSED(data)

	Mem_Mirror(d);

    if ( (*d>=0x4000) && (*d<=0x57FF) )	{ return(1); }										// RAM area(0000-3FFF) 16K
	if ( (*d>=0x5800) && (*d<=0x67FF) )	{ return(EXTENSION_CE155_CHECK | EXTENSION_CE151_CHECK); }	// RAM area(0000-3FFF) 16K
	if ( (*d>=0x6800) && (*d<=0x6FFF) )	{ return(EXTENSION_CE155_CHECK); }						// RAM area(0000-3FFF) 16K
    if ( (*d>=0xC000) && (*d<=0xFFFF) ) {
        qWarning()<<"PC="<<QString("%1").arg(pCPU->get_PC(),4,16,QChar('0'));
        qWarning()<<"Write :"<<QString("%1").arg(*d,4,16,QChar('0'))<<"="<<QString("%1").arg(data,2,16,QChar('0'));
        if (((CbusPc1500*)bus)->isINHIBIT()) writeBus(bus,d,data);
        return false;
         }										// RAM area(4000-7FFFF)

    return Cpc15XX::Chk_Adr(d,data);
}


bool Cpc15XX::Chk_Adr_R(UINT32 *d,UINT32 *data)
{ 
    Q_UNUSED(data)

    Mem_Mirror(d);

    if ( (*d>=0x8000) && (*d<=0xBFFF) )
    {
        readBus(bus,d,data);
        return false;
    }

    if ( (*d>=0x1F000) && (*d<=0x1F00F) ) {
        *data = lh5810_read(*d);
        return false;
    }

//    if (((CbusPc1500*)bus)->isINHIBIT())
//        qWarning()<<"Bus is INHIBIT adr="<<QString("%1").arg(*d,5,16,QChar('0'))<<"  iswrite:"<<bus->isWrite();

    if ( (*d>=0xC000) && (*d<=0xFFFF) && ((CbusPc1500*)bus)->isINHIBIT() ) {
        readBus(bus,d,data); return false; }

    if ( (*d>=0x10000) && (*d<=0x1FFFF) ) { readBus(bus,d,data); return false; }


    return true ;
}

 
bool Cpc1500A::Chk_Adr_R(UINT32 *d,UINT32 *data)
{ 
    return Cpc15XX::Chk_Adr_R(d,data);

}

void Cpc15XX::Set_Port(PORTS Port,BYTE data){
    Q_UNUSED(Port)
    Q_UNUSED(data)
}
BYTE Cpc15XX::Get_Port(PORTS Port){
    Q_UNUSED(Port)
    return(0);
}

#define KS		( pKEYB->Get_KS()) 
//#define KEY(c)	( TOUPPER(pKEYB->LastKey) == TOUPPER(c) )
#define KEY(c)	((pKEYB->keyPressedList.contains(TOUPPER(c)) || \
                  pKEYB->keyPressedList.contains(c) || \
                  pKEYB->keyPressedList.contains(TOLOWER(c)))?1:0)
UINT8 Cpc15XX::in(UINT8 address)
{
    Q_UNUSED(address)

//return 1^0xff;
//	static int cnt=0;
	UINT8 data=0;
	if (KS&1) {
		if (KEY('2'))			data|=1;
		if (KEY('5'))			data|=2;
		if (KEY('8'))			data|=4;
		if (KEY('H'))			data|=8;
		if (KEY(K_SHT))			data|=0x10;
		if (KEY('Y'))			data|=0x20;
		if (KEY('N'))			data|=0x40;
		if (KEY(K_UA))			data|=0x80;			// UP ARROW
	}
	if (KS&2) {
		if (KEY('.'))			data|=1;
		if (KEY('-'))			data|=2;
		if (KEY(K_OF))			data|=4;			// OFF
		if (KEY('S'))			data|=8;
		if (KEY(K_F1))			data|=0x10;
		if (KEY('W'))			data|=0x20;
		if (KEY('X'))			data|=0x40;
		if (KEY(K_RSV))			data|=0x80;
	}
	if (KS&4) {
		if (KEY('1'))			data|=1;
		if (KEY('4'))			data|=2;
		if (KEY('7'))			data|=4;
		if (KEY('J'))			data|=8;
		if (KEY(K_F5))			data|=0x10;
		if (KEY('U'))			data|=0x20;
		if (KEY('M'))			data|=0x40;
		if (KEY('0'))			data|=0x80;
	}
	if (KS&8) {
		if (KEY(')'))			data|=1;
		if (KEY('L'))			data|=2;
		if (KEY('O'))			data|=4;
		if (KEY('K'))			data|=8;
		if (KEY(K_F6))			data|=0x10;
		if (KEY('I'))			data|=0x20;
		if (KEY('('))			data|=0x40;
		if (KEY(K_RET))			data|=0x80;
	}
	if (KS&0x10) {
		if (KEY('+'))			data|=1;			// +
		if (KEY('*'))			data|=2;			// *
		if (KEY('/'))			data|=4;			// /
		if (KEY('D'))			data|=8;
		if (KEY(K_F2))			data|=0x10;			// Key F2
		if (KEY('E'))			data|=0x20;
		if (KEY('C'))			data|=0x40;
		if (KEY(K_RCL))			data|=0x80;
	}
	if (KS&0x20) {
		if (KEY('='))			data|=1;			// =
		if (KEY(K_LA))			data|=2;			// LEFT ARROW
		if (KEY('P'))			data|=4;
		if (KEY('F'))			data|=8;
		if (KEY(K_F3))			data|=0x10;
		if (KEY('R'))			data|=0x20;
		if (KEY('V'))			data|=0x40;
		if (KEY(' '))			data|=0x80;
	}
	if (KS&0x40) {
		if (KEY(K_RA))			data|=1;			// R ARROW
		if (KEY(K_MOD))			data|=2;			// MODE
		if (KEY(K_CLR))			data|=4;			// CLS
		if (KEY('A'))			data|=8;
        if (KEY(K_CTRL))		data|=0x10;         // DEF
		if (KEY('Q'))			data|=0x20;
		if (KEY('Z'))			data|=0x40;
		if (KEY(K_SML))			data|=0x80;
	}
	if (KS&0x80) {
		if (KEY('3'))			data|=1;
		if (KEY('6'))			data|=2;
		if (KEY('9'))			data|=4;
		if (KEY('G'))			data|=8;
		if (KEY(K_F4))			data|=0x10;			// Key F4
		if (KEY('T'))			data|=0x20;
		if (KEY('B'))			data|=0x40;
		if (KEY(K_DA))			data|=0x80;			// DOWN ARROW
	}

	return data^0xff;
}


void Cpc15XX::Regs_Info(UINT8 Type)
{
    Q_UNUSED(Type)

	strcat(Regs_String,	"");
	CpcXXXX::Regs_Info(0);
	pLH5810->Regs_Info(0);
	pCPU->Regs_Info(0);
	strcat(Regs_String,	"\r\n");
	strcat(Regs_String,pLH5810->Regs_String);
}


bool Cpc15XX::Set_Connector(Cbus *_bus)
{
    Q_UNUSED(_bus)

    // transfert busValue to Connector

    ((CbusPc1500*)bus)->setPU(((CLH5801 *)pCPU)->lh5801.pu);
    ((CbusPc1500*)bus)->setPV(((CLH5801 *)pCPU)->lh5801.pv);
    ((CbusPc1500*)bus)->setCMTOUT(pLH5810->SDO);
    pCONNECTOR->Set_values(bus->toUInt64());
//    qWarning()<<"Cpc15XX::Set_Connector:"<<bus->toLog();
    return true;
}
bool Cpc15XX::Get_Connector(Cbus *_bus)
{
    Q_UNUSED(_bus)

    bus->fromUInt64(pCONNECTOR->Get_values());
    bus->setEnable(false);

    return true;
}

bool	CLH5810_PC1500::init(void)
{
    CLH5810::init();
    SetRegBit(OPB,3,true);	// Export model vs domestic model
    SetRegBit(OPB,4,false);	// PB4 to GND

	return(1);
}						//initialize



bool CLH5810_PC1500::step()
{
    Cpc15XX *pc1500 = (Cpc15XX *)pPC;

	////////////////////////////////////////////////////////////////////
	//	INT FROM connector to IRQ
	////////////////////////////////////////////////////////////////////
    IRQ= ((CbusPc1500*)pc1500->bus)->getINT();

	////////////////////////////////////////////////////////////////////
	//	Send Data to PD1990AC -- TIMER
	////////////////////////////////////////////////////////////////////

    if (New_OPC) {
        UINT8 t = lh5810.r_opc;
        pPD1990AC->Set_data(READ_BIT(t,0));		// PC0
        pPD1990AC->Set_stb(READ_BIT(t,1));		// PC1
        pPD1990AC->Set_clk(READ_BIT(t,2));		// PC2
        pPD1990AC->Set_out_enable(READ_BIT(t,3));	// PC3
        pPD1990AC->Set_c0(READ_BIT(t,3));			// PC3
        pPD1990AC->Set_c1(READ_BIT(t,4));			// PC4
        pPD1990AC->Set_c2(READ_BIT(t,5));			// PC5

        pPD1990AC->step();
        New_OPC = false;
    }
	// PB5 = TP
	// PB6 = DATA
    SetRegBit(OPB,5,pPD1990AC->Get_tp());
    SetRegBit(OPB,6,pPD1990AC->Get_data());

	
	////////////////////////////////////////////////////////////////////
	//	ON/Break
	////////////////////////////////////////////////////////////////////
    SetRegBit(OPB,7,pPC->pKEYB->Kon);

	////////////////////////////////////////////////////////////////////
	//	TAPE READER
	////////////////////////////////////////////////////////////////////
    SetRegBit(OPB,2,((CbusPc1500*)pc1500->bus)->isCMTIN());
    CLI = CLO;

    SetRegBit(OPB,3,true);	// Export model vs domestic model
    SetRegBit(OPB,4,false);	// PB4 to GND

	  //----------------------//
	 // Standard LH5810 STEP //
	//----------------------//
	CLH5810::step();

	return(1);
}

void Cpc15XX::contextMenuEvent ( QContextMenuEvent * event )
{
    QMenu *menu = new QMenu(this);
	
    BuildContextMenu(menu);
		
    menu->popup(event->globalPos () );
}

extern int ask(QWidget *parent,QString msg,int nbButton);
#define KEY(c)	((pKEYB->keyPressedList.contains(TOUPPER(c)) || \
                  pKEYB->keyPressedList.contains(c) || \
                  pKEYB->keyPressedList.contains(TOLOWER(c)))?1:0)
void Cpc15XX::ComputeKey(KEYEVENT ke,int scancode)
{
    Q_UNUSED(ke)
    Q_UNUSED(scancode)

    // Manage left connector click
    if (KEY(0x240) && (currentView==LEFTview)) {
        pKEYB->keyPressedList.removeAll(0x240);
        FluidLauncher *launcher = new FluidLauncher(mainwindow,
                                     QStringList()<<P_RES(":/pockemul/configExt.xml"),
                                     FluidLauncher::PictureFlowType,QString(),
                                     "Sharp_60");
        launcher->show();
    }
}
