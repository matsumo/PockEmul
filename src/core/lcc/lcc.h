#ifndef LCC_H
#define LCC_H

#include <QString>
#include <QMap>
#include <QtGui>

#include "mainwindowpockemul.h"
extern MainWindowPockemul *mainwindow;

#define BO 200
#define BA 201
#define EQ 202
#define NE 203
#define GR 204
#define SM 205
#define GE 206
#define SE 207
#define PP 210
#define MM 211

#define ptrREF 1
#define ptrADR 2
#define ptrREFARR 3

#define MUL8  0
#define DIVMOD8  1
#define SR8  3
#define SL8  4
#define XOR8  5

#define MUL16  12
#define DIVMOD16  13
#define AND16  15
#define OR16  16
#define XOR16  21
#define SR16  17
#define SL16  18
#define NOT16  19
#define NEG16  20

#define CPE16  30
#define CPNE16  31
#define CPS16  32
#define CPG16  33
#define CPSE16  34
#define CPGE16  35

class Cvar {
public:

    Cvar() {
        address=0;
        locproc=0;
        size=0;
        local=false;
        array=false;
        xram=false;
        pointer=false;
        at=false;
        initn=0;
    }

    QByteArray varname;
    QByteArray typ;
    QByteArray pnttyp;
    int address;
    int locproc;
    int size;
    bool local;
    bool array;
    bool xram;
    bool pointer;
    bool at;
    QByteArray inits;//QList<unsigned char> inits;
    int initn;

    QByteArray getLabel() {
        return ((address>=0) ? (QByteArray::number(address)) : varname);
    }

    bool operator==( const Cvar &v ) const {
            if (    (v.varname == varname) &&
                    (v.local == local) &&
                    (v.locproc == locproc))
                return true;
            else
                return false;
    }
};

class Cparam {
public:
    QByteArray paramName;
    QByteArray paramType;
    Cparam(QByteArray n,QByteArray t) {
        this->paramName = n;
        this->paramType = t;
    }
};

class Cproc {
public:
    Cproc() {

        this->ProcName = "";
        this->hasreturn = false;
        this->ReturnIsWord = false;
        this->ParCnt = 0;
        this->LocCnt = 0;
        this->called = false;
        this->start = -1;
        this->end = -1;
    }

    QByteArray ProcName;
    QByteArray ProcCode;
    bool hasreturn, ReturnIsWord;
    QByteArray Params;
    int ParCnt;
    QList<Cparam> paramList;
    int LocCnt;
    QList<QByteArray> LocName;
    QList<QByteArray> LocTyp;
    QList<QByteArray> partyp;
    QList<QByteArray> parname;
    bool called;
    int start,end;
};

class Clcc : public QObject{
    Q_OBJECT
public:

    enum OP_STD { OP_RTN,OP_JP,OP_INCLUDE};

    Clcc(QMap<QString,QByteArray> *sources,QMap<QString,QByteArray> *out,bool showErrors=true);



    void Error(QString s);
    void Expected(QString s);


    void writln(QString,QString);
    void writeln(QString ,QString);
    void write(QString,QString);
    void LoadVariableMain(QByteArray name);
    void LoadVariable(QByteArray name);
    void LoadVariableArray(Cvar v);
    bool StoreVariable(QByteArray);
    void StoreVariableArray(Cvar v);
    bool  IsVarAtAdr(int adr,int size);
    int AllocVar(bool xr,bool at,int  size, int adr);
    QByteArray ExtrList(QByteArray *list);
    QByteArray ExtrWord(QByteArray *word);
    QByteArray ExtrCust(QByteArray *word,char c);
    void printproclist(QString);
    void printvarlist(QString);
    void inc(int a) {a++;}
    void dec(int a) {a--;}
    void FirstScan(QByteArray filen);
    void SecondScan(void);
    char Read(void);
    void rd(char *,QByteArray *);
    void GetToken(int,QByteArray *);
    QByteArray GetName(void);
    QByteArray vardecl();
    bool FindVar(QByteArray t);
    bool FindVarCurrProc(QByteArray t);
    bool FindProc(QByteArray t);
    bool AddVar(QByteArray t,QByteArray typ, bool xr, bool pnt, bool loc,int proc);
    bool AddProc(QByteArray t, QByteArray c, QByteArray p, int pc, bool hr, bool wd,QList<QByteArray> partype,QList<QByteArray> parname);

    QMap<QString,QByteArray> *sources;
    QMap<QString,QByteArray> *out;

