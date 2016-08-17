// TODO: LCD indicators see SnapFORTH vol 2 12-14
#define USEBUS
#include <QDebug>

#include "fluidlauncher.h"
#include "rlh1000.h"
#include "m6502/m6502.h"
#include "Inter.h"
#include "Keyb.h"
#include "Connect.h"
#include "Log.h"
#include "Lcdc_rlh1000.h"
#include "ui/dialogdasm.h"
#include "clink.h"
#include "watchpoint.h"
#include "overlay.h"

#include "renderView.h"

extern CrenderView* view;

Crlh1000::Crlh1000(CPObject *parent)	: CpcXXXX(parent)
{								//[constructor]
    setfrequency( (int) 4194300/4);
    setcfgfname(QString("rlh1000"));

    SessionHeader	= "RLH1000PKM";
    Initial_Session_Fname ="rlh1000.pkm";

    BackGroundFname	= P_RES(":/rlh1000/rlh1000.png");

    LeftFname   = P_RES(":/rlh1000/rlh1000Left.png");
//    RightFname  = P_RES(":/rlh1000/rlh1000Right.png");
    BackFname   = P_RES(":/rlh1000/rlh1000BackOpen.png");
    TopFname    = P_RES(":/rlh1000/rlh1000Top.png");
    BottomFname = P_RES(":/rlh1000/rlh1000Bottom.png");
    backDoorImage = new QImage(QString(P_RES(":/rlh1000/trappe.png")));

    memsize		= 0x20000;
    InitMemValue	= 0x7F;

    SlotList.clear();
    SlotList.append(CSlot(8 , 0x0000 ,	""    , ""	, CSlot::RAM , "RAM"));
    SlotList.append(CSlot(8 , 0x2000 ,	""    , ""	, CSlot::ROM , "Ext ROM"));
    SlotList.append(CSlot(16, 0x4000 ,	""    , ""	, CSlot::ROM , "ROM Capsules 1"));
    SlotList.append(CSlot(16, 0x8000 ,	""    , ""	, CSlot::NOT_USED , "Ext RAM"));
    SlotList.append(CSlot(16, 0xC000 ,	P_RES(":/rlh1000/HHC-rom-C000-FFFF.bin"),""	, CSlot::ROM , "ROM"));
    SlotList.append(CSlot(16, 0x10000 ,	""     , ""	, CSlot::ROM , "ROM Capsules 2"));
    SlotList.append(CSlot(16, 0x14000 ,	""    , ""	, CSlot::ROM , "ROM Capsules 3"));

// Ratio = 3,57
    setDXmm(227);
    setDYmm(95);
    setDZmm(31);

    setDX(811);
    setDY(340);

    PowerSwitch = 0;

    pLCDC		= new Clcdc_rlh1000(this,
                                    QRect(205,55,159*2.5,8*3),
                                    QRect());
    pCPU		= new Cm6502(this);    m6502 = (Cm6502*)pCPU;
    pTIMER		= new Ctimer(this);
    pKEYB->setMap("rlh1000.map");
    backdoorKeyIndex=0;
    capsuleKeyIndex=0;


    ioFreq = 0;

    extrinsicRAM = 0xff;
    extrinsicROM = 0xff;

    bus = new CbusPanasonic();
    backdoorOpen = false;
    backdoorFlipping = false;
    m_backdoorAngle = 0;
}

Crlh1000::~Crlh1000() {
    delete backDoorImage;
}


bool Crlh1000::init(void)				// initialize
{

//pCPU->logsw = true;
//    if (!fp_log) fp_log=fopen("rlh1000.log","wt");	// Open log file
#ifndef QT_NO_DEBUG
//    pCPU->logsw = true;
#endif
    qWarning()<<"Crlh1000::init";
    CpcXXXX::init();

    pCONNECTOR	= new Cconnector(this,44,0,Cconnector::Panasonic_44,"44 pins connector",false,
                                     QPoint(0,72),
                                 Cconnector::WEST);
    publish(pCONNECTOR);

    WatchPoint.add(&pCONNECTOR_value,64,44,this,"44 pins connector");

    pTIMER->resetTimer(1);

    latchByte = 0x00;

    timercnt1=0;
    timercnt2=0;
    timercnt3=10;
    memset(&lineFD,0xff,sizeof(lineFD));
    memset(&lineFE,0xff,sizeof(lineFE));
    memset(&lineFF,0xff,sizeof(lineFF));

    qWarning()<<pKEYB->Keys.size();
    // search key 0x242 index
    for (int i=0;i<pKEYB->Keys.size();i++)
    {
//        qWarning()<<i<<" - "<<pKEYB->Keys.at(i).ScanCode;
        if (pKEYB->Keys.at(i).ScanCode==0x242) {
            backdoorKeyIndex = i;
        }
        if (pKEYB->Keys.at(i).ScanCode==0x243) {
            capsuleKeyIndex = i;
            pKEYB->Keys[capsuleKeyIndex  ].enabled = backdoorOpen;
            pKEYB->Keys[capsuleKeyIndex+1].enabled = backdoorOpen;
            pKEYB->Keys[capsuleKeyIndex+2].enabled = backdoorOpen;
        }
    }

    overlays << new COverlay("Scientific Calculator",
                    QImage(P_RES(":/rlh1000/ScientificCalculator-Overlay.png")),
                    pKEYB->getKey(K_OVERLAY).Rect);


    return true;
}


