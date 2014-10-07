#include <QDebug>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QPainter>
#include <QBitmap>

#include "viewobject.h"

#include "mainwindowpockemul.h"

#include "Keyb.h"

extern MainWindowPockemul* mainwindow;

CViewObject::CViewObject(CViewObject *parent):QWidget(mainwindow->centralwidget)
{
    Q_UNUSED(parent)

    FrontImage=TopImage=LeftImage=RightImage=BottomImage=BackImage=0;
    Pc_DX_mm=Pc_DY_mm=Pc_DZ_mm=0;
    PosX = PosY	= Pc_DX = Pc_DY = 0;
    pKEYB = 0;
}

CViewObject::~CViewObject()
{
    delete FinalImage;
    delete FrontImage;
    delete TopImage;
    delete LeftImage;
    delete RightImage;
    delete BottomImage;
    delete BackImage;

    delete pKEYB;
}

float	CViewObject::posx()
{
    return PosX;
}
float CViewObject::posy()
{
    return PosY;
}
void CViewObject::setPosX(float val)
{
    PosX = val;
    QWidget::move(QPoint(PosX,PosY));
    emit movePObject(this,QPoint(PosX,PosY));
}
void CViewObject::setPosY(float val)
{
    PosY = val;
    QWidget::move(QPoint(PosX,PosY));
    emit movePObject(this,QPoint(PosX,PosY));
}
void CViewObject::MoveRel(QPoint p)
{
    PosX += p.x();
    PosY += p.y();
    QWidget::move(QPoint(PosX,PosY));
    emit movePObject(this,QPoint(PosX,PosY));
#ifdef AVOID
    mainwindow->router->moveShape(mainwindow->shapeRefList[this],p.x(),p.y());
    mainwindow->router->processTransaction();
    mainwindow->router->outputInstanceToSVG("test-connectionpin01");
#endif
}

void CViewObject::MoveAbs(QPoint p)
{
    PosX = p.x();
    PosY = p.y();
    QWidget::move(QPoint(PosX,PosY));
    emit movePObject(this,QPoint(PosX,PosY));
#ifdef AVOID
    mainwindow->router->moveShape(mainwindow->shapeRefList[this],p.x(),p.y());
    mainwindow->router->processTransaction();
    mainwindow->router->outputInstanceToSVG("test-connectionpin01");
#endif
}

QPoint CViewObject::pos()
{
    return QPoint(PosX,PosY);
}

int CViewObject::getDX() {return Pc_DX;}

int CViewObject::getDY() {return Pc_DY;}

int CViewObject::getDZ() {return 0;}

void CViewObject::setDX(int v) {Pc_DX = v;}

void CViewObject::setDY(int v) {Pc_DY = v;}

void CViewObject::setDZ(int v) {Q_UNUSED(v)}

int CViewObject::getDXmm() {return Pc_DX_mm;}

int CViewObject::getDYmm() {return Pc_DY_mm;}

int CViewObject::getDZmm() {return Pc_DZ_mm;}

void CViewObject::setDXmm(int v) {Pc_DX_mm = v;}

void CViewObject::setDYmm(int v) {Pc_DY_mm = v;}

void CViewObject::setDZmm(int v) {Pc_DZ_mm = v;}

QRect CViewObject::rect()
{
    return QRect(PosX,PosY,Pc_DX,Pc_DY);
}

QImage * CViewObject::CreateImage(QSize size,QString fname,bool Hmirror,bool Vmirror,int angle)
{
    //    qWarning("LoadImage : %s",fname.toLatin1().data());
    QImage *tempImage;
    QMatrix matrix;
    matrix.rotate(angle);
    QImage loc;
    if (fname.isEmpty()) {
        loc = QImage(size,QImage::Format_ARGB32);
    }
    else
        if (size.isValid()) {
            loc = QImage(fname).mirrored(Hmirror,Vmirror).transformed(matrix).scaled(size,Qt::IgnoreAspectRatio,Qt::SmoothTransformation).convertToFormat(QImage::Format_ARGB32);
        }
        else {
            loc = QImage(fname).mirrored(Hmirror,Vmirror).transformed(matrix).convertToFormat(QImage::Format_ARGB32);
        }
    tempImage = new QImage(loc);
    return tempImage;
}

