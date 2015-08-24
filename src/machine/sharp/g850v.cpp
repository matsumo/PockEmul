//TODO: PIO output
#include <QDebug>

#include "common.h"

#include "g850v.h"
#include "Connect.h"

#include "z80.h"
#include "sed1560.h"
#include "Inter.h"
#include "Keyb.h"
#include "Lcdc_g850.h"
#include "Log.h"

#define PIN11IF_3IO     0	/* 3in 3out I/F */
#define PIN11IF_8PIO	1	/* 8bits PIO    */
#define PIN11IF_UART	2	// UART

Cg850v::Cg850v(CPObject *parent)	: CpcXXXX(parent)
{
    Q_UNUSED(parent)

    //[constructor]
#ifndef QT_NO_DEBUG
    if (!fp_log) fp_log=fopen("g850.log","wt");	// Open log file
#endif

    setfrequency( (int) 8000000);
    ioFreq = 0;
    setcfgfname(QString("g850"));

    SessionHeader	= "G850PKM";
    Initial_Session_Fname ="g850.pkm";
    BackGroundFname	= P_RES(":/G850V/g850v.png");

    LeftFname = P_RES(":/G850V/g850vLeft.png");

    memsize			= 0x060000;
    InitMemValue	= 0xff;

    SlotList.clear();

    setDXmm(196);
    setDYmm(95);
    setDZmm(20);

    setDX(683);
    setDY(330);

    SoundOn			= false;

    pLCDC		= new Clcdc_g850(this,
                                 QRect(67,60,288,96),
                                 QRect(50,60,47+288,96));
    pCPU        = new CZ80(this);
    pTIMER		= new Ctimer(this);
    pKEYB		= new Ckeyb(this,"g850v.map");
    pSED1560    = new CSED1560(this);

    Tape_Base_Freq=2500;

    SlotList.clear();
    SlotList.append(CSlot(1, 0x00000 ,	P_RES(":/G850V/base.bin")          , "" , CSlot::RAM , "RAM BASE"));
    SlotList.append(CSlot(32, 0x00000 ,	""                          , "" , CSlot::RAM , "RAM"));

    SlotList.append(CSlot(16, 0x08000 ,	P_RES(":/G850V/rom00.bin")         , "" , CSlot::ROM , "ROM BANK 00"));
    SlotList.append(CSlot(16, 0x0C000 ,	P_RES(":/G850V/rom01.bin")         , "" , CSlot::ROM , "ROM BANK 01"));

    SlotList.append(CSlot(16, 0x10000 ,	P_RES(":/G850V/rom02.bin")         , "" , CSlot::ROM , "ROM BANK 02"));
    SlotList.append(CSlot(16, 0x14000 ,	P_RES(":/G850V/rom03.bin")         , "" , CSlot::ROM , "ROM BANK 03"));
    SlotList.append(CSlot(16, 0x18000 ,	P_RES(":/G850V/rom04.bin")         , "" , CSlot::ROM , "ROM BANK 04"));
    SlotList.append(CSlot(16, 0x1C000 ,	P_RES(":/G850V/rom05.bin")         , "" , CSlot::ROM , "ROM BANK 05"));

    SlotList.append(CSlot(16, 0x20000 ,	P_RES(":/G850V/rom06.bin")         , "" , CSlot::ROM , "ROM BANK 06"));
    SlotList.append(CSlot(16, 0x24000 ,	P_RES(":/G850V/rom07.bin")         , "" , CSlot::ROM , "ROM BANK 07"));
    SlotList.append(CSlot(16, 0x28000 ,	P_RES(":/G850V/rom08.bin")         , "" , CSlot::ROM , "ROM BANK 08"));
    SlotList.append(CSlot(16, 0x2C000 ,	P_RES(":/G850V/rom09.bin")         , "" , CSlot::ROM , "ROM BANK 09"));

    SlotList.append(CSlot(16, 0x30000 ,	P_RES(":/G850V/rom0a.bin")         , "" , CSlot::ROM , "ROM BANK 0a"));
    SlotList.append(CSlot(16, 0x34000 ,	P_RES(":/G850V/rom0b.bin")         , "" , CSlot::ROM , "ROM BANK 0b"));
    SlotList.append(CSlot(16, 0x38000 ,	P_RES(":/G850V/rom0c.bin")         , "" , CSlot::ROM , "ROM BANK 0c"));
    SlotList.append(CSlot(16, 0x3C000 ,	P_RES(":/G850V/rom0d.bin")         , "" , CSlot::ROM , "ROM BANK 0d"));

    SlotList.append(CSlot(16, 0x40000 ,	P_RES(":/G850V/rom0e.bin")         , "" , CSlot::ROM , "ROM BANK 0e"));
    SlotList.append(CSlot(16, 0x44000 ,	P_RES(":/G850V/rom0f.bin")         , "" , CSlot::ROM , "ROM BANK 0f"));
    SlotList.append(CSlot(16, 0x48000 ,	P_RES(":/G850V/rom10.bin")         , "" , CSlot::ROM , "ROM BANK 10"));
    SlotList.append(CSlot(16, 0x4C000 ,	P_RES(":/G850V/rom11.bin")         , "" , CSlot::ROM , "ROM BANK 11"));

    SlotList.append(CSlot(16, 0x50000 ,	P_RES(":/G850V/rom12.bin")         , "" , CSlot::ROM , "ROM BANK 12"));
    SlotList.append(CSlot(16, 0x54000 ,	P_RES(":/G850V/rom13.bin")         , "" , CSlot::ROM , "ROM BANK 13"));
    SlotList.append(CSlot(16, 0x58000 ,	P_RES(":/G850V/rom14.bin")         , "" , CSlot::ROM , "ROM BANK 14"));
    SlotList.append(CSlot(16, 0x5C000 ,	P_RES(":/G850V/rom15.bin")         , "" , CSlot::ROM , "ROM BANK 15"));

    keyBreak = pin11If = 0;

}