bool Crlh1000::run() {

    // 0x5802 : Shift 0x80   ???
    // 0x5820 : 2nd 0x80     ???
    // timercnt = timer (1/256 sec)


    //TODO: change this !!!
    if (dialogdasm)
        dialogdasm->imem=false;

#ifndef QT_NO_DEBUG
//    if (pCPU->get_PC()==0xc854) m6502->set_PC(0xc856);
#endif

    Get_Connector();
    if (bus->getINT()) {
//        qWarning()<<"INT RECEIVED FROM BUS";
        m6502->write_signal(101,1,1);
        bus->setINT(false);
    }
    Set_Connector();

     CpcXXXX::run();



    if (pKEYB->LastKey>0) { m6502->write_signal(101,1,1); }

    if (pTIMER->usElapsedId(1)>=4096) {
//        if (timercnt1!=0)
        {
            timercnt1--;
            if ((timercnt1==0)&&(timercnt2==0)&&(timercnt3==0)) {
                m6502->write_signal(101,1,1);
                if (pCPU->fp_log) fprintf(pCPU->fp_log,"\n READ ROM KBD TIMER\n");
            }
            if (timercnt1==0xff) {
                m6502->write_signal(101,1,1);
                AddLog(LOG_CONSOLE,"INTERRUPT\n");
                timercnt2--;
                if (timercnt2==0xff) {
                    timercnt3--;
                }
            }
        }
        pTIMER->resetTimer(1);
    }

    Set_Connector();
    return true;
}

#define LINEID (islineFC?"FC":islineFD?"FD":islineFE?"FE":islineFF?"FF":"??")

