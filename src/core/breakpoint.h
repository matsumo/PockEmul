#ifndef BREAKPOINT_H
#define BREAKPOINT_H

#include "common.h"


class QXmlStreamWriter;
class QXmlStreamReader;
class CpcXXXX;
class Parser;


class Cbreakpoint {

public:
enum TYPE {
    EXEC,
    READ,
    WRITE,
    READWRITE,
    UNDEFINED
};
    Cbreakpoint() {}
    Cbreakpoint(TYPE _type, UINT32 _from, UINT32 _to, QString cond = QString() , bool _enabled=true);

    bool isEnabled() { return enabled;}
    void setEnabled(bool _val) { enabled = _val;}
    UINT32 From() { return adrFrom;}
    UINT32 To() { return adrTo;}
    bool isType(TYPE _val) { return (type == _val); }
    TYPE Type() { return type;}

    QString Cond() { return cond; }
    void setCond(QString _cond);
    QString toText();

    void serialize(QXmlStreamWriter *xmlOut);
    static Cbreakpoint* unserialize(QXmlStreamReader *xmlIn);
    static QString typeToText(TYPE _type);
    static TYPE textToType(QString _type);

private:
    UINT32 adrFrom;
    UINT32 adrTo;
    TYPE type;
    bool enabled;
    QString cond;
};

class CbreakpointManager {
public:

    CbreakpointManager(CpcXXXX * parent);
    bool isBreak(Cbreakpoint::TYPE _type, UINT32 _adr, int _val=-1);
    void addBreakPoint(Cbreakpoint *_b);

    QList<Cbreakpoint*> breakList;

    void serialize(QXmlStreamWriter *xmlOut);
    void unserialize(QXmlStreamReader *xmlIn);

    QString breakMsg;
    CpcXXXX *pPC;

    Parser *p;
};

#endif // BREAKPOINT_H