Cg850v::~Cg850v()
{

}


bool Cg850v::init()
{
    // if DEBUG then log CPU
//    pCPU->logsw = true;
#ifndef QT_NO_DEBUG
    pCPU->logsw = true;
#endif
    CpcXXXX::init();
    pCONNECTOR	= new Cconnector(this,11,0,Cconnector::Sharp_11,"Connector 11 pins",false,QPoint(0,90));	publish(pCONNECTOR);

    pCPU->init();
    ks1=ks2=romBank = exBank = ramBank = 0;
    return true;
}


bool Cg850v::Set_Connector(Cbus *_bus)
{
    Q_UNUSED(_bus)

#if 0
    switch(pin11If) {
    case PIN11IF_3IO:
        return (io3Out & 0x03) | ((io3Out >> 4) & 0x08);
    case PIN11IF_8PIO:
//        return ~pio8Io & pio8Out;
    case PIN11IF_UART:
//		return 0;
    }
#endif


    return true;
}

bool Cg850v::Get_Connector(Cbus *_bus)
{
    Q_UNUSED(_bus)

    pin11If = pCONNECTOR->Get_values() >> 3;

    return true;
}


void Cg850v::TurnON()
{
    pSED1560->updated = true;
//    CpcXXXX::TurnON();


#if 1
    if (!Power && pKEYB->LastKey == K_BRK) {
        AddLog(LOG_MASTER,"TURN ON");
        if (!hardreset) {
            Initial_Session_Load();
        }
        else hardreset = false;
        off = 0;
        Power = true;
        PowerSwitch = PS_RUN;
        if (pLCDC) pLCDC->TurnON();
        this->Reset();
    }
#endif
//    pSED1560->updated = true;
}

void Cg850v::TurnOFF(void) {
    mainwindow->saveAll = YES;
    CpcXXXX::TurnOFF();
    mainwindow->saveAll = ASK;
}

void Cg850v::Reset()
{
    pCPU->Reset();
    Mem_Load(0);

    out( 0x11, 0);
    out( 0x12, 0);
    out( 0x14, 0);
    out( 0x15, 1);
    out( 0x16, 0xff);
    out( 0x17, 0x0f);
    out( 0x18, 0);
    out( 0x19, 0);
    out( 0x1b, 3);
    out( 0x1c, 1);
    ((CZ80*)pCPU)->z80.r.im = 1;
    mem[0x790d] = 0;

    mem[0x779c] = (mem[0x779c] >= 0x07 && mem[0x779c] <= 0x1f ? mem[0x779c]: 0x0f);
    out( 0x40, 0x24);
    out( 0x40, mem[0x790d] + 0x40);
    out( 0x40, mem[0x779c] + 0x80);
    out( 0x40, 0xa0);
    out( 0x40, 0xa4);
    out( 0x40, 0xa6);
    out( 0x40, 0xa9);
    out( 0x40, 0xaf);
    out( 0x40, 0xc0);
    out( 0x40, 0x25);
    out( 0x60, 0);
    out( 0x61, 0xff);
    out( 0x62, 0);
    out( 0x64, 0);
    out( 0x65, 1);
    out( 0x66, 1);
    out( 0x67, 0);
    out( 0x6b, 4);
    out( 0x6c, 0);
    out( 0x6d, 0);
    out( 0x6e, 4);


}