bool Crlh1000::Chk_Adr(UINT32 *d, UINT32 data)
{

    if(*d < 0x2000) {
        if (pCPU->fp_log) {
            snprintf(pCPU->Log_String,sizeof(pCPU->Log_String),"%s Write[%04x]=%02x ",pCPU->Log_String,*d,data);
        }
        return true; /* RAM */
    }


    if((*d>=0x2000) && (*d < 0x4000)) { // ROM
        if (fp_log) {
            fprintf(fp_log,"**ERROR** WRITE ROM [%04X]=%02X ",*d,data);
            pCPU->Regs_Info(1);
            fprintf(fp_log," %s\n",pCPU->Regs_String);
        }
        if (pCPU->fp_log) {
            snprintf(pCPU->Log_String,sizeof(pCPU->Log_String),"%s Write ROM[%04x]=%02x ",pCPU->Log_String,*d,data);
        }
        if (extrinsicROM!=0xff) {
            bus->setDest(extrinsicROM);
            bus->setAddr(*d);
            bus->setData(data);
            bus->setFunc(BUS_WRITEDATA);
            bus->setWrite(true);
            manageBus();
            //        if (fp_log) fprintf(fp_log,"  AFTER DEST=%i  \n",bus->getDest());
            bus->setFunc(BUS_SLEEP);
            return false;
        }
        return false;
    }

    if (*d==0x58FE) {
        latchByte = data;
        if ((latchByte & 0x40) == 0) {
            // HALT CPU
            pCPU->halt = true;
//            qWarning()<<"CPU HALT";
            AddLog(LOG_CONSOLE,"CPU HALT\n");
            if (fp_log) fprintf(fp_log,"\nCPU HALT\n");
            if (pCPU->fp_log) fprintf(pCPU->fp_log,"\nCPU HALT\n");
        }
        if ( (latchByte & 0x03)==0x03) {
//            qWarning()<<"EXT ROM";
            if (pCPU->fp_log) fprintf(pCPU->fp_log,"\nEXT CAPSULE\n");
        }
        if (pCPU->fp_log) fprintf(pCPU->fp_log,"\n ROM latchByte=%02X\n",data);

        return false;
    }
    if (*d==0x58FB) { timercnt1=data; return false; }
    if (*d==0x58FC) { timercnt2=data; return false; }
    if (*d==0x58FD) { timercnt3=data; return false; }
    if (*d==0x58FF) {                 return false; }
    if((*d>=0x4000) && (*d <=0x7FFF)) {

        if (latchByte & 0x80){
            // LCD I/O Mapping
            if ((*d>=0x5800)&&(*d<0x58A0)) {
                ((Clcdc_rlh1000*)pLCDC)->mem[*d-0x5800] = data;
                pLCDC->SetDirtyBuf(*d-0x5800);
                return(false);
            }
            if (pCPU->fp_log) fprintf(pCPU->fp_log,"\n WRITE ROM LCD [%04X]=%02X\n",*d,data);

            return false;
        }


        // 47FF : write from ext, read by nucleus

        if ( (*d>=0x47FC) && (*d<=(0x47FF+0xff))) {
            if (pCPU->fp_log) snprintf(pCPU->Log_String,sizeof(pCPU->Log_String),"%s,WRITE ROM LINE [%04X]=%02X ",pCPU->Log_String,*d,data);
            bool islineFC = ((*d-0x47FC)%4==0);
            bool islineFD = ((*d-0x47FD)%4==0);
            bool islineFE = ((*d-0x47FE)%4==0);
            bool islineFF = ((*d-0x47FF)%4==0);

            if (islineFF) {
                quint8 t = (*d-0x47FF)/4;

                bus->setDest(t);
                bus->setData(data);
                bus->setWrite(true);
                bus->setFunc(BUS_LINE3);
                if (fp_log) fprintf(fp_log,"BUS_TOUCH DEST=%i data=%02x \n",bus->getDest(),bus->getData());
                manageBus();
            }

            if (islineFC) {
                quint8 t = (*d-0x47FC)/4;
                bus->setDest(t);
                bus->setData(data);
                bus->setWrite(true);
                bus->setFunc(BUS_LINE0);
                if (fp_log) fprintf(fp_log,"BUS_ACKINT DEST=%i data=%02x \n",bus->getDest(),bus->getData());
//                qWarning()<<"Write LINE FD:"<<t<<" - "<<data;
                manageBus();

            }

            if (islineFD) {
                quint8 t = (*d-0x47FD)/4;
                bus->setDest(t);
                bus->setData(data);
                bus->setWrite(true);
                bus->setFunc(BUS_LINE1);
                if (fp_log) fprintf(fp_log,"BUS_ACKINT DEST=%i data=%02x \n",bus->getDest(),bus->getData());
//                qWarning()<<"Write LINE FD:"<<t<<" - "<<data;
                manageBus();

            }
            if (islineFE) {
                quint8 t = (*d-0x47FE)/4;

                bus->setDest(t);
                bus->setData(data);
                bus->setWrite(true);
                bus->setFunc(BUS_LINE2);
                if (fp_log) fprintf(fp_log,"BUS_SELECT DEST=%i data=%02x \n",bus->getDest(),bus->getData());
                manageBus();
                if (bus->getFunc()==BUS_READDATA) {
                    if (bus->getData()==0x00) extrinsicRAM=t;
                    if (bus->getData()==0x01) extrinsicROM=t;
                }
                // there shoud be a way to reset extrinsic values ??? .... ????
                // Perhaps when dest = 32 ???
//                else
//                    extrinsic=0xff;
                //                    if (fp_log) fprintf(fp_log," AFTER DEST=%i data \n",bus->getDest());
                bus->setFunc(BUS_SLEEP);
                if (fp_log) {
                    fprintf(fp_log,"WRITE LINE%s [%2i]=%i",LINEID,t,data);
                    for (int i=0;i<=32;i++) {
                        if (i%4 == 0) fprintf(fp_log," ");
                        fprintf(fp_log,"%i",lineFE[i]);

                    }
                    pCPU->Regs_Info(1);
                    fprintf(fp_log," %s\n",pCPU->Regs_String);
                }
            }

                return false;
        }

        if (fp_log) {
            fprintf(fp_log,"WRITE ROM UNHANDLED [%04X]=%i",*d,data);
            for (int i=0;i<=32;i++) {
                if (i%4 == 0) fprintf(fp_log," ");
                fprintf(fp_log,"%i",lineFE[i]);

            }
            pCPU->Regs_Info(1);
            fprintf(fp_log," %s\n",pCPU->Regs_String);
        }
        return false;

    }
    if((*d>=0x8000) && (*d < 0xC000)) {
        if (fp_log) {
            fprintf(fp_log,"WRITE RAM [%04X]=%02X ",*d,data);
            pCPU->Regs_Info(1);
            fprintf(fp_log," %s\n",pCPU->Regs_String);
        }

        if (fp_log) fprintf(fp_log,"BUS_WRITEDATA DEST=%i  data=%02X\n",bus->getDest(),data);
        if (extrinsicRAM!=0xff) {
            bus->setDest(extrinsicRAM);
            bus->setAddr(*d);
            bus->setData(data);
            bus->setWrite(true);
            bus->setFunc(BUS_WRITEDATA);
            manageBus();
            //        if (fp_log) fprintf(fp_log,"  AFTER DEST=%i  \n",bus->getDest());
            bus->setFunc(BUS_SLEEP);

            // do not write hhc memory. Data has been sent to external
            return false;
        }

        return false; /* RAM */
    }


    return false;
}

