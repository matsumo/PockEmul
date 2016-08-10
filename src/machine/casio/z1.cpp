// a lot of details at http://homepage2.nifty.com/stear/fx890p/indx.htm

#include <QtGui>
#include <QString>

#include "common.h"
#include "z1.h"
#include "i80L188EB.h"
#include "pit8253.h"
#include "Inter.h"
#include "Keyb.h"
#include "Log.h"
#include "Lcdc_z1.h"
#include "hd66108.h"
#include "cf79107pj.h"
#include "Log.h"
#include "Connect.h"
#include "ctronics.h"
#include "cextension.h"
#include "watchpoint.h"
#include "renderView.h"

#ifdef POCKEMUL_BIG_ENDIAN
#	define LOW(x)	((uint8 *)&(x) + 1)
#	define HIGH(x)	((uint8 *)&(x) + 0)
#else
#	define LOW(x)	((uint8 *)&(x) + 0)
#	define HIGH(x)	((uint8 *)&(x) + 1)
#endif

extern CrenderView* view;

Cz1::Cz1(CPObject *parent, Models mod)	: CpcXXXX(parent)
{								//[constructor]
    setfrequency( (int) 3686400);
    model = mod;
    switch (model) {
    case Z1GR:
        setcfgfname(QString("z1gr"));
        SessionHeader	= "Z1GGRPKM";
        Initial_Session_Fname ="z1gr.pkm";
        BackGroundFname	= P_RES(":/z1/z1gr.png");
        break;
    case FX890P:
        setcfgfname(QString("fx890p"));
        SessionHeader	= "FX890PPKM";
        Initial_Session_Fname ="fx890p.pkm";
        BackGroundFname	= P_RES(":/z1/fx-890p.png");
        break;
    default:
        setcfgfname(QString("z1"));
        SessionHeader	= "Z1PKM";
        Initial_Session_Fname ="z1.pkm";
        BackGroundFname	= P_RES(":/z1/z1.png");
    }

    LeftFname       = P_RES(":/z1/z1Left.png");

    memsize		= 0x100000;
    InitMemValue	= 0x00;

    SlotList.clear();
    SlotList.append(CSlot(64 , 0x00000 ,	""                  , ""	, CSlot::RAM , "RAM"));
    SlotList.append(CSlot(64 , 0x10000 ,	""                  , ""	, CSlot::RAM , "RAM"));
    SlotList.append(CSlot(64 , 0x20000 ,	""                  , ""	, CSlot::RAM , "RAM"));
    SlotList.append(CSlot(64 , 0x30000 ,	""                  , ""	, CSlot::RAM , "RAM"));
    SlotList.append(CSlot(32 , 0x40000 ,	""                  , ""	, CSlot::RAM , "RAM"));
    SlotList.append(CSlot(64 , 0xa0000 ,	""                  , ""	, CSlot::RAM , "VIDEO RAM"));

    if (model==Z1GR)
        SlotList.append(CSlot(128 , 0xE0000 ,	P_RES(":/z1/romz1gr.bin")	, ""	, CSlot::ROM , "ROM"));
    else
        SlotList.append(CSlot(128 , 0xE0000 ,	P_RES(":/z1/romz1.bin")	, ""	, CSlot::ROM , "ROM"));


    PowerSwitch	= 0;

    setDXmm(206);
    setDYmm(83);
    setDZmm(25);

    setDX(736);
    setDY(297);

    PowerSwitch = 0;

    pLCDC		= new Clcdc_z1(this,
                               QRect(77,44,384,64),
                               QRect(67,44,75,64));
    pCPU		= new Ci80L188EB(this);
    pFPU        = new CCF79107PJ(this);
    pTIMER		= new Ctimer(this);
    pHD66108    = new CHD66108(this);
    pKEYB->setMap("z1.map");
    pCENT       = new Cctronics(this); pCENT->pTIMER = pTIMER;

    lastKeyBufSize = 0;
    newKey = false;

    i80l188ebcpu = (Ci80L188EB*)pCPU;

    ioFreq = 0;

    pCENTflip = true;
}

Cz1::~Cz1() {
    delete pFPU;
    delete pHD66108;
    delete pCENTCONNECTOR;
    delete pCENT;
}


