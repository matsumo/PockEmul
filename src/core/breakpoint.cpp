#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <QDebug>

#include "breakpoint.h"
#include "lcc/parser/parser.h"
#include "pcxxxx.h"
#include "cpu.h"
#include "Debug.h"
#include "ui/dialogdasm.h"

Cbreakpoint::Cbreakpoint(Cbreakpoint::TYPE _type, UINT32 _from, UINT32 _to, QString _cond, bool _enabled)
{
    type = _type;
    adrFrom = _from;
    adrTo   = _to;
    enabled = _enabled;
    cond = _cond;
}

void Cbreakpoint::setCond(QString _cond) {
    cond = _cond;

}

QString Cbreakpoint::toText() {
    QString ret;
    switch (type) {
    case Cbreakpoint::EXEC: ret = "(EXEC)  "; break;
    case Cbreakpoint::READ: ret = "(READ)  "; break;
    case Cbreakpoint::WRITE:ret = "(WRITE) "; break;
    case Cbreakpoint::READWRITE:ret = "(R/W) "; break;
    }

    ret += QString("%1").arg(adrFrom,6,16,QChar('0'));
    if (adrFrom != adrTo)
        ret += QString(" - %1").arg(adrTo,6,16,QChar('0'));

    return QString(ret);
}

void Cbreakpoint::serialize(QXmlStreamWriter *xmlOut)
{
    xmlOut->writeStartElement("breakpoint");
    xmlOut->writeAttribute("type",typeToText(type));
    xmlOut->writeAttribute("adrFrom",QString("%1").arg(adrFrom,6,16,QChar('0')));
    xmlOut->writeAttribute("adrTo",QString("%1").arg(adrTo,6,16,QChar('0')));
    xmlOut->writeAttribute("enabled",enabled?"true":"false");
    xmlOut->writeAttribute("cond",cond);
    xmlOut->writeEndElement();  // breakpoints
}

Cbreakpoint* Cbreakpoint::unserialize(QXmlStreamReader *xmlIn)
{
    QString eltname = xmlIn->name().toString();
    if (eltname=="breakpoint") {
        TYPE _type = textToType(xmlIn->attributes().value("type").toString());
        UINT32 _adrFrom = xmlIn->attributes().value("adrFrom").toString().toUInt(0,16);
        UINT32 _adrTo = xmlIn->attributes().value("adrTo").toString().toUInt(0,16);
        bool _enabled = xmlIn->attributes().value("enabled").toString()=="true" ? true : false;
        QString _cond = xmlIn->attributes().value("cond").toString();
        Cbreakpoint *_bpt = new Cbreakpoint(_type,_adrFrom,_adrTo,_cond,_enabled);

        return _bpt;
    }
    return 0;
}

QString Cbreakpoint::typeToText(Cbreakpoint::TYPE _type)
{
    switch (_type) {
    case EXEC: return QString("EXEC");
    case READ: return QString("READ");
    case WRITE: return QString("WRITE");
    case READWRITE: return QString("R/W");
    default: return "UNDEFINED";
    }

    return "UNDEFINED";
}

Cbreakpoint::TYPE Cbreakpoint::textToType(QString _type)
{
    if (_type=="EXEC") return EXEC;
    if (_type=="READ") return READ;
    if (_type=="WRITE") return WRITE;
    if (_type=="R/W") return READWRITE;

    return UNDEFINED;
}

CbreakpointManager::CbreakpointManager(CpcXXXX *parent)
{
    pPC = parent;
    p = new Parser("");
}

bool CbreakpointManager::isBreak(Cbreakpoint::TYPE _type,UINT32 _adr, int _val )
{
    switch (_type) {

    case Cbreakpoint::EXEC:
        for (int i=0; i<breakList.count();i++) {
            Cbreakpoint *_bpt = breakList[i];
            if ( _bpt->isEnabled() &&
                 _bpt->isType(Cbreakpoint::EXEC) &&
                 (_adr >= _bpt->From()) &&
                 (_adr <= _bpt->To())) {

                if (!_bpt->Cond().isEmpty())
                {
                    // Evaluate condition
                    Parser p(_bpt->Cond().toLatin1().data());
                    pPC->pCPU->pDEBUG->injectReg(&p);
                    p.symbols_ ["Val"]=	_val;
                    if (p.Evaluate()<=0) return false;
                }

                breakMsg = _bpt->toText();
//                qWarning()<<breakMsg;
                return true;
            }
        }
        break;

    case Cbreakpoint::READ:
        for (int i=0; i<breakList.count();i++) {
            Cbreakpoint *_bpt = breakList[i];
            if ( _bpt->isEnabled() &&
                 _bpt->isType(Cbreakpoint::READ) &&
                 (_adr >= _bpt->From()) &&
                 (_adr <= _bpt->To()) ) {

                if (!_bpt->Cond().isEmpty())
                {
                    // Evaluate condition
                    Parser p(_bpt->Cond().toLatin1().data());

                    pPC->pCPU->pDEBUG->injectReg(&p);
                    p.symbols_ ["Val"]=	_val;
                    if (p.Evaluate()<=0) return false;
                }

                breakMsg = _bpt->toText();
//                qWarning()<<breakMsg;
                return true;
            }
        }
        break;
    case Cbreakpoint::WRITE:
        for (int i=0; i<breakList.count();i++) {
            Cbreakpoint *_bpt = breakList[i];
            if ( _bpt->isEnabled() &&
                 _bpt->isType(Cbreakpoint::WRITE) &&
                 (_adr >= _bpt->From()) &&
                 (_adr <= _bpt->To()) ) {

                if (!_bpt->Cond().isEmpty())
                {
                    // Evaluate condition
                    pPC->pCPU->pDEBUG->injectReg(p);
                    p->symbols_ ["Val"]=	_val;
                    if (p->Evaluate(_bpt->Cond().toLatin1().data())<=0) return false;
                }

//                qWarning()<<"break write";
                breakMsg = _bpt->toText();
//                qWarning()<<breakMsg;
                return true;
            }
        }
        break;

    default: return false;
    }
    return false;
}

void CbreakpointManager::addBreakPoint(Cbreakpoint *_b)
{
    breakList.append(_b);
}

void CbreakpointManager::serialize(QXmlStreamWriter *xmlOut) {
    xmlOut->writeStartElement("breakpoints");
        for (int i=0; i < breakList.count(); i++) {
            breakList.at(i)->serialize(xmlOut);
        }
    xmlOut->writeEndElement();  // breakpoints
}

void CbreakpointManager::unserialize(QXmlStreamReader *xmlIn) {
    if (xmlIn->readNextStartElement() &&
            (xmlIn->name() == "breakpoints")) {
        while (xmlIn->readNextStartElement()) {
            Cbreakpoint *_bpt = Cbreakpoint::unserialize(xmlIn);
            if (_bpt) breakList.append(_bpt);
            xmlIn->skipCurrentElement();
        }
    }
    if (pPC->dialogdasm) pPC->dialogdasm->refreshBreakPoints();
}
