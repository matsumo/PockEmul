#include <QtGui>

#include "dialogstartup.h"
#include "init.h"
#include "mainwindowpockemul.h"

extern MainWindowPockemul *mainwindow;

// TODO: populate tabs by parsin config.xml file

DialogStartup::DialogStartup( QWidget * parent, Qt::WindowFlags f) : QDialog(parent, f)
{
	setupUi(this);
	
}
//
void DialogStartup::on_listWidget_itemDoubleClicked()
{
	QString ItemText = listWidget->currentItem()->text();
	create(ItemText);
}

void DialogStartup::on_listWidget_2_itemDoubleClicked()
{
	QString ItemText = listWidget_2->currentItem()->text();
	create(ItemText);
}

void DialogStartup::on_listWidget_3_itemDoubleClicked()
{
	QString ItemText = listWidget_3->currentItem()->text();
	create(ItemText);
}

void DialogStartup::create(QString ItemText)
{
    int result = 0;

    if (mainwindow->objtable.contains(ItemText))
        result = mainwindow->objtable.value(ItemText);



    if (result != 0)	{
		this->done(result);
	}
//	else
//		QMessageBox::about(this, tr("Attention"),"Please choose a pocket model or Cancel");
}
//

//