bool Cz1::init(void)				// initialize
{
//    if (!fp_log) fp_log=fopen("z1.log","wt");	// Open log file
//    pCPU->logsw = true;
#ifndef QT_NO_DEBUG
//    pCPU->logsw = true;

#endif
    initExtension();
    CpcXXXX::init();

//    for(int i = 0; i < 0x10000; i++) mem[i] = i & 0xff;

    pHD66108->init();

    i80l188ebcpu->eoi = 0x8000;
    io_b8 = timer0Control = timer2Control = 0;

    intPulseId = pTIMER->initTP(240);

    lastIntPulse = false;

    pCENTCONNECTOR = new Cconnector(this,36,1,Cconnector::Centronics_36,"Parrallel Connector",false,QPoint(715,50));
    publish(pCENTCONNECTOR);
    pSIOCONNECTOR = new Cconnector(this,9,2,Cconnector::DB_25,"Serial Connector",false,QPoint(0,50));
    publish(pSIOCONNECTOR);
    pSERIALCONNECTOR = new Cconnector(this,3,3,Cconnector::Jack,"Serial Jack Connector",false,QPoint(0,20));
    publish(pSERIALCONNECTOR);

//    pUART->init();
//    pUART->pTIMER = pTIMER;

    pCENT->init();
    pCENT->setBufferSize(10);

    QHash<int,QString> lbl;
    lbl.clear();
    lbl[1] = "STROBE";
    lbl[2] = "D1";
    lbl[3] = "D2";
    lbl[4] = "D3";
    lbl[5] = "D4";
    lbl[6] = "D5";
    lbl[7] = "D6";
    lbl[8] = "D7";
    lbl[9] = "D8";
    lbl[10]= "ACK";
    lbl[11]= "BUSY";
    lbl[31]= "INIT";
    lbl[32]= "ERROR";
    QHash<int,QString> lbl_serial;
    lbl_serial.clear();
    lbl_serial[1] = "RXD";
    lbl_serial[2] = "TXD";
    WatchPoint.remove(this);
    WatchPoint.add(&pCENTCONNECTOR_value,64,36,this,"Centronic 36pins connector",lbl);
    WatchPoint.add(&pSERIALCONNECTOR_value,64,3,this,"Synchronous serial connector",lbl_serial);

    return true;
}

void	Cz1::initExtension(void)
{

    // initialise ext_MemSlot1
    ext_MemSlot1 = new CExtensionArray("Internal Memory","Custom internal memory");
    ext_MemSlot1->setAvailable(ID_RP_256,true);

    // initialise ext_MemSlot1
    ext_MemSlot2 = new CExtensionArray("Memory Slot","Add memory module");
    ext_MemSlot2->setAvailable(ID_RP_8,true);
    ext_MemSlot2->setAvailable(ID_RP_33,true);  ext_MemSlot1->setChecked(ID_RP_33,true);

    addExtMenu(ext_MemSlot1);
    addExtMenu(ext_MemSlot2);

    extensionArray[0] = ext_MemSlot1;
    extensionArray[1] = ext_MemSlot2;
}

bool Cz1::UpdateFinalImage(void) {

    CpcXXXX::UpdateFinalImage();

    // Draw switch by 180° rotation
    QPainter painter;
    painter.begin(FinalImage);

    // POWER SWITCH
    painter.drawImage(0,41*internalImageRatio,
                      BackgroundImageBackup->copy(0,41*internalImageRatio,
                                                  11*internalImageRatio,70*internalImageRatio).mirrored(false,off));

    painter.end();

    emit updatedPObject(this);
    return true;
}

