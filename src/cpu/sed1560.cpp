

#include "sed1560.h"
#include "pcxxxx.h"
#include "Log.h"
#include "cpu.h"


CSED1560::CSED1560(CpcXXXX *parent)
{
    pPC = parent;

    for (int i = 0 ; i < IMEMSIZE ; i++)
    {
        info.imem[i] = 0;
    }

    info.on_off = false;
    info.displaySL = 0;
    info.ColAdrReg = 0;
    info.PgAdrReg = 0;
    info.busy = info.ADC = info.reset = false;
    updated = true;
    info.ReadModifyWrite = true;
    info.ElectCtrl = 0x0f;
}

CSED1560::~CSED1560() {

}

BYTE CSED1560::get8(qint16 adr)
{
    if (adr >= IMEMSIZE)
    {
        // ERROR
        AddLog(LOG_TEMP,tr("LCD : ERROR adr [%1] out of range").arg(adr,4,16,QChar('0')));
        if (pPC->fp_log) fprintf(pPC->fp_log,"LCD : ERROR adr [%04x] out of range [0,200h]\n",adr);
        return 0;
    }
    return info.imem[adr];
}

void CSED1560::set8(qint16 adr,BYTE val)
{
//    AddLog(LOG_DISPLAY,tr("SET[%1]=%2").arg(adr).arg(val));
    if (adr >= IMEMSIZE)
    {
        // ERROR
        if (pPC->fp_log) fprintf(pPC->fp_log,"LCD : ERROR adr [%04x] out of range [0,200h]\n",adr);

        return;
    }
    info.imem[adr] = val;
//    if (info.PgAdrReg > 7) { AddLog(LOG_DISPLAY,tr("status set [%1]=%2").arg(adr).arg(val));}
}



#define MASK_status     0x100
#define MASK_ColAdrLo   0x200
#define MASK_ColAdrHi   0x210
#define MASK_RevLineRst 0x220
#define MASK_BuildInPow 0x224
#define MASK_RevLineSet 0x230
#define MASK_displaySL  0x240
#define MASK_ElecCtrlReg 0x280
#define MASK_ADCSel     0x2a0
#define MASK_AllIndic   0x2a4
#define MASK_NormRevDsp 0x2a6
#define MASK_DutySel    0x2a8
#define MASK_DutyPlus   0x2aa
#define MASK_on_off     0x2ae
#define MASK_setPgAdr   0x2b0
#define MASK_OutStatusRegSet 0x2c0
#define MASK_ReadModWrt 0x2e0
#define MASK_Reset      0x2e2
#define MASK_End        0x2ee
#define MASK_read       0x500
#define MASK_write      0x600

BYTE CSED1560::instruction(qint16 cmd)
{
//    updated = true;
    if (pPC->pCPU->fp_log)fprintf(pPC->pCPU->fp_log,"SED1560 CMD: %04x\n",cmd);

//    AddLog(LOG_DISPLAY,tr("SED1560 CMD:%1").arg(cmd,4,16,QChar('0')));

    if ((cmd & MASK_write) == MASK_write ) { cmd_write(cmd); }
    else
    if ((cmd & MASK_read) == MASK_read ) { return cmd_read(cmd); }
    else
    if ((cmd & MASK_End) == MASK_End ) { info.ReadModifyWrite = false; }
    else
    if ((cmd & MASK_Reset) == MASK_Reset ) { cmd_Reset(cmd); }
    else
    if ((cmd & MASK_ReadModWrt) == MASK_ReadModWrt ) { info.ReadModifyWrite = true;  }
    else
    if ((cmd & MASK_OutStatusRegSet) == MASK_OutStatusRegSet ) { cmd_OutStatusRegSet(cmd); }
    else
    if ((cmd & MASK_setPgAdr) == MASK_setPgAdr ) { cmd_setPgAdr(cmd); }
    else
    if ((cmd & MASK_on_off) == MASK_on_off ) { cmd_on_off(cmd); }
    else
    if ((cmd & MASK_DutyPlus) == MASK_DutyPlus ) { cmd_DutyPlus(cmd); }
    else
    if ((cmd & MASK_DutySel) == MASK_DutySel ) { cmd_DutySel(cmd); }
    else
    if ((cmd & MASK_NormRevDsp) == MASK_NormRevDsp ) { cmd_NormRevDsp(cmd); }
    else
    if ((cmd & MASK_AllIndic) == MASK_AllIndic ) { cmd_AllIndic(cmd); }
    else
    if ((cmd & MASK_ElecCtrlReg) == MASK_ElecCtrlReg ) { cmd_ElecCtrlReg(cmd); }
    else
    if ((cmd & MASK_ADCSel) == MASK_ADCSel ) { cmd_ADCSel(cmd); }
    else
    if ((cmd & MASK_displaySL) == MASK_displaySL ) { cmd_displaySL(cmd); }
    else
    if ((cmd & MASK_RevLineSet) == MASK_RevLineSet ) { cmd_RevLineSet(cmd); }
    else
    if ((cmd & MASK_BuildInPow) == MASK_BuildInPow ) { cmd_BuildInPow(cmd); }
    else
    if ((cmd & MASK_RevLineRst) == MASK_RevLineRst ) { cmd_RevLineRst(cmd); }
    else
    if ((cmd & MASK_ColAdrHi) == MASK_ColAdrHi ) { cmd_ColAdrHi(cmd); }
    else
    if ((cmd & MASK_ColAdrLo) == MASK_ColAdrLo ) { cmd_ColAdrLo(cmd); }
    else
    if ((cmd & MASK_status) == MASK_status ) { return cmd_status(cmd); }

    return 0;
}
void CSED1560::cmd_ElecCtrlReg(qint16 cmd) {
    info.ElectCtrl = cmd & 0x1f;
}

