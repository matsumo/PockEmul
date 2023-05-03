#include <QPainter>
#include <QPainterPath>

#include "mainwindowpockemul.h"
#include "dialoganalog.h"

#include "ccable.h"
#include "Connect.h"
#include "watchpoint.h"
#include "Inter.h"


extern MainWindowPockemul *mainwindow;

Ccable::Ccable(CPObject *parent)	: CPObject(parent)
{							//[constructor]

    setfrequency( 0);
    BackGroundFname	= P_RES(":/ext/cable.png");

    pTIMER		= new Ctimer(this);
    setDX(150);
    setDY(450);
    setDXmm(196);
    setDYmm(155);
    setDZmm(33);
    standard = false;
}

Ccable::~Ccable(){
}


bool Ccable::init(void) {
    CPObject::init();

    clearMask();

    pCONNECTOR_A = new Cconnector(this,11,0,Cconnector::Sharp_11,"11 pins Connector A",true,QPoint(135,6)); publish(pCONNECTOR_A);
    pCONNECTOR_B = new Cconnector(this,11,1,Cconnector::Sharp_11,"11 pins Connector B",true,QPoint(135,300)); publish(pCONNECTOR_B);

    WatchPoint.add(&pCONNECTOR_A_value,64,11,this,"11pins connector A");
    WatchPoint.add(&pCONNECTOR_B_value,64,11,this,"11pins connector B");
    return true;
}
//********************************************************/
// PIN_MT_OUT2	1
// PIN_GND		2
// PIN_VGG		3
// PIN_BUSY		4
// PIN_D_OUT	5
// PIN_MT_IN	6
// PIN_MT_OUT1	7
// PIN_D_IN		8
// PIN_ACK		9
// PIN_SEL2		10
// PIN_SEL1		11
//********************************************************/
bool Ccable::run(void)
{

    if (standard) {
        qint64 a = pCONNECTOR_A->Get_values();
        pCONNECTOR_A->Set_values(pCONNECTOR_B->Get_values());
        pCONNECTOR_B->Set_values(a);
    }
    else {
        // Invert pins
        // Identification input/output pins
        // 6 : tape in
        // 7 : tape out

        bool _tmp7A = pCONNECTOR_A->Get_pin(7);
        bool _tmp7B = pCONNECTOR_B->Get_pin(7);
        pCONNECTOR_B->Set_pin(6,_tmp7A);
        pCONNECTOR_A->Set_pin(6,_tmp7B);

    }
//    if (mainwindow->dialoganalogic) mainwindow->dialoganalogic->dataplot.Marker = 8;

    pCONNECTOR_A_value = pCONNECTOR_A->Get_values();
    pCONNECTOR_B_value = pCONNECTOR_B->Get_values();
	return true;
}


bool Ccable::exit(void)				//end
{
    return true;
}

bool Ccable::Adapt(Cconnector *A, Cconnector *B)
{
    QRectF r = QRectF(A->pos(),B->pos());

    bool reverse = ((r.width() * r.height())<0);
    r=r.normalized();
    changeGeometry(r.x(),r.y(),r.width(),r.height());
    setDX(r.width());
    setDY(r.height());
    if (reverse) {
        pCONNECTOR_A->setSnap(QPoint(0,r.height()));
        pCONNECTOR_B->setSnap(QPoint(r.width(),0));
    }
    else {
        pCONNECTOR_A->setSnap(QPoint(0,0));
        pCONNECTOR_B->setSnap(QPoint(r.width(),r.height()));

    }
    BackGroundFname	= P_RES(":/ext/cable_blank.png");
    InitDisplay();

    return reverse;
}

bool Ccable::UpdateFinalImage(void) {
    CPObject::UpdateFinalImage();
    paintingImage.lock();
    clearMask();

    // Draw switch by 180° rotation



    // draw a bezier curve between 2 points A and B
    QPainterPath path;


    int dx1,dx2,dy1,dy2;
    dx1=dx2=dy1=dy2=0;

    switch (pCONNECTOR_A->getDir()) {
    case Cconnector::NORTH: dy1 = 100; break;
    case Cconnector::EAST: dx1 = -100; break;
    case Cconnector::SOUTH: dy1 = -100; break;
    case Cconnector::WEST: dx1 = 100; break;
    }
    switch (pCONNECTOR_B->getDir()) {
    case Cconnector::NORTH: dy2 = 100; break;
    case Cconnector::EAST: dx2 = -100; break;
    case Cconnector::SOUTH: dy2 = -100; break;
    case Cconnector::WEST: dx2 = 100; break;
    }
    QPointF start= pCONNECTOR_A->getSnap();
    QPointF x1   = pCONNECTOR_A->getSnap() + QPoint(dx1,dy1);
    QPointF x2   = pCONNECTOR_B->getSnap() + QPoint(dx2,dy2);
    QPointF end  = pCONNECTOR_B->getSnap();
     path.moveTo(start);
     path.cubicTo(x1,x2,end);


     QPainter painter;
     painter.begin(FinalImage);

     QPen pen;
     pen.setWidth(10);
     painter.setPen(pen);
     painter.setBrush(QColor(122, 163, 39));

     painter.drawPath(path);


    painter.end();

    paintingImage.unlock();
    return true;
}