bool Crlh1000::Chk_Adr_R(UINT32 *d, UINT32 *data)
{
    if (*d < 0x2000) {
        if (pCPU->fp_log) {
            snprintf(pCPU->Log_String,sizeof(pCPU->Log_String),"%s Read[%04x]=%02x ",pCPU->Log_String,*d,mem[*d]);
        }
        return true;   // RAM
    }

    if((*d>=0x2000) && (*d < 0x4000)) { // ROM
        *data = ReadBusMem(BUS_READDATA,*d,extrinsicROM);
        return false;
    }

    if((*d>=0x4000) && (*d <= 0x7FFF)) {
        if ((latchByte & 0x04)==0) {
            // I/O mapping

            if (latchByte & 0x80){
                // LCD mapping
                if ((*d>=0x5800)&&(*d<0x58A0)) {
                    *data = ((Clcdc_rlh1000*)pLCDC)->mem[*d-0x5800];
                    return false;
                }
                if (pCPU->fp_log) fprintf(pCPU->fp_log,"\n READ ROM LCD [%04X]\n",*d);

            }
            else {
                // KBD mapping
                if ( (*d>=0x5800) && (*d<=(0x5800+0xfa))) {
                    quint8 t = *d - 0x5800;
                    *data = getKey(t);
                    return false;
                }
            }
                // EXT management
            if ( (*d>=0x47FC) && (*d<=(0x47FF+0xff))) {
                bool islineFC = ((*d-0x47FC)%4==0);
                bool islineFD = ((*d-0x47FD)%4==0);
                bool islineFE = ((*d-0x47FE)%4==0);
                bool islineFF = ((*d-0x47FF)%4==0);
                quint8 t=0;

                if (islineFC) {
                    // return &FF : Scan next line
                    // return &FB :

                    t =(*d-0x47FC)/4;

                    bus->setDest(t);
                    if (fp_log) fprintf(fp_log,"BUS_QUERY DEST=%i  ",bus->getDest());
                    bus->setData(0xff);
                    bus->setWrite(false);
                    bus->setFunc(BUS_LINE0);
                    manageBus();
                    if (fp_log) fprintf(fp_log,"  data=%02X  \n",bus->getData());

                    *data = bus->getData();
                    bus->setFunc(BUS_SLEEP);

                }
                if (islineFD) {
                    t = (*d-0x47FD)/4;
                    bus->setDest(t);
                    if (fp_log) fprintf(fp_log,"BUS_QUERY DEST=%i  ",bus->getDest());
                    bus->setData(0xff);
                    bus->setWrite(false);
                    bus->setFunc(BUS_LINE1);
                    manageBus();
                    if (fp_log) fprintf(fp_log,"  data=%02X  \n",bus->getData());

                    *data = bus->getData();
                    bus->setFunc(BUS_SLEEP);
                }
                if (islineFE) {
                    t = (*d-0x47FE)/4;
//                    qWarning()<<"Read bus line FE";
                    bus->setDest(t);
                    if (fp_log) fprintf(fp_log,"BUS_QUERY DEST=%i  ",bus->getDest());
                    bus->setData(0xff);
                    bus->setWrite(false);
                    bus->setFunc(BUS_LINE2);
                    manageBus();
                    if (fp_log) fprintf(fp_log,"  data=%02X  \n",bus->getData());

                    *data = bus->getData();
                    bus->setFunc(BUS_SLEEP);
                }
                if (islineFF) {
                    t = (*d-0x47FF)/4;
                    bus->setDest(t);
                    if (fp_log) fprintf(fp_log,"BUS_QUERY_LINE DEST=%i  ",bus->getDest());
//                    qWarning()<<"Read LINE FF:"<<t<<" - "<<data;
                    bus->setData(0xff);
                    bus->setWrite(false);
                    bus->setFunc(BUS_LINE3);
                    manageBus();
                    if (fp_log) fprintf(fp_log,"  data=%02X  \n",bus->getData());

                    *data = bus->getData();
                    bus->setFunc(BUS_SLEEP);
                }

                if (fp_log) {
                    fprintf(fp_log,"READ LINE%s (%2i) data=%i ",LINEID,t,*data);
                    pCPU->Regs_Info(1);
                    fprintf(fp_log," %s\n",pCPU->Regs_String);
                }
                if (pCPU->fp_log) snprintf(pCPU->Log_String,sizeof(pCPU->Log_String),"%s,READ LINE%s [%i]=%04X ",pCPU->Log_String,LINEID,t,*data);
                return false;
            }

            // 0x58FA??? I don't know what it is.
            if (*d==0x58FA) { *data=0x00; return false; }
            if (*d==0x58FB) { *data=timercnt1; return false; }
            if (*d==0x58FC) { *data=timercnt2; return false; }
            if (*d==0x58FD) { *data=timercnt3; return false; }
            if (*d==0x58FF) {  *data = pKEYB->LastKey; return false; }

            if (pCPU->fp_log) fprintf(pCPU->fp_log,"\n READ ROM UNHANDLED[%04X]\n",*d);
            if (fp_log) {
                fprintf(fp_log,"READ ROM [%04X]  ",*d);
                pCPU->Regs_Info(1);
                fprintf(fp_log," %s\n",pCPU->Regs_String);
            }

            return false;
        }
        else {
            UINT32 offset = 0;
            switch (latchByte & 0x03) {
            case 0x00 : offset = 0; return true;
            case 0x01 : offset = 0x0C000; *d += offset; return true;
            case 0x02 : offset = 0x10000; *d += offset; return true;
            case 0x03 :  *data = ReadBusMem(BUS_READDATA,*d,30);
//                qWarning()<<"READ EXT ROM: dest="<<extrinsic<<" - adr="<<*d<<"="<<*data;
                if (pCPU->fp_log) fprintf(pCPU->fp_log,"\nEXT CAPSULE dest:%02x\n",extrinsicRAM);
                // External ROM ????
                return false;
            default: break;
            }
            return true;
        }

        return true; /* ROM */
    }


    if((*d>=0x8000) && (*d < 0xC000)) {
        *data = ReadBusMem(BUS_READDATA,*d,extrinsicRAM);
        return false; /* RAM */
    }

    return true;
}

