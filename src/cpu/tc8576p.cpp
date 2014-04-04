#include <QDebug>

#include "tc8576p.h"
#include "pcxxxx.h"


CTC8576P::CTC8576P(CpcXXXX *parent, int clk)
{
    pPC = parent;

    r.xclk = clk;

    r.psr=0;
    r.ssr=0;
}

CTC8576P::~CTC8576P() {

}

bool CTC8576P::init(void)
{
    return true;
}

bool CTC8576P::exit(void)
{
    return true;
}

void CTC8576P::Reset(void)
{
    if (pPC->fp_log) fprintf(pPC->fp_log,"TC8576P - RESET\n");
    r.psr = r.ssr = 0;

    r.IntF = false;
    r.XBUSY = false;
    r.BUSY = false;
    r.PRIME = false;
    r.P5V = false;
    r.PE = false;
    r.SLCT = false;
    r.FAULT = false;
    r.IM1 = false;
    r.IM2 = false;

    r.DSR = false;
    r.RBRK = false;
    r.FE = false;
    r.OE = false;
    r.PERR = false;
    r.TxE = false;
    r.RxRDY = false;
    r.TxRDY = false;

    r.TxEN = false;
    r.DTR = false;
    r.RxEN = false;
    r.SBRK = false;
    r.ERS = false;
    r.RTS = false;

    r.SO = false;
    r.TxINTM = false;
    r.CL = 5;
    r.PEN = false;
    r.EP = false;
    r.ERINTM = false;
    r.RXINTM = false;


}

bool CTC8576P::step(void)
{
// Get the connector Informations depending of the correct comm device (RS232 - SIO)

    // Set ssr
//    ssr = 0;

    // Set prs
//    psr = 0;

    r.TxE = true;
//    BUSY = true;

    return true;
}


// Write operation
bool CTC8576P::instruction(BYTE cmd)
{
    qWarning()<<"TC8576P - instruction";
    if (pPC->fp_log) fprintf(pPC->fp_log,"TC8576P - COMMAND - %02x\n",cmd);
    // Serial Command Register
    // b7 = 0; command byte=b0->b6
    if (!(cmd & 0x80)) {
        SCR(cmd & 0x7f);
    }

    switch (cmd >> 6) {
    case 0x02:  // Parallel Command Register
                PCR(cmd & 0x3f);
                break;
    case 0x03: // Parameter Address Set Register
                if (cmd & 0x20) Reset();
                else
                    r.par = (cmd & 0x07);
                break;
    }

    return true;
}

BYTE CTC8576P::get_psr(void)
{
    r.psr = 0;

    PUT_BIT(r.psr,7,r.IntF);
    PUT_BIT(r.psr,6,r.XBUSY);
    PUT_BIT(r.psr,5,r.BUSY);
    PUT_BIT(r.psr,4,r.PRIME);
    PUT_BIT(r.psr,3,r.P5V);
    PUT_BIT(r.psr,2,r.PE);
    PUT_BIT(r.psr,1,r.SLCT);
    PUT_BIT(r.psr,0,r.FAULT);

    return r.psr;
}

BYTE CTC8576P::get_ssr(void)
{
    r.ssr = 0;

    PUT_BIT(r.ssr,7, r.DSR);
    PUT_BIT(r.ssr,6,r.RBRK);
    PUT_BIT(r.ssr,5,r.FE);
    PUT_BIT(r.ssr,4,r.OE);
    PUT_BIT(r.ssr,3,r.PERR);
    PUT_BIT(r.ssr,2,r.TxE);
    PUT_BIT(r.ssr,1,r.RxRDY);
    PUT_BIT(r.ssr,0,r.TxRDY);

    //if (pPC->fp_log) fprintf(pPC->fp_log,"TC8576P - getSSR - %02x\n",ssr);
    return r.ssr;
}

void CTC8576P::SCR(BYTE cmd)
{
    //if (pPC->fp_log) fprintf(pPC->fp_log,"TC8576P - SCR - %02x\n",cmd);

    r.scr = cmd;
    r.TxEN = READ_BIT(r.scr,0);
    r.DTR = READ_BIT(r.scr,1);
    r.RxEN = READ_BIT(r.scr,2);
    r.SBRK = READ_BIT(r.scr,3);
    r.ERS = READ_BIT(r.scr,4);
    r.RTS = READ_BIT(r.scr,5);

}

