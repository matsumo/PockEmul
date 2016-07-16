#include <QtGui>

#if QT_VERSION >= 0x050000
#   include <QtWidgets>
#else
#   include <QtCore>
#   include <QtGui>
#endif
//FIXME: JPC not working

#include "parser/parser.h"
#include "pasm.h"
Cpasm::Cpasm(QMap<QString,QByteArray> *sources,QMap<QString,QByteArray> *out) {
    this->sources = sources;
    this->out = out;
    nlabp =-1;
    nlabcnt = 0;

    labp = 0;
    labcnt =0;
    codpos = 0;
    mcase = false;

    startadr = 0;
    symcnt = 0;
    opp = 0;
    ccase = 0;

    lcnt = 0;
    result="";
}

void Cpasm::writeln(QString srcName,QString s) {
    write(srcName,s+"\r");
}

void Cpasm::write(QString srcName,QString s) {
    QByteArray locs = out->value(srcName);
    out->insert(srcName,locs+s.toLatin1());
}

int Cpasm::mathparse(QByteArray s, int w) {

    int y=0;
    bool lf=false;

    QString sr = s;
    // replace label
    if (labcnt > 0)
        for (int i = 0; i < lab.size(); i++)
            sr = replace_text(sr, lab[i], QString("%1").arg(labpos[i] + startadr));

    /*
parcourir la formule pour trouver des labels inconnus
si oui, renvoyer 0 et fair un addnlabel
Fonctions connues:
    HB,LB,

*/
    QStringList mathOp;
    mathOp << "HB" << "LB";

    QStringList list = QString(sr).split(QRegExp("\\W+"), QString::SkipEmptyParts);
    QString cout="";
    for (int i = 0; i < list.size(); ++i) {
        QString locToken=list.at(i);
        cout += locToken + " : ";
        if ( (QString("_ABCDEFGHIJKLMNOPQRSTUVWXYZ").contains(locToken.at(0).toUpper())) &&
             (!mathOp.contains(locToken))){
            if (!findlabel(locToken)) {
                addnlabel(locToken);
                lf = true;
            }
        }
    }
    if (lf) {
        //QMessageBox::about(0,"LOG",tr("%1[%2]  - %3").arg(sr).arg(codpos).arg(cout));
    }
    else {
        Parser op(sr.toLatin1().data());
        //    int y = op.evevaluate_expression(sr.toLatin1().data());
        y = op.Evaluate();
    }

    return y;
}