bool CViewObject::InitDisplay(void)
{
    delete TopImage;
    delete LeftImage;
    delete RightImage;
    delete BottomImage;
    delete BackImage;

    if (!TopFname.isEmpty()) TopImage = CreateImage(viewRect(TOPview),TopFname);
    if (!LeftFname.isEmpty()) LeftImage = CreateImage(viewRect(LEFTview),LeftFname);
    if (!RightFname.isEmpty()) RightImage = CreateImage(viewRect(RIGHTview),RightFname);
    if (!BottomFname.isEmpty()) BottomImage = CreateImage(viewRect(BOTTOMview),BottomFname);
    if (!BackFname.isEmpty()) BackImage = CreateImage(viewRect(BACKview),BackFname);

    return(1);
}

#define RANGEPERCENT 50
#define THIN 5          // Percentage
#define MIN_THIN 30
Direction CViewObject::borderClick(QPoint pt) {
    int x1 = this->width()*(50 - RANGEPERCENT/2) /100;
    int x2 = this->width()*(50 + RANGEPERCENT/2) /100;
    int y1 = this->height()*(50 - RANGEPERCENT/2) /100;
    int y2 = this->height()*(50 + RANGEPERCENT/2) /100;

    if ( (pt.x()>=x1) && (pt.x()<=x2) ) {
        if (pt.y() <= MAX(this->height() *THIN / 100,MIN(MIN_THIN,this->height()/2))) return TOPdir;
        if (pt.y() >= MIN(this->height()-MIN(MIN_THIN,this->height()/2),this->height() *(100-THIN) / 100)) return BOTTOMdir;
    }
    if ( (pt.y()>=y1) && (pt.y()<=y2) ) {
        if (pt.x() <= MAX(this->width() *THIN / 100,MIN(MIN_THIN,this->width()/2))) return LEFTdir;
        if (pt.x() >= MIN(this->width()-MIN(MIN_THIN,this->width()/2),this->width() *(100-THIN) / 100)) return RIGHTdir;
    }

    return NONEdir;

}

void CViewObject::setAngle(int value) {
    this->m_angle = value;
}

void CViewObject::setZoom(qreal value)
{
    this->m_zoom = value;
}

QSize CViewObject::viewRect(View v) {
    if (getDXmm()==0) {
        qWarning()<<"ERROR DXùù not set";
        return QSize(0,0);
    }
    float _ratio = this->getDX()/this->getDXmm();

    switch (v) {

    case FRONTview:
    case BACKview: return QSize(this->getDX(),this->getDY());
    case TOPview:
    case BOTTOMview: return QSize(this->getDX(),this->getDZmm()*_ratio);
    case LEFTview:
    case RIGHTview: return QSize(this->getDZmm()*_ratio,this->getDY());
    }
    return QSize(0,0);
}

QSize CViewObject::currentViewRect() {
    return viewRect(currentView);
}

QImage * CViewObject::getViewImage(View v) {

    switch (v) {
    case FRONTview: return FinalImage;
    case TOPview: return TopImage;
    case LEFTview: return LeftImage;
    case RIGHTview: return RightImage;
    case BOTTOMview: return BottomImage;
    case BACKview: return BackImage;
    }

    return 0;
}

