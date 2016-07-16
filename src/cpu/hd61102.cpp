#include <QDebug>

#include "hd61102.h"
#include "pcxxxx.h"
#include "cpu.h"
#include "Log.h"
#include "Inter.h"

#define MASK_on_off     0x3e
#define MASK_setY       0x40
#define MASK_setX       0xb8
#define MASK_displaySL  0xc0
#define MASK_write      0x100
#define MASK_status     0x200
#define MASK_read       0x300

CHD61102::CHD61102(CPObject *parent)
{
    pPC = parent;

    for (int i = 0 ; i < 0x200 ; i++)
    {
        info.imem[i] = 0;
    }

    info.on_off = 0;
    info.displaySL = 0;
    info.Xadr = 0;
    info.Yadr = 0;
    info.status = 0;
    updated = false;
    last_state_cmd = 0;
    outputRegister = -1;
}

CHD61102::~CHD61102() {

}

BYTE CHD61102::get8(qint16 adr)
{
    if (adr >= 0x200)
    {
        // ERROR
        qWarning()<<tr("LCD : ERROR adr [%1] out of range").arg(adr,4,16,QChar('0'));
        if (pPC->fp_log) fprintf(pPC->fp_log,"LCD : ERROR adr [%04x] out of range [0,200h]\n",adr);
        return 0;
    }
//    qWarning()<<"get8["<<adr<<"]="<<info.imem[adr];
    return info.imem[adr];
}

void CHD61102::set8(qint16 adr,BYTE val)
{

    if (adr >= 0x200)
    {
        // ERROR
        if (pPC->fp_log) fprintf(pPC->fp_log,"LCD : ERROR adr [%04x] out of range [0,200h]\n",adr);

        return;
    }
//    qWarning()<<"set8["<<adr<<"]="<<val;
    info.imem[adr] = val;
}

bool CHD61102::init()
{
    for (int i = 0 ; i < 0x200 ; i++)
    {
        info.imem[i] = 0;
    }

    info.on_off = 0;
    info.displaySL = 0;
    info.Xadr = 0;
    info.Yadr = 0;
    info.status = 0;
    updated = true;
    outputRegister = -1;

    return true;
}

BYTE CHD61102::instruction(qint16 cmd)
{
//    if (pPC->pCPU->fp_log)fprintf(pPC->pCPU->fp_log,"HD61102 CMD: %04x\n",cmd);
//    qWarning()<<tr("HD61102 CMD:%1").arg(cmd,4,16,QChar('0'));

    if (pPC->pTIMER->nsElapsed(last_state_cmd)<(2000/pPC->pTIMER->CPUSpeed)) {
        info.status |= 0x80;
        qWarning()<<"LCD BUSY !!!!!";
        return 00;
    }
    else {
        info.status &= 0x7f;
//        qWarning()<<"delta t="<<pPC->pTIMER->nsElapsed(last_state_cmd)<<" : "<<pPC->pTIMER->state-last_state_cmd ;
        last_state_cmd = pPC->pTIMER->state;
    }

    AddLog(LOG_DISPLAY,tr("HD61102 CMD:%1").arg(cmd,4,16,QChar('0')));

    if ((cmd & MASK_read) == MASK_read ) { return cmd_read(cmd); }
    else
    if ((cmd & MASK_status) == MASK_status ) { return cmd_status(cmd); }
    else
    if ((cmd & MASK_write) == MASK_write ) { cmd_write(cmd); }
    else
    if ((cmd & MASK_displaySL) == MASK_displaySL ) { cmd_displaySL(cmd); }
    else
    if ((cmd & MASK_setX) == MASK_setX ) { cmd_setX(cmd); }
    else
    if ((cmd & MASK_setY) == MASK_setY ) { cmd_setY(cmd); }
    else
    if ((cmd & MASK_on_off) == MASK_on_off ) { cmd_on_off(cmd); }



    return 0;
}

void CHD61102::cmd_on_off(qint16 cmd)
{
//    qWarning()<<"LCD "<<(info.on_off?"ON":"OFF")<< "  cmd:"<<cmd;

    BYTE newon_off = cmd & 0x01;
    if (newon_off != info.on_off) {
        info.on_off = newon_off;
        updated = true;
        AddLog(LOG_DISPLAY,tr("UPDATED on_off"));
    }

//    info.on_off = 1;

//    if (pPC->fp_log) fprintf(pPC->fp_log,"LCD :%s\n",on_off?"on":"off");
}