UINT8 Crlh1000::ReadBusMem(BUS_FUNC f,UINT32 adr,quint8 dest)
{
    UINT8 data=0xff;

    if (fp_log) {
        fprintf(fp_log,"BUS=%02X READ RAM [%04X] ",bus->getDest(),adr);
        pCPU->Regs_Info(1);
        fprintf(fp_log," %s\n",pCPU->Regs_String);
    }
    if (fp_log) fprintf(fp_log,"BUS_READDATA DEST=%i  ",bus->getDest());
    if (dest!=0xff) {
        bus->setDest(dest);
        bus->setAddr(adr);
        bus->setData(0xff);
        bus->setWrite(false);
        bus->setFunc(f);
        manageBus();
        if (fp_log) fprintf(fp_log," DATA=%02X  \n",bus->getData());

        if ( (bus->getFunc()==BUS_READDATA)||(bus->getFunc()==BUS_ACK) ) {

            data = bus->getData();
            if (fp_log) {
                fprintf(fp_log,"***READ RAM [%04X]=%02X",adr,data);
                pCPU->Regs_Info(1);
                fprintf(fp_log," %s\n",pCPU->Regs_String);
            }
        }
        else data = 0xff;
        bus->setFunc(BUS_SLEEP);
    }
    return data;
}



UINT8 Crlh1000::in(UINT8 Port,QString)
{
    Q_UNUSED(Port)

    return 0;
}



UINT8 Crlh1000::out(UINT8 Port, UINT8 x, QString sender)
{
    Q_UNUSED(Port)
    Q_UNUSED(x)
    Q_UNUSED(sender)

    return 0;
}


bool Crlh1000::Set_Connector(Cbus *_bus)
{
    Q_UNUSED(_bus)

    // transfert busValue to Connector
    pCONNECTOR->Set_values(bus->toUInt64());
    return true;
}

bool Crlh1000::Get_Connector(Cbus *_bus)
{
    Q_UNUSED(_bus)

    bus->fromUInt64(pCONNECTOR->Get_values());

    return true;
}


void Crlh1000::TurnOFF(void) {
    ASKYN _tmp = mainwindow->saveAll;
    mainwindow->saveAll = YES;
    CpcXXXX::TurnOFF();
    mainwindow->saveAll = _tmp;
    AddLog(LOG_TEMP,"TURN OFF");

}

void Crlh1000::TurnON(void){
    CpcXXXX::TurnON();

    pCPU->Reset();
}


void Crlh1000::Reset()
{
    CpcXXXX::Reset();
    pLCDC->init();

}

