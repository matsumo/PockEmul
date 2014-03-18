#include "tapandholdgesture.h"

#include <QDebug>
#include <QMouseEvent>
#include <QTimer>

TapAndHoldGesture::TapAndHoldGesture( QObject *parent )
{
    _timer = new QTimer(this);
    connect(_timer,SIGNAL(timeout()),this,SLOT(timeout()));
}

void TapAndHoldGesture::handleEvent( QMouseEvent *event)
{
//    qWarning()<<"TapAndHoldEvent"<<this;
    if( event->type() == QMouseEvent::MouseButtonPress ) {
//        qWarning("tapevent\n");
        _e = new QMouseEvent(event->type(),event->pos(),event->globalPos(),event->button(),event->buttons(),event->modifiers());
        _timer->start( 1000 );
    } else if( event->type() == QMouseEvent::MouseButtonRelease ) {
        if( _timer->isActive() ) {
            // tap and hold canceled
            _timer->stop();
        }
    } else if( event->type() == QMouseEvent::MouseMove ) {
        // tap and hold canceled
        _timer->stop();
    }
}

void TapAndHoldGesture::timeout()
{
    emit handleTapAndHold(_e);
//    qWarning("tap event pos (%i,%i)\n",_e->globalPos().x(),_e->globalPos().y());
    _timer->stop();
}
