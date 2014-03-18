#include "buspanasonic.h"

CbusPanasonic::CbusPanasonic(quint8 dest, BUS_FUNC func, quint8 data,bool write)
{
    this->dest = dest;
    this->func = func;
    this->data = data;
    this->writeMode = write;
}

quint64 CbusPanasonic::toUInt64() const
{
    quint64 serialized = 0;

    serialized |= ((quint64)addr & 0xffffff);
    serialized |= ((quint64)data << 24);
    serialized |= ((quint64)dest << 32);
    quint64 _val = func;
    serialized |= ((_val&0xff) << 40);
    serialized |=( (quint64)(interrupt?1:0) << 48);
    serialized |=( (quint64)(writeMode?1:0) << 49);
//    if (interrupt) qWarning()<<"test:"<<((serialized >> 40) &0x01);

    return serialized;
}

void CbusPanasonic::fromUInt64(quint64 val)
{
    addr = val & 0xffffff;
    data = (val >>24) & 0xff;
    dest = (val >>32) & 0xff;
    func = static_cast<BUS_FUNC>((val >>40) & 0xff);
    interrupt = (val >> 48) &0x01;
    writeMode = (val >> 49) &0x01;
    //    if (interrupt) qWarning()<<"INTERRUPT";
}

QString CbusPanasonic::toLog() const
{
    QString ret;
    ret = QString("Dest:%1  - %2").arg(getDest()).arg(getFunc());
    switch (getFunc()) {
    case BUS_QUERY: ret += "BUS_QUERY"; break;
    case BUS_SELECT: ret += "BUS_SELECT"; break;
    case BUS_READDATA: ret += "BUS_READDATA";
        ret += QString(" - addr=%1").arg(getAddr(),6,16,QChar('0'));
        break;
    case BUS_WRITEDATA: ret += "BUS_WRITEDATA";
        ret += QString(" - addr=%1").arg(getAddr(),6,16,QChar('0'));
        break;
    case BUS_LINE0: ret += "BUS_LINE0"; break;
    case BUS_LINE1: ret += "BUS_LINE1"; break;
    case BUS_LINE2: ret += "BUS_LINE2"; break;
    case BUS_LINE3: ret += "BUS_LINE3"; break;
    }
    ret += QString(" - data=%1").arg(getData(),2,16,QChar('0'));

    return ret;
}