bool Crlh1000::LoadConfig(QXmlStreamReader *xmlIn)
{
    if (xmlIn->readNextStartElement() && (xmlIn->name() == "config") ) {
        if (xmlIn->readNextStartElement() && xmlIn->name() == "modules" ) {
            Mem_Load(xmlIn,2);
            Mem_Load(xmlIn,5);
            Mem_Load(xmlIn,6);

            xmlIn->skipCurrentElement();
        }
    }
    xmlIn->skipCurrentElement();
    return true;
}

bool Crlh1000::SaveConfig(QXmlStreamWriter *xmlOut)
{
    xmlOut->writeStartElement("config");
        xmlOut->writeStartElement("modules");
           Mem_Save(xmlOut,2,(SlotList[2].getType()==CSlot::ROM)?false:true);
           Mem_Save(xmlOut,5,(SlotList[5].getType()==CSlot::ROM)?false:true);
           Mem_Save(xmlOut,6,(SlotList[6].getType()==CSlot::ROM)?false:true);
        xmlOut->writeEndElement();
    xmlOut->writeEndElement();
    return true;
}

extern int ask(QWidget *parent,QString msg,int nbButton);
//#define KEY(c)	((pKEYB->keyPressedList.contains(TOUPPER(c)) || \
//                  pKEYB->keyPressedList.contains(c) || \
//                  pKEYB->keyPressedList.contains(TOLOWER(c)))?1:0)



void Crlh1000::ComputeKey(KEYEVENT ke, int scancode, QMouseEvent *event)
{
    Q_UNUSED(ke)
    Q_UNUSED(scancode)
    Q_UNUSED(event)


    // Manage left connector click
    if (KEY(0x240) && (currentView==LEFTview)) {
        pKEYB->keyPressedList.remove(0x240);
#if 1
      view->pickExtensionConnector("Panasonic_44");
#else
        FluidLauncher *launcher = new FluidLauncher(mainwindow,
                                     QStringList()<<P_RES(":/pockemul/configExt.xml"),
                                     FluidLauncher::PictureFlowType,QString(),
                                     "Panasonic_44");
        launcher->show();
#endif
    }

    // Manage backdoor click
    if ((KEY(0x241)||KEY(0x242)) && ((currentView==BACKview)||(currentView==BACKviewREV))) {
        pKEYB->keyPressedList.remove(0x241);
        pKEYB->keyPressedList.remove(0x242);
        backdoorOpen = !backdoorOpen;
        // Enable or disable capsule slot click
        pKEYB->Keys[capsuleKeyIndex  ].enabled = backdoorOpen;
        pKEYB->Keys[capsuleKeyIndex+1].enabled = backdoorOpen;
        pKEYB->Keys[capsuleKeyIndex+2].enabled = backdoorOpen;

        animateBackDoor();
    }

    // Manage capsules  click
    if (backdoorOpen && !backdoorFlipping) {
        int _slot = -1;
        quint32 _adr = 0;
        if (KEY(0x243)) {_slot = 2; _adr = 0x4000;}
        if (KEY(0x244)) {_slot = 5; _adr = 0x10000;}
        if (KEY(0x245)) {_slot = 6; _adr = 0x14000;}
        pKEYB->keyPressedList.remove(0x243);
        pKEYB->keyPressedList.remove(0x244);
        pKEYB->keyPressedList.remove(0x245);
        if (_slot == -1) return;
        int _response = 0;
        BYTE* capsule = &mem[_adr];
        if (!SlotList[_slot].isEmpty() || (capsule[0]!=0x7f)) {
            _response=ask(this,
                          "The "+SlotList[_slot].getLabel()+ " capsule is already plugged in this slot.\nDo you want to unplug it ?",
                          2);
        }

        if (_response == 1) {
            // Clear slot
            SlotList[_slot].setEmpty(true);
            SlotList[_slot].setFileName("");
            memset((void *)capsule ,0x7f,0x4000);
            SlotList[_slot].setLabel(QString("ROM bank %1").arg(_slot+1));
            SlotList[_slot].setType(CSlot::ROM);
            slotChanged = true;
        }

        if (_response==2) return;

        currentSlot = _slot;
        currentAdr = _adr;
#if 1
        connect(view,SIGNAL(Launched(QString,CPObject *)),this,SLOT(addModule(QString,CPObject *)));
        view->pickExtensionConnector("Panasonic_Capsule");
#else
        FluidLauncher *launcher = new FluidLauncher(mainwindow,
                                                    QStringList()<<P_RES(":/pockemul/configExt.xml"),
                                                    FluidLauncher::PictureFlowType,QString(),
                                                    "Panasonic_Capsule");
        connect(launcher,SIGNAL(Launched(QString,CPObject *)),this,SLOT(addModule(QString,CPObject *)));
        launcher->show();
#endif
    }
}

