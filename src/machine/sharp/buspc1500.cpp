#include "buspc1500.h"

CbusPc1500::CbusPc1500()
{
    inhibit = false;
}

quint64 CbusPc1500::toUInt64() const
{
    quint64 serialized = 0;

    serialized |= (interrupt?1:0) ;
    serialized |= ((writeMode?1:0) << 1);
    serialized |= ((enable?1:0) << 2);
    serialized |= ((me1?1:0) << 3);
    serialized |= ((pu?1:0) << 4);
    serialized |= ((pv?1:0) << 5);
    serialized |= ((inhibit?1:0) << 6);
    serialized |= ((cmtIn?1:0) << 7);
    serialized |= ((cmtOut?1:0) << 8);
    serialized |= ((pt?1:0) << 9);
    serialized |= ((quint64)addr & 0xffff)<<10;
    serialized |= ((quint64)data & 0xff) << 26;

    return serialized;
}

void CbusPc1500::fromUInt64(quint64 val)
{
    interrupt = val &0x01;
    writeMode = (val >> 1) &0x01;
    enable    = (val >> 2) &0x01;
    me1       = (val >> 3) &0x01;
    pu        = (val >> 4) &0x01;
    pv        = (val >> 5) &0x01;
    inhibit   = (val >> 6) &0x01;
    cmtIn     = (val >> 7) &0x01;
    cmtOut    = (val >> 8) &0x01;
    pt        = (val >> 9) &0x01;

    addr = (val>>10) & 0xffff;     // 16 bits
    data = (val >>26) & 0xff;     // 8 bits
}

void CbusPc1500::setAddr(quint32 val)
{
    me1=false;
    addr = val & 0xffff;
    if (val >> 16) {
        me1=true;
    }
}

QString CbusPc1500::toLog() const
{
    QString ret;
    ret = QString("addr=%1").arg(getAddr(),4,16,QChar('0'));
    ret += QString(" - data=%1").arg(getData(),2,16,QChar('0'));
    ret += QString(" Write:%1").arg(writeMode);
    ret += QString(" INT:%1").arg(interrupt);
    ret += QString(" ME1:%1").arg(me1);
    ret += QString(" INHIBIT:%1").arg(inhibit);
    ret += QString(" CMTIn:%1").arg(cmtIn);
    ret += QString(" CMTOut:%1").arg(cmtOut);
    ret += QString(" PT:%1").arg(pt);
    ret += QString(" PU:%1").arg(pu);
    ret += QString(" PV:%1").arg(pv);
    return ret;
}

