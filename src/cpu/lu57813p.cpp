

#include "common.h"
#include "lu57813p.h"
#include "Log.h"
#include "Inter.h"
#include "Keyb.h"
#include "cpu.h"

#define ADIN_ADR    0x20

CLU57813P::CLU57813P(CpcXXXX *parent)
{
        c0=c1=c2=stb=cs=data_in=gnd=clk=data_out=tp=out_enable=n_xtal=xtal=vdd=false;
        Current_Bit = 0;
        pPC = parent;
        digitalvalue = 0;
}

CLU57813P::~CLU57813P(){}

// TODO: STORE AND read stack enstead of imem
void CLU57813P::Load_Internal(QXmlStreamReader *xmlIn)
{
    if (xmlIn->readNextStartElement()) {
        if ( (xmlIn->name()=="cpu") &&
             (xmlIn->attributes().value("model").toString() == "lu57813")) {
            QByteArray ba_reg = QByteArray::fromBase64(xmlIn->attributes().value("registers").toString().toLatin1());
            memcpy((char *) &imem,ba_reg.data(),sizeof(imem));
            sp = xmlIn->attributes().value("stackpointer").toString().toInt(0,16);
        }
        xmlIn->skipCurrentElement();
    }
}

void CLU57813P::save_internal(QXmlStreamWriter *xmlOut)
{
    xmlOut->writeStartElement("cpu");
        xmlOut->writeAttribute("model","lu57813");
        QByteArray ba_reg((char*)&imem,sizeof(imem));
        xmlOut->writeAttribute("registers",ba_reg.toBase64());
        xmlOut->writeAttribute("stackpointer",QString("%1").arg(sp,2,16));
    xmlOut->writeEndElement();
}

bool	CLU57813P::exit(void)						//end
{
    return true;
}

INLINE WORD HEX2BCD(BYTE d)
{

    BYTE	a,b,c;
    a=d/100;
    b=d-(a*100);
    c=b/10;
    return((a<<8)+(c<<4)+b-(c*10));
}

bool CLU57813P::init(void)
{

    lastDateTime = QDateTime::currentDateTime();
    datetime.seconds	= HEX2BCD(lastDateTime.time().second());	/* seconds BCD */
    datetime.minutes	= HEX2BCD(lastDateTime.time().minute());	/* minutes BCD */
    datetime.hours      = HEX2BCD(lastDateTime.time().hour());		/* hours   BCD */
    datetime.days		= HEX2BCD(lastDateTime.date().day());		/* days    BCD */
    datetime.weekday	= 0;										/* weekday BCD */
    datetime.month      = lastDateTime.date().month();				/* month   Hexadecimal form */

    bitno = 0;
    prev_mode = 0x10;
    tp = 0;
    TP_FREQUENCY=1;
    previous_state = 0;
    previous_state_tp = 0;
    Kon = false;
    output_pending = false;

    cmd = 0;
    irq = 0;
    irq_mask = 0;

    tpIndex64 = pPC->pTIMER->initTP(64);
    tpIndex2 = pPC->pTIMER->initTP(2);

    memset(password,0,0x10);
    sp = 0;

    return(1);
}
                //initialize
void CLU57813P::Reset(void){
}

INLINE void CLU57813P::addretrace (void)
{
    datetime.seconds++;
    if ( (datetime.seconds & 0x0f) < 10 )
        return;
    datetime.seconds &= 0xf0;
    datetime.seconds += 0x10;
    if (datetime.seconds < 0x60)
        return;
    datetime.seconds = 0;
    datetime.minutes++;
    if ( (datetime.minutes & 0x0f) < 10 )
        return;
    datetime.minutes &= 0xf0;
    datetime.minutes += 0x10;
    if (datetime.minutes < 0x60)
        return;
    datetime.minutes = 0;
    datetime.hours++;
    if ( (datetime.hours & 0x0f) < 10 )
        return;
    datetime.hours &= 0xf0;
    datetime.hours += 0x10;
    if (datetime.hours < 0x24)
        return;
    datetime.hours = 0;
    increment_day();
}

void CLU57813P::increment_day(void)
{
    datetime.days++;
    if ((datetime.days & 0x0f) >= 10)
    {
        datetime.days &= 0xf0;
        datetime.days += 0x10;
    }

    datetime.weekday++;
    if (datetime.weekday == 7)
        datetime.weekday=0;

    switch(datetime.month)
    {
    case 1: case 3: case 5: case 7: case 8: case 10: case 12:
        if (datetime.days == 0x32)
        {
            datetime.days = 1;
            increment_month();
        }
        break;
    case 2:	case 4: case 6: case 9: case 11:
        if (datetime.days == 0x31)
        {
            datetime.days = 1;
            increment_month();
        }
        break;
    }
}

