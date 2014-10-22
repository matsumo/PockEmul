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
    connect(lwKeys,SIGNAL(itemSelectionChanged()),this,SLOT(slotSelectKey()));
	connect(pbInit,SIGNAL(clicked()),this,SLOT(slotInitSize()));
	connect(pbDel,SIGNAL(clicked()),this,SLOT(slotDelKey()));
	connect(sbHor,SIGNAL(valueChanged ( int )),this,SLOT(slotHorResize(int)));
	connect(sbVer,SIGNAL(valueChanged ( int )),this,SLOT(slotVerResize(int)));

    connect(pbUA,SIGNAL(clicked()),this,SLOT(moveUp()));
    connect(pbDA,SIGNAL(clicked()),this,SLOT(moveDown()));
    connect(pbLA,SIGNAL(clicked()),this,SLOT(moveLeft()));
    connect(pbRA,SIGNAL(clicked()),this,SLOT(moveRight()));

    connect(pbShowAll,SIGNAL(clicked()),this,SLOT(showAll()));

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
    for (int i=0; i < pPC->pKEYB->Keys.count(); i++)
 	{
//        if (it->MasterScanCode == 0 )
        {
            item = new QListWidgetItem(pPC->pKEYB->Keys.at(i).Description, lwKeys);
            item->setData( Qt::UserRole, qVariantFromValue( pPC->pKEYB->Keys.at(i).ScanCode ) );
        }
    }
}

void DialogKeyList::slotDelKey()
{
    return;
    //TODO: Manage list

    keyIter = pPC->pKEYB->Keys.erase( keyIter );
	keyFound = false;
	pPC->pKEYB->modified = true;
	delete lwKeys->currentItem();

}

void DialogKeyList::slotInitSize()
{
    for (int i=0;i<listRect.count();i++) {
        listRect[i].moveTo(0,0);
    }
	pPC->pKEYB->modified = true;
    pPC->Refresh_Display = true;
    pPC->update();
}

void DialogKeyList::slotHorResize(int width)
{

    for (int i=0;i<listRect.count();i++) {
        listRect[i].setWidth(width);
    }
	pPC->pKEYB->modified = true;
    pPC->Refresh_Display = true;
    pPC->update();
}

void DialogKeyList::slotVerResize(int height)
{
    for (int i=0;i<listRect.count();i++) {
        listRect[i].setHeight(height);
    }
	pPC->pKEYB->modified = true;
    pPC->Refresh_Display = true;
    pPC->update();
}

void DialogKeyList::moveUp()
{
    for (int i=0;i<lwKeys->count();i++) {
        if (lwKeys->item(i)->isSelected()) {
            pPC->pKEYB->Keys[i].Rect.adjust(0,-1,0,-1);
            pPC->pKEYB->modified = true;
        }
    }
    pPC->Refresh_Display = true;
    pPC->update();
}

void DialogKeyList::moveDown()
{
    for (int i=0;i<lwKeys->count();i++) {
        if (lwKeys->item(i)->isSelected()) {
            pPC->pKEYB->Keys[i].Rect.adjust(0,+1,0,1);
            pPC->pKEYB->modified = true;
        }
    }
    pPC->Refresh_Display = true;
    pPC->update();
}

void DialogKeyList::moveLeft()
{
    for (int i=0;i<lwKeys->count();i++) {
        if (lwKeys->item(i)->isSelected()) {
            pPC->pKEYB->Keys[i].Rect.adjust(-1,0,-1,0);
            pPC->pKEYB->modified = true;
        }
    }
    pPC->Refresh_Display = true;
    pPC->update();
}

void DialogKeyList::moveRight()
{
    for (int i=0;i<lwKeys->count();i++) {
        if (lwKeys->item(i)->isSelected()) {
            pPC->pKEYB->Keys[i].Rect.adjust(+1,0,1,0);
            pPC->pKEYB->modified = true;
        }
    }
    pPC->Refresh_Display = true;
    pPC->update();
}

void DialogKeyList::showAll()
{
    for (keyIter = pPC->pKEYB->Keys.begin(); keyIter != pPC->pKEYB->Keys.end(); ++keyIter)
    {
        listRect.append(keyIter->Rect);
    }
    pPC->Refresh_Display = true;
    pPC->update();
}

CKey DialogKeyList::findKey(QString desc) {
    for (int i=0; i < pPC->pKEYB->Keys.count();i++)
    {
        if (pPC->pKEYB->Keys.at(i).Description == desc)
        {
            return pPC->pKEYB->Keys.at(i);
        }
    }
    return CKey();
}

void DialogKeyList::slotSelectKey()
{
	// Find the correct Keys in List
    // Draw the Boundary
    listRect.clear();;

    for (int i=0;i<lwKeys->count();i++) {
        if (lwKeys->item(i)->isSelected()) {
//            CKey _key = findKey(lwKeys->at(i)->text());
//            //        sbHor->setValue(_key.Rect.width());
//            //        sbVer->setValue(_key.Rect.height());
//            listRect.append(_key.Rect);
        }
    }
    pPC->Refresh_Display = true;
    pPC->update();
}

QList<QRect> DialogKeyList::getkeyFoundRect(void)
{
    return listRect;
}

void DialogKeyList::closeEvent(QCloseEvent *event)
{
	pPC->dialogkeylist=0;
	event->accept();
}