bool Cz1::run() {

    pCENTCONNECTOR_value = pCENTCONNECTOR->Get_values();
    pSIOCONNECTOR_value = pSIOCONNECTOR->Get_values();
    pSERIALCONNECTOR_value = pSERIALCONNECTOR->Get_values();

//    quint64 _states = pTIMER->state;

    CpcXXXX::run();
    pCENT->run();

    bool pulse = pTIMER->GetTP(intPulseId);
    if (pulse != lastIntPulse) {

        lastIntPulse = pulse;

        // Check if keybuffer size change
        if (lastKeyBufSize != pKEYB->keyPressedCount()) {
            newKey = true;
            lastKeyBufSize = pKEYB->keyPressedCount();
        }

//        if (pCPU->halt && (pKEYB->LastKey == K_BRK)) {
//            newKey = false;
//            AddLog(LOG_MASTER,"INT NMI");
//            i80l188ebcpu->i86nmi(&i80l188ebcpu->i86stat);
//        }
        if (newKey) {
            if(i80l188ebcpu->eoi & 0x8000) {
                if(i80l188ebcpu->i86int(&(i80l188ebcpu->i86stat), 0x0c)) {
                    newKey = false;
                    if (pCPU->fp_log) fprintf(pCPU->fp_log,"INT 0x0C\n");
                    AddLog(LOG_MASTER,"INT 0x0C");
                    i80l188ebcpu->eoi = 0;
                }
            }
        }
    }

    pCENTCONNECTOR_value = pCENTCONNECTOR->Get_values();
    pSIOCONNECTOR_value = pSIOCONNECTOR->Get_values();
    pSERIALCONNECTOR_value = pSERIALCONNECTOR->Get_values();

    return true;
}

bool Cz1::Chk_Adr(UINT32 *d, UINT32 data)
{
    /*
    if(0x400 <= p && p <= 0x40f)
        printf("WRITE IP=%04x p=%04x v=%02x\n", cpu->r16.ip, p, v);
    */
    if (*d==0x16BB) {
        if (pCPU->fp_log) fprintf(pCPU->fp_log,"\nWRITE 16BB:%04X\n",data);
        return true;

    }
    if(*d < 0x50000) return true; /* RAM */
    if(*d < 0xa0000) return false;
    if(*d < 0xb0000){
        if (pCPU->fp_log) fprintf(pCPU->fp_log,"\nWRITE %06X:%04X\n",*d,data);
        AddLog(LOG_DISPLAY,tr("WriteVram[%1]=%2").arg(*d,5,QChar('0')).arg(data));
//        if(pCPU->fp_log) fprintf(pCPU->fp_log,"Write VRAM %c\n",data);
//        qWarning()<<"VRAM:"<<*d<<data;
        pHD66108->writeVram( *d, data);
        return false;
    }

    return false;
}

bool Cz1::Chk_Adr_R(UINT32 *d, UINT32 *data)
{
    Q_UNUSED(data)

    /*
    if(0x400 <= p && p <= 0x40f)
        printf("READ IP=%04x p=%04x v=%02x\n", cpu->r16.ip, p, cpu->m[p]);
    */
    if (*d==0x16BB) {
        if (pCPU->fp_log) fprintf(pCPU->fp_log,"\nREAD 16BB\n");
        return true;

    }


    if( *d < 0x20000) return true;
    else if (*d < 0xa0000) { mem[*d] = *d & 0xff; return true; }
    else if (*d < 0xb0000) {
        AddLog(LOG_DISPLAY,tr("ReadVram[%1]").arg(*d,5,QChar('0')));
        mem[*d] = pHD66108->readVram(*d);
        return true;
    }
    else if (*d < 0xe0000) { mem[*d] = *d & 0xff; return true; }

    return true;
}

UINT8 Cz1::in(UINT8 Port,QString)
{
    Q_UNUSED(Port)

    return 0;
}

