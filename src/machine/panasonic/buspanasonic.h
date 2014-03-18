#ifndef BUSPANASONIC_H
#define BUSPANASONIC_H


#include <QtCore>
#include <QMetaType>
#include "bus.h"


enum BUS_FUNC{
    BUS_SLEEP,
    BUS_WRITEDATA,BUS_READDATA,
    BUS_QUERY,BUS_SELECT,BUS_ACK,
    BUS_LINE0,BUS_LINE1,BUS_LINE2,BUS_LINE3};


class CbusPanasonic:public Cbus {

public:
    CbusPanasonic(quint8 dest=0, BUS_FUNC func=BUS_SLEEP, quint8 data=0, bool write= false);

    virtual quint64 toUInt64() const;
    virtual void fromUInt64(quint64 val);

    quint8 getDest() const { return dest;}
    void setDest(quint8 val) { dest = val;}

    BUS_FUNC getFunc() const  { return func;}
    void setFunc(BUS_FUNC val) { func = val;}


    virtual QString toLog() const;

private:
    quint8 dest;
    BUS_FUNC func;


};


#endif // BUSPANASONIC_H