void Crlh1000::addModule(QString item,CPObject *pPC)
{
    Q_UNUSED(pPC)

    disconnect(view,SIGNAL(Launched(QString,CPObject *)),this,SLOT(addModule(QString,CPObject *)));

    qWarning()<<"Add Module:"<< item;
    if ( (currentSlot<0) || (currentSlot>7)) return;

    int _res = 0;
    CSlot::SlotType customModule = CSlot::ROM;
    QString moduleName;
    if (item=="SNAPBASIC")  moduleName = P_RES(":/rlh1000/SnapBasic.bin");
    if (item=="SNAPFORTH")  moduleName = P_RES(":/rlh1000/SnapForth.bin");
    if (item=="MSBASIC")    moduleName = P_RES(":/rlh1000/HHCbasic.bin");
    if (item=="PORTACALC")  moduleName = P_RES(":/rlh1000/Portacalc.bin");
    if (item=="SCICALC") {
        moduleName = P_RES(":/rlh1000/ScientificCalculator.bin");

        currentOverlay=0;
    }
    if (item=="PANACAPSFILE") {
        moduleName = QFileDialog::getOpenFileName(
                    mainwindow,
                    tr("Choose a Capsule file"),
                    ".",
                    tr("Module File (*.bin)"));
        customModule = CSlot::CUSTOM_ROM;
    }

    if (moduleName.isEmpty()) return;

    bool result = true; // check this is a true capsule

    qWarning()<<"loaded:"<<_res;
    if (result) {
        SlotList[currentSlot].setEmpty(false);
        SlotList[currentSlot].setResID(moduleName);
        SlotList[currentSlot].setType(customModule);
        Mem_Load(currentSlot);
        // Analyse capsule
        // 0x01 = 'C'
        // 0x01 - 0x28 : Copyright
        // 0x2C : title lenght
        // 0x2D - .. : title

        BYTE* capsule = &mem[currentAdr];
        if (capsule[1]=='C') {
            QString copyright = QString::fromLocal8Bit(QByteArray((const char*)&capsule[1],0x26));
            QString title  = QString::fromLocal8Bit(QByteArray((const char*)&capsule[0x2d],capsule[0x2c]));
            qWarning()<<"title:"<<title;
            SlotList[currentSlot].setLabel(title);
        }

        slotChanged = true;
    }

    currentSlot = -1;
    currentAdr=0;

    Refresh_Display = true;
}

UINT8 Crlh1000::getKey(quint8 row )
{
    quint8 data = 0;

//    if (fp_log) fprintf(fp_log,"READ KEYBOARD [%2i] ",row);
    if ((pKEYB->LastKey) )
    {
        if (row&0x01) {
            if (KEY(K_F5)) data |= 0x01;    // CLEAR
            if (KEY('1')) data |= 0x02;
            if (KEY('2')) data |= 0x04;
            if (KEY('3')) data |= 0x08;
            if (KEY('4')) data |= 0x10;
            if (KEY('5')) data |= 0x20;
            if (KEY('6')) data |= 0x40;
            if (KEY('7')) data |= 0x80;
        }
        if (row&0x02) {
            if (KEY('8')) data |= 0x01;
            if (KEY('9')) data |= 0x02;
            if (KEY('0')) data |= 0x04;
            if (KEY(K_HELP)) data |= 0x08;
            if (KEY(K_UA)) data |= 0x10;
            if (KEY(K_F6)) data |= 0x20; // IO
            if (KEY('.')) data |= 0x40;
            if (KEY(K_CTRL)) data |= 0x80; // 2nd SFT
        }

        if (row&0x04) {
            if (KEY('M')) data |= 0x01;
            if (KEY('Q')) data |= 0x02;
            if (KEY('W')) data |= 0x04;
            if (KEY('E')) data |= 0x08;
            if (KEY('R')) data |= 0x10;
            if (KEY('T')) data |= 0x20;
            if (KEY('Y')) data |= 0x40;
            if (KEY('U')) data |= 0x80;
        }

        if (row&0x08) {
            if (KEY('I')) data |= 0x01;
            if (KEY('O')) data |= 0x02;
            if (KEY('P')) data |= 0x04;
            if (KEY(K_LA)) data |= 0x08;
            if (KEY(K_STP)) data |= 0x10;
            if (KEY(K_RA)) data |= 0x20;
            if (KEY(K_F2)) data |= 0x40;
            if (KEY(K_F3)) data |= 0x80;
        }

        if (row&0x10) {
            if (KEY('N')) data |= 0x01;
            if (KEY('A')) data |= 0x02;
            if (KEY('S')) data |= 0x04;
            if (KEY('D')) data |= 0x08;
            if (KEY('F')) data |= 0x10;
            if (KEY('G')) data |= 0x20;
            if (KEY('H')) data |= 0x40;
            if (KEY('J')) data |= 0x80;
        }

        if (row&0x20) {
            if (KEY('K')) data |= 0x01;
            if (KEY('L')) data |= 0x02;
            if (KEY(K_F1)) data |= 0x04;
            if (KEY(K_SEARCH)) data |= 0x08;
            if (KEY(K_DA)) data |= 0x10;
            if (KEY(K_ROTATE)) data |= 0x20;
            if (KEY(K_INS)) data |= 0x40;
            if (KEY(K_SHT)) data |= 0x80;
        }

        if (row&0x40) {
            if (KEY('B')) data |= 0x01;
            if (KEY(K_C1)) data |= 0x02;
            if (KEY(K_C2)) data |= 0x04;
            if (KEY(',')) data |= 0x08;
            if (KEY('?')) data |= 0x10;
            if (KEY(' ')) data |= 0x20;
            if (KEY(K_C3)) data |= 0x40;
            if (KEY(K_C4)) data |= 0x80;
        }

        if (row&0x80) {
            if (KEY(K_LOCK)) data |= 0x01;
            if (KEY('Z')) data |= 0x02;
            if (KEY('X')) data |= 0x04;
            if (KEY('C')) data |= 0x08;
            if (KEY('V')) data |= 0x10;
            if (KEY(K_OF)) data |= 0x20;
            if (KEY(K_DEL)) data |= 0x40;
            if (KEY(K_RET)) data |= 0x80;
        }

    }

    if (KEY(K_F7)) {
        qWarning()<<"LOG";
        pCPU->logsw = true;
        pCPU->Check_Log();
        if (!fp_log) fp_log=fopen("rlh1000.log","wt");	// Open log file
    }
    return data;

}


