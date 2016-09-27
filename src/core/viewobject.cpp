#include <QDebug>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QPainter>
#include <QBitmap>

#include "viewobject.h"

#include "mainwindowpockemul.h"

#include "Keyb.h"
#include "renderView.h"

#define DPMM 3.6   // dot per mm

extern MainWindowPockemul* mainwindow;
extern bool hiRes;
extern bool flipOnEdge;
extern int ask(QWidget *parent,QString msg,int nbButton);

CViewObject::CViewObject(CViewObject *parent):MAINCLASS(parent?parent:mainwindow->centralwidget)
{
    Q_UNUSED(parent)

//    if (parent==0)
//        setParent(mainwindow->centralwidget);

    FrontImage=TopImage=LeftImage=RightImage=BottomImage=BackImage=0;
    Pc_DX_mm=Pc_DY_mm=Pc_DZ_mm=0;
    PosX = PosY	= Pc_DX = Pc_DY = 0;

    FinalImage = 0;
    BackgroundImageBackup = 0;
    AnimatedImage = 0;
    internalImageRatio = 1;
    rotation = 0;

    visible = true;

    pKEYB = new Ckeyb(this);
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
    delete BackgroundImageBackup;
    delete AnimatedImage;
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
    move(QPoint(PosX,PosY));
    emit movePObject(this,QPointF(PosX,PosY));
}
void CViewObject::setPosY(float val)
{
    PosY = val;
    move(QPoint(PosX,PosY));
    emit movePObject(this,QPointF(PosX,PosY));
}
void CViewObject::MoveRel(QPointF p)
{
    PosX += p.x();
    PosY += p.y();
    move(QPoint(PosX,PosY));
    emit movePObject(this,QPointF(PosX,PosY));
#ifdef AVOID
    mainwindow->router->moveShape(mainwindow->shapeRefList[this],p.x(),p.y());
    mainwindow->router->processTransaction();
    mainwindow->router->outputInstanceToSVG("test-connectionpin01");
#endif
}

void CViewObject::MoveAbs(QPointF p)
{
    PosX = p.x();
    PosY = p.y();
    move(QPoint(PosX,PosY));
    emit movePObject(this,QPointF(PosX,PosY));
#ifdef AVOID
    mainwindow->router->moveShape(mainwindow->shapeRefList[this],p.x(),p.y());
    mainwindow->router->processTransaction();
    mainwindow->router->outputInstanceToSVG("test-connectionpin01");
#endif
}

QPointF CViewObject::pos()
{
    return QPointF(PosX,PosY);
}

float CViewObject::getDX() {return Pc_DX;}

float CViewObject::getDY() {return Pc_DY;}

float CViewObject::getDZ() {return 0;}

void CViewObject::setDX(float v) {Pc_DX = v;}

void CViewObject::setDY(float v) {Pc_DY = v;}

void CViewObject::setDZ(float v) {Q_UNUSED(v)}

int CViewObject::getDXmm() {return Pc_DX_mm;}

int CViewObject::getDYmm() {return Pc_DY_mm;}

int CViewObject::getDZmm() {return Pc_DZ_mm;}

void CViewObject::setDXmm(int v) {Pc_DX_mm = v;}

void CViewObject::setDYmm(int v) {Pc_DY_mm = v;}

void CViewObject::setDZmm(int v) {Pc_DZ_mm = v;}

void CViewObject::setSizeMM(int w,int h, int p) {
    Pc_DX_mm = w;
    Pc_DY_mm = h;
    Pc_DZ_mm = p;

    Pc_DX = w * DPMM;
    Pc_DY = h * DPMM;
}

void CViewObject::setRotation(int v) {
    rotation = v;
    emit rotPObject(this,v);
}

QRectF CViewObject::rect()
{
    return QRectF(PosX,PosY,Pc_DX*mainwindow->zoom,Pc_DY*mainwindow->zoom);
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
            if (loc.isNull()) {
                qWarning()<<"Cannot create Image";
            }
        }
        else {
            loc = QImage(fname).mirrored(Hmirror,Vmirror).transformed(matrix).convertToFormat(QImage::Format_ARGB32);
        }
    tempImage = new QImage(loc);
    if (tempImage->isNull()) {
        qWarning()<<"Cannot create Image 2";
    }
    return tempImage;
}

