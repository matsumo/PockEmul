/* BASED ON Piotr Piatek emulator  (http://www.pisi.com.pl/piotr433/index.htm)
 *
 *
 *
 */


#include <QtGui>

#include "common.h"
#include "pb2000.h"
#include "Keyb.h"
#include "Log.h"
#include "cpu.h"
#include "Lcdc_pb1000.h"
#include "hd61700.h"
#include "cextension.h"
#include "Connect.h"
#include "dialoganalog.h"

#define PD_RES 0x10;	// 1=reset, 0=normal_operation
#define PD_PWR 0x08;	// power control: 1=power_off, 0=power_on
#define PD_STR 0x04;	// transfer direction strobe: 1=write, 0=read
#define PD_ACK 0x01;	// transfer direction acknowledge

Cpb2000::Cpb2000(CPObject *parent)	: Cpb1000(parent)
{								//[constructor]
    setfrequency( (int) 910000/1);
    ioFreq = 0;
    setcfgfname(QString("pb2000"));

    SessionHeader	= "pb2000PKM";
    Initial_Session_Fname ="pb2000.pkm";

    BackGroundFname	= P_RES(":/pb2000/pb-2000.png");
    pLCDC->LcdFname		= P_RES(":/pb2000/pb-2000lcd.png");
//    SymbFname		= "";

    memsize         = 0x70000;
    InitMemValue	= 0xff;


    SlotList.clear();
    SlotList.append(CSlot(64, 0x00000 ,	P_RES(":/pb2000/rom1.bin")	, ""	, CSlot::ROM , "ROM 1"));
    SlotList.append(CSlot(64, 0x10000 ,	""					, ""	, CSlot::RAM , "RAM 0"));
    SlotList.append(CSlot(6 , 0x20000 ,	P_RES(":/pb2000/rom0.bin") , ""	, CSlot::ROM , "CPU ROM"));
    SlotList.append(CSlot(32, 0x28000 ,	""					, ""	, CSlot::RAM , "RAM 1"));
    SlotList.append(CSlot(64, 0x30000 ,	"EMPTY"             , ""	, CSlot::ROM , "EXT ROM"));      // Originally in 70000

    setDXmm(188);
    setDYmm(83);
    setDZmm(15);

    setDX(668);//715);
    setDY(294);//465);

//    Lcd_X		= 58;
//    Lcd_Y		= 45;
//    Lcd_DX		= 192;//168;//144 ;
//    Lcd_DY		= 32;
//    Lcd_ratio_X	= 2;// * 1.18;
//    Lcd_ratio_Y	= 2;// * 1.18;

    //delete pLCDC;       pLCDC		= new Clcdc_pb1000(this);
    pLCDC->rect.moveTo(58,45);
    pLCDC->Color_Off.setRgb(
                (int) (81*pLCDC->contrast),
                (int) (89*pLCDC->contrast),
                (int) (85*pLCDC->contrast));

    pKEYB->fn_KeyMap = "pb2000.map";



    closed = false;

    overlay = new QImage(P_RES(":/pb2000/coverlay.bmp"));

}

Cpb2000::~Cpb2000() {

}

bool Cpb2000::UpdateFinalImage(void) {

    CpcXXXX::UpdateFinalImage();

    // Draw switch by 180° rotation
    QPainter painter;
    painter.begin(FinalImage);

    // POWER SWITCH
    painter.drawImage(0,29,BackgroundImageBackup->copy(0,29,10,95).mirrored(false,!off));

    //TODO: Manage keyboard overlay
    // DRAW overlay depending of inserted module
    painter.drawImage(55,218,overlay->copy(0,0,432,6));
    painter.drawImage(55,251,overlay->copy(0,6,432,6));
    painter.end();

    return true;
}