void CViewObject::flip(Direction dir) {
    // IF CONNECTED to printer , exit
//    QList<CPObject *> ConList;
//    mainwindow->pdirectLink->findAllObj(this,&ConList);
//    if (!ConList.isEmpty()) return;
    // Animate close


    targetSize = viewRect(targetView);
    currentFlipDir = dir;

//    qWarning()<<"targetdir:"<<targetSize;

    QPropertyAnimation *animation1 = new QPropertyAnimation(this, "angle");
    QPropertyAnimation *animation2 = new QPropertyAnimation(this, "zoom");
     animation1->setDuration(500);
     animation2->setDuration(500);

     switch (currentFlipDir) {
     case TOPdir:
         animation1->setStartValue(0);
         animation1->setEndValue(90);
         animation2->setKeyValueAt(0.0,1.0);
         animation2->setKeyValueAt(0.5,.75);
         animation2->setKeyValueAt(1.0,1.0);
         animationView1 = currentView;
         animationView2 = targetView;
         clearMask();
         break;
     case BOTTOMdir:
         animation1->setStartValue(90);
         animation1->setEndValue(0);
         animation2->setKeyValueAt(0,1.0);
         animation2->setKeyValueAt(0.5,.75);
         animation2->setKeyValueAt(1,1.0);
         animationView1 = targetView;
         animationView2 = currentView;
         clearMask();
         break;
     case LEFTdir:
         animation1->setStartValue(0);
         animation1->setEndValue(90);
         animation2->setKeyValueAt(0.0,1.0);
         animation2->setKeyValueAt(0.5,.75);
         animation2->setKeyValueAt(1.0,1.0);
         animationView1 = currentView;
         animationView2 = targetView;
         clearMask();
         break;
     case RIGHTdir:
         animation1->setStartValue(90);
         animation1->setEndValue(0);
         animation2->setKeyValueAt(0.0,1.0);
         animation2->setKeyValueAt(0.5,.75);
         animation2->setKeyValueAt(1.0,1.0);
         animationView1 = targetView;
         animationView2 = currentView;
         clearMask();
         break;
     default: break;
     }

     QParallelAnimationGroup *group = new QParallelAnimationGroup;
     group->addAnimation(animation1);
     group->addAnimation(animation2);

     connect(animation1,SIGNAL(valueChanged(QVariant)),this,SLOT(update()));
     connect(animation1,SIGNAL(finished()),this,SLOT(endAnimation()));
     flipping = true;
     group->start();

}
#define RATIO .25
void CViewObject::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    if (flipping)
    {
        UpdateFinalImage();
        QPainter painter;


        if (FinalImage)
        {
            int w = viewRect(animationView1).width() * mainwindow->zoom/100.0;//this->width();
            int h = viewRect(animationView1).height() * mainwindow->zoom/100.0;//this->height();
            int wt = viewRect(animationView2).width() * mainwindow->zoom/100.0;
            int ht = viewRect(animationView2).height()* mainwindow->zoom/100.0;
//            qWarning()<<"angle:"<<m_angle;
            painter.begin(this);

            QTransform matrix,matrix2;
            matrix.scale(m_zoom,m_zoom);

            switch (currentFlipDir) {
            case TOPdir:
            case BOTTOMdir:
                painter.translate(w/2 ,ht * m_angle/90);
                painter.setTransform(matrix,true);
                matrix2.rotate(-m_angle, Qt::XAxis);
                painter.setTransform(matrix2,true);
                painter.drawImage(QPoint(-w/2,0),
                                  getViewImage(animationView1)->scaled(QSize(w,h*(90 -m_angle)/90),Qt::IgnoreAspectRatio,Qt::SmoothTransformation)
                                  );
                painter.end();
                painter.begin(this);
                painter.translate(w/2 ,ht * m_angle/90);
                painter.setTransform(matrix,true);
                matrix2.reset();
                matrix2.rotate(90-m_angle, Qt::XAxis);
                painter.setTransform(matrix2,true);
                painter.drawImage(QPoint(-w/2,-ht * m_angle/90),
                                  getViewImage(animationView2)->scaled(QSize(w,ht * m_angle/90),Qt::IgnoreAspectRatio,Qt::SmoothTransformation)
                                  );
                break;
            case LEFTdir:
            case RIGHTdir:
                painter.translate(wt*m_angle/90,h/2);
                painter.setTransform(matrix,true);
                matrix2.rotate(-m_angle, Qt::YAxis);
                painter.setTransform(matrix2,true);
                painter.drawImage(QPoint(0,-h/2),
                                  getViewImage(animationView1)->scaled(QSize(w*(90 -m_angle)/90,h),Qt::IgnoreAspectRatio,Qt::SmoothTransformation)
                                  );
                painter.end();
                painter.begin(this);
                painter.translate(wt*m_angle/90,h/2);
                painter.setTransform(matrix,true);
                matrix2.reset();
                matrix2.rotate(90-m_angle, Qt::YAxis);
                painter.setTransform(matrix2,true);
                painter.drawImage(QPoint(-wt * m_angle/90,-h/2),
                                  getViewImage(animationView2)->scaled(QSize(wt * m_angle/90,ht),Qt::IgnoreAspectRatio,Qt::SmoothTransformation)
                                  );
                break;
            default: break;
            }

            painter.end();

        }
    }
}

