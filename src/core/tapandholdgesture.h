#ifndef TAPANDHOLDGESTURE_H
#define TAPANDHOLDGESTURE_H

#include <QObject>

class QTimer;
class QMouseEvent;

class TapAndHoldGesture: public QObject
{
    Q_OBJECT
public:
    TapAndHoldGesture( QObject *parent = 0 );
    void handleEvent( QMouseEvent *event);

signals:
    void handleTapAndHold(QMouseEvent *);

private slots:
    void timeout();

private:
    QTimer* _timer;
    QMouseEvent* _e;
};

#endif // TAPANDHOLDGESTURE_H