UINT8 Cz1::in8(UINT16 Port,QString sender)
{
    Q_UNUSED(sender)

    UINT16 v=0;

    switch(Port) {
    case 0x36 : if (fp_log) fprintf(fp_log,"IN[%04x]    pc=%08x\n",Port,pCPU->get_PC());
        return 0x00; break;
    case 0x37 :
        if (fp_log) fprintf(fp_log,"IN[%04x]    pc=%08x\n",Port,pCPU->get_PC());
        return 0x10; break;
    case 0x003e: /* ? */
        return 0x20;
    case 0x005a: // model type  used by SYSTEM* test command
        return 0x04 | 0x80;

    case 0x62: return *LOW(i80l188ebcpu->pserial0->BxCNT);
    case 0x63: return *HIGH(i80l188ebcpu->pserial0->BxCNT);
    // RS232C communication input
    case 0x64:
        return  *LOW(i80l188ebcpu->pserial0->SxCON);
        if (fp_log) fprintf(fp_log,"IN[%04x]=80    pc=%08x\n",Port,pCPU->get_PC());
        return 0x08;
        break;
    case 0x65:
        if (fp_log) fprintf(fp_log,"IN[%04x]    pc=%08x\n",Port,pCPU->get_PC());
        return  *HIGH(i80l188ebcpu->pserial0->SxCON);
        break;
    case 0x66:
//        AddLog(LOG_SIO,tr("Read SxSTSL=%1").arg(*LOW(i80l188ebcpu->pserial0->SxSTS),2,16,QChar('0')));
        if (fp_log) fprintf(fp_log,"IN[%04x]    pc=%08x\n",Port,pCPU->get_PC());
        return  *LOW(i80l188ebcpu->pserial0->SxSTS);
        return 0x08;
    case 0x67:
//        AddLog(LOG_SIO,tr("Read SxSTSH=%1").arg(*HIGH(i80l188ebcpu->pserial0->SxSTS),2,16,QChar('0')));
        if (fp_log) fprintf(fp_log,"IN[%04x]    pc=%08x\n",Port,pCPU->get_PC());
        return  *HIGH(i80l188ebcpu->pserial0->SxSTS);
        break;
    case 0x68 : {
     quint8 x= i80l188ebcpu->pserial0->SxRBUF & 0xff;
        if (( x!=0xff)&&( x!=0x0d)) {
            AddLog(LOG_CONSOLE,tr("%1").arg(QChar(x)));
        }
    }
        return  *LOW(i80l188ebcpu->pserial0->SxRBUF);
    case 0x69 : return *HIGH(i80l188ebcpu->pserial0->SxRBUF);
    case 0x0082:
        return 0x0;
    case 0x0083:
if (ext_MemSlot1->ExtArray[ID_RP_256]->IsChecked) return 0x40;
        switch (model) {
        case Z1GR:
        case Z1: return 0x08;
        case FX890P : return 0x10;
        default: return 0x08;
        }

        return 0x08;
    case 0x0086:
        return 0x00;
    case 0x0087:
    if (ext_MemSlot1->ExtArray[ID_RP_256]->IsChecked) return 0x48;
        switch (model) {
        case Z1GR:
        case Z1: return 0x10;
        case FX890P : return 0x18;
        default: return 0x10;
        }

        return 0x10;

    /*
     *case 0x00a2:
        return 0x0a;
    case 0x00a3:
        return 0x08;
    */
    case 0x0202:
    case 0x0203:
        v = getKey();
        if(!v) v = 0x8000;
        v = (Port == 0x0202 ? v & 0xff: v >> 8);
        if (v) { AddLog(LOG_KEYBOARD,tr("Read Keyboard[%1] = %2").arg(Port,4,16,QChar('0')).arg(v&0xff,2,16,QChar('0'))); }
        return v;
    case 0x220:
        AddLog(LOG_TEMP,tr("IN [%1]=%2").arg(Port,4,16,QChar('0')).arg( pFPU->get_status(),4,16,QChar('0')));
        if (fp_log) fprintf(fp_log,"IN[%04x]=%02X\n",Port, pFPU->get_status());
        return pFPU->get_status(); break;
    case 0x221:
    case 0x222: {AddLog(LOG_TEMP,tr("IN [%1]").arg(Port,4,16,QChar('0')));
        if (fp_log) fprintf(fp_log,"IN[%04x]\n",Port);
        return 0x01;
        break;}
    case 0x241: // Ack from centronic
        return (pCENT->isAvailable()?0x00:0x01);
        break;
    default:
        if (fp_log) fprintf(fp_log,"IN[%04x]    pc=%08x\n",Port,pCPU->get_PC());
        AddLog(LOG_TEMP,tr("IN[%1]\t%2").arg(Port,4,16,QChar('0')).arg(pCPU->get_PC(),8,16,QChar('0')));
        return 0;
    }
     return (0);
}