void CTC8576P::PCR(BYTE cmd)
{
    if (pPC->fp_log) fprintf(pPC->fp_log,"TC8576P - PCR - %02x\n",cmd);

    r.pcr = cmd;

    switch (cmd & 0x07){
    case 0: r.FAULT = 0; break;
    case 1: r.SLCT = 0; break;
    case 2: r.PE = 0; break;
    case 3: r.P5V = 0; break;
    case 4: r.PRIME = 1; break;
    case 5: r.PRIME = 1; break;
    case 6: r.PRIME = 0; break;
    case 7: break;
    }

    r.IM1 = READ_BIT(cmd,5);
    r.IM2 = READ_BIT(cmd,4);
}

void CTC8576P::SPR(void)
{
    r.SO = READ_BIT(r.pr[5],0);
    r.TxINTM = READ_BIT(r.pr[5],1);
    r.CL = ((r.pr[5]>> 2) & 0x03) + 5;
    r.PEN = READ_BIT(r.pr[5],4);
    r.EP = READ_BIT(r.pr[5],5);
    r.ERINTM = READ_BIT(r.pr[5],6);
    r.RXINTM = READ_BIT(r.pr[5],7);
}

#define NB_STOP_BITS    ((r.pr[5] & 0x01) ? 2 : 1)
#define INT_CTRL_TxRDY  (!((r.pr[5]>>1) &0x01 ))
#define CHAR_LENGTH     ( 5 + ((r.pr[5]>>2) & 0x03))


bool CTC8576P::in(BYTE data)
{
    qWarning()<<"TC8576P";
    if (pPC->fp_log) fprintf(pPC->fp_log,"TC8576P - IN - %02x, par=%i\n",data,r.par);

    r.pr[r.par] = data;

    switch (r.par)
    {
    case 0x00: baud(data); break;       // Baud divider BL
    case 0x01: baud((data&0x0f) << 8); break;  // Baud divider BH
    case 0x02: break;      // Delay time
    case 0x05: SPR(); break;
    case 0x07: prescaler(data & 0x0f); break;  // Prescaler set
    }

    return true;
}

void CTC8576P::baud(qint16 data)
{


    int b = r.pr[0] + ((r.pr[1] & 0x0f) << 8);

    switch (b) {
        case 0: r.baudrate = r.fsysclk / 4096 / 8; break;
        case 1: r.baudrate = 0; break;
        default: r.baudrate = r.fsysclk / b / 8; break;
    }
    if (pPC->fp_log) fprintf(pPC->fp_log,"TC8576P - BAUD - %02x = %d\n",data,r.baudrate);
}

void CTC8576P::prescaler(qint8 data)
{
    switch (data){
        case 0: r.fsysclk = r.xclk / 16; break;
        default: r.fsysclk = r.xclk / data; break;
    }
    if (pPC->fp_log) fprintf(pPC->fp_log,"TC8576P - PRESCALER - %02x\n",data);
}

void	CTC8576P::Load_Internal(QFile *file){
    char t[16];
    QDataStream in(file);

    in.readRawData(t, 10);
    in.readRawData((char*)&r,sizeof(r));
}

void	CTC8576P::save_internal(QFile *file){
    QDataStream out(file);

    out.writeRawData("TC8576PSTA", 10);					//header
    out.writeRawData((char*)&r,sizeof(r));		//reg
}

void CTC8576P::Load_Internal(QXmlStreamReader *xmlIn)
{
    if (xmlIn->readNextStartElement()) {
        if ( (xmlIn->name()=="cpu") &&
             (xmlIn->attributes().value("model").toString() == "tc8576")) {
            QByteArray ba_reg = QByteArray::fromBase64(xmlIn->attributes().value("registers").toString().toLatin1());
            memcpy((char *) &r,ba_reg.data(),sizeof(r));
        }
        xmlIn->skipCurrentElement();
    }
}

void CTC8576P::save_internal(QXmlStreamWriter *xmlOut)
{
    xmlOut->writeStartElement("cpu");
        xmlOut->writeAttribute("model","tc8576");
        QByteArray ba_reg((char*)&r,sizeof(r));
        xmlOut->writeAttribute("registers",ba_reg.toBase64());
    xmlOut->writeEndElement();
}