void CSED1560::cmd_OutStatusRegSet(qint16 cmd) {

    AddLog(LOG_DISPLAY,tr("cmd_OutStatusRegSet : %1").arg(cmd,2,16,QChar('0')));
}

void CSED1560::cmd_DutyPlus(qint16 cmd) {
    AddLog(LOG_DISPLAY,tr("cmd_DutyPlus : %1").arg(cmd,2,16,QChar('0')));
}

void CSED1560::cmd_DutySel(qint16 cmd) {
    AddLog(LOG_DISPLAY,tr("cmd_DutySel : %1").arg(cmd,2,16,QChar('0')));
}

void CSED1560::cmd_NormRevDsp(qint16 cmd) {
    AddLog(LOG_DISPLAY,tr("cmd_NormRevDsp : %1").arg(cmd,2,16,QChar('0')));
}

void CSED1560::cmd_AllIndic(qint16 cmd) {
    AddLog(LOG_DISPLAY,tr("cmd_AllIndic : %1").arg(cmd,2,16,QChar('0')));
}

void CSED1560::cmd_ADCSel(qint16 cmd) {
    AddLog(LOG_DISPLAY,tr("cmd_ADCSel : %1").arg(cmd,2,16,QChar('0')));
}


void CSED1560::cmd_BuildInPow(qint16 cmd) {
    AddLog(LOG_DISPLAY,tr("cmd_OutStatusRegSet : %1").arg(cmd,2,16,QChar('0')));
}

void CSED1560::cmd_RevLineRst(qint16 cmd) {
    AddLog(LOG_DISPLAY,tr("cmd_RevLineRst : %1").arg(cmd,2,16,QChar('0')));
}

void CSED1560::cmd_RevLineSet(qint16 cmd) {
    AddLog(LOG_DISPLAY,tr("cmd_RevLineSet : %1").arg(cmd,2,16,QChar('0')));
}

void CSED1560::cmd_Reset(qint16 cmd) {
    info.displaySL = info.ColAdrReg = info.PgAdrReg = 0;
    AddLog(LOG_DISPLAY,tr("cmd_Reset : %1").arg(cmd,2,16,QChar('0')));
}

void CSED1560::cmd_on_off(qint16 cmd)
{

    bool newon_off = ((cmd & 0x01) ? true:false);
    AddLog(LOG_DISPLAY,tr("TURN %1").arg(newon_off?"ON":"OFF"));
    if (newon_off != info.on_off) {
        info.on_off = newon_off;
        updated = true;
        AddLog(LOG_DISPLAY,tr("TURN %1").arg(newon_off?"ON":"OFF"));
    }
//    if (pPC->fp_log) fprintf(pPC->fp_log,"LCD :%s\n",on_off?"on":"off");
}