void CLU57813P::increment_month(void)
{
    datetime.month++;
    if (datetime.month == 13)
    {
        datetime.month = 1;
    }
}

void CLU57813P::store(BYTE bank,BYTE value) {
    if (pPC->fp_log) fprintf(pPC->fp_log,"LU57813P - STORE (%01x) - [%02x]=%02x\n",bank,sp,value);
    imem[sp] = value;
    sp++;
}

void CLU57813P::SetDigital(BYTE val) {
    digitalvalue = val;
}

BYTE CLU57813P::get_irq() {
    BYTE irq = 0;

    bool tp64 = pPC->pTIMER->GetTP(tpIndex64);
    if (previous_tp64 != tp64)
    {
        if (tp64)
        {
            irq &= 0x04;
        }
        previous_tp64 = tp64;
    }

    bool tp2 = pPC->pTIMER->GetTP(tpIndex2);
    if (previous_tp2 != tp2)
    {
        if (tp2)
        {
            irq &= 0x02;
        }
        previous_tp2 = tp2;
    }

    return irq;
}

void CLU57813P::request(BYTE value)
{

    switch (value) {

    case 0x01 :
        reg_out = 0x00;
        if (pPC->fp_log) fprintf(pPC->fp_log,"LU57813P - Unknown request 01.\n");
        break;

    case 0x03 :
        // 53 : during initalisation. Read a return value just after.
        // Unknown effect !!!!
        reg_out = 0x00;
        if (pPC->fp_log) fprintf(pPC->fp_log,"LU57813P - Unknown request 53.\n");
        break;


    case 0x05 :
        // 55 : Read the digital value of the CE-1600P battery voltage.
//        This routine is used for monitoring the Ni-Cd battery supply voltage cf the
//        CE-1600P. If the value is Iess than A8H, it is judged as the low battery.
        reg_out = 0xc0;
        if (pPC->fp_log) fprintf(pPC->fp_log,"LU57813P - Read the digital value of the voltage input at the analog input port..\n");
        break;

        case 0x06 :
        // 56 : Read the digital value of the voltage input at the analog input port.
        reg_out = digitalvalue;
        if (pPC->fp_log) fprintf(pPC->fp_log,"LU57813P - Read the digital value of the voltage input at the analog input port..\n");
        break;

        //57 : Read the digital value cf the supply voltage cf the PC-1600 main unit.
//        This routine is used for monitoring the supply voltage of the PC-1600 main unit. If
//        the value is less than AFH, it is judged as the Low battery. The low battery state is
//        released when the value becomes greater than BEH.
    case 0x07 : reg_out = 0xC0;
        if (pPC->fp_log) fprintf(pPC->fp_log,"LU57813P - Read the digital value cf the supply voltage cf the PC-1600 main unit.\n");
        break;

        // 5A : first call from the main CPU. Unknown action
    case 0x0a : reg_out = 0xa0;
        if (pPC->fp_log) fprintf(pPC->fp_log,"LU57813P - Unknown action, perhaps voltage monitor.\n");
        break;

    case 0x0c :
        // 5C : (Bit 5 AND the byte in A register indicates the state of the CI signal: If the CI
        //          signal is high, then bit 5 = 0. If it is Iow, then bit 5 = 1.)
        //      (Bit 2 AND the byte in A register indicates the state of the PASSWORD signal: If the PASSWORD IS SET
        //          signal is high, then bit 2 = 0. If it is Iow, then bit 2 = 1.)

        reg_out = 0x20 | (password[0]==0?0x00 : 0x04);
        if (pPC->fp_log) fprintf(pPC->fp_log,"LU57813P - indicates the state of the signal\n");
        if (pPC->pCPU->fp_log) fprintf(pPC->pCPU->fp_log,"\n Check LU57813P\n");
        break;

        // 5D : Read the current interrupt cause for SC-7852.
    case 0x0d: reg_out = get_irq();
//        if (pPC->fp_log) fprintf(pPC->fp_log,"LU57813P - Read the current interrupt cause for SC-7852.\n");
        break;

        // 5E : Read the current setting of the interrupt mask for SC-7852.
    case 0x0e : reg_out = irq_mask;
        if (pPC->fp_log) fprintf(pPC->fp_log,"LU57813P - Read the current setting of the interrupt mask for SC-7852.\n");
        break;

        // 5F : Set the interrupt mask for SC-7852. from sp[0] and sp[1]
    case 0x0f :
        irq_mask = (imem[0]<<4) + imem[1];

        // 0x20 : ALARM$
        // 0x40 : TIME_CHECK
        // 0x80 : WAKE$(0)

        if (pPC->fp_log) fprintf(pPC->fp_log,"LU57813P - Set the interrupt mask for SC-7852. from sp[0] and sp[1]\n");
        break;

    default: // UNKNOWN COMMAND
        if (pPC->fp_log) fprintf(pPC->fp_log,"LU57813P - Unknown request %02X.\n",value);
    }
    output_pending = true;
}