UINT8 Cz1::out(UINT8 Port, UINT8 x, QString sender)
{
    Q_UNUSED(Port)
    Q_UNUSED(x)
    Q_UNUSED(sender)


    return 0;
}
UINT8 Cz1::out8(UINT16 Port,UINT8 x,QString sender)
{
    Q_UNUSED(sender)

    switch(Port) {
    case 0x0002:
        *LOW(i80l188ebcpu->eoi) = x;
//        AddLog(LOG_MASTER,tr("Set eoi Low[%1]=%2").arg(x,2,16,QChar('0')).arg(eoi,4,16,QChar('0')));
        break;
    case 0x0003:
        *HIGH(i80l188ebcpu->eoi) = x;
//        AddLog(LOG_MASTER,tr("Set eoi High[%1]=%2").arg(x,2,16,QChar('0')).arg(eoi,4,16,QChar('0')));
        break;
    case 0x0032:
        *LOW(i80l188ebcpu->p8253->t0->tcmpA) = x;
        break;
    case 0x0033:
        *HIGH(i80l188ebcpu->p8253->t0->tcmpA) = x;
        break;
    case 0x0034:
        *LOW(i80l188ebcpu->p8253->t0->tcmpB) = x;
        break;
    case 0x0035:
        *HIGH(i80l188ebcpu->p8253->t0->tcmpB) = x;
        break;
    case 0x0036:
        qWarning()<<"low tcon 0="<<x;
        *LOW(i80l188ebcpu->p8253->t0->tcon) = x;
        break;
    case 0x0037:
        qWarning()<<"high tcon 0="<<x;
        *HIGH(i80l188ebcpu->p8253->t0->tcon) = x;
        break;
    case 0x003a:
        *LOW(i80l188ebcpu->p8253->t1->tcmpA) = x;
        break;
    case 0x003b:
        *HIGH(i80l188ebcpu->p8253->t1->tcmpA) = x;
        break;
    case 0x003c:
        *LOW(i80l188ebcpu->p8253->t1->tcmpB) = x;
        break;
    case 0x003d:
        *HIGH(i80l188ebcpu->p8253->t1->tcmpB) = x;
        break;
    case 0x003e:
        qWarning()<<"low tcon 1="<<x;
        *LOW(i80l188ebcpu->p8253->t1->tcon) = x;
        break;
    case 0x003f:
        qWarning()<<"high tcon 1="<<x;
        *HIGH(i80l188ebcpu->p8253->t1->tcon) = x;
        break;
    case 0x0042:
        *LOW(i80l188ebcpu->p8253->t2->tcmpA) = x;
        break;
    case 0x0043:
        *HIGH(i80l188ebcpu->p8253->t2->tcmpA) = x;
        break;
    case 0x0046:
        qWarning()<<"low tcon 2="<<x;
        *LOW(i80l188ebcpu->p8253->t2->tcon) = x;
        AddLog(LOG_MASTER,tr("Set T2Control Low[%1]=%2").arg(x,2,16,QChar('0')).arg(timer2Control,4,16,QChar('0')));
        break;
    case 0x0047:
        qWarning()<<"high tcon 2="<<x;
        *HIGH(i80l188ebcpu->p8253->t2->tcon) = x;
        AddLog(LOG_MASTER,tr("Set T2Control High[%1]=%2").arg(x,2,16,QChar('0')).arg(timer2Control,4,16,QChar('0')));
        break;
    case 0x60: // B0CMP
        AddLog(LOG_SIO,tr("Set BxCMP Low=%1").arg(x,2,16,QChar('0')));
        *LOW(i80l188ebcpu->pserial0->BxCMP) = x;
        break;
    case 0x61:
        *HIGH(i80l188ebcpu->pserial0->BxCMP) = x;
        AddLog(LOG_SIO,tr("Set BxCMP High=%1").arg(x,2,16,QChar('0')));
        break;
    case 0x64:
        *LOW(i80l188ebcpu->pserial0->SxCON) = x;
        AddLog(LOG_SIO,tr("Write SxCONL:%1  mode=%2").arg(x,2,16,QChar('0')).arg(x&0x07,2,16,QChar('0')));
        if (fp_log) fprintf(fp_log,"IN[%04x]=80    pc=%08x\n",Port,pCPU->get_PC());
        break;
    case 0x65:
        if (fp_log) fprintf(fp_log,"IN[%04x]    pc=%08x\n",Port,pCPU->get_PC());
        *HIGH(i80l188ebcpu->pserial0->SxCON) = x;
        AddLog(LOG_SIO,tr("Write SxCONH:%1").arg(x,2,16,QChar('0')));
        break;
    case 0x6a:
        if (( x!=0xff)&&( x!=0x0d)) {
            AddLog(LOG_SIO,tr("send L:[%1]=%2").arg(x,2,16,QChar('0')).arg(QChar(x)));
        }
        i80l188ebcpu->pserial0->set_SxTBUFL(x);
        //*LOW(i80l188ebcpu->pserial0->SxTBUF) = x;
        break;
    case 0x6b:
        if (( x!=0xff)&&( x!=0x0d)) {
            AddLog(LOG_SIO,tr("send H:[%1]=%2").arg(x,2,16,QChar('0')).arg(QChar(x)));
        }
        //*HIGH(i80l188ebcpu->pserial0->SxTBUF) = x;
        break;
    case 0x00b8:
        io_b8 = x;
        break;
    case 0x0200:
//        AddLog(LOG_KEYBOARD,tr("Set KSL[%1]=%2").arg(x,2,16,QChar('0')).arg(ks,4,16,QChar('0')));
        *LOW(ks) = x;
        break;
    case 0x0201:
//        AddLog(LOG_KEYBOARD,tr("Set KSH[%1]=%2").arg(x,2,16,QChar('0')).arg(ks,4,16,QChar('0')));
        *HIGH(ks) = x;
        break;
    case 0x206 : // buzzer : 3 active, 0 stop
        break;
    case 0x0220: /* ?? */
        if (fp_log) fprintf(fp_log,"OUT[%04x]=%02x\tpc=%08x\n",Port,x,pCPU->get_PC());
        pFPU->instruction2(x);
        break;
    case 0x0221:
        if (fp_log) fprintf(fp_log,"OUT[%04x]=%02x\tpc=%08x\n",Port,x,pCPU->get_PC());
        pFPU->instruction1(x);
        break;
    case 0x280: AddLog(LOG_PRINTER,tr("[%1]:'%2'").arg(x,2,16,QChar('0')).arg(QChar(x)));
        if (( x!=0xff)&&( x!=0x0d)) {
            AddLog(LOG_CONSOLE,tr("%1").arg(QChar(x)));
        }
        // The Z1 seems to send each char followed by 0xff to the printer
        // For now we will cancel the second char sent if 0xff

        pCENTflip = !pCENTflip;
        if (pCENTflip) {
//            qWarning()<<"Z1: new CENT Char:"<<x<< "=("<<QChar(x);
            pCENT->newOutChar( x );
        }
        else if (x!=0xff) {
            pCENT->newOutChar( x );
//            qWarning()<<"Z1: double CENT Char:"<<x<< "=("<<QChar(x);
        }
        break;

    default:
        if (fp_log) fprintf(fp_log,"OUT[%04x]=%02x\tpc=%08x\n",Port,x,pCPU->get_PC());
        break;
    }

//    pCPU->imem[Port] = x;
    return 0;
}