bool CViewObject::InitDisplay(void)
{
    paintingImage.lock();

    delete BackgroundImageBackup;
    delete TopImage;
    delete LeftImage;
    delete RightImage;
    delete BottomImage;
    delete BackImage;

    BackgroundImageBackup = CreateImage(QSize(),BackGroundFname);
    // if high resolution active and Image size < 2 x Object size resize to 2x
    if (hiRes & BackgroundImageBackup->width() < (2*getDX())) {
        delete BackgroundImageBackup;
        BackgroundImageBackup = CreateImage(QSize(2*getDX(),2*getDY()),BackGroundFname);
    }
    internalImageRatio = (float) BackgroundImageBackup->size().width() / getDX();
//    qWarning()<<"internalImageRatio="<<internalImageRatio<<BackgroundImageBackup->size().width()<<getDX();

    if (!TopFname.isEmpty()) {
        TopImage = CreateImage(viewRect(TOPview)*internalImageRatio,TopFname);
        TopImageBackup = TopImage->copy();
    }
    if (!LeftFname.isEmpty()) {
        LeftImage = CreateImage(viewRect(LEFTview)*internalImageRatio,LeftFname);
        LeftImageBackup = LeftImage->copy();
    }
    if (!RightFname.isEmpty()) {
        RightImage = CreateImage(viewRect(RIGHTview)*internalImageRatio,RightFname);
        RightImageBackup = RightImage->copy();
    }
    if (!BottomFname.isEmpty()) {
        BottomImage = CreateImage(viewRect(BOTTOMview)*internalImageRatio,BottomFname);
        BottomImageBackup = BottomImage->copy();
    }
    if (!BackFname.isEmpty()) {
        BackImage = CreateImage(viewRect(BACKview)*internalImageRatio,BackFname);
        BackImageBackup = BackImage->copy();
    }

    paintingImage.unlock();

    return(1);
}

void CViewObject::PreFlip(Direction dir, View targetView)
{

}

void CViewObject::PostFlip()
{

}

void CViewObject::InitView(View v) {

    paintingImage.lock();

//    switch (v) {
//    case TOPview:   if (!TopFname.isEmpty()) TopImage = CreateImage(viewRect(TOPview)*internalImageRatio,TopFname); break;
//    case LEFTview:  if (!LeftFname.isEmpty()) LeftImage = CreateImage(viewRect(LEFTview)*internalImageRatio,LeftFname); break;
//    case RIGHTview: if (!RightFname.isEmpty()) RightImage = CreateImage(viewRect(RIGHTview)*internalImageRatio,RightFname); break;
//    case BOTTOMview:if (!BottomFname.isEmpty()) BottomImage = CreateImage(viewRect(BOTTOMview)*internalImageRatio,BottomFname); break;
//    case BACKviewREV:
//    case BACKview:  if (!BackFname.isEmpty()) {
//            delete BackImage;
//            BackImage = CreateImage(viewRect(BACKview)*internalImageRatio,BackFname);
//        }
//        break;

//    }
    switch (v) {
    case TOPview:   if (TopImage) { delete TopImage; TopImage =  new QImage(TopImageBackup);} break;
    case LEFTview:  if (LeftImage) { delete LeftImage; LeftImage =  new QImage(LeftImageBackup);} break;
    case RIGHTview: if (RightImage) { delete RightImage; RightImage =  new QImage(RightImageBackup);} break;
    case BOTTOMview:if (BottomImage) { delete BottomImage; BottomImage =  new QImage(BottomImageBackup);} break;
    case BACKviewREV:
    case BACKview:  if (BackImage) { delete BackImage; BackImage =  new QImage(BackImageBackup);} break;
    }

    paintingImage.unlock();

}

extern CrenderView *view;
void CViewObject::showObject()
{
    visible = true;
    if (view)
        view->showPObject(this);
    else
        show();
}