void CHD61102::cmd_displaySL(qint16 cmd)
{
    BYTE newSL = cmd & 0x3f;
    if (newSL != info.displaySL) {
        info.displaySL = newSL;
//        qWarning()<<tr("HD61102 SL:%1").arg(cmd&0x3f,4,16,QChar('0'));
        updated = true;
        AddLog(LOG_DISPLAY,tr("UPDATED displaySL"));
    }
}

void CHD61102::cmd_setX(qint16 cmd)
{
    BYTE newXadr = cmd & 0x07;
    if (newXadr != info.Xadr) {
        info.Xadr = newXadr;
        updated = true;
        AddLog(LOG_DISPLAY,tr("UPDATED setX"));
    }
//    qWarning()<<"setX = "<<info.Xadr;
}

void CHD61102::cmd_setY(qint16 cmd)
{
    BYTE newYadr = cmd & 0x3f;
    if (newYadr != info.Yadr) {
        info.Yadr = newYadr;
        updated = true;
        AddLog(LOG_DISPLAY,tr("UPDATED setY"));
    }
//    if (pPC->fp_log) fprintf(pPC->fp_log,"LCD Y:%i\n",Yadr);
//    qWarning()<<"setY = "<<info.Yadr;
}

BYTE CHD61102::cmd_status(qint16 cmd)
{
//    qWarning()<<"read status";
//    if (pPC->fp_log) fprintf(pPC->fp_log,"LCD request status\n");

    return info.status | (info.on_off ? 0x00 : 0x20) ;
}

void CHD61102::cmd_write(qint16 cmd)
{
//    qWarning()<<"LCD Write:"<<(cmd&0xff);
//    if ((pPC->fp_log) && (cmd & 0xff))fprintf(pPC->fp_log,"LCD Write:%02x\n",cmd & 0xff);
//    if ((pPC->fp_log) && (cmd & 0xff)) fprintf(pPC->fp_log,"LCD Write:x=%02x y=%02x val=%02x\n",Xadr,Yadr,cmd & 0xff);
    set8( (info.Xadr * 0x40) + info.Yadr , (cmd & 0xff));
    (info.Yadr)++;
    if (info.Yadr == 64) {
        info.Yadr=0;
    }
    updated = true;
    AddLog(LOG_DISPLAY,tr("UPDATED write"));
}


BYTE CHD61102::cmd_read(qint16 cmd)
{


    BYTE value = get8((info.Xadr * 0x40) + (info.Yadr==0 ? 63 : info.Yadr - 1) );
    AddLog(LOG_TEMP,tr("HD61102 READ CMD : x=%1   Y=%2  v=%3").arg(info.Xadr).arg(info.Yadr).arg(value,2,16,QChar('0')));
//    qWarning()<<tr("HD61102 READ CMD : x=%1   Y=%2  v=%3").arg(info.Xadr).arg(info.Yadr).arg(value,2,16,QChar('0'));
    (info.Yadr)++;
    if (info.Yadr == 64) {
        info.Yadr=0;
    }

//    if (pPC->fp_log) fprintf(pPC->fp_log,"LCD Read:%02x\n",value);
    return value;
}


void	CHD61102::Load_Internal(QFile *file){
    char t[16];
    QDataStream in(file);

    in.readRawData(t, 10);
    in.readRawData((char*)&info,sizeof(info));
}

void	CHD61102::save_internal(QFile *file){
    QDataStream out(file);

    out.writeRawData("HD61102STA", 10);					//header
    out.writeRawData((char*)&info,sizeof(info));		//reg
}

void CHD61102::Load_Internal(QXmlStreamReader *xmlIn)
{
    if (xmlIn->readNextStartElement()) {
        if ( (xmlIn->name()=="cpu") &&
             (xmlIn->attributes().value("model").toString() == "hd61102")) {
            QByteArray ba_reg = QByteArray::fromBase64(xmlIn->attributes().value("registers").toString().toLatin1());
            memcpy((char *) &info,ba_reg.data(),sizeof(info));
            updated = true;
        }
        xmlIn->skipCurrentElement();
    }
}

void CHD61102::save_internal(QXmlStreamWriter *xmlOut)
{
    xmlOut->writeStartElement("cpu");
        xmlOut->writeAttribute("model","hd61102");
        QByteArray ba_reg((char*)&info,sizeof(info));
        xmlOut->writeAttribute("registers",ba_reg.toBase64());
    xmlOut->writeEndElement();
}