    int mathparse(QByteArray s,int size);
    int skiphex(QByteArray hs,int hi);
    int skipbin(QByteArray hs,int hi);
    int converthex(QByteArray s);
    int convertbin(QByteArray s);
    int Evaluate(QByteArray Formula,int wid,int *ErrPosf);
    void NextCh(QByteArray Formula);
    void Block(void);
    void repadr(void);
    void Assignment(void);

    void DoSwitch(void);
    void DoIf(void);
    void DoGoto(void);
    void DoLabel(void);
    void DoBreak(void);
    void DoReturn(void);
    void DoLoop(void);
    void DoWhile(void);
    void DoFor(void);
    void DoLoad(void);
    void DoSave(void);
    void DoDoWhile(void);
    virtual void DoSaveState(void);
    virtual void DoRestoreState(void);

    void ProcCall();

    QByteArray NewLabel(void);
    void PostLabel(QByteArray L);

    int find_text(QByteArray such, QByteArray text);

    void Expression(void);
    void BoolExpression(void);
    void Factor(void);
    bool IsDigit(char c);
    QByteArray GetNumber(void);
    void Term(void);
    void NotFactor(void);
    bool IsAddop(char c);
    void Add(void);
    void Subtract(void);
    void Multiply(void);
    void Divide(void);
    void _Or(void);
    void ShiftR(void);
    void ShiftL(void);
    void _Xor(void);
    void _And(void);
    void _Mod(void);

    void SignedTerm(void);
    void CompTerm(void);
    void NotCompTerm(void);
    void BoolOr(void);
    void BoolAnd(void);

    void removelocvars(QByteArray pn);

    void varxram(int value, int adr, int size, QByteArray nm);
//    void varxarr(QList<unsigned char> value,int adr, int size,QByteArray nm, QByteArray typ);
    void varxarr(QByteArray value,int adr, int size,QByteArray nm, QByteArray typ);
    void load_x(QByteArray s);
    void load_y(QByteArray s);
    void varcode(int value, int adr, int size,QByteArray nm);
//    void varcarr(QList<unsigned char> value, int adr, int size,QByteArray nm, QByteArray typ);
    void varcarr(QByteArray value, int adr, int size,QByteArray nm, QByteArray typ);
    void varreg(int value, int adr, int size, QByteArray nm);
//    void varrarr(QList<unsigned char> value,int adr, int size,QByteArray nm, QByteArray typ);
    void varrarr(QByteArray value,int adr, int size,QByteArray nm, QByteArray typ);

    void CompGreater(void);
    void CompSmaller(void);
    void CompSmOrEq(void);
    void CompGrOrEq(void);
    void CompEqual(void);
    void CompNotEqual(void);
    void Branch(QByteArray);
    void BranchFalse(QByteArray L);
    void Negate(void);
    void Push(void);
    void PopAdd(void);
    void PopSub(void);
    void PopMul(void);
    void PopDiv(void);
    void PopOr(void);
    void PopSr(void);
    void PopSl(void);
    void PopXor(void);
    void PopAnd(void);
    void PopMod(void);
    void NotIt(void);

    void addlib(int lib);

    void LoadConstant(QByteArray n);

     void addasm(QByteArray s);


    void run(void);

public:

    QMap<OP_STD,QString> stdOp;

    QList<Cvar> varlist;
    QList<Cproc> proclist;


    int currproc,proccount;

    int level;
    bool outfile;
    QByteArray org;
    QByteArray pc;
    bool nosave;
    bool isword;
    int VarFound;
    int VarPos;
    bool procd;
    int VarCount;
    int ProcFound;
    char Look;


    int Posf;
    char Ch;
    int currentQuote;
    int md;
    QByteArray Tok;
    QByteArray dummy;

    QByteArray asmtext;
    int pointer;
    bool firstp;
    int LCount;
    QByteArray InnerLoop;
    QByteArray ExitLabel;

    int libcnt;
    QByteArray libtext;

    QMap<int,QByteArray> libname;
    QList<int> libins;
    QList<QByteArray> asmlist;
    int asmcnt;

    int pushcnt;

    QString inpf,outf;

    QByteArray LState;


    void generateProcCode(QString f, int i);
    QMap<int,int> calledProc;
    QList<int> insertedProc;





    bool ExpressionType(QByteArray e);
    void save();
    void restore();
signals:
    void outputSignal(QString,QString);

protected:
    bool showErrors;
    bool sourceinASM;
};

class ClccPC1500: public Clcc {

    virtual void DoSaveState(void);
    virtual void DoRestoreState(void);

    void save();
    void restore();

public:
    bool StoreVariable(QByteArray name);
    void LoadVariable(QByteArray name);
    void LoadVariableArray(Cvar v);
    void LoadVariableMain(QByteArray s);
    void varxram(int value, int adr, int size, QByteArray nm);
};

#endif // LCC_H