void Cpb2000::TurnON(void){

    Cpb1000::TurnON();

    memset((void *)&mem[SlotList[4].getAdr()] ,InitMemValue,SlotList[4].getSize()*1024);
    SlotList[4].setLabel("EMPTY");

    if (ext_MemSlot1->ExtArray[ID_OM51P]->IsChecked) {
        SlotList[4].setFileName(P_RES(":/pb2000/om51p.bin"));
        SlotList[4].setLabel("PROLOG");
        overlay = new QImage(P_RES(":/pb2000/prologoverlay.png"));
        Mem_Load(4);
    }
    else
    if (ext_MemSlot1->ExtArray[ID_OM53B]->IsChecked) {
        SlotList[4].setFileName(P_RES(":/pb2000/om53b.bin"));
        SlotList[4].setLabel("BASIC");
        overlay = new QImage(P_RES(":/pb2000/basicoverlay.bmp"));
        Mem_Load(4);
    }
    else
    if (ext_MemSlot1->ExtArray[ID_OM55L]->IsChecked) {
        SlotList[4].setFileName(P_RES(":/pb2000/om55l.bin"));
        SlotList[4].setLabel("LISP");
        overlay = new QImage(P_RES(":/pb2000/lispoverlay.bmp"));
        Mem_Load(4);
    }
    else
        overlay = new QImage(P_RES(":/pb2000/coverlay.bmp"));



    pCPU->init();
}


void	Cpb2000::initExtension(void)
{
    AddLog(LOG_MASTER,"INIT EXT PB2000");
    // initialise ext_MemSlot1
    ext_MemSlot1 = new CExtensionArray("ROM Slot 1","Add ROM Module");
    ext_MemSlot1->setAvailable(ID_OM51P,true);
    ext_MemSlot1->setAvailable(ID_OM53B,true);
    ext_MemSlot1->setAvailable(ID_OM55L,true);

//    ext_MemSlot1->setChecked(ID_OM53B,true);

    addExtMenu(ext_MemSlot1);
    extensionArray[0] = ext_MemSlot1;
}


bool Cpb2000::init(void)				// initialize
{
//pCPU->logsw = true;
#ifndef QT_NO_DEBUG
    pCPU->logsw = true;
#endif

    initExtension();
    Cpb1000::init();
    pCONNECTOR->setSnap(QPoint(668,77));

    pdi = 0xfb;
    return true;
}

void Cpb2000::Reset()
{

    CpcXXXX::Reset();

    // Init I/O Port memory
    memset((char*)&mem[0x0c00],0x00,0x0F);

    pdi = (pdi & 0x03) | 0xf8;

}

bool Cpb2000::run() {

    if (off) {
        pCONNECTOR->Set_pin(12	,1);
    }
    lcd_on_timer_rate = pHD44352->on_timer_rate;
CpcXXXX::run();
    if (off && pKEYB->LastKey == K_POW_ON)
    {
        TurnON();
        pKEYB->LastKey = 0;
    }
//    if (pKEYB->LastKey) {
//        ((CHD61700*)pCPU)->execute_set_input(HD61700_KEY_INT,1);
//    }
    return true;
}

void Cpb2000::MemBank(UINT32 *d) {
    if ( (*d>=0x00C00) && (*d<=0x00C0F) )	{
        adrBus = *d;
    }
//    else adrBus = 0;

    if (*d<0x00C00) { *d |= 0x20000; return; }
    if ((*d >= 0x00C12)) {
        BYTE m = 1 <<  (*d >> 15);
        if (mem[0x0C10] & m) {
            AddLog(LOG_RAM,"SWITCH BANK1");
//            if (pCPU->fp_log) fprintf(pCPU->fp_log,"SWITCH BANK 1  [%05X]  m=%i    [C10]=%02x\n",*d,m,mem[0x0C10]);
            if (pCPU->fp_log) fprintf(pCPU->fp_log,"S");
                *d |= 0x30000;
        }
        else if (mem[0x0C11] & m) {
            if (pCPU->fp_log) fprintf(pCPU->fp_log,"SWITCH BANK 2  [%05X]  m=%i    [C10]=%02x\n",*d,m,mem[0x0C11]);
            *d |= 0x40000;
            AddLog(LOG_RAM,"SWITCH BANK2");
        }
    }
}


WORD Cpb2000::Get_16rPC(UINT32 adr)
{
    UINT32	a;
//    MemBank(&adr);
    adr += 0x20000;
    a=adr+1;
    return((mem[adr]<<8)+mem[a]);
}

