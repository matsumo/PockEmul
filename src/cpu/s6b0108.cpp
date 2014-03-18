#include <QDebug>

#include "s6b0108.h"
#include "Log.h"

CS6B0108::CS6B0108(CPObject *parent):CHD61102(parent)
{

}

CS6B0108::~CS6B0108() {

}

void CS6B0108::cmd_write(qint16 cmd)
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

BYTE CS6B0108::cmd_read(qint16 cmd)
{


    BYTE value = get8((info.Xadr * 0x40) + info.Yadr );
    AddLog(LOG_TEMP,tr("HD61102 READ CMD : x=%1   Y=%2  v=%3").arg(info.Xadr).arg(info.Yadr).arg(value,2,16,QChar('0')));
//    qWarning()<<tr("HD61102 READ CMD : x=%1   Y=%2  v=%3").arg(info.Xadr).arg(info.Yadr).arg(value,2,16,QChar('0'));
    (info.Yadr)++;
    if (info.Yadr == 64) {
        info.Yadr=0;
    }

//    if (pPC->fp_log) fprintf(pPC->fp_log,"LCD Read:%02x\n",value);
    return value;
}