void CViewObject::hideObject()
{
    visible = false;
    if (view)
        view->hidePObject(this);
    else
        hide();
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
        qWarning()<<"ERROR DXmm not set";
        return QSize();
    }
    float _ratio = this->getDX()/this->getDXmm();
//    qWarning()<<"ration="<<_ratio;
    switch (v) {

    case FRONTview:
    case BACKviewREV:
    case BACKview: return QSize(this->getDX(),this->getDY());
    case TOPview:
    case BOTTOMview: return QSize(this->getDX(),this->getDZmm()*_ratio);
    case LEFTview:
    case RIGHTview: return QSize(this->getDZmm()*_ratio,this->getDY());
    }
    return QSize();
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
    case BACKviewREV: return BackImage;//new QImage(BackImage->mirrored(true,false));
    }

    return 0;
}

void CViewObject::flip(Direction dir) {
    // IF CONNECTED to printer , exit
//    QList<CPObject *> ConList;
//    mainwindow->pdirectLink->findAllObj(this,&ConList);
//    if (!ConList.isEmpty()) return;
    // Animate close

    if (flipping) return;

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
        case TOPdir: targetView = BACKview; break;; //BACKviewREV; break;
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
    case BACKviewREV:
        switch (dir) {
        case TOPdir: targetView = BOTTOMview; break;
        case LEFTdir: targetView = LEFTview; break;
        case RIGHTdir: targetView = RIGHTview; break;
        case BOTTOMdir: targetView = TOPview; break;
        default: break;
        }
        break;
    }
qWarning()<<"target"<<targetView;
    if ( (targetView != currentView) && getViewImage(targetView) ) {
        qWarning()<<"GO";
        QSize _s = viewRect(currentView).expandedTo(viewRect(targetView));
        delete AnimatedImage;
        AnimatedImage = new QImage(_s*mainwindow->zoom,QImage::Format_ARGB32);

//        QPainter painter(AnimatedImage);
//        painter.drawImage(0,0,*getViewImage(currentView)); //->scaled(this->size(),Qt::IgnoreAspectRatio,Qt::FastTransformation));
//        painter.end();

//        emit updatedPObject(this);
//        changeGeometry(this->posx(),this->posy(),
//                       _s.width()*mainwindow->zoom,_s.height()*mainwindow->zoom);
//        Refresh_Display = true;

    }
    else return;





    PreFlip(dir,targetView);
    targetSize = viewRect(targetView);
    currentFlipDir = dir;

    qWarning()<<"targetdir:"<<targetSize;

    QPropertyAnimation *animation1 = new QPropertyAnimation(this, "angle");
    QPropertyAnimation *animation2 = new QPropertyAnimation(this, "zoom");
     animation1->setDuration(1000);
     animation2->setDuration(1000);

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

     connect(animation1,SIGNAL(valueChanged(QVariant)),this,SLOT(renderAnimation()));
     connect(animation1,SIGNAL(finished()),this,SLOT(endAnimation()));
     group->start();
     flipping = true;

}
#define RATIO .25
void CViewObject::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    if (flipping)
    {
        QPainter painter;

        painter.begin(this);
        painter.setCompositionMode(QPainter::CompositionMode_Source);
        painter.drawImage(0,0,AnimatedImage->scaled(this->size(),Qt::IgnoreAspectRatio,Qt::FastTransformation));
        painter.end();
    }
}