bool Cg850v::Mem_Mirror(UINT32 *d)
{
    if ( (*d>=0x8000) && (*d<=0xbfff) ) {
        if (exBank != 0) {
            *d = 0x8000 + 0x1840;   // Tricks to return 0x2d value :-(
        }
    }
    else if ( (*d>=0xc000) && (*d<=0xffff) ) {
        if (romBank < 23) {
            *d += ((romBank-1) * 0x4000);
        }

    }
    return true;
}

bool Cg850v::Chk_Adr(UINT32 *d, UINT32 data)
{
    Q_UNUSED(data)

    Mem_Mirror(d);
    if (*d<=0x7fff) return true;
    return false;
}

bool Cg850v::Chk_Adr_R(UINT32 *d, UINT32 *data)
{
    Q_UNUSED(data)

    Mem_Mirror(d);
    return true;
}

UINT8 Cg850v::in(UINT8 address,QString)
{
    switch(address) {
    case 0x10: pCPU->imem[address] = getKey();
        return 0;
    case 0x11: pCPU->imem[address] = 0x00;
        return 0;
    case 0x12:
        return 0;
    case 0x13: pCPU->imem[address] = (ks1 & 0x08) ? (pKEYB->isShift?1:0):0;
        return 0;
    case 0x14:
        return 0;
    case 0x15:
        return 0;
    case 0x16: pCPU->imem[address] = interruptType;
        return 0;
    case 0x17: pCPU->imem[address] = interruptMask;
        return 0;
    case 0x18:
        pCPU->imem[address] =
                (pCONNECTOR->Get_pin(PIN_MT_OUT1)?0x80:0x00) |
                (pCONNECTOR->Get_pin(PIN_D_OUT)?0x02:0x00) |
                (pCONNECTOR->Get_pin(PIN_BUSY)?0x01:0x00) ;
        return 0;
    case 0x19:
        pCPU->imem[address] = ((exBank & 0x07) << 4) | (romBank & 0x0f);
        return 0;
    case 0x1a: pCPU->imem[address] = 0x00;
        return 0;
    case 0x1b:
        AddLog(LOG_CONSOLE,tr("IN RAM bank:%1\n").arg(ramBank));
        pCPU->imem[address] = ramBank;
        return 0;
    case 0x1c: pCPU->imem[address] = 0x00;
        return 0;
    case 0x1d:
        pCPU->imem[address] = 0x08;
        return 0;
    case 0x1e:
        pCPU->imem[address] = 0x00;
        return 0;
    case 0x1f:
        pCPU->imem[address] = (keyBreak) |
                (pCONNECTOR->Get_pin(PIN_MT_IN)?0x04:0x00) |
                (pCONNECTOR->Get_pin(PIN_ACK)?0x02:0x00) |
                (pCONNECTOR->Get_pin(PIN_D_IN)?0x01:0x00) ;

        return 0;
    case 0x40:
    case 0x42:
    case 0x44:
    case 0x46:
    case 0x48:
    case 0x4a:
    case 0x4c:
    case 0x4e:
    case 0x50:
    case 0x52:
    case 0x54:
    case 0x56:
    case 0x58:
    case 0x5a:
    case 0x5c:
    case 0x5e: pCPU->imem[address] = pSED1560->instruction(0x100);
        return 0;
    case 0x41:
    case 0x43:
    case 0x45:
    case 0x47:
    case 0x49:
    case 0x4b:
    case 0x4d:
    case 0x4f:
    case 0x51:
    case 0x53:
    case 0x55:
    case 0x57:
    case 0x59:
    case 0x5b:
    case 0x5d:
    case 0x5f: pCPU->imem[address] = pSED1560->instruction(0x500);
        return 0;
    case 0x60: pCPU->imem[address] = pin11If;
        AddLog(LOG_MASTER,tr("in 60 = %1").arg(pin11If));
        return 0;
    case 0x62: pCPU->imem[address] = (pCONNECTOR->Get_values()>>3) & ~pio8Io;
        return 0;
    case 0x6d: pCPU->imem[address] = 0x00;
        return 0;
    case 0x6f: pCPU->imem[address] = 0xff;
        return 0;
    }

    return 0;
}

