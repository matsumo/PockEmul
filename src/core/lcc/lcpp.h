#ifndef LCPP_H
#define LCPP_H

#include <QtGui>
#include <QMap>

//#include "mainwindowpockemul.h"
class MainWindowPockemul;


#include "cstdlib.h"

class CDOxyItem {

public:
    QString brief;
    QString fn;
    QList<QString> params;
    QString returnTyp;
};

class Clcpp:public QObject {
    Q_OBJECT
public:

    Clcpp(QMap<QString,QByteArray> *sources,QMap<QString,QByteArray> *out,QString model,bool showErrors=true);

    void run();
    QString parsefile(QString srcName,QString source);
    QString getModel(void);

    void doDefine(QString tok);
    void doInclude(QString, QString);
    Cstdlib *pStdLibs;

    QString computeDefine(QString result);
    void initDOxygen(QString line);
    void closeDOxygen(QString line);
    void addDOxygen(QString line);
    QList<CDOxyItem*> getDoxygenList();

private:
    void abort(QString t);
    QString replace_text(QString text, QString such, QString ers);
    bool findsymbol(QString l);
    void addsymbol(QString s1, QString s2);
    QString extractparam(QString s,int p) ;
    QString readline(QStringListIterator *linesIter);

    void writeln(QString srcName,QString s);

    QMap<QString,QByteArray> *sources;
    QMap<QString,QByteArray> *out;
    QString model;



    int cline;
    int lcnt;
    QString inpf,outf;
    bool lcom;
    bool doxycom;

    QList<QString> sym;
    QList<QString> symval;
    int symcnt;
    QString currentTag;
    CDOxyItem *currentDoxyItem;
    QList<CDOxyItem*> doxygenlist;

    bool showErrors;

};

#endif // LCPP_H