void CViewObject::renderAnimation()
{
    if (flipping)
    {
//        UpdateFinalImage();

        QPainter painter;

//        paintingImage.lock();


//        qWarning()<<"AnimatedImage"<<AnimatedImage;
        if (FinalImage)
        {
            paintingImage.lock();
            AnimatedImage->fill(Qt::transparent);

            QSize _size1 = viewRect(animationView1);
            QSize _size2 = viewRect(animationView2);
            int w = _size1.width() * mainwindow->zoom;
            int h = _size1.height() * mainwindow->zoom;
            int wt = _size2.width() * mainwindow->zoom;
            int ht = _size2.height()* mainwindow->zoom;
//            qWarning()<<"angle:"<<m_angle;
            painter.begin(AnimatedImage);
            painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
//            AnimatedImage->fill(Qt::transparent);

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
                                  getViewImage(animationView1)->scaled(QSize(w,h*(90 -m_angle)/90),Qt::IgnoreAspectRatio,Qt::FastTransformation)
                                  );
                painter.end();
                painter.begin(AnimatedImage);
                painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
//                AnimatedImage->fill(Qt::transparent);
                painter.translate(w/2 ,ht * m_angle/90);
                painter.setTransform(matrix,true);
                matrix2.reset();
                matrix2.rotate(90-m_angle, Qt::XAxis);
                painter.setTransform(matrix2,true);
                painter.drawImage(QPoint(-w/2,-ht * m_angle/90),
                                  getViewImage(animationView2)->scaled(QSize(w,ht * m_angle/90),Qt::IgnoreAspectRatio,Qt::FastTransformation)
                                  );
                break;
            case LEFTdir:
            case RIGHTdir:
                painter.translate(wt*m_angle/90,h/2);
                painter.setTransform(matrix,true);
                matrix2.rotate(-m_angle, Qt::YAxis);
                painter.setTransform(matrix2,true);
                painter.drawImage(QPoint(0,-h/2),
                                  getViewImage(animationView1)->scaled(QSize(w*(90 -m_angle)/90,h),Qt::IgnoreAspectRatio,Qt::FastTransformation)
                                  );
                painter.end();
                painter.begin(AnimatedImage);
                painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
//                AnimatedImage->fill(Qt::transparent);
                painter.translate(wt*m_angle/90,h/2);
                painter.setTransform(matrix,true);
                matrix2.reset();
                matrix2.rotate(90-m_angle, Qt::YAxis);
                painter.setTransform(matrix2,true);
                painter.drawImage(QPoint(-wt * m_angle/90,-h/2),
                                  getViewImage(animationView2)->scaled(QSize(wt * m_angle/90,ht),Qt::IgnoreAspectRatio,Qt::FastTransformation)
                                  );
                break;
            default: break;
            }

            painter.end();

            paintingImage.unlock();

//            qWarning()<<"animation - currentview="<<currentView;
            if (this->size() != AnimatedImage->size()) {
                changeGeometry(this->posx(),this->posy(),AnimatedImage->width(),AnimatedImage->height());
            }
            Refresh_Display = true;
//            update();
        }

//        paintingImage.unlock();
    }
}

void CViewObject::endAnimation(){
    currentView = targetView;
//    currentFlipDir = NONEdir;
    flipping = false;

//    qWarning()<<"endAnimation";
    changeGeometry(this->posx(),
                   this->posy(),
                   viewRect(currentView).width()*mainwindow->zoom,
                   viewRect(currentView).height()*mainwindow->zoom);
    PostFlip();
    Refresh_Display = true;
}

void CViewObject::changeGeometrySize(float newposx,float newposy,float newwidth,float newheight) {


    changeGeometry(newposx,newposy,newwidth,newheight);
    setDX(newwidth);
    setDY(newheight);
}
void CViewObject::changeGeometry(float newposx,float newposy,float newwidth,float newheight) {

    setPosX(newposx);
    setPosY(newposy);
//    setDX(newwidth);
//    setDY(newheight);
    setGeometry(newposx,newposy,newwidth,newheight);
    setMask(mask.scaled(newwidth,newheight).mask());
#ifdef AVOID
    Avoid::Rectangle rectangle(Avoid::Point(newposx-10, newposy-10),
                               Avoid::Point(newposx+newwidth+20, newposy+newheight+20));
    mainwindow->router->moveShape(mainwindow->shapeRefList[this], rectangle);
#endif
    emit sizePObject(this,QSizeF(newwidth,newheight));
}

void CViewObject::mousePressEvent(QMouseEvent *event) {
    // if click on the border
    // width ?
//qWarning()<<"CViewObject::mousePressEvent"<<event;

    if (flipOnEdge) {
        QPoint pts(event->x() , event->y());
        if (pKEYB && pKEYB->KeyClick(pts)) {
            return;
        }

        Direction dir = borderClick(event->pos());

        flip(dir);
    }

}
