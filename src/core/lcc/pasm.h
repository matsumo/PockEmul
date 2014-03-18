#ifndef PASM_H
#define PASM_H

#include <QtGui>

class Cpasm:public QObject {
    Q_OBJECT
public:

    Cpasm(QMap<QString,QByteArray> *sources,QMap<QString,QByteArray> *out);

    int mathparse(QByteArray s, int w);
    static const QString opcode[256];
    static const unsigned char nbargu[];
    static QString replace_text(QString text, QString such, QString ers);

    void abort(QString t);

    bool findnlabel(QString l);
    void addnlabel(QString l);
    void delnlabel(int l);

    bool findlabel(QString label);
    void addlabel(QString l);

    bool findsymbol(QString l);
    void addsymbol(QString s1, QString s2);

    bool findop(QString l);
    void addcode(unsigned char b);
    void extractop(QString s);
    int calcadr(void);

    void doasm(void);

    QString readline(QStringListIterator *linesIter);

    void writeln(QString srcName,QString s);
    void write(QString srcName,QString s);

    void savefile(QString fname);
    void parsefile(QString fname,QString source);
    void run(QString fname, QString source);

    QMap<QString,QByteArray> *sources;
    QMap<QString,QByteArray> *out;
    QString model;

    int nlabp;
    int nlabcnt;
    QList<QString> nlab;
    QList<int> nlabpos;
    QList<QString> nlabasm;


    int labcnt;

    int labp;
    QList<QString> lab;
    QList<int> labpos;

    QList<QString> sym;
    QList<QString> symval;
    int symcnt;

    int codpos;
    QByteArray code;

    int cline;
    QString cf;
    bool mcase;
    QString op;
    QString params;
    int startadr;
    QString tok;
    int opp;
    QString param1,param2;

    static const QList<char> JRPLUS;
    static const QList<char> JRMINUS;
    static const QList<char> JR;

    int ccase,casecnt;
    int lcnt;

    QString result;

};

#endif // PASM_H