void CLU57813P::action(BYTE value)
{

    switch (value) {
    case 0x04 : // 64 : Clear PASSWORD regiser from stack
        if (pPC->fp_log) fprintf(pPC->fp_log,"LU57813P - STACK -> RESET PASSWORD - %02x\n",value);
        if (memcmp(password,imem,0x10)==0) {
            password[0] = 0;
            if (pPC->fp_log) fprintf(pPC->fp_log,"LU57813P - PASSWORD CLEARED\n");
        }
        break;
    case 0x05 : // 65 : Affect PASSWORD regiser from stack
        if (pPC->fp_log) fprintf(pPC->fp_log,"LU57813P - STACK -> PASSWORD - %02x\n",value);
        memcpy(password,imem,0x10);
        break;
    case 0x07 : // 67 : Affect ALARM regiser from stack
        if (pPC->fp_log) fprintf(pPC->fp_log,"LU57813P - STACK -> ALARM$ - %02x\n",value);
//        for (int i=0;i<9;i++)
//            wakeReg[i] = stack.dequeue();//imem[i];
    break;
    case 0x09 : // 69 : Affect TIME_CHECK$ register from stack
    if (pPC->fp_log) fprintf(pPC->fp_log,"LU57813P - STACK -> TIME_CHECK - %02x\n",value);
//        for (int i=0;i<9;i++)
//            wakeReg[i] = stack.dequeue();//imem[i];

    break;

    case 0x0b : // 6B : Affect WAKE regiser from stack
        if (pPC->fp_log) fprintf(pPC->fp_log,"LU57813P - STACK -> WAKE$0 - %02x\n",value);
//        for (int i=0;i<9;i++)
//            wakeReg[i] = stack.dequeue();//imem[i];
        break;
        // 6C : Ask for TIME
    case 0x0c :
        if (pPC->fp_log) fprintf(pPC->fp_log,"LU57813P - DATE -> STACK - %02x\n",value);
        sp=0;
        store(0,datetime.month);
        store(0,datetime.days >> 4);
        store(0,datetime.days & 0x0f);
        store(0,datetime.hours >> 4);
        store(0,datetime.hours & 0x0f);
        store(0,datetime.minutes >> 4);
        store(0,datetime.minutes & 0x0f);
        store(0,datetime.seconds >> 4);
        store(0,datetime.seconds & 0x0f);
        sp=0;
        break;
    case 0x0d : // Store new date
        if (pPC->fp_log) fprintf(pPC->fp_log,"LU57813P - STACK -> DATE- %02x\n",value);
        if (pPC->fp_log) fflush(pPC->fp_log);

        if ( imem[0] != 0x0f) datetime.month = imem[0];
        if ( (imem[1] != 0x0f) && (imem[2] != 0x0f)) datetime.days = ((imem[1]&0x0f)<<4)|(imem[2]&0x0f);
        if ( (imem[3] != 0x0f) && (imem[4] != 0x0f)) datetime.hours = ((imem[3]&0x0f)<<4)|(imem[4]&0x0f);
        if ( (imem[5] != 0x0f) && (imem[6] != 0x0f)) datetime.minutes = ((imem[5]&0x0f)<<4)|(imem[6]&0x0f);
        if ( (imem[7] != 0x0f) && (imem[8] != 0x0f)) datetime.seconds = ((imem[7]&0x0f)<<4)|(imem[8]&0x0f);

        break;


    case 0x0e: // Key click ????
        if (pPC->fp_log) fprintf(pPC->fp_log,"LU57813P - KEYCLICK- %02x\n",value);
        break;
    case 0x0f :
        output_pending = true;
        reg_out = imem[sp];
        sp++;
        if (pPC->fp_log) fprintf(pPC->fp_log,"LU57813P - POP - %02X\n",reg_out);
        break;
    default: if (pPC->fp_log) fprintf(pPC->fp_log,"LU57813P - ACTION UNKNOWN- %02x\n",value);
    }
}