UINT8 Cg850v::out(UINT8 address, UINT8 value, QString sender)
{
    switch(address) {
    case 0x11: ks1 = value;
        if(value & 0x10) interruptType |= 0x01;
        return 0;
    case 0x12: ks2 = value & 0x03;
        return 0;
    case 0x13:
        return 0;
    case 0x14:
        return 0;
    case 0x15:
        return 0;
    case 0x16: interruptType &= ~value & 0x0f;
        return 0;
    case 0x17: interruptMask = value;
        return 0;
    case 0x18:
//        io3Out = value & 0xc3;

        pCONNECTOR->Set_pin(PIN_MT_OUT1,value&0x80?true:false);
        pCONNECTOR->Set_pin(PIN_D_OUT,value&0x02?true:false);
        pCONNECTOR->Set_pin(PIN_BUSY,value&0x01?true:false);
        return 0;
    case 0x19:
        romBank = (value & 0x0f);
        exBank = ((value & 0x70) >> 4);
        if (pCPU->fp_log) fprintf(pCPU->fp_log,"BANK SWITCH: %i - %i\n",romBank,exBank);
        return 0;
    case 0x1a:
        return 0;
    case 0x1b:
        ramBank = value & 0x04;
        AddLog(LOG_CONSOLE,tr("RAM bank:%1\n").arg(ramBank));
        qWarning()<<"RAM BANK"<<ramBank;
        if (pCPU->fp_log) fprintf(pCPU->fp_log,"RAM BANK SWITCH: %i\n",ramBank);
        return 0;
    case 0x1c:
        return 0;
    case 0x1e:
        return 0;
    case 0x1f:
        return 0;
    case 0x40: pSED1560->instruction(0x200 | value);
        return 0;
    case 0x41: pSED1560->instruction(0x600 | value);
        return 0;
    case 0x60:
        AddLog(LOG_MASTER,tr("out 60 = %1").arg(value));
        pin11If = value & 0x03;
        return 0;
    case 0x61: pio8Io = value;
        return 0;
    case 0x62: 	pio8Out = value;
    case 0x69: romBank = value;
        if (pCPU->fp_log) fprintf(pCPU->fp_log,"ROM BANK SWITCH: %i\n",romBank);
        return 0;
    }

    return 0;
}




bool Cg850v::run()
{
    CpcXXXX::run();

#if 1
    if (pKEYB->LastKey==0) {
            keyBreak &= ~0x80;
        }
    else if ( (pKEYB->LastKey == K_BRK) && (interruptMask & 0x02) )  {
//        AddLog(LOG_MASTER,"BREAK");
        keyBreak |= 0x80;
//        pKEYB->LastKey = 0;
        interruptType |= 0x02;
        ((CZ80*)pCPU)->z80int1(&((CZ80*)pCPU)->z80);
    }
    else if ( (pKEYB->LastKey != 0) && (interruptMask & 0x01) )  {
//        AddLog(LOG_MASTER,"Key intr");
//        pKEYB->LastKey = 0;
        interruptType |= 0x01;
        ((CZ80*)pCPU)->z80int1(&((CZ80*)pCPU)->z80);
    }

#endif

    return true;
}


bool Cg850v::LoadConfig(QXmlStreamReader *xmlIn)
{
    Q_UNUSED(xmlIn)

//    pSED1560->Load_Internal(xmlIn);
    return true;
}

bool Cg850v::SaveConfig(QXmlStreamWriter *xmlOut)
{
    Q_UNUSED(xmlOut)

//    pSED1560->save_internal(xmlOut);
    return true;
}

bool Cg850v::InitDisplay()
{
    CpcXXXX::InitDisplay();

    return(1);
}

bool Cg850v::CompleteDisplay()
{
    return true;
}

//#define KEY(c)	( TOUPPER(pKEYB->LastKey) == TOUPPER(c) )
#define KEY(c)	( pKEYB->keyPressedList.contains(TOUPPER(c)) || pKEYB->keyPressedList.contains(c) || pKEYB->keyPressedList.contains(TOLOWER(c)))