void CViewObject::endAnimation(){
    currentView = targetView;
//    currentFlipDir = NONEdir;
//    flipping = false;

    changeGeometry(this->posx(),this->posy(),viewRect(currentView).width()*mainwindow->zoom/100.0,viewRect(currentView).height()*mainwindow->zoom/100.0);

}

void CViewObject::changeGeometrySize(int newposx,int newposy,int newwidth,int newheight) {


    changeGeometry(newposx,newposy,newwidth,newheight);
    setDX(newwidth);
    setDY(newheight);
}
void CViewObject::changeGeometry(int newposx,int newposy,int newwidth,int newheight) {

    setPosX(newposx);
    setPosY(newposy);
    setGeometry(newposx,newposy,newwidth,newheight);
    setMask(mask.scaled(newwidth,newheight).mask());
#ifdef AVOID
    Avoid::Rectangle rectangle(Avoid::Point(newposx-10, newposy-10),
                               Avoid::Point(newposx+newwidth+20, newposy+newheight+20));
    mainwindow->router->moveShape(mainwindow->shapeRefList[this], rectangle);
#endif
    emit sizePObject(this,QSize(newwidth,newheight));
}

void CViewObject::mousePressEvent(QMouseEvent *event) {
    // if click on the border
    // width ?
//qWarning()<<"CViewObject::mousePressEvent"<<event;

    QPoint pts(event->x() , event->y());
    if (pKEYB && pKEYB->KeyClick(pts)) {
        return;
    }

    Direction dir = borderClick(event->pos());

    targetView = currentView;
    switch (currentView) {
    case FRONTview:
        switch (dir) {
        case TOPdir: targetView = TOPview; break;
        case LEFTdir: targetView = LEFTview; break;
        case RIGHTdir: targetView = RIGHTview; break;
        case BOTTOMdir: targetView = BOTTOMview; break;
        default: break;
        }
        break;
    case TOPview:
        switch (dir) {
        case TOPdir: targetView = BACKview; break;
//        case LEFTdir: targetView = LEFTview; break;
//        case RIGHTdir: targetView = RIGHTview; break;
        case BOTTOMdir: targetView = FRONTview; break;
        default: break;
        }
        break;
    case LEFTview:
        switch (dir) {
//        case TOPdir: targetView = TOPview; break;
        case LEFTdir: targetView = BACKview; break;
        case RIGHTdir: targetView = FRONTview; break;
//        case BOTTOMdir: targetView = BOTTOMview; break;
        default: break;
        }
        break;
    case RIGHTview:
        switch (dir) {
//        case TOPdir: targetView = TOPview; break;
        case LEFTdir: targetView = FRONTview; break;
        case RIGHTdir: targetView = BACKview; break;
//        case BOTTOMdir: targetView = BOTTOMview; break;
        default: break;
        }
        break;
    case BOTTOMview:
        switch (dir) {
        case TOPdir: targetView = FRONTview; break;
//        case LEFTdir: targetView = LEFTview; break;
//        case RIGHTdir: targetView = RIGHTview; break;
        case BOTTOMdir: targetView = BACKview; break;
        default: break;
        }
        break;
    case BACKview:
        switch (dir) {
        case TOPdir: targetView = BOTTOMview; break;
        case LEFTdir: targetView = RIGHTview; break;
        case RIGHTdir: targetView = LEFTview; break;
        case BOTTOMdir: targetView = TOPview; break;
        default: break;
        }
        break;

    }

    if ( (targetView != currentView) && getViewImage(targetView) ) {
        QSize _s = viewRect(currentView).expandedTo(viewRect(targetView));
        changeGeometry(this->posx(),this->posy(),
                       _s.width()*mainwindow->zoom/100.0,_s.height()*mainwindow->zoom/100.0);
        flip(dir);
    }

}
