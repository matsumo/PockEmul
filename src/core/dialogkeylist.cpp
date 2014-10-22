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

    connect(pbUA,SIGNAL(clicked()),this,SLOT(moveUp()));
    connect(pbDA,SIGNAL(clicked()),this,SLOT(moveDown()));
    connect(pbLA,SIGNAL(clicked()),this,SLOT(moveLeft()));
    connect(pbRA,SIGNAL(clicked()),this,SLOT(moveRight()));

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
    keyIter = pPC->pKEYB->Keys.erase( keyIter );
	keyFound = false;
	pPC->pKEYB->modified = true;
	delete lwKeys->currentItem();

}

void DialogKeyList::slotInitSize()
{
    keyIter->Rect.moveTo(0,0);
	pPC->pKEYB->modified = true;
}

void DialogKeyList::slotHorResize(int width)
{
	if (!keyFound) return;
		
    keyIter->Rect.setWidth(width);
	pPC->pKEYB->modified = true;
	pPC->update();
}

void DialogKeyList::slotVerResize(int height)
{
	if (!keyFound) return;
		
    keyIter->Rect.setHeight(height);
	pPC->pKEYB->modified = true;
    pPC->update();
}

void DialogKeyList::moveUp()
{
    keyIter->Rect.adjust(0,-1,0,0);
    pPC->Refresh_Display = true;
    pPC->update();
}
void DialogKeyList::moveDown()
{
    keyIter->Rect.adjust(0,+1,0,1);
    pPC->Refresh_Display = true;
    pPC->update();
}
void DialogKeyList::moveLeft()
{
    keyIter->Rect.adjust(-1,0,-1,0);
    pPC->Refresh_Display = true;
    pPC->update();
}
void DialogKeyList::moveRight()
{
    keyIter->Rect.adjust(+1,0,1,0);
    pPC->Refresh_Display = true;
    pPC->update();
}

void DialogKeyList::slotSelectKey(QListWidgetItem * item , QListWidgetItem * previous)
{
	// Find the correct Keys in List
	// Draw the Boundary	
    for (keyIter = pPC->pKEYB->Keys.begin(); keyIter != pPC->pKEYB->Keys.end(); ++keyIter)
 	{
        if (keyIter->Description == item->text())
		{
			keyFound = true;
            AddLog(LOG_MASTER,tr("Rect %1,%2 - %3,%4").arg(keyIter->Rect.left()).arg(keyIter->Rect.right()).arg(keyIter->Rect.width()).arg(keyIter->Rect.height()));
            sbHor->setValue(keyIter->Rect.width());
            sbVer->setValue(keyIter->Rect.height());
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
    _result.setTop(keyIter->Rect.top() * mainwindow->zoom/100);
    _result.setLeft(keyIter->Rect.left() * mainwindow->zoom/100);
    _result.setWidth(keyIter->Rect.width() * mainwindow->zoom/100);
    _result.setHeight(keyIter->Rect.height() * mainwindow->zoom/100);
    return _result;
}

void DialogKeyList::closeEvent(QCloseEvent *event)
{
	pPC->dialogkeylist=0;
	event->accept();
}