void CSED1560::cmd_displaySL(qint16 cmd)
{
    BYTE newSL = cmd & 0x3f;
    if (newSL != info.displaySL) {
        info.displaySL = newSL;
//        AddLog(LOG_DISPLAY,tr("SED1560 SL:%1").arg(cmd&0x3f,4,16,QChar('0')));
        updated = true;
//        AddLog(LOG_DISPLAY,tr("UPDATED displaySL"));
    }
}

void CSED1560::cmd_setPgAdr(qint16 cmd)
{
    BYTE newPgAdr = cmd & 0x0f;
    if (newPgAdr != info.PgAdrReg) {
        info.PgAdrReg = newPgAdr;
        updated = true;
//        if (newPgAdr>7) { AddLog(LOG_DISPLAY,tr("UPDATED pgAdr : %1").arg(newPgAdr));}
    }
}

void CSED1560::cmd_ColAdrHi(qint16 cmd) {
    BYTE newColAdrHi = (cmd & 0x0f);
    if (newColAdrHi != (info.ColAdrReg >> 4)) {
        info.ColAdrReg = (newColAdrHi<<4) | (info.ColAdrReg & 0x0f);
        updated = true;
//        AddLog(LOG_DISPLAY,tr("UPDATED ColAdrHi"));
    }
}

void CSED1560::cmd_ColAdrLo(qint16 cmd) {
    BYTE newColAdrLo = (cmd & 0x0f);
    if (newColAdrLo != (info.ColAdrReg & 0x0f)) {
        info.ColAdrReg = (info.ColAdrReg & 0xf0) | newColAdrLo ;
        updated = true;
//        AddLog(LOG_DISPLAY,tr("UPDATED ColAdrLo"));
    }
}

BYTE CSED1560::cmd_status(qint16 cmd)
{
//    if (pPC->fp_log) fprintf(pPC->fp_log,"LCD request status\n");
    BYTE status =   (info.busy  << 7) |
                    (info.ADC   << 6) |
                    ((!info.on_off)<< 5) |
                    (info.reset << 4);
    return status;
}

void CSED1560::cmd_write(qint16 cmd)
{
    if ((pPC->pCPU->fp_log) && (cmd & 0xff))fprintf(pPC->pCPU->fp_log,"LCD Write:%02x\n",cmd & 0xff);
//    if ((pPC->fp_log) && (cmd & 0xff)) fprintf(pPC->fp_log,"LCD Write:x=%02x y=%02x val=%02x\n",Xadr,Yadr,cmd & 0xff);
    set8( info.ColAdrReg  + info.PgAdrReg * 0xa6 , (cmd & 0xff));

    if (info.ReadModifyWrite) {
        info.ColAdrReg++;
    }

    if (info.ColAdrReg > 0xa6) {
        info.ColAdrReg=0xa6;
    }
    updated = true;
//    AddLog(LOG_DISPLAY,tr("UPDATED write"));
}

BYTE CSED1560::cmd_read(qint16 cmd)
{


    BYTE value = get8( info.ColAdrReg  + info.PgAdrReg * 0xa6 );
//    AddLog(LOG_TEMP,tr("HD61102 READ CMD : x=%1   Y=%2  v=%3").arg(info.Xadr).arg(info.Yadr).arg(value,2,16,QChar('0')));
    if (!info.ReadModifyWrite) {
        info.ColAdrReg++;
    }

    if (info.ColAdrReg > 0xa6) {
        info.ColAdrReg = 0xa6;
    }

//    if (pPC->fp_log) fprintf(pPC->fp_log,"LCD Read:%02x\n",value);
    return value;
}


void CSED1560::Load_Internal(QXmlStreamReader *xmlIn)
{
    if (xmlIn->readNextStartElement()) {
        if ( (xmlIn->name()=="cpu") &&
             (xmlIn->attributes().value("model").toString() == "sed1560")) {
            QByteArray ba_reg = QByteArray::fromBase64(xmlIn->attributes().value("registers").toString().toLatin1());
            memcpy((char *) &info,ba_reg.data(),sizeof(info));
        }
        xmlIn->skipCurrentElement();
    }
}

void CSED1560::save_internal(QXmlStreamWriter *xmlOut)
{
    xmlOut->writeStartElement("cpu");
        xmlOut->writeAttribute("model","sed1560");
        QByteArray ba_reg((char*)&info,sizeof(info));
        xmlOut->writeAttribute("registers",ba_reg.toBase64());
    xmlOut->writeEndElement();
}