void CLU57813P::command(BYTE val)
{
    BYTE oper = val >> 4;
    BYTE value = val & 0x0f;

//    if (pPC->fp_log) fprintf(pPC->fp_log,"LU57813P - COMMAND - %02x\n",val);
    switch (oper) {
    case 0x00: sp=0; store(oper,0x0f - value); break;
    case 0x05: request(value); break;
    case 0x06: action(value); break;
    case 0x07: store(oper,0x0f - value); break;
    case 0x09: // Initiate
                // WAKE$(0) param 06
                // ALARM$   param 05
        if (pPC->fp_log) fprintf(pPC->fp_log,"LU57813P - Unknown 0x09 - %02x\n",value);
        break;
    case 0x0a: // End ON ADIN command ?
        if (pPC->fp_log) fprintf(pPC->fp_log,"LU57813P - Unknown 0x0A - %02x\n",value);
        break;
    case 0x0c:
        if (value == 0x03) { // initiate ON ADIN command ?
            if (pPC->fp_log) fprintf(pPC->fp_log,"LU57813P - StackPointer = ADIN_ADR (%02x) - %02x\n",ADIN_ADR,value);
            sp = ADIN_ADR;
        }
        else
            if (pPC->fp_log) fprintf(pPC->fp_log,"LU57813P - Unknown 0x0C - %02x\n",value);
        break;
//    case 0x0f: store(oper,0x0f - value);break;
    default:
        if (pPC->fp_log) fprintf(pPC->fp_log,"LU57813P - Unknown oper %02X-%02x\n",oper,value);
        break;

    }

    return;
}

bool CLU57813P::step(void)
{

    if (previous_state == 0) previous_state = pPC->pTIMER->state;

    if ( (pPC->pTIMER->state - previous_state) >= pPC->getfrequency() )
    {
        addretrace();
        previous_state = pPC->pTIMER->state;
    };

    Kon = pPC->pKEYB->CheckKon();

    return true;
}

BYTE    CLU57813P::Get_reg_out(void) {
    output_pending = false;
    return reg_out;
}

bool	CLU57813P::Get_tp(void)
{
// generate tp signal. Used by tape functionnality
#define TP_STATE		(pPC->getfrequency() / TP_FREQUENCY)
qint64 delta_state;

    if (previous_state_tp == 0) previous_state_tp = pPC->pTIMER->state;
    while ( (delta_state = (pPC->pTIMER->state - previous_state_tp)) >= (TP_STATE/2) )
    {
        tp ^= 1;
        previous_state_tp += (TP_STATE / 2);
    };

    return(tp);
}


void	CLU57813P::Set_c0(bool bit)			{ c0			= bit;	}
void	CLU57813P::Set_c1(bool bit)			{ c1			= bit;	}
void	CLU57813P::Set_c2(bool bit)			{ c2			= bit;	}
void	CLU57813P::Set_stb(bool bit)		{ stb			= bit;	}
void	CLU57813P::Set_cs(bool bit)			{ cs			= bit;	}
void	CLU57813P::Set_data(bool bit)		{ data_in		= bit;	}
void	CLU57813P::Set_gnd(bool bit)		{ gnd			= bit;	}
void	CLU57813P::Set_clk(bool bit)		{ clk			= bit;	}
void	CLU57813P::Set_out_enable(bool bit)	{ out_enable	= bit;	}
void	CLU57813P::Set_n_xtal(bool bit)		{ n_xtal		= bit;	}
void	CLU57813P::Set_xtal(bool bit)		{ xtal			= bit;	}
void	CLU57813P::Set_vdd(bool bit)		{ vdd			= bit;	}



void CLU57813P::Regs_Info(UINT8 Type)
{
#if 1
    switch(Type)
    {
    case 0:			// Monitor Registers Dialog
    case 1:			// For Log File
        sprintf(Regs_String,	"C0:%s C1:%s C2:%s STB:%s CS:%s D_In:%s CLK:%s D_Ou:%s TP:%s Out_en:%s",
                            c0?"1":"0",c1?"1":"0",c2?"1":"0",stb?"1":"0",cs?"1":"0",
                            data_in?"1":"0",clk?"1":"0",data_out?"1":"0",tp?"1":"0",
                            out_enable?"1":"0");
        break;
    }
#endif
}