UINT16 Cz1::in16(UINT16 address,QString sender)
{
    Q_UNUSED(sender)

    switch (address) {
    case 0x36 : return 0x1000; break;
    case 0x82 : return 0x400; break;
    case 0x86 : return 0x800; break;
    }

    return 0;
}

UINT16 Cz1::out16(UINT16 Port,UINT16 x,QString sender)
{
    Q_UNUSED(sender)

    switch (Port) {
    case 0x200: break;
    case 0x220: break;
    default:
        if (fp_log) fprintf(fp_log,"OUT16[%04x]=%04x\tpc=%08x\n",Port,x,pCPU->get_PC());
        break;
    }
    out8(Port, x & 0xff);
    out8(Port + 1, x >> 8);
    return 0;
}



void Cz1::TurnOFF(void) {
    mainwindow->saveAll = YES;
    CpcXXXX::TurnOFF();
    mainwindow->saveAll = ASK;
    AddLog(LOG_TEMP,"TURN OFF");

}

void Cz1::TurnON(void){
    CpcXXXX::TurnON();

    pHD66108->init();
}


void Cz1::Reset()
{
    CpcXXXX::Reset();

}

bool Cz1::LoadConfig(QXmlStreamReader *xmlIn)
{
    Q_UNUSED(xmlIn)

    return true;
}

bool Cz1::SaveConfig(QXmlStreamWriter *xmlOut)
{
    Q_UNUSED(xmlOut)

    return true;
}

