#include <QPainter>


#include "common.h"
#include "Log.h"
#include "pcxxxx.h"
#include "dialogpotar.h"

#include "potar.h"

Cpotar::Cpotar(CPObject *parent )	: CPObject(this)
{							//[constructor]
    Q_UNUSED(parent)

    setfrequency( 0);
    BackGroundFname	= P_RES(":/ext/jack.png");

    setDX(75);//Pc_DX	= 75;
    setDY(20);//Pc_DY	= 20;
    value = 0;
}

Cpotar::~Cpotar(){
    delete(pADCONNECTOR);
}

BYTE Cpotar::get_value(void) {
    return this->value;
}

void Cpotar::set_value(BYTE value) {
    this->value = value;
}

bool Cpotar::run(void)
{
    pADCONNECTOR->Set_values(this->value);

    return true;
}




/*****************************************************************************/
/* Initialize Potar															 */
/*****************************************************************************/
bool Cpotar::init(void)
{
    AddLog(LOG_MASTER,"Potar initializing...");

    CPObject::init();

    pADCONNECTOR = new Cconnector(this,8,0,Cconnector::Jack,"Digital connector 2 pins",true,QPoint(20,0)); publish(pADCONNECTOR);

    dialogpotar = new DialogPotar(this);
    dialogpotar->show();

    AddLog(LOG_MASTER,"done.\n");
    return true;
}

/*****************************************************************************/
/* Exit Potar																	 */
/*****************************************************************************/
bool Cpotar::exit(void)
{
    return true;
}


void Cpotar::paintEvent(QPaintEvent *event)
{
    CPObject::paintEvent(event);
//    dialogpotar->refresh();
}

void Cpotar::contextMenuEvent ( QContextMenuEvent * event )
{
    QMenu *menu= new QMenu(this);

    BuildContextMenu(menu);

    menu->addSeparator();

    menu->addAction(tr("Show console"),this,SLOT(ShowConsole()));
    menu->addAction(tr("Hide console"),this,SLOT(HideConsole()));

    menu->popup(event->globalPos () );
    event->accept();
}

void Cpotar::ShowConsole(void) {
    dialogpotar->show();
}
void Cpotar::HideConsole(void) {
    dialogpotar->hide();
}