BYTE Cg850v::getKey()
{

    UINT8 data=0;

    WORD ks = ks1 | (ks2 << 8);
    if ((pKEYB->LastKey) && ks )
    {
        if (ks&1) {
            if (KEY(K_OF))			data|=0x01;
            if (KEY('Q'))			data|=0x02;
            if (KEY('W'))			data|=0x04;
            if (KEY('E'))			data|=0x08;
            if (KEY('R'))			data|=0x10;
            if (KEY('T'))			data|=0x20;
            if (KEY('Y'))			data|=0x40;
            if (KEY('U'))			data|=0x80;
        }
        if (ks&2) {
            if (KEY('A'))			data|=0x01;
            if (KEY('S'))			data|=0x02;
            if (KEY('D'))			data|=0x04;
            if (KEY('F'))			data|=0x08;
            if (KEY('G'))			data|=0x10;
            if (KEY('H'))			data|=0x20;
            if (KEY('J'))			data|=0x40;
            if (KEY('K'))			data|=0x80;
        }
        if (ks&4) {
            if (KEY('Z'))			data|=0x01;
            if (KEY('X'))			data|=0x02;
            if (KEY('C'))			data|=0x04;
            if (KEY('V'))			data|=0x08;
            if (KEY('B'))			data|=0x10;
            if (KEY('N'))			data|=0x20;
            if (KEY('M'))			data|=0x40;
            if (KEY(','))			data|=0x80;
        }
        if (ks&8) {
            if (KEY(K_BASIC))		data|=0x01;
            if (KEY(K_TXT))			data|=0x02;
            if (KEY(K_SML))			data|=0x04;
            if (KEY(K_EXTMENU))		data|=0x08;
            if (KEY(K_TAB))			data|=0x10;
            if (KEY(' '))			data|=0x20;
            if (KEY(K_DA))			data|=0x40;
            if (KEY(K_UA))			data|=0x80;
        }
        if (ks&0x10) {
            if (KEY(K_LA))			data|=0x01;
            if (KEY(K_RA))			data|=0x02;
            if (KEY(K_ANS))			data|=0x04;
            if (KEY('0'))			data|=0x08;
            if (KEY('.'))			data|=0x10;
            if (KEY('='))			data|=0x20;
            if (KEY('+'))			data|=0x40;
            if (KEY(K_RET))			data|=0x80;
        }
        if (ks&0x20) {
            if (KEY('L'))			data|=0x01;
            if (KEY(';'))			data|=0x02;
            if (KEY(K_CON))			data|=0x04;
            if (KEY('1'))			data|=0x08;
            if (KEY('2'))			data|=0x10;
            if (KEY('3'))			data|=0x20;
            if (KEY('-'))			data|=0x40;
            if (KEY(K_MPLUS))		data|=0x80;
        }
        if (ks&0x40) {
            if (KEY('I'))			data|=0x01;
            if (KEY('O'))			data|=0x02;
            if (KEY(K_INS))			data|=0x04;
            if (KEY('4'))			data|=0x08;
            if (KEY('5'))			data|=0x10;
            if (KEY('6'))			data|=0x20;
            if (KEY('*'))			data|=0x40;
            if (KEY(K_RM))			data|=0x80;
        }
        if (ks&0x80) {
            if (KEY('P'))			data|=0x01;
            if (KEY(K_BS))			data|=0x02;
            if (KEY(K_PI))			data|=0x04;
            if (KEY('7'))			data|=0x08;
            if (KEY('8'))			data|=0x10;
            if (KEY('9'))			data|=0x20;
            if (KEY('/'))			data|=0x40;
            if (KEY(')'))			data|=0x80;
        }
        if (ks&0x100) {
            if (KEY(K_NPR))     	data|=0x01;
            if (KEY(K_DEG))			data|=0x02;
            if (KEY(K_ROOT))		data|=0x04;
            if (KEY(K_SQR))			data|=0x08;
            if (KEY(K_POT))			data|=0x10;
            if (KEY('('))			data|=0x20;
            if (KEY(K_1X))			data|=0x40;
            if (KEY(K_MDF))			data|=0x80;
        }

        if (ks&0x200) {
            if (KEY(K_SHT2))		data|=0x01;
            if (KEY(K_SIN))			data|=0x02;
            if (KEY(K_COS))			data|=0x04;
            if (KEY(K_LN))			data|=0x08;
            if (KEY(K_LOG))			data|=0x10;
            if (KEY(K_TAN))			data|=0x20;
            if (KEY(K_FSE))			data|=0x40;
            if (KEY(K_CCE))			data|=0x80;
        }
//        if (fp_log) fprintf(fp_log,"Read key [%02x]: strobe=%02x result=%02x\n",pKEYB->LastKey,ks,data^0xff);

    }
    return data;

}