/*
    KO0     KO1     KO2     KO3     KO4     KO5     KO6     KO7     KO8     KO9     KO10	KOS
Ki0	BRK
KI1         TAB     W       R       Y       I       P       MENU	CAL     DEGR	SQR
KI2         Q       E       T       U       O       2nd     log     ln      sin     X2
KI3         RESET   S       F       H       K       ;       MR      M +     (       ENG
KI4         A       D       G       J       L       :       7       8       )       CLS
KI5         CAPS	X       V       N       ,       ?       4       5       9       cos
KI6         Z       C       B       M       INS     DEL     1       2       6       ^
KI7                 SRCH	OUT     SPACE	LA      RA      .       3       *       ANS
KI8                 IN      CALC	=       DA      0       E       +       /
KI9                                                         ENTER   -       BS      tan
KIS                                                                                         SHFT

*/

UINT16 Cz1::getKey()
{

    UINT16 data=0;

    if ((pKEYB->LastKey) && ks )
    {
        if (ks&1) {
            if (KEY(K_BREAK))		data|=0x01;
        }
        if (ks&2) {

            if (KEY(K_TAB))			data|=0x02;
            if (KEY('Q'))			data|=0x04;
            if (KEY(K_INIT))		data|=0x08;
            if (KEY('A'))			data|=0x10;
            if (KEY(K_SML))			data|=0x20;
            if (KEY('Z'))			data|=0x40;
        }
        if (ks&4) {
            if (KEY('W'))			data|=0x02;
            if (KEY('E'))			data|=0x04;
            if (KEY('S'))			data|=0x08;
            if (KEY('D'))			data|=0x10;
            if (KEY('X'))			data|=0x20;
            if (KEY('C'))			data|=0x40;
            if (KEY(K_SEARCH))			data|=0x80;
            if (KEY(K_IN))			data|=0x100;
        }
        if (ks&8) {
            if (KEY('R'))			data|=0x02;
            if (KEY('T'))			data|=0x04;
            if (KEY('F'))           data|=0x08;
            if (KEY('G'))			data|=0x10;
            if (KEY('V'))			data|=0x20;
            if (KEY('B'))			data|=0x40;
            if (KEY(K_OUT))			data|=0x80;
            if (KEY(K_CALC))		data|=0x100;
        }
        if (ks&0x10) {
            if (KEY('Y'))			data|=0x02;
            if (KEY('U'))			data|=0x04;
            if (KEY('H'))			data|=0x08;
            if (KEY('J'))			data|=0x10;
            if (KEY('N'))			data|=0x20;
            if (KEY('M'))			data|=0x40;
            if (KEY(' '))			data|=0x80;
            if (KEY('='))			data|=0x100;
        }
        if (ks&0x20) {
            if (KEY('I'))			data|=0x02;
            if (KEY('O'))			data|=0x04;
            if (KEY('K'))			data|=0x08;
            if (KEY('L'))			data|=0x10;
            if (KEY(','))			data|=0x20;
            if (KEY(K_INS))			data|=0x40;
            if (KEY(K_LA))   		data|=0x80;
            if (KEY(K_DA))			data|=0x100;
        }
        if (ks&0x40) {
            if (KEY('P'))			data|=0x02;
            if (KEY(K_SHT2))         data|=0x04;
            if (KEY(';'))			data|=0x08;
            if (KEY(':'))			data|=0x10;
            if (KEY(K_UA))			data|=0x20;
            if (KEY(K_DEL))			data|=0x40;
            if (KEY(K_RA))			data|=0x80;
            if (KEY('0'))			data|=0x100;
        }
        if (ks&0x80) {
            if (KEY(K_MENU))		data|=0x02;
            if (KEY(K_LOG))			data|=0x04;
            if (KEY(K_RM))			data|=0x08;
            if (KEY('7'))			data|=0x10;
            if (KEY('4'))			data|=0x20;
            if (KEY('1'))			data|=0x40;
            if (KEY('.'))			data|=0x80;
            if (KEY(K_EXP))			data|=0x100;
            if (KEY(K_RET))			data|=0x200;
        }
        if (ks&0x100) {
            if (KEY(K_CAL))			data|=0x02;
            if (KEY(K_LN))  		data|=0x04;
            if (KEY(K_MPLUS))		data|=0x08;
            if (KEY('8'))			data|=0x10;
            if (KEY('5'))			data|=0x20;
            if (KEY('2'))			data|=0x40;
            if (KEY('3'))			data|=0x80;
            if (KEY('+'))			data|=0x100;
            if (KEY('-'))			data|=0x200;
        }

        if (ks&0x200) {
            if (KEY(K_DEG))			data|=0x02;
            if (KEY(K_SIN))			data|=0x04;
            if (KEY('('))			data|=0x08;
            if (KEY(')'))			data|=0x10;
            if (KEY('9'))			data|=0x20;
            if (KEY('6'))			data|=0x40;
            if (KEY('*'))			data|=0x80;
            if (KEY('/'))			data|=0x100;
            if (KEY(K_BS))			data|=0x200;
        }
        if (ks&0x400) {
            if (KEY(K_ROOT))		data|=0x02;
            if (KEY(K_SQR))			data|=0x04;
            if (KEY(K_ENG))			data|=0x08;
            if (KEY(K_CLR))			data|=0x10;
            if (KEY(K_COS))			data|=0x20;
            if (KEY(K_POT))			data|=0x40;
            if (KEY(K_ANS))			data|=0x80;
            if (KEY(K_TAN))			data|=0x200;
        }
        if (ks&0x800) {
            if (KEY(K_SHT))			data|=0x800;
            if (pKEYB->isShift)     data|=0x800;
        }
//        if (fp_log) fprintf(fp_log,"Read key [%02x]: strobe=%02x result=%02x\n",pKEYB->LastKey,ks,data^0xff);

    }
    return data;

}