bool Crlh1000::UpdateFinalImage(void) {

    CpcXXXX::UpdateFinalImage();

    // Draw backdoor when not in frontview
    if ((currentView != FRONTview) ) {
        delete BackImage;
        if (!BackFname.isEmpty()) BackImage = CreateImage(viewRect(BACKview),BackFname);
    }

    if (((currentView==BACKview)||(currentView==BACKviewREV))) {
        slotChanged = false;
        QPainter painter;
        painter.begin(BackImage);

        QImage _capsule = QImage(QString(P_RES(":/rlh1000/capsule.png"))).scaled(pKEYB->Keys[capsuleKeyIndex].Rect.size());
        int _slot=0;
        for (int i=0;i<3;i++) {
            switch (i) {
            case 0: _slot = 2; break;
            case 1: _slot = 5; break;
            case 2: _slot = 6; break;
            }

            if (!SlotList[_slot].isEmpty()) {
                // draw capsule
                painter.drawImage(pKEYB->Keys[capsuleKeyIndex+i].Rect.left(),
                                  pKEYB->Keys[capsuleKeyIndex+i].Rect.top(),
                                  _capsule);
            }
        }
        painter.end();
    }

    if ((currentView != FRONTview) ) {
        QPainter painter;
        painter.begin(BackImage);
        painter.translate(192,25);
        QTransform matrix2;
        matrix2.rotate(m_backdoorAngle, Qt::YAxis);
        painter.setTransform(matrix2,true);
        painter.drawImage(0,0,*backDoorImage);
        painter.end();
    }

//    emit updatedPObject(this);
    return true;
}


void Crlh1000::animateBackDoor(void) {
//    qWarning()<<"ANIMATE";
    QPropertyAnimation *animation1 = new QPropertyAnimation(this, "backdoorangle");
     animation1->setDuration(1500);
     if (backdoorOpen) {
         animation1->setStartValue(m_backdoorAngle);
         animation1->setEndValue(80);
     }
     else {
         animation1->setStartValue(m_backdoorAngle);
         animation1->setEndValue(0);
     }

     QParallelAnimationGroup *group = new QParallelAnimationGroup;
     group->addAnimation(animation1);

     connect(animation1,SIGNAL(valueChanged(QVariant)),this,SLOT(RefreshDisplay()));
     connect(animation1,SIGNAL(finished()),this,SLOT(endbackdoorAnimation()));
     backdoorFlipping = true;
     group->start();

}

void Crlh1000::setbackdoorAngle(int value) {
    this->m_backdoorAngle = value;
}

void Crlh1000::endbackdoorAnimation()
{
    backdoorFlipping = false;
    if (backdoorOpen) {
        pKEYB->Keys[backdoorKeyIndex].Rect.setSize(QSize(105,145));
    }
    else {
        pKEYB->Keys[backdoorKeyIndex].Rect.setSize(QSize(365,145));
    }
}
