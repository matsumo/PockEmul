#ifndef BUS_H
#define BUS_H




#include <QtCore>
#include <QMetaType>



class Cbus {

public:
    Cbus(QString _desc = QString());
    virtual ~Cbus() {}

    virtual quint64 toUInt64() const;
    virtual void fromUInt64(quint64 val);

    quint32 getData() const { return data;}
    void setData(quint32 val) { data = val;}

    quint32 getAddr() const { return addr;}
    virtual void setAddr(quint32 val) { addr=val;}

    bool getINT() const { return interrupt; }
    void setINT(bool val) { interrupt = val; }

    bool getRESET() const { return reset; }
    void setRESET(bool val) { reset = val; }

    bool isWrite() const { return writeMode; }
    void setWrite(bool val) { writeMode = val; }

    bool isEnable() const { return enable; }
    void setEnable(bool val) { enable = val; }

    virtual QString toLog() const;

protected:
    QString desc;
    quint32 addr;
    quint32 data;
    bool    enable;
    bool    interrupt;
    bool    writeMode;
    bool    reset;

};

Q_DECLARE_METATYPE(Cbus)

QDebug operator<<(QDebug dbg, const Cbus &bus);




#endif // BUS_H