bool Cpb2000::Chk_Adr(UINT32 *d, UINT32 data)
{
    MemBank(d);
//    AddLog(LOG_TEMP,tr("Write %1").arg(*d));
    if ( (*d>=0x00C00) && (*d<=0x00C11) )	{
        if ( (*d>=0x00C00) && (*d<=0x00C0F) ) {
            if (*d != 0xC04) writeIO = true;
        }
        AddLog(LOG_TEMP,tr("Write Port [%1] = %2").arg(*d&7).arg(data,2,16,QChar('0')));
        if (*d==0xC04) {AddLog(LOG_PRINTER,tr("Write 0C04 : %1").arg(data,0,16,QChar('0')));}
        if (*d==0xC03) {
            if (mem[*d] != data) {
                if (mainwindow->dialoganalogic) mainwindow->dialoganalogic->setMarker(21);
                AddLog(LOG_PRINTER,tr("Write 0C03 : %1").arg(data,2,16,QChar('0')));
            }
        }
        if ((*d==0xC04) && (mem[*d]!=data)){
//            qWarning("[%02X] Write 0C04 : %02X",pdi,data);
            AddLog(LOG_PRINTER,tr("Write 0C04 : %1").arg(data,0,16,QChar('0')));
        }
        if (*d==0xC05) {
//            qWarning("[%02X] Write 0C05 : %02X",pdi,data);
            AddLog(LOG_PRINTER,tr("Write 0C05 : %1").arg(data,0,16,QChar('0')));
        }
        if (*d==0xC06) {
//            qWarning("[%02X] Write 0C06 : %02X",pdi,data);
            AddLog(LOG_PRINTER,tr("Write 0C06 : %1").arg(data,0,16,QChar('0')));
        }
        if (pCPU->fp_log) fprintf(pCPU->fp_log,"Write port [%05X] = %02X\n",*d,data);
        return(true);		// RAM area()
    }

    if ( (*d>=0x10000) && (*d<0x20000) )	return(true);		// RAM 0 area()
    if ( (*d>=0x28000) && (*d<0x30000) )	return(true);		// RAM 1 area()


    return false;
}


bool Cpb2000::Chk_Adr_R(UINT32 *d, UINT32 *data)
{
    Q_UNUSED(data)

    MemBank(d);

    if ( (*d>=0x0C00) && (*d<=0x0C0F) )	{
//        mem[*d] = 0xff;
        if (*d==(0xC03)) {
            if (mainwindow->dialoganalogic) mainwindow->dialoganalogic->setMarker(20);
            AddLog(LOG_PRINTER,tr("Read 0C03 : %1").arg(mem[*d],0,16,QChar('0')));
        }
        if (*d==(0xC04)) {
            writeIO=true;
            if (mainwindow->dialoganalogic) mainwindow->dialoganalogic->setMarker(22);
            AddLog(LOG_PRINTER,tr("Read 0C04 : %1").arg(mem[*d],0,16,QChar('0')));
        }
        AddLog(LOG_TEMP,tr("Read Port:%1").arg(*d&7));
//        if (pCPU->fp_log) fprintf(pCPU->fp_log,"LECTURE IO [%04X]\n",*d);
        return(true);		// RAM area()
    }

    return true;
}