#if 0
int Cpasm::mathparse2(QString s, int w) {
int i, p;
QString c, s2, s3;
bool lf;

        if (labcnt > 0)
            for (int i = 0; i< lab.size();i++)
                s = replace_text(s, lab[i], QString("%1").arg(labpos[i] + startadr));
        if (s.contains("\'")) {
                i = 0;
                while (i < s.length()) {
                    if (s[i] == "\'") {
//                        c = inttostr(ord(s[i+1]));
                        s.remove(i,3);
                        c.insert(i,s);
                        i = i - 3 + c.length();
                    }
                    i++;
                }
            }

        i = 0;
        lf = false;
        while (i < s.length()) {
            if ((s[i] == '0') && ((i < s.length()-2) && (s[i+1].toUpper() == 'X'))) {
                    i+=2;
                    while (QString("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ").contains(s[i].toUpper())) i++;
                }

                if upcase(s[i]) in ['_','A'..'Z'] then
                begin
                    if (i > 1) and (s[i - 1] = '0') then inc(i)
                    else begin
                        s2 := '';
                        while (i <= length(s)) and (upcase(s[i]) in ['0'..'9','_','A'..'Z']) do
                        begin
                                s2 := s2 + s[i];
                                inc(i);
                        end;
                        if (uppercase(s2) <> 'LB') and
                        (uppercase(s2) <> 'HB') and (uppercase(s2) <> 'NOT') and
                        (uppercase(s2) <> 'SIN') and (uppercase(s2) <> 'TAN') and
                        (uppercase(s2) <> 'COS') and (uppercase(s2) <> 'FAK') and
                        (uppercase(s2) <> 'ABS') and (uppercase(s2) <> 'SQRT') and
                        (uppercase(s2) <> 'SQR') and (uppercase(s2) <> 'LN') and
                        (uppercase(s2) <> 'LOG') and (uppercase(s2) <> 'EXP') and
                        (uppercase(s2) <> 'ARCTAN') and (uppercase(s2) <> 'PI') and
                        (uppercase(s2) <> 'E') then
                        begin
                            if not findlabel(s2) then
                            begin
                                dec(codpos);
                                addnlabel(s2);
                                inc(codpos);
                                lf := true;
                            end else
                            begin
                                s3 := inttostr(startadr + labpos[labp]);
                                s := replace_text(s, s2, s3);
                                i := i - length(s2) + length(s3);
                            end;
                        end;
                    end;
                end;
                inc(i);
        end;
        if lf then begin result := 0; exit; end;

    Evaluate(s, w, i, p);
    result := i;
        if p <> 0 then
        begin
                writeln('Erroneous formula: ' + s);
                abort('Formula error!');
        end;
    }

#endif

const QString Cpasm::opcode[256] = {
    "LII","LIJ","LIA","LIB","IX",
    "DX","IY","DY","MVW","EXW",
    "MVB","EXB","ADN","SBN","ADW",
    "SBW","LIDP","LIDL","LIP","LIQ",
    "ADB","SBB","?022?","?023?","MVWD",
    "EXWD","MVBD","EXBD","SRW","SLW",
    "FILM","FILD","LDP","LDQ","LDR",
    "RA","IXL","DXL","IYS","DYS",
    "JRNZP","JRNZM","JRNCP","JRNCM","JRP",
    "JRM","?046?","LOOP","STP","STQ",
    "STR","?051?","PUSH","DATA","?054?",
    "RTN","JRZP","JRZM","JRCP","JRCM",
    "?060?","?061?","?062?","?063?","INCI",
    "DECI","INCA","DECA","ADM","SBM",
    "ANMA","ORMA","INCK","DECK","INCM",
    "DECM","INA","NOPW","WAIT","WAITI",
    "INCP","DECP","STD","MVDM","READM",
    "MVMD","READ","LDD","SWP","LDM",
    "SL","POP","?092?","OUTA","?094?",
    "OUTF","ANIM","ORIM","TSIM","CPIM",
    "ANIA","ORIA","TSIA","CPIA","?104?",
    "DTJ","?106?","TEST","?108?","?109?",
    "?110?","?111?","ADIM","SBIM","?114?",
    "?115?","ADIA","SBIA","?118?","?119?",
    "CALL","JP","PTJ","?123?","JPNZ",
    "JPNC","JPZ","JPC","LP00","LP01",
    "LP02","LP03","LP04","LP05","LP06",
    "LP07","LP08","LP09","LP10","LP11",
    "LP12","LP13","LP14","LP15","LP16",
    "LP17","LP18","LP19","LP20","LP21",
    "LP22","LP23","LP24","LP25","LP26",
    "LP27","LP28","LP29","LP30","LP31",
    "LP32","LP33","LP34","LP35","LP36",
    "LP37","LP38","LP39","LP40","LP41",
    "LP42","LP43","LP44","LP45","LP46",
    "LP47","LP48","LP49","LP50","LP51",
    "LP52","LP53","LP54","LP55","LP56",
    "LP57","LP58","LP59","LP60","LP61",
    "LP62","LP63","INCJ","DECJ","INCB",
    "DECB","ADCM","SBCM","TSMA","CPMA",
    "INCL","DECL","INCN","DECN","INB",
    "?205?","NOPT","?207?","SC","RC",
    "SR","?211?","ANID","ORID","TSID",
    "?215?","LEAVE","?217?","EXAB","EXAM",
    "?220?","OUTB","?222?","OUTC","CAL00",
    "CAL01","CAL02","CAL03","CAL04","CAL05",
    "CAL06","CAL07","CAL08","CAL09","CAL10",
    "CAL11","CAL12","CAL13","CAL14","CAL15",
    "CAL16","CAL17","CAL18","CAL19","CAL20",
    "CAL21","CAL22","CAL23","CAL24","CAL25",
    "CAL26","CAL27","CAL28","CAL29","CAL30",
    "CAL31"};

const unsigned char Cpasm::nbargu [] = {
    2,2,2,2,1,
    1,1,1,1,1,
    1,1,1,1,1,
    1,3,2,2,2,
    1,1,1,1,1,
    1,1,1,1,1,
    1,1,1,1,1,
    1,1,1,1,1,
    2,2,2,2,2,
    2,1,2,1,1,
    1,1,1,1,1,
    1,2,2,2,2,
    1,1,1,1,1,
    1,1,1,1,1,
    1,1,1,1,1,
    1,1,1,2,1,
    1,1,1,1,1,
    1,1,1,1,1,
    1,1,1,1,1,
    1,2,2,2,2,
    2,2,2,2,1,
    1,1,2,1,1,
    1,1,2,2,1,
    1,2,2,1,1,
    3,3,1,1,3,
    3,3,3,1,1,
    1,1,1,1,1,
    1,1,1,1,1,
    1,1,1,1,1,
    1,1,1,1,1,
    1,1,1,1,1,
    1,1,1,1,1,
    1,1,1,1,1,
    1,1,1,1,1,
    1,1,1,1,1,
    1,1,1,1,1,
    1,1,1,1,1,
    1,1,1,1,1,
    1,1,1,1,1,
    1,1,1,1,1,
    1,1,1,1,1,
    1,1,1,1,1,
    1,1,2,2,2,
    1,1,1,1,1,
    1,1,1,1,2,
    2,2,2,2,2,
    2,2,2,2,2,
    2,2,2,2,2,
    2,2,2,2,2,
    2,2,2,2,2,
    2,2,2,2,2,
    2};


QString Cpasm::replace_text(QString text, QString such, QString ers) {

    if (text.contains(such)) {
        text = " "+text+" ";
        QString regex = "([^_0-9A-Za-z])("+such+")([^_0-9A-Za-z])";
        return text.replace(QRegExp(regex),"\\1"+ers+"\\3").trimmed();
    }
    return text;
}

void Cpasm::abort(QString t) {
    writeln("ERROR",tr("Line %1: ").arg(cline+1) + t + " in file " + cf);
    QMessageBox::about(0,"ERROR",tr("Line %1: \n").arg(cline+1)+t);
 // halt;
}

bool Cpasm::findnlabel(QString l) {
    nlabp = nlab.indexOf(l);
    return (nlabp >=0);
}

void Cpasm::addnlabel(QString label) {
    label = label;
    nlab.append(label);

    nlabpos.append(codpos);
    nlabasm.append((mcase?"#":"")+op + " " + params);

    nlabcnt++;
}

void Cpasm::delnlabel(int l) {
    nlab.removeAt(l);
    nlabpos.removeAt(l);
    nlabasm.removeAt(l);
    nlabcnt--;
}

bool Cpasm::findlabel(QString label) {
    labp = lab.indexOf(label);
    return (labp >=0);
}

void Cpasm::addlabel(QString l) {
    int  tpos;
    bool bup;

    //l = l.toUpper();

    if (findlabel(l)) abort("Label " + l + " already defined!");
    writeln("output","SYMBOL: " + l + tr(" - %1").arg(codpos + startadr));
    lab.append(l);
    labpos.append(codpos);
    labcnt++;
    while (findnlabel(l)) {
        tpos = codpos;
        bup = mcase;
        codpos = nlabpos[nlabp];
        tok = nlabasm[nlabp];
        if (tok[0] == '#') { mcase = true; tok.remove(0,1); }
        extractop(tok);
        doasm();
        codpos = tpos;
        delnlabel(nlabp);
        mcase = bup;
    }
}

bool Cpasm::findsymbol(QString l) {
 return sym.contains(l);
}

void Cpasm::addsymbol(QString s1, QString s2) {
    if (findsymbol(s1)) abort("Symbol " + s1 + " already defined!");
    sym.append(s1);//.toUpper());
    symval.append(s2);
    writeln("output","SYMBOL: " + s1 + " - " + s2);
    symcnt++;
}

bool Cpasm::findop(QString l) {
    for (int i =0; i< 256;i++) {
        if (opcode[i] == l) {
            opp = i;
            return true;
        }
    }
    opp = -1;
    return false;
}

void Cpasm::addcode(unsigned char b) {
    if (code.size() == codpos) code.append(b);
    else code[codpos] = b;
    codpos++;
    if ((codpos + startadr) >= 65536) abort("Code exceeds maximum memory!");
}

void Cpasm::extractop(QString s) {
    int i;

    if (s.isEmpty()) return;
    //s.append(" ");

    i = 1;
    //    while s[i] in [' ', #9] do inc(i);
    //    delete(s, 1, i - 1);
    s = s.trimmed().append(" ");

    i = 0;
    while (! QString(" \t").contains(s[i])) i++;
    op = s.left(i).trimmed().toUpper();
    s.remove(0,i); s = s.trimmed();
    params = s;

    if (s.contains(",")) s[s.indexOf(",")] = ' ' ;

    i = 1; s.append(" ");
    while ((i < s.size()) && !QString(" \t").contains(s[i])) i++;
    param1 = s.left(i).trimmed();
    s.remove(0,i); s = s.trimmed();

    if (!s.isEmpty()) param2 = s; else param2 = "";
}

int Cpasm::calcadr(void) {
    int i,result;
    QString s, s2;
    bool lf;

    result = 0;
    i = 0;
    lf = false;
    while (i < params.length()) {
        if ((params[i] == '0') &&
            ((i < (params.length()-1)) && (params[i+1].toUpper() == 'X'))) {
            i+=2;
            while (QString("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ").contains(params[i])) i++;
        }
        if ((params[i] == '0') &&
            ((i < (params.length()-1)) && (params[i+1].toUpper() == 'B'))) {
            i+=2;
            while (QString("01").contains(params[i])) i++;
        }
        if (QString("_ABCDEFGHIJKLMNOPQRSTUVWXYZ").contains(params[i].toUpper())) {
            s = "";
            while ((i < params.length()) &&
                   QString("_0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ").contains(params[i].toUpper())) {
                s.append(params[i]);
                i++;
            }
            if (!findlabel(s)) {
                addnlabel(s);
                lf = true;
            }else {
                s2 = QString("%1").arg(startadr + labpos[labp]);
                params = replace_text(params, s, s2);
                param1 = params;
                param2 = "";
                i = i - s.length() + s2.length();
            }
        }
        i++;
    }
    if (lf) result = 0;
    else if (param2.isEmpty()) result = mathparse(param1.toLatin1(), 16);
    else result = mathparse(param1.toLatin1(), 8) * 256 + mathparse(param2.toLatin1(), 8);

    return result;
}


#define NOP 77
#define JRNZP 40
#define JRNZM 41
#define JRNCP 42
#define JRNCM 43
#define JRP 44
#define JRM 45
#define LOOP 47
#define JRZP 56
#define JRZM 57
#define JRCP 58
#define JRCM 59
const QList<char> Cpasm::JRPLUS = QList<char> ()<<JRNZP<<JRNCP<<JRP<<JRZP<<JRCP;
const QList<char> Cpasm::JRMINUS = QList<char> ()<<JRNZM<<JRNCM<<JRM<<LOOP<<JRZM<<JRCM;
const QList<char> Cpasm::JR = QList<char> ()<<JRNZP<<JRNCP<<JRP<<JRZP<<JRCP<<JRNZM<<JRNCM<<JRM<<LOOP<<JRZM<<JRCM;


void Cpasm::doasm(void) {
    int adr;

    param1 = param1.trimmed();
    if (!param1.isEmpty())
        if (param1.right(1) == ",")
            param1.chop(1);
    param2 = param2.trimmed();

    QList<char> l1 = QList<char> () << 120<<121<<16<<124<<125<<126<<127;
    if (findop(op)) {
        if (l1.contains(opp)) {
            adr = calcadr();
            if (adr > 0) {
                addcode(opp);
                addcode(adr / 256);
                addcode(adr % 256);
            }
            else {
                addcode(NOP);
                addcode(NOP);
                addcode(NOP);
            }
        }
        else
        if (JR.contains(opp)) {              // relativ
            adr = calcadr();
            if (adr >= 8192) {
                addcode(opp);
                //FIXME:
                // if opp in JRPLUS then addcode(adr - codpos - startadr)
                //                   else
                addcode(abs(codpos + startadr - adr));
            }
            else
            if (adr > 0) {
                addcode(opp); addcode(adr);
            }
            else {
                addcode(NOP); addcode(NOP);
            }
        }
        else {
#if 1

            if (nbargu[opp] == 2)
            {
                int loc1 = mathparse(param1.toLatin1(), 8);
                addcode(opp);
                addcode(loc1);
            }
            else if (nbargu[opp] == 3) {
                if (param2.isEmpty()) {
                    int loc1 = (mathparse(param1.toLatin1(), 16) << 8) & 0xFF;
                    int loc2 = (mathparse(param1.toLatin1(), 16)) & 0xFF;
                    addcode(opp);
                    addcode(loc1);
                    addcode(loc2);
                }
                else {
                    int loc1 = (mathparse(param1.toLatin1(), 8));
                    int loc2 = (mathparse(param2.toLatin1(), 8));
                    addcode(opp);
                    addcode(loc1);
                    addcode(loc2);
                }
            }
            else
                addcode(opp);
#else
            addcode(opp);
            if (nbargu[opp] == 2) addcode(mathparse(param1.toLatin1(), 8));
            else if (nbargu[opp] == 3) {
                if (param2.isEmpty()) {
                    addcode((mathparse(param1.toLatin1(), 16) << 8) & 0xFF);
                    addcode((mathparse(param1.toLatin1(), 16)) & 0xFF);
                }
                else {
                    addcode(mathparse(param1.toLatin1(), 8));
                    addcode(mathparse(param2.toLatin1(), 8));
                }
            }
#endif
        }
    }
    else {

        if (op == "LP") {
            if (mathparse(param1.toLatin1(), 8) > 63) abort("LP comm&& exceeds range!");
            addcode(128 + mathparse(param1.toLatin1(), 8));
        }
        else if (op == "RTN") addcode(55);
        else if (op == "SUBW") addcode(0x15);
        else if (op == "SUBC") addcode(0xC5);
        else if (op == "ADDW") addcode(0x14);
        else if (op == "ADDC") addcode(0xC4);
        else if (op == "ADD") {
            if ((param1 == "[P]") && (param2 == "A")) addcode(0x44);
            else if (param1 == "[P]") { addcode(0x70); addcode(mathparse(param2.toLatin1(), 8)); }
            else if (param1 == "A") { addcode(0x74); addcode(mathparse(param2.toLatin1(), 8)); }
        }
        else if (op == "ADDB") {
            if ((param1 == "[P]") && (param2 == "A")) addcode(0x0C);
            else if ((param1 == "[P]") && (param2 == "[Q]")) addcode(0x0E);
        }
        else if (op == "SUB") {
            if ((param1 == "[P]") && (param2 == "A")) addcode(0x45);
            else if (param1 == "[P]") { addcode(0x71); addcode(mathparse(param2.toLatin1(), 8)); }
            else if (param1 == "A") { addcode(0x74); addcode(mathparse(param2.toLatin1(), 8)); }
        }
        else if (op == "SUBB") {
            if ((param1 == "[P]") && (param2 == "A")) addcode(0x0D);
            else if ((param1 == "[P]") && (param2 == "[Q]")) addcode(0x0F);
        }
        else if (op == "ROL") addcode(0x5A);
        else if (op == "SLB") addcode(0x1D);
        else if (op == "ROR") addcode(210);
        else if (op== "SRB") addcode(28);
        else if (op == "SWAP") addcode(88);
        else if (op == "RC") addcode(209);
        else if (op == "SC") addcode(208);
        else if (op == "OR") {
            if ((param1 == "[P]") && (param2 == "A")) addcode(0x47);
            else if (param1 == "[P]") { addcode(0x61); addcode(mathparse(param2.toLatin1(), 8)); }
            else if (param1 == "[DP]") { addcode(0xD5); addcode(mathparse(param2.toLatin1(), 8)); }
            else if (param1 == "A") { addcode(0x65); addcode(mathparse(param2.toLatin1(), 8)); }
        }
        else if (op == "&&") {
            if ((param1 == "[P]") && (param2 == "A")) addcode(0x46);
            else if (param1 == "[P]") { addcode(0x60); addcode(mathparse(param2.toLatin1(), 8)); }
            else if (param1 == "[DP]") { addcode(0xD4); addcode(mathparse(param2.toLatin1(), 8)); }
            else if (param1 == "A") { addcode(0x64); addcode(mathparse(param2.toLatin1(), 8)); }
        }
        else if (op == "OUT") {
            if (param1 == "A") addcode(93);
            else if (param1 == "B") addcode(221);
            else if (param1 == "C") addcode(223);
            else if (param1 == "F") addcode(95);
        }
        else if (op == "IN"){
            if (param1 == "A")  addcode(76);
            else if (param1 == "B") addcode(204);
        }
        else if (op == "INC") {
            if ((param1 == "A") || (mathparse(param1.toLatin1(), 8) == 2)) addcode(66);
            else if ((param1 == "B") || (mathparse(param1.toLatin1(), 8) == 3)) addcode(194);
            else if ((param1 == "J") || (mathparse(param1.toLatin1(), 8) == 1)) addcode(192);
            else if ((param1 == "K") || (mathparse(param1.toLatin1(), 8) == 8)) addcode(72);
            else if ((param1 == "L") || (mathparse(param1.toLatin1(), 8) == 9)) addcode(200);
            else if ((param1 == "M") || (mathparse(param1.toLatin1(), 8) == 10)) addcode(74);
            else if ((param1 == "N") || (mathparse(param1.toLatin1(), 8) == 11)) addcode(202);
            else if (param1 == "P") addcode(80);
            else if ((param1 == "X") || (mathparse(param1.toLatin1(), 8) == 4)) addcode(4);
            else if ((param1 == "Y") || (mathparse(param1.toLatin1(), 8) == 6)) addcode(6);
            else if ((param1 == "I") || (mathparse(param1.toLatin1(), 8) == 0)) addcode(64);
        }
        else if (op == "DEC") {
            if ((param1 == "A") || (mathparse(param1.toLatin1(), 8) == 2)) addcode(67);
            else if ((param1 == "B") || (mathparse(param1.toLatin1(), 8) == 3)) addcode(195);
            else if ((param1 == "J") || (mathparse(param1.toLatin1(), 8) == 1)) addcode(193);
            else if ((param1 == "K") || (mathparse(param1.toLatin1(), 8) == 8)) addcode(73);
            else if ((param1 == "L") || (mathparse(param1.toLatin1(), 8) == 9)) addcode(201);
            else if ((param1 == "M") || (mathparse(param1.toLatin1(), 8) == 10)) addcode(75);
            else if ((param1 == "N") || (mathparse(param1.toLatin1(), 8) == 11)) addcode(203);
            else if (param1 == "P") addcode(81);
            else if ((param1 == "X") || (mathparse(param1.toLatin1(), 8) == 4)) addcode(5);
            else if ((param1 == "Y") || (mathparse(param1.toLatin1(), 8) == 6)) addcode(7);
            else if ((param1 == "I") || (mathparse(param1.toLatin1(), 8) == 0)) addcode(65);
        }
        else if (op == "CALL") {
            adr = calcadr();
            if (adr > 0) {
                if (adr < 8192) { addcode(0xE0 + adr / 256); addcode(adr % 256); }
                else { addcode(0x78); addcode(adr / 256); addcode(adr % 256); }
            }
            else {
                addcode(NOP); addcode(NOP); addcode(NOP);
            }
        }
        else if (op == "JMP") {
            adr = calcadr();
            if (adr > 0) {
                addcode(121); addcode(adr / 256); addcode(adr % 256);
            }
            else {
                addcode(NOP); addcode(NOP); addcode(NOP);
            }
        }
        else if (op == "JPLO") {
            adr = calcadr();
            if (adr > 0) {
                addcode(127); addcode(adr / 256); addcode(adr % 256);
            }
            else {
                addcode(NOP); addcode(NOP); addcode(NOP);
            }
        }
        else if (op == "JPSH") {
            adr = calcadr();
            if (adr > 0) {
                addcode(125); addcode(adr / 256); addcode(adr % 256);
            }
            else {
                addcode(NOP); addcode(NOP); addcode(NOP);
            }
        }
        else if (op == "JPNE") {
            adr = calcadr();
            if (adr > 0) {
                addcode(124); addcode(adr / 256); addcode(adr % 256);
            }
            else {
                addcode(NOP); addcode(NOP); addcode(NOP);
            }
        }
        else if (op == "JPEQ") {
            adr = calcadr();
            if (adr > 0) {
                addcode(126); addcode(adr / 256); addcode(adr % 256);
            }
            else {
                addcode(NOP); addcode(NOP); addcode(NOP);
            }
        }
        else if (op == "RJMP") {
            adr = calcadr();
            if (adr >= 8192) {
                if (abs(adr - startadr - codpos) <= 255) {
                    if (adr > startadr + codpos) addcode(44);
                    else addcode(45);
                    addcode(abs(adr - startadr - codpos));
                }
                else {
                    addcode(121); addcode(adr / 256); addcode(adr % 256);   // Do absolute jump then
                }
            } else
                if ((adr >= 1) && (adr <= 255)) {
                addcode(44); addcode(adr);
            }
            else
                if ((adr <= -1) && (adr >= -255)) {
                addcode(45); addcode(adr);
            } else {
                addcode(NOP); addcode(NOP);
            }
        }
        else if (op == "BRLO") {
            adr = calcadr();
            if (adr >= 8192) {
                if (abs(adr - startadr - codpos) <= 255) {
                    if (adr > startadr + codpos) addcode(58);
                    else addcode(59);
                    addcode(abs(adr - startadr - codpos));
                } else abort("Relative jump exceeds 255 bytes!");
            } else
                if ((adr >= 1) && (adr <= 255)) {
                addcode(58); addcode(adr);
            } else
                if ((adr <= -1) && (adr >= -255)) {
                addcode(59); addcode(adr);
            } else
                if ((abs(adr) >= 256) && (abs(adr) < 8192)) {
                adr = startadr + codpos + adr;
                addcode(127); addcode(adr / 256); addcode(adr % 256);
            } else
            {
                addcode(NOP); addcode(NOP);
            }
        }
        else if (op == "BRSH") {
            adr = calcadr();
            if (adr >= 8192) {
                if (abs(adr - startadr - codpos) <= 255) {
                    if (adr > startadr + codpos) addcode(42);
                    else addcode(43);
                    addcode(abs(adr - startadr - codpos));
                } else abort("Relative jump exceeds 255 bytes!");
            } else
                if ((adr >= 1) && (adr <= 255)) {
                addcode(42); addcode(adr);
            } else
                if ((adr <= -1) && (adr >= -255)) {
                addcode(43); addcode(adr);
            } else
                if ((abs(adr) >= 256) && (abs(adr) < 8192)){
                adr = startadr + codpos + adr;
                addcode(125); addcode(adr / 256); addcode(adr % 256);
            } else
            {
                addcode(NOP); addcode(NOP);
            };
        }
        else if (op == "BRNE") {
            adr = calcadr();
            if (adr >= 8192) {
                if (abs(adr - startadr - codpos) <= 255) {
                    if (adr > startadr + codpos) addcode(40);
                    else addcode(40);
                    addcode(abs(adr - startadr - codpos));
                } else abort("Relative jump exceeds 255 bytes!");
            } else
                if ((adr >= 1) && (adr <= 255)) {
                addcode(40); addcode(adr);
            } else
                if ((adr <= -1) && (adr >= -255)) {
                addcode(41); addcode(adr);
            } else
                if ((abs(adr) >= 256) && (abs(adr) < 8192)) {
                adr = startadr + codpos + adr;
                addcode(124); addcode(adr / 256); addcode(adr % 256);
            } else
            {
                addcode(NOP); addcode(NOP);
            }
        }
        else if (op == "BREQ") {
            adr = calcadr();
            if (adr >= 8192) {
                if (abs(adr - startadr - codpos) <= 255) {
                    if (adr > startadr + codpos) addcode(56);
                    else addcode(57);
                    addcode(abs(adr - startadr - codpos));
                } else abort("Relative jump exceeds 255 bytes!");
            } else
                if ((adr >= 1) && (adr <= 255)) {
                addcode(56); addcode(adr);
            } else
                if ((adr <= -1) && (adr >= -255)) {
                addcode(57); addcode(adr);
            } else
                if ((abs(adr) >= 256) && (abs(adr) < 8192)) {
                adr = startadr + codpos + adr;
                addcode(126); addcode(adr / 256); addcode(adr % 256);
            } else
            {
                addcode(NOP); addcode(NOP);
            }
        }
        else if (op == "CASE") {
            addcode(122);
            mcase = true;
            ccase = codpos;
            casecnt = 0;
            addcode(NOP); addcode(NOP); addcode(NOP);
            addcode(105);
        }
        else if (mcase) {
            if (op == "ENDCASE") {
                mcase = false;
                adr = codpos;
                codpos = ccase;
                addcode(casecnt);
                addcode((adr+startadr) / 256); addcode((adr+startadr) % 256);
                codpos = adr;
            } else
            {
                if (op != "ELSE") { addcode(mathparse(op.toLatin1(), 8)); casecnt++; }
                codpos--;
                adr = calcadr();
                codpos++;
                if (adr > 0) { addcode(adr / 256); addcode(adr % 256); }
                else
                { addcode(NOP); addcode(NOP); }
            }
        }
        else if (op == "MOV") {
            if ((param1 == "A") && (param2 == "[+X]")) addcode(36);
            else if ((param1 == "A") && (param2 == "[-X]")) addcode(37);
            else if ((param1 == "[+Y]") && (param2 == "A")) addcode(38);
            else if ((param1 == "[-Y]") && (param2 == "A")) addcode(39);
            else if ((param1 == "A") && (param2 == "R")) addcode(34);
            else if ((param1 == "R") && (param2 == "A")) addcode(50);
            else if ((param1 == "A") && (param2 == "Q")) addcode(33);
            else if ((param1 == "Q") && (param2 == "A")) addcode(49);
            else if ((param1 == "A") && (param2 == "P")) addcode(32);
            else if ((param1 == "P") && (param2 == "A")) addcode(48);
            else if ((param1 == "[P]") && (param2 == "[DP]")) addcode(85);
            else if ((param1 == "[DP]") && (param2 == "[P]")) addcode(83);
            else if ((param1 == "A") && (param2 == "[DP]")) addcode(87);
            else if ((param1 == "[DP]") && (param2 == "A")) addcode(82);
            else if ((param1 == "A") && (param2 == "[P]")) addcode(89);
            else if ((param1 == "[P]") && (param2 == "A")) { addcode(219); addcode(89); }
            else if (param1 == "A") { addcode(2); addcode(mathparse(param2.toLatin1(), 8)); }
            else if (param1 == "B") { addcode(3); addcode(mathparse(param2.toLatin1(), 8)); }
            else if (param1 == "I") { addcode(0); addcode(mathparse(param2.toLatin1(), 8)); }
            else if (param1 == "J") { addcode(1); addcode(mathparse(param2.toLatin1(), 8)); }
            else if (param1 == "P") { addcode(18); addcode(mathparse(param2.toLatin1(), 8)); }
            else if (param1 == "Q") { addcode(19); addcode(mathparse(param2.toLatin1(), 8)); }
            else if (param1 == "DPL") { addcode(17); addcode(mathparse(param2.toLatin1(), 8)); }
            else if (param1 == "DP") { addcode(16); adr = mathparse(param2.toLatin1(), 16); addcode(adr / 256); addcode(adr % 256); }
        }
        else if (op == "NOP") {
            if (param1 == "")  addcode(NOP);
            else { addcode(78); addcode(mathparse(param1.toLatin1(), 8)); }
        }
        else
            abort("Unknown OP-code: "+op);

    }
}

QString Cpasm::readline(QStringListIterator *linesIter) {

    bool c;
   // QString result="";

    do {
        do {
            if (linesIter->hasNext())
                result = linesIter->next();
            lcnt++;
            cline = lcnt;
            result = result.trimmed();
        }
        while (linesIter->hasNext() && result.isEmpty());
        //until eof(datei) or (result <> '');
        c = false;
        for (int i = 0 ;i<result.length();i++) {
            if ((result[i] == '\'') || (result[i] == '"')) c = ! c;
            if ((! c) && ((result[i] == ';')||(result[i]=='#'))) {
                result.remove(i, result.length());
                break;
            }
        }
        if (!result.isEmpty()) {
            int i = result.indexOf(':');
            if (i >= 0) {
                addlabel(result.mid(0, i).trimmed());
                result.remove(0, i+1);
                result = result.trimmed();
            }
        }
        if (!result.isEmpty()) {
            if (result.toLower().indexOf("ifdef") <0) {
                if (symcnt > 0) for (int i = 0;i< symcnt;i++) result = replace_text(result, sym[i], symval[i]);
                if (labcnt > 0) for (int i = 0;i< labcnt;i++) result = replace_text(result, lab[i], QString("%1").arg(labpos[i] + startadr));
            }
        }
    }
    //until eof(datei) or (result <> '');
    while (linesIter->hasNext() && result.isEmpty());
    result = result.trimmed();

    return result;
}

void Cpasm::savefile(QString fname) {
    QString s;
    unsigned char b;



    if (fname == "DEC") {
        for (int wr = 0; wr < code.size(); wr++) {
            b = code[wr];
            writeln("DEC", QString("%1").arg(b));
        }
    }
    else if ((fname == "BAS") || (fname == "HEX")) {
        int blcnt = 10;
        int locadr = startadr;
        s = "";
        int i = 0;
        QChar fill = '0';
        for (int wr = 0; wr < code.size(); wr++) {
            b = code[wr];
            s.append((fname=="BAS") ?   QString(",&%1").arg(b,2,16,fill).toUpper() :
                                        QString(" %1").arg(b,2,16,fill).toUpper());
            if (s.length() >= ((fname=="BAS") ? 64:48)) {
                writeln(fname, (fname=="BAS")?  QString("%1 POKE &%2").arg(blcnt).arg((int)locadr,4,16,fill).toUpper() + s :
                                                QString("%2").arg((int)locadr,4,16,fill).toUpper() +":"+ s);

                s = "";
                blcnt += 10;
                locadr += i + 1;
                i = -1;
            }
            i++;
        }
        if (!s.isEmpty()) {
            writeln(fname, (fname=="BAS")?  QString("%1 POKE &%2").arg(blcnt).arg((int)locadr,4,16,fill).toUpper() + s :
                                            QString("%2").arg((int)locadr,4,16,fill).toUpper() +":"+ s);
        }
    }
    else
    {
        writeln("_ORG",QString("%1").arg(startadr));
        out->insert("BIN", code);
    }
}

void Cpasm::run(QString fname,QString source) {
    parsefile(fname,source);
    if (nlabcnt > 0) {
        QString s="";
        for (int i=0;i< nlab.size();i++) {
            s +=QString("In line '") + nlabasm.at(i) + "': " + nlab.at(i)+"\n";

        }
        abort("Labels were not available!"+s);
    }
}

void Cpasm::parsefile(QString fname,QString source) {

    int lcnt;
    QString s;

    lcnt=0;
    QStringList lines = source.split("\n");
    QStringListIterator linesIter(lines);

    qWarning("start parse: %s",fname.toLatin1().data());
    tok = readline(&linesIter);
    while ((linesIter.hasNext()) || !tok.isEmpty()) {

        if (tok.indexOf(".endif") <0) {

            extractop(tok);
            if (op == ".ORG") {
                startadr = mathparse(param1.toLatin1(), 16);
            }
            else if (op == ".EQU") {
                addsymbol(param1, param2);
            }
            else if (op == ".DB") {
                while (params.indexOf(",") >= 0) {
                    s = params.mid(0,params.indexOf(",") );
                    //if s[1] = '''' then addcode(ord(s[2])) else
                    addcode(mathparse(s.toLatin1(), 8));
                    params.remove(0,params.indexOf(",")+1);
                }
                params = params.trimmed();
                //if params[1] = '''' then addcode(ord(params[2])) else
                addcode(mathparse(params.toLatin1(), 8));
            }
            else if (op == ".DW") {
                while (params.indexOf(",") >= 0) {
                    s = params.mid(0,params.indexOf(","));
                    addcode((mathparse(s.toLatin1(), 16) << 8) && 0xFF);
                    addcode(mathparse(s.toLatin1(), 16) && 0xFF);
                    params.remove(0,params.indexOf(",")+1);
                }
                addcode((mathparse(params.toLatin1(), 16) << 8) && 0xFF);
                addcode(mathparse(params.toLatin1(), 16) && 0xFF);
            }
            else if (op == ".DS") {
                params.remove(0,params.indexOf('"'));
                while (!params.isEmpty() && (params[1] != '\"')) {
                    if (params[1] == '\\') {
//                        if (params[2] == "\\") addcode(ord("\\"));
//                        else {
//                            addcode(converthex(uppercase(copy(params, 3, 2))));
//                            params.remove(0, 4);
//                        }
                    } else
                    {
                        addcode(params[1].toLatin1());
                        params.remove(0,1);
                    }
                }
            }
            else if (op == ".IFDEF") {
                if (!findsymbol(param1)) {
                    while (linesIter.hasNext() && (op != ".ENDIF")) {
                        tok = readline(&linesIter);
                        //extractop(tok);
                        if (tok.toLower().indexOf(".endif") >= 0) op = ".ENDIF";
                    }
                }
            }
            else if (op == ".INCLUDE") {
            // INLUDE LIB
                QFile f(":/asmlibs/sc61860/"+params);
                f.open(QFile::ReadOnly);
                if (f.exists()) {
                    parsefile(params,f.readAll());
                } else abort("Include file " + params + " not found!");
            }
            else
                doasm();
        }
        tok = readline(&linesIter);
    }
    qWarning("fin parse : %s",fname.toLatin1().data());
}

#if 0
program pasm;
{$APPTYPE CONSOLE}
{%File 'ModelSupport\default.txvpck'}

uses
  sysutils,
  calcunit;

var
      param1, param2: string;




    blcnt: integer;


        ccase: integer = 0;
        casecnt: integer;
         i: integer;

//	f: textfile;
//	f2: file of byte;
//        wr: integer;
        s, cf: string;

const
        NOP = 77;
        JRNZP = 40;
        JRNZM = 41;
        JRNCP = 42;
        JRNCM = 43;
        JRP = 44;
        JRM = 45;
        LOOP = 47;
        JRZP = 56;
        JRZM = 57;
        JRCP = 58;
        JRCM = 59;
        JRPLUS = [JRNZP,JRNCP,JRP,JRZP,JRCP];
        JRMINUS = [JRNZM,JRNCM,JRM,LOOP,JRZM,JRCM];
        JR = [JRNZP,JRNCP,JRP,JRZP,JRCP,JRNZM,JRNCM,JRM,LOOP,JRZM,JRCM];


begin
        writeln('pasm v1.1 - Assembler for Pocket Computers with SC61860 CPU');
        writeln('(c) Simon Lehmayr 2004');
    if paramcount < 2 then
        begin
          writeln('Usage: pasm asmfile outputfile [mode]');
          writeln('       mode: bin = binary file output (default)');
          writeln('             dec = decimal file output');
          writeln('             bas = basic file output');
          exit;
        end;

        parsefile(paramstr(1));
        if nlabcnt > 0 then
        begin
          for i := 0 to nlabcnt - 1 do
            writeln('In line "' + nlabasm[i] + '": ' + nlab[i]);
          abort('Labels were not available!');
        end;

        writeln;
        writeln('Start address:'#9,startadr);
        writeln('End address:'#9,startadr+codpos-1);

        if codpos = 0 then
        begin
                addcode(55);
                writeln('An empty program was produced!');
                exit;
        end;

        savefile(paramstr(2));
end.


#endif
