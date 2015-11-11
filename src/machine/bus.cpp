#include "bus.h"

Cbus::Cbus::Cbus(QString _desc)
{
    enable = false;
    this->desc = _desc;
}

quint64 Cbus::toUInt64() const
{
    quint64 serialized = 0;

    serialized |= (interrupt?1:0) ;
    serialized |= ((writeMode?1:0) << 1);
    serialized |= ((enable?1:0) << 2);
    serialized |= ((reset?1:0) << 3);
    serialized |= ((quint64)addr & 0xffffffff)<<4;
    serialized |= ((quint64)data & 0xffffff) << 36;

    return serialized;
}

void Cbus::fromUInt64(quint64 val)
{
    interrupt = val &0x01;
    writeMode = (val >> 1) &0x01;
    enable    = (val >> 2) &0x01;
    reset     = (val >> 3) &0x01;
    addr = (val>>4) & 0xffffffff;       // 32bits
    data = (val >>36) & 0xffffff;     // 24bits
}

QString Cbus::toLog() const
{
    QString ret = desc.isEmpty() ? "" : desc+":";
    ret += QString("addr=%1").arg(getAddr(),6,16,QChar('0'));
    ret += QString(" - data=%1").arg(getData(),2,16,QChar('0'));
    ret += QString(" Write:%1").arg(writeMode);
    ret += QString(" INT:%1").arg(interrupt);
    ret += QString(" RESET:%1").arg(reset);

    return ret;
}

QDebug operator<<(QDebug dbg, const Cbus &bus)
{
    dbg.nospace() << bus.toLog();
    return dbg.maybeSpace();
}


