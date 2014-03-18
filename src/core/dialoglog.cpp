#include "dialoglog.h"
#include "init.h"
#include "common.h"
#include "Log.h"


DialogLog::DialogLog(QWidget * parent, Qt::WindowFlags f) : QDialog(parent, f)
{
	setupUi(this);
	connect(mainwindow,SIGNAL(AddLogItem(QString)),this,SLOT(additem(QString)));
    connect(mainwindow,SIGNAL(AddLogConsole(QString)),this,SLOT(addconsole(QString)));
    connect(listWidget_2,SIGNAL(itemChanged( QListWidgetItem *  )),this,SLOT(computeLogLevel(QListWidgetItem * )));

	addCkeckableItem("CPU",			LOG_CPU);
	addCkeckableItem("Keyboard",	LOG_KEYBOARD);
	addCkeckableItem("PockEmul",	LOG_MASTER);
	addCkeckableItem("Display",		LOG_DISPLAY);
	addCkeckableItem("Temp",		LOG_TEMP);
	addCkeckableItem("Tape",		LOG_TAPE);
	addCkeckableItem("SIO",			LOG_SIO);
	addCkeckableItem("Printer",		LOG_PRINTER);
	addCkeckableItem("Time",		LOG_TIME);
	addCkeckableItem("ROM",			LOG_ROM);
	addCkeckableItem("RAM",			LOG_RAM);
	addCkeckableItem("Functions",	LOG_FUNC);
	addCkeckableItem("11 Pins",		LOG_11PORT);
    addCkeckableItem("Analogic",	LOG_ANALOG);
    addCkeckableItem("Simulator",	LOG_SIMULATOR);
    addCkeckableItem("Canon X-07",	LOG_CANON);

	LogLevel = 0;
}

void DialogLog::addCkeckableItem(QString str,int Id)
{
	QListWidgetItem *item;	
	item = new QListWidgetItem(str, listWidget_2);	
	item->setData( Qt::UserRole, qVariantFromValue( Id ) );
	item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsTristate | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
	item->setCheckState(Qt::Unchecked);
}

void DialogLog::additem( QString  str)
{
	listWidget->addItem(str);
	// Add here the update()
}
void DialogLog::addconsole( QString  str)
{
    consoleTextEdit->textCursor().insertText(str);
    // Add here the update()
}
int DialogLog::computeLogLevel( QListWidgetItem * item )
{
	QVariant var = item->data(Qt::UserRole);
	int a = var.toInt();
	
	LogLevel = LogLevel & ~(a);
	LogLevel |= ( item->checkState() ? (a) : 0 );

    return(LogLevel);

}

void DialogLog::keyPressEvent (QKeyEvent * event )
{
	event->ignore();
}

void DialogLog::closeEvent(QCloseEvent *event)
{
    mainwindow->dialoglog = 0;
}
