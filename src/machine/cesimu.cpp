#include <QTextBrowser>

#include "common.h"
#include "Log.h"
#include "cesimu.h"
#include "dialogsimulator.h"
#include "dialoganalog.h"
#include "Connect.h"
#include "Inter.h"

#define DOWN	0
#define UP		1

Ccesimu::Ccesimu(CPObject *parent): CPObject(this)
{							//[constructor]
    Q_UNUSED(parent)

    //ToDestroy = false;

    setfrequency( 0);
    BackGroundFname	= P_RES(":/ext/simu.png");

    pTIMER		= new Ctimer(this);
    setDX(160);//Pc_DX	= 160;
    setDY(160);//Pc_DY	= 160;
    pCONNECTOR = 0;
    pSavedCONNECTOR = 0;
    engine = 0;
    script = 0;
    mainfunction = 0;
    helpDialog = 0;
    textbrowser = 0;
    layout = 0;
}

Ccesimu::~Ccesimu() {
    if (pCONNECTOR) delete pCONNECTOR;
    if (pSavedCONNECTOR) delete pSavedCONNECTOR;
    if (engine) delete engine;
    if (script)  delete script;
    if (mainfunction) delete mainfunction;

    if (helpDialog) delete helpDialog;
    if (textbrowser) delete textbrowser;
    if (layout) delete layout;
}

void Ccesimu::contextMenuEvent ( QContextMenuEvent * event )
{
    QMenu *menu= new QMenu(this);

    BuildContextMenu(menu);

    menu->addSeparator();

    menu->addAction(tr("Show console"),this,SLOT(ShowDialog()));
    menu->addAction(tr("Hide console"),this,SLOT(HideDialog()));
    menu->addAction(tr("Help"),this,SLOT(HelpDialog()));


    menu->popup(event->globalPos () );
    event->accept();
}

void Ccesimu::ShowDialog(void) {
    dialogconsole->show();
}
void Ccesimu::HideDialog(void) {
    dialogconsole->hide();
}
void Ccesimu::HelpDialog(void) {

    helpDialog = new QDialog(this);
    textbrowser = new QTextBrowser();
    layout = new QHBoxLayout;
    layout->addWidget(textbrowser);
    helpDialog->setLayout(layout);
    helpDialog->setWindowTitle(tr("Script Help"));
    textbrowser->setSource(QUrl("qrc:/pockemul/script_help.html"));
    helpDialog->show();
}

bool Ccesimu::init(void){
    pCONNECTOR = new Cconnector(this,11,0,Cconnector::Sharp_11,"Connector 11 pins",true,QPoint(130,7)); publish(pCONNECTOR);
    pSavedCONNECTOR = new Cconnector(this,11,0,Cconnector::Sharp_11,"Saved Connector 11 pins",true,QPoint(130,7));

    WatchPoint.add(&pCONNECTOR_value,64,15,this,"Connector 11 pins");
    dialogconsole = new DialogSimulator(this);
    dialogconsole->setWindowTitle("Simulator Script Editor");
    dialogconsole->show();
    engine = new QScriptEngine(this);
    QScriptValue objectValue = engine->newQObject(pCONNECTOR);
    engine->globalObject().setProperty("Connector", objectValue);

    objectValue = engine->newQObject(this);
    engine->globalObject().setProperty("Simulator", objectValue);
    mainfunction = 0;
    run_oldstate = 0;
    for (int i=0;i<20;i++) states[i]=0;

    return true;
}

bool Ccesimu::exit(void){
    return true;
}
bool Ccesimu::run(void){

    if (!pTIMER) return true;

// Try to introduce a latency
//    quint64			deltastate = 0;

    if (run_oldstate == 0) run_oldstate = pTIMER->state;
//    deltastate = pTIMER->state - run_oldstate;
//    if (deltastate < CESIMULATENCY ) return true;
//    run_oldstate	= pTIMER->state;
    if (pTIMER->usElapsed(run_oldstate)<500) return true;
    run_oldstate	= pTIMER->state;

    if (mainfunction) {
    QString s = mainfunction->call(QScriptValue()).toString();
    }

    pSavedCONNECTOR->Set_values(pCONNECTOR->Get_values());
    pCONNECTOR_value = pCONNECTOR->Get_values();

    return true;
}

void Ccesimu::paintEvent(QPaintEvent *event)
{
    CPObject::paintEvent(event);
    //dialogconsole->refresh();
}

void Ccesimu::ScriptLog(QString s) {
    AddLog(LOG_SIMULATOR,s)
}

bool Ccesimu::GoDown(int pin) {

    return (( pCONNECTOR->Get_pin(pin) == DOWN ) && (pSavedCONNECTOR->Get_pin(pin) == UP)) ? true:false;
}
bool Ccesimu::GoUp(int pin) {

    return (( pCONNECTOR->Get_pin(pin) == UP ) && (pSavedCONNECTOR->Get_pin(pin) == DOWN)) ? true:false;
}
bool Ccesimu::Change(int pin) {
    return (pCONNECTOR->Get_pin(pin) != pSavedCONNECTOR->Get_pin(pin) ) ? true:false;
}

void Ccesimu::timerInit(int id) {
    states[id] = pTIMER->state;
}

int Ccesimu::timerMsElapsed(int id) {
    return pTIMER->msElapsed(states[id]);
}
int Ccesimu::timerUsElapsed(int id) {
    return pTIMER->msElapsed(states[id]);
}
void Ccesimu::setMarker(int markId){
    if (mainwindow->dialoganalogic) mainwindow->dialoganalogic->setMarker(markId);
}