#define TOUPPER( a )	(  ((a >= 'a' && a <= 'z') ? a-('a'-'A') : a ) )
#define KEY(c)	( TOUPPER(pKEYB->LastKey) == TOUPPER(c) )
UINT16 Cpb2000::getKey(void) {
    UINT32 ko = 0;
    UINT16 data = 0;
//qWarning("getkey");
//AddLog(LOG_KEYBOARD,tr("Enter GetKEY PB-2000C"));
    switch (m_kb_matrix & 0x0f) {
        case 0: return 0;
        case 13: ko = 0xffff; break;
        case 14:
        case 15: ko = 0; break;
        default: ko = (1<<(m_kb_matrix-1)); break;
    }
    if ((pKEYB->LastKey) )
    {

        if (ko >= 0xfff) AddLog(LOG_KEYBOARD,tr("matrix=%1 ko=%2").arg(m_kb_matrix,2,16,QChar('0')).arg(ko,4,16,QChar('0')));

//AddLog(LOG_KEYBOARD,tr("GetKEY : %1").arg(ko,4,16,QChar('0')));
        if (ko&1) {
            if (KEY(K_POW_OFF))     data|=0x20;
            if (KEY(K_BRK))         data|=0x80;
        }

        if (ko&0x02) {
            if (KEY('W'))           data|=0x01;
            if (KEY('Q'))			data|=0x02;
            if (KEY('A'))			data|=0x04;

            if (KEY('Z'))			data|=0x10;
            if (KEY(K_SML))			data|=0x20;

            if (KEY(K_QUOTE))		data|=0x4000;
            if (KEY(K_TAB))         data|=0x8000;
        }

        if (ko&0x04) {
            if (KEY('R'))			data|=0x01;
            if (KEY('E'))			data|=0x02;
            if (KEY('D'))			data|=0x04;
            if (KEY('S'))			data|=0x08;
            if (KEY('C'))			data|=0x10;
            if (KEY('X'))			data|=0x20;

            if (KEY(K_F1))			data|=0x1000;
            if (KEY(')'))			data|=0x4000;
            if (KEY('('))			data|=0x8000;
        }
        if (ko&0x08) {
            if (KEY('Y'))			data|=0x01;
            if (KEY('T'))			data|=0x02;
            if (KEY('G'))			data|=0x04;
            if (KEY('F'))			data|=0x08;
            if (KEY('B'))			data|=0x10;
            if (KEY('V'))			data|=0x20;

            if (KEY(K_F2))			data|=0x1000;
            if (KEY(']'))           data|=0x4000;
            if (KEY('['))           data|=0x8000;
        }
        if (ko&0x10) {
            if (KEY('I'))			data|=0x01;
            if (KEY('U'))			data|=0x02;
            if (KEY('J'))			data|=0x04;
            if (KEY('H'))			data|=0x08;
            if (KEY('M'))			data|=0x10;
            if (KEY('N'))			data|=0x20;

            if (KEY(K_F3))			data|=010000;
            if (KEY(K_MEMO))		data|=0x4000;
            if (KEY('|'))			data|=0x8000;
        }

        if (ko&0x20) {
            if (KEY('P'))			data|=0x01;
            if (KEY('O'))			data|=0x02;
            if (KEY('L'))   		data|=0x04;
            if (KEY('K'))			data|=0x08;
            if (KEY(' '))   		data|=0x10;
            if (KEY(','))			data|=0x20;

            if (KEY(K_F4))          data|=0x1000;
            if (KEY(K_OUT))         data|=0x4000;
            if (KEY(K_IN))          data|=0x8000;
        }

        if (ko&0x40) {

            if (KEY('='))			data|=0x02;
            if (KEY(':'))			data|=0x04;
            if (KEY(';'))			data|=0x08;

            if (KEY(K_ANS))			data|=0x20;
            if (KEY(K_SHT))			data|=0x40;

            if (KEY(K_F5))          data|=0x1000;
            if (KEY(K_CALC))		data|=0x8000;


        }

        if (ko&0x80) {
            if (KEY('5'))			data|=0x01;
            if (KEY('4'))			data|=0x02;
            if (KEY('2'))			data|=0x04;
            if (KEY('1'))			data|=0x08;

            if (KEY('0'))			data|=0x20;

            if (KEY('7'))			data|=0x80;

            if (KEY(K_NEWALL))		data|=0x2000;

        }

        if (ko&0x100) {
            if (KEY('-'))			data|=0x01;
            if (KEY('6'))			data|=0x02;
            if (KEY('+'))			data|=0x04;     // OK
            if (KEY('3'))			data|=0x08;
            if (KEY(K_RET))			data|=0x10;
            if (KEY('.'))			data|=0x20;

            if (KEY('9'))			data|=0x80;

        }

        if (ko&0x200) {
            if (KEY(K_CAL))			data|=0x01;
            if (KEY(K_RA))			data|=0x02;
            if (KEY('/'))			data|=0x04;
            if (KEY(K_BS))			data|=0x08;

            if (KEY('*'))			data|=0x80;
            if (KEY(K_MENU))		data|=0x4000;
            if (KEY(K_DEL))			data|=0x8000;
        }

        if (ko&0x400) {
            if (KEY(K_DA))			data|=0x01;
            if (KEY(K_LA))			data|=0x02;
            if (KEY(K_CLR))			data|=0x04;

            if (KEY('8'))			data|=0x80;

            if (KEY(K_UA))			data|=0x4000;
            if (KEY(K_INS))			data|=0x8000;
        }
    }

if (pCPU->fp_log) fprintf(pCPU->fp_log,"KEY : %05X\n",data);

    return data;

}

void Cpb2000::TurnCLOSE(void) { }



void Cpb2000::paintEvent(QPaintEvent *event)
{
    CPObject::paintEvent(event);
}