bool Cz1::Get_Connector(Cbus *_bus) {
    Q_UNUSED(_bus)

    Get_SerialConnector();
    Get_CentConnector();
    Get_SIOConnector();

    return true;
}

bool Cz1::Set_Connector(Cbus *_bus) {
    Q_UNUSED(_bus)

    Set_SerialConnector();
    Set_SIOConnector();
    Set_CentConnector();


    return true;
}

void Cz1::Get_SerialConnector(void) {

    i80l188ebcpu->pserial0->set_RXD(pSERIALCONNECTOR->Get_pin(1));
}
void Cz1::Set_SerialConnector(void) {
    pSERIALCONNECTOR->Set_pin(2,!i80l188ebcpu->pserial0->get_TXD());
}

void Cz1::Get_CentConnector(void) {

    pCENT->Set_ACK( pCENTCONNECTOR->Get_pin(10));
    pCENT->Set_BUSY( pCENTCONNECTOR->Get_pin(11));
    pCENT->Set_ERROR( pCENTCONNECTOR->Get_pin(32));
}

void Cz1::Set_CentConnector(void) {

    pCENTCONNECTOR->Set_pin((1) ,pCENT->Get_STROBE());

    quint8 d = pCENT->Get_DATA();
    if ((d>0)&&(d!=0xff)) {
//        qWarning()<< "centdata"<<d;
    }
    pCENTCONNECTOR->Set_pin(2	,READ_BIT(d,0));
    pCENTCONNECTOR->Set_pin(3	,READ_BIT(d,1));
    pCENTCONNECTOR->Set_pin(4	,READ_BIT(d,2));
    pCENTCONNECTOR->Set_pin(5	,READ_BIT(d,3));
    pCENTCONNECTOR->Set_pin(6	,READ_BIT(d,4));
    pCENTCONNECTOR->Set_pin(7	,READ_BIT(d,5));
    pCENTCONNECTOR->Set_pin(8	,READ_BIT(d,6));
    pCENTCONNECTOR->Set_pin(9	,READ_BIT(d,7));

    pCENTCONNECTOR->Set_pin(31	,pCENT->Get_INIT());

}

void Cz1::Get_SIOConnector(void) {

}

void Cz1::Set_SIOConnector(void) {

}

void Cz1::ComputeKey(CPObject::KEYEVENT ke, int scancode, QMouseEvent *event)
{
    Q_UNUSED(ke)
    Q_UNUSED(scancode)
    Q_UNUSED(event)

    if ((currentView==LEFTview) && KEY(K_CASIO30PINS)) {
#if 1
        view->pickExtensionConnector("Centronics_36");
#else
        FluidLauncher *launcher = new FluidLauncher(mainwindow,
                                     QStringList()<<P_RES(":/pockemul/configExt.xml"),
                                     FluidLauncher::PictureFlowType,QString(),
                                     "Sharp_11");
        launcher->show();
#endif
    }
}
