#include <QPainter>
#include <QCloseEvent>
#include <QDebug>

#include "common.h"
#include "pcxxxx.h"
#include "Keyb.h"
#include "dialogkeylist.h"
#include "Log.h"
//


DialogKeyList::DialogKeyList(CPObject * parent, Qt::WindowFlags f)
	: QDialog(parent, f)
{
	setupUi(this);
	
	keyFound = false;
	connect(lwKeys,SIGNAL(currentItemChanged( QListWidgetItem * ,QListWidgetItem * )),this,SLOT(slotSelectKey(QListWidgetItem * , QListWidgetItem *)));
	connect(pbInit,SIGNAL(clicked()),this,SLOT(slotInitSize()));
	connect(pbDel,SIGNAL(clicked()),this,SLOT(slotDelKey()));
	connect(sbHor,SIGNAL(valueChanged ( int )),this,SLOT(slotHorResize(int)));
	connect(sbVer,SIGNAL(valueChanged ( int )),this,SLOT(slotVerResize(int)));

	pPC = parent;
	// populate lvKeys
	InsertKeys();
}
//

void DialogKeyList::InsertKeys(void)
{
	QListWidgetItem *item;	

	AddLog(LOG_MASTER,tr("pPC=%1").arg((long) pPC));
	QList<CKey>::iterator it;
 	for (it = pPC->pKEYB->Keys.begin(); it != pPC->pKEYB->Keys.end(); ++it)
 	{
//        if (it->MasterScanCode == 0 )
        {
            item = new QListWidgetItem(it->Description, lwKeys);
            item->setData( Qt::UserRole, qVariantFromValue( it->ScanCode ) );
        }
    }
}

void DialogKeyList::slotDelKey()
{
	i = pPC->pKEYB->Keys.erase( i );
	keyFound = false;
	pPC->pKEYB->modified = true;
	delete lwKeys->currentItem();

}

void DialogKeyList::slotInitSize()
{
    i->Rect.moveTo(0,0);
	pPC->pKEYB->modified = true;
}

void DialogKeyList::slotHorResize(int width)
{
	if (!keyFound) return;
		
	i->Rect.setWidth(width);
	pPC->pKEYB->modified = true;
	pPC->update();
}

void DialogKeyList::slotVerResize(int height)
{
	if (!keyFound) return;
		
	i->Rect.setHeight(height);
	pPC->pKEYB->modified = true;
	pPC->update();
}

void DialogKeyList::slotSelectKey(QListWidgetItem * item , QListWidgetItem * previous)
{
	// Find the correct Keys in List
	// Draw the Boundary	
 	for (i = pPC->pKEYB->Keys.begin(); i != pPC->pKEYB->Keys.end(); ++i)
 	{
		if (i->Description == item->text())
		{
			keyFound = true;
			AddLog(LOG_MASTER,tr("Rect %1,%2 - %3,%4").arg(i->Rect.left()).arg(i->Rect.right()).arg(i->Rect.width()).arg(i->Rect.height()));
			sbHor->setValue(i->Rect.width());
			sbVer->setValue(i->Rect.height());
            qWarning()<<"FOUND!!!";
            pPC->Refresh_Display = true;
			pPC->update();
			return;
		}
	}
	
}

QRect DialogKeyList::getkeyFoundRect(void)
{
	if (!keyFound) return QRect();

    QRect _result;
    _result.setTop(i->Rect.top() * mainwindow->zoom/100);
    _result.setLeft(i->Rect.left() * mainwindow->zoom/100);
    _result.setWidth(i->Rect.width() * mainwindow->zoom/100);
    _result.setHeight(i->Rect.height() * mainwindow->zoom/100);
    return _result;
}

void DialogKeyList::closeEvent(QCloseEvent *event)
{
	pPC->dialogkeylist=0;
	event->accept();
}
