#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QDebug>

#include "extslot.h"
#include "pobject.h"
#include "Connect.h"

void CextSlot::animateDoor(bool _open) {
    qWarning()<<"ANIMATE doopr";
    if (doorOpen == _open) return;

    doorOpen = _open;


    QPropertyAnimation *animation1 = new QPropertyAnimation(this, "backdoorangle");
     animation1->setDuration(1500);
     if (doorOpen) {
         animation1->setStartValue(m_backdoorAngle);
         animation1->setEndValue(-70);
     }
     else {
         manageCardVisibility();
         animation1->setStartValue(m_backdoorAngle);
         animation1->setEndValue(0);
     }

     QParallelAnimationGroup *group = new QParallelAnimationGroup;
     group->addAnimation(animation1);

     connect(animation1,SIGNAL(valueChanged(QVariant)),(CPObject*)parent(),SLOT(RefreshDisplay()));
     connect(animation1,SIGNAL(finished()),this,SLOT(endDoorAnimation()));
     doorFlipping = true;
     group->start();

}

void CextSlot::endDoorAnimation()
{
    doorFlipping = false;
    manageCardVisibility();
}

CextSlot::CextSlot(QObject *parent):QObject(parent)
{

}

void CextSlot::manageCardVisibility() {

    CPObject * _parent = (CPObject*)parent();

    if (_parent->currentView == BACKview) {
        // show memory cards
        CPObject * _cardPC = pConn->LinkedToObject();
        if (_cardPC) {
            if (doorOpen) {
                _cardPC->showObject();
            }
            else {
                _cardPC->hideObject();
            }
        }
    }
}