UINT8 Cpb2000::readPort()
{
    if (pCPU->fp_log) fprintf(pCPU->fp_log,"READ PORT\n");

    if ( (ext_MemSlot1->ExtArray[ID_OM51P]->IsChecked) ||
         (ext_MemSlot1->ExtArray[ID_OM53B]->IsChecked) ||
         (ext_MemSlot1->ExtArray[ID_OM55L]->IsChecked) ) pdi = pdi & 0xfd;

    pdi = pdi & 0xfe;   // no kana key
    return pdi;
}


bool Cpb2000::LoadConfig(QXmlStreamReader *xmlIn)
{
    if (xmlIn->readNextStartElement()) {
        if (xmlIn->name() == "config" && xmlIn->attributes().value("version") == "1.0") {
            if (xmlIn->readNextStartElement() && xmlIn->name() == "internal" ) {
                pdi = xmlIn->attributes().value("pdi").toString().toInt(0,16);
                xmlIn->skipCurrentElement();
            }
            // call extension Load Config
        }
        xmlIn->skipCurrentElement();
    }

    return true;
}


bool Cpb2000::SaveConfig(QXmlStreamWriter *xmlOut)
{
    xmlOut->writeStartElement("config");
    xmlOut->writeAttribute("version", "1.0");
        xmlOut->writeStartElement("internal");
            xmlOut->writeAttribute("pdi",QString("%1").arg(pdi,2,16));
            // call extension Save Config
        xmlOut->writeEndElement();
    xmlOut->writeEndElement();

    return true;
}

#define P(a)  (((x)>>(a)) & 0x01)
#define I(a) (((d)>>(a))&0x01)
#define PIN(x)    pCONNECTOR->Get_pin(x)
bool Cpb2000::Set_Connector(void)
{
    pCONNECTOR->Set_pin(4	,READ_BIT(adrBus,0));
    pCONNECTOR->Set_pin(18	,READ_BIT(adrBus,1));
    pCONNECTOR->Set_pin(6	,READ_BIT(adrBus,2));
    pCONNECTOR->Set_pin(3	,READ_BIT(adrBus,3));

    BYTE x = pdi;


    if ((prev_P2 && (P(3)==0)) || writeIO)
//    if (writeIO)
    {
        writeIO = false;
        UINT32 adr = (0xC00 | adrBus);
        MemBank(&adr);
        BYTE d=mem[adr];
//        BYTE d = Get_8(0x0C00|adrBus);

        pCONNECTOR->Set_pin(22	,I(0));
        pCONNECTOR->Set_pin(19	,I(1));
        pCONNECTOR->Set_pin(9	,I(2));
        pCONNECTOR->Set_pin(24	,I(3));
        pCONNECTOR->Set_pin(21	,I(4));
        pCONNECTOR->Set_pin(8	,I(5));
        pCONNECTOR->Set_pin(20	,I(6));
        pCONNECTOR->Set_pin(23	,I(7));
    }

        pCONNECTOR->Set_pin(25	,P(2));
        //    pCONNECTOR->Set_pin(11	,P(1));
        pCONNECTOR->Set_pin(26	,P(3));
        pCONNECTOR->Set_pin(12	,P(4));
        pCONNECTOR->Set_pin(27	,P(5));

    prev_P2 = P(3);
    return(1);
}



bool Cpb2000::Get_Connector(void)
{
    BYTE p = PIN(22) |
            (PIN(19)<<1) |
            (PIN(9) <<2) |
            (PIN(24)<<3) |
            (PIN(21)<<4) |
            (PIN(8) <<5) |
            (PIN(20)<<6) |
            (PIN(23)<<7);

    if (PIN(25))
    {
        BYTE recv_adrBus = PIN(4) |
                (PIN(18)<<1) |
                (PIN( 6)<<2) |
                (PIN( 3)<<3);
        UINT32 adr = (0xC00 | recv_adrBus);
        MemBank(&adr);
        mem[adr] = p;
//        Set_8(0x0C00 | recv_adrBus,p);
    }
    PUT_BIT(pdi,2,PIN(25));
//    PUT_BIT(pdi,1,PIN(11));
    PUT_BIT(pdi,3,PIN(26));
    PUT_BIT(pdi,4,PIN(12));
    PUT_BIT(pdi,5,PIN(27));

    return(1);
}
