#ifndef CESIMU_H
#define CESIMU_H

#include <QMenu>
#include <QtScript/QScriptEngine>
#include <QTextBrowser>
#include <QHBoxLayout>


#include "pobject.h"


#define TICKS_BDS	(pTIMER->pPC->getfrequency()/baudrate)
class DialogSimulator;
class Cconnector;

class Ccesimu:public CPObject{
Q_OBJECT

public:
    const char*	GetClassName(){ return("Ccesimu");}


    Cconnector	*pCONNECTOR; qint64 pCONNECTOR_value;
    Cconnector  *pSavedCONNECTOR;

    virtual bool run(void);

    virtual bool init(void);				//initialize
    virtual bool exit(void);				//end

    DialogSimulator *dialogconsole;

    Q_INVOKABLE void ScriptLog(QString s);
    Q_INVOKABLE bool GoDown(int pin);
    Q_INVOKABLE bool GoUp(int pin);
    Q_INVOKABLE bool Change(int pin);
    Q_INVOKABLE void timerInit(int id);
    Q_INVOKABLE int timerMsElapsed(int id);
    Q_INVOKABLE int timerUsElapsed(int id);
    Q_INVOKABLE void setMarker(int markId);

    Ccesimu(CPObject *parent = 0);

    virtual ~Ccesimu();

public:
    void paintEvent(QPaintEvent *);
    QScriptEngine   *engine;
    QScriptValue    *script;
    QScriptValue    *mainfunction;

    QDialog         *helpDialog;
    QTextBrowser    *textbrowser;
    QHBoxLayout     *layout;

    quint64  states[20];

protected slots:
    void contextMenuEvent ( QContextMenuEvent * );
    void ShowDialog(void);
    void HideDialog(void);
    void HelpDialog(void);

private:
    quint64  run_oldstate;
    quint64  latency;

};


#endif // CESIMU_H
