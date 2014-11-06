// TODO: move MemSave and MemLoad ti CSlot object


#include <qglobal.h>
#if QT_VERSION >= 0x050000
#   include <QtWidgets>
#   include <QSensorReading>
#else
#   include <QtCore>
#   include <QtGui>
#endif
#include <QMutex>

#ifdef P_AVOID
#include "libavoid.h"
#endif

#include "common.h"
#include "pobject.h"
#include "Inter.h"
#include "Log.h"
#include "pcxxxx.h"
#include "Lcdc.h"
#include "Keyb.h"
#include "cextension.h"
#include "Connect.h"
#include "clink.h"
#include "dialogkeylist.h"
#include "dialogdump.h"
#include "ui/dialogdasm.h"
#include "ui/dialogvkeyboard.h"
#include "dialoganalog.h"
#include "weblinksparser.h"

#include "fluidlauncher.h"

#include "mainwindowpockemul.h"
#include "cloud/cloudwindow.h"

#include "bus.h"

extern QList<CPObject *> listpPObject; 
FILE	*fp_tmp=NULL;


extern MainWindowPockemul* mainwindow;
extern int ask(QWidget *parent,QString msg,int nbButton);
extern bool soundEnabled;

CPObject::CPObject(CPObject *parent):CViewObject(parent)
{
    pPC = (CpcXXXX*) parent;
    Parent	= parent;
    toDestroy = false;


    pKEYB	= 0;
    pTIMER	= 0;
    pLCDC	= 0;
    bus     = 0;
    BackgroundImage = 0;


    flipping = false;
    currentView = FRONTview;
    extensionArray[0] = 0;
    extensionArray[1] = 0;
    extensionArray[2] = 0;
    extensionArray[3] = 0;
    extensionArray[4] = 0;
    frequency	= 0;
    PowerSwitch	= 0;
    setFocusPolicy(Qt::StrongFocus);
    Refresh_Display = false;

    memsize			= 0;
    InitMemValue	= 0x00;

    SnapPts = QPoint(0,0);

    Front = true;
    fullscreenMode = false;
    fillSoundBuffer_old_state = 0;

    forceStackOver = forceStackUnder = false;

    dialogkeylist	= 0;
    dialogdump		= 0;
    dialogdasm = 0;
    dialogVKeyboard = 0;
    Power = false;
    audioBuff.clear();

    contextMenu=menupocket=menuext=menuconfig=menucpuspeed=menulcd=menulink=menuunlink=menuweblink=menuDocument=0;
    ioFreq = 0;
    off =true;
    closed = false;
    resetAt = 0;
    hardresetAt = 0;
    hardreset = false;

    grabGesture(Qt::TapAndHoldGesture);

    // ERROR MESSAGE
    connect( this,SIGNAL(msgError(QString)),mainwindow,SLOT(slotMsgError(QString)));

}


CPObject::~CPObject()
{
    if (dialogdasm) delete dialogdasm;


    //FIXME: When extension are connected , they share the timer with the host.
    //delete pTIMER;
    if (pLCDC) delete pLCDC;
    if (bus) delete bus;
	

    delete BackgroundImage;

    delete contextMenu;

//    delete extensionArray[0];
//    delete extensionArray[1];
//    delete extensionArray[2];
//    delete extensionArray[3];
//    delete extensionArray[4];
}



void CPObject::serialize(QXmlStreamWriter *xml,int id) {
    xml->writeStartElement("object");
    xml->writeAttribute("name", getName());
    xml->writeAttribute("id", QString("%1").arg(id));
    xml->writeAttribute("front",Front?"true":"false");
    xml->writeAttribute("power",Power?"true":"false");
        xml->writeStartElement("position");
        xml->writeAttribute("x", QString("%1").arg(posx()));
        xml->writeAttribute("y", QString("%1").arg(posy()));
        xml->writeAttribute("width", QString("%1").arg(getDX()));
        xml->writeAttribute("height", QString("%1").arg(getDY()));
        xml->writeAttribute("rotation", QString("%1").arg(getRotation()));
        xml->writeEndElement(); // position
        this->SaveSession_File(xml);
    xml->writeEndElement(); // object
    qWarning()<<"end serialize";
}


QRectF CPObject::RectWithLinked(void) {
    QRectF r(rect());
    // Search all conected objects then compute resulted rect
    QList<CPObject *> ConList;
//    ConList.append(this);
    mainwindow->pdirectLink->findAllObj(this,&ConList);
    for (int i=0;i<ConList.size();i++)
    {
        r = r.united(ConList.at(i)->rect());
    }
    return r;
}

extern void m_addShortcut(QString name,QString param);

void CPObject::createShortcut() {
#ifdef Q_OS_ANDROID
    m_addShortcut(getName(),"-r \""+getName()+"\"");
#endif
}

void CPObject::maximizeWidth()
{
    if (mainwindow->zoom <= 100) {
        // Compute global rect
        QRectF rs = RectWithLinked();
        float rw= 100*mainwindow->centralwidget->rect().width()/rs.width();
        float rh= 100*mainwindow->centralwidget->rect().height()/rs.height();
        int r = MAX(rw,rh);
        if (r>100) {
            mainwindow->doZoom(QPoint(0,0),1,r-mainwindow->zoom);
            //move to upper left
            // Fetch all_object and move them
            rs = RectWithLinked();
            mainwindow->MoveAll(- rs.topLeft());
            fullscreenMode = true;
            grabGesture(Qt::SwipeGesture);
//            qWarning()<<"GrabGesture SwipeGesture";
        }
    }
    else {
        mainwindow->doZoom(QPoint(0,0),-1,mainwindow->zoom-100);
        fullscreenMode = false;
        ungrabGesture(Qt::SwipeGesture);
//        qWarning()<<"unGrab Gesture SwipeGesture";
    }
}

void CPObject::maximizeHeight()
{
    if (mainwindow->zoom <= 100) {
        // Compute global rect
        QRectF rs = RectWithLinked();
        int rw= 100*mainwindow->centralwidget->rect().width()/rs.width();
        int rh= 100*mainwindow->centralwidget->rect().height()/rs.height();
        int r = MIN(rw,rh);
        if (r>100) {
            mainwindow->doZoom(QPoint(0,0),1,r-mainwindow->zoom);
            //move to upper left
            // Fetch all_object and move them
            rs = RectWithLinked();
            mainwindow->MoveAll(- rs.topLeft());
            fullscreenMode = true;
        }
    }
    else {
        mainwindow->doZoom(QPoint(0,0),-1,mainwindow->zoom-100);
        fullscreenMode = false;
    }
}

void CPObject::FindAllLinked(CPObject * search, QList<CPObject *> * liste) {
    int _size = liste->size();
#ifdef AVOID
    mainwindow->pdirectLink->findAllObj(search,liste);
#else
    mainwindow->pdirectLink->findAllObj(search,liste,false);
#endif

    if (liste->size()>_size) {
        for (int i=_size;i<liste->size();i++){
            FindAllLinked(liste->at(i), liste);
        }
    }
}

void CPObject::MoveWithLinkedAbs(QPointF p) {
    // Search all conected objects then move them
    QList<CPObject *> ConList;
    ConList.append(this);

    QPointF delta = p - pos();
    FindAllLinked(this,&ConList);

    for (int i=0;i<ConList.size();i++)
    {
        ConList.at(i)->MoveRel(delta);

    }

    // YES, but how to connect closely extendions ?
//    for (int i=0;i<listpPObject.size();i++) {
//        CPObject * locpc = listpPObject.at(i);

//        if ((locpc != this) && !locpc->geometry().intersect(this->geometry()).isNull()) locpc->MoveWithLinked(p);
    //    }
}

void CPObject::MoveWithLinkedRel(QPointF p) {
    // Search all conected objects then move them
    QList<CPObject *> ConList;
    ConList.append(this);

    FindAllLinked(this,&ConList);

    for (int i=0;i<ConList.size();i++)
    {
        ConList.at(i)->MoveRel(p);

    }

    // YES, but how to connect closely extendions ?
//    for (int i=0;i<listpPObject.size();i++) {
//        CPObject * locpc = listpPObject.at(i);

//        if ((locpc != this) && !locpc->geometry().intersect(this->geometry()).isNull()) locpc->MoveWithLinked(p);
    //    }
}

void CPObject::setfrequency(int f)
{
    frequency = f;
    if (pTIMER) {
        pTIMER->state = pTIMER->currentState();
    }
}



bool CPObject::init()
{
    startKeyDrag = false;
    startPosDrag = false;
    setMouseTracking(true);
    resize(getDX(),getDY());
    move(QPoint(posx(),posy()));
    setAttribute(Qt::WA_AlwaysShowToolTips,true);

    AddLog(LOG_MASTER,tr("Memory initialisation"));
    if (memsize>0)  {
        if ((mem=(BYTE *)malloc(memsize*sizeof(BYTE)))==NULL) return(0);                /* alloc main ram */
        ClearRam(InitMemValue);

        AddLog(LOG_MASTER,tr("Memory loading nb slot:%1").arg(SlotList.size()));
        for (int s=0; s < SlotList.size(); ++s)
        {
            if (SlotList[s].getType() == CSlot::ROM)   Mem_Load(s);
        }
    }
        return true;
}

bool CPObject::exit()
{
    if (pKEYB)	pKEYB->exit();
    if (pLCDC)  pLCDC->exit();
    if (pTIMER) pTIMER->exit();
    if (dialogVKeyboard) { dialogVKeyboard->close(); dialogVKeyboard->deleteLater(); }
    if (dialogdump) { dialogdump->close(); dialogdump->deleteLater(); }
    if (dialogdasm) { dialogdasm->close(); dialogdasm->deleteLater(); }

	return true;
}

quint64 CPObject::runRange(quint64 step) {
    if (pTIMER) {

        quint64 t = pTIMER->state;

        while (/*!off &&*/ ((pTIMER->state - t) < step)) {
            run();

#if 0
            // refresh display when disp_on switch to 1
            if (pLCDC)
            {
                if (dynamic_cast<CpcXXXX *>(this) )
                {
                    CpcXXXX *tmpPC = (CpcXXXX*)this;
                    if (tmpPC->getdisp_onRaised()) {
                        tmpPC->pLCDC->disp();
                        if (pLCDC->Refresh) Refresh_Display = true;
                    }
                }
            }
#endif
        }
        return (pTIMER->state - t);
    }
    else
    {
        run();
        return 0;
    }
    return 0;
}

bool CPObject::run(void){
//    if (fullscreenMode) {
//        if (QSensorReading *reading = mainwindow->sensor->reading()) {
//            qreal x = reading->property("x").value<qreal>();
//            qreal y = reading->property("y").value<qreal>();
//            qreal z = reading->property("z").value<qreal>();
//            //qreal y = reading->value(1).value<qreal>();
//            //                    qWarning()<<"x="<<x<<"  y="<<y<<"  z="<<z;
//            if (abs((int)x)>5) {
//                if (x<0) {
//                    // move left
//                    if (posx()<50) {
//                        MoveWithLinked(QPoint(10,0));//centralwidget->rect().width() - CurrentpPC->width()*zoom/100,0));
//                        update();
//                    }
//                }
//                else {
//                    // move right
//                    if ((posx()+width()) > mainwindow->centralWidget()->rect().width()) {
//                        MoveWithLinked(QPoint( -10,0));//CurrentpPC->width()*zoom/100 - centralwidget->rect().width(),0));
//                        update();
//                    }
//                }
//            }
//        }
//    }

    if ((resetAt>0) && (pTIMER->state >= resetAt)) {
        qWarning()<<"Reset";
        Reset();
        resetAt = 0;
    }
    if ((hardresetAt>0) && (pTIMER->state >= hardresetAt)) {
        qWarning()<<"HardReset";
        // FIXME: cannot call init from CPU Thread. Use a SIGNAL
        hardreset = true;
        init();
        Reset();
        hardresetAt = 0;
    }
   return true;
}

#define SAMPLERATE 8000
#define BUFFLEN 500

#ifndef NO_SOUND
void CPObject::audioStateChanged(QAudio::State state)
{
    if (state == QAudio::IdleState) {

    }
    //AddLog(LOG_TEMP,tr("state = %1").arg(state))
//    qWarning() << "state = " << state;
}
#endif
int CPObject::initsound()
{
    int DataFrequencyHz = 8000;
    //int BufferSize      = 800;
#ifndef NO_SOUND
    QAudioDeviceInfo m_device(QAudioDeviceInfo::defaultOutputDevice());
    m_format.setSampleRate(DataFrequencyHz);
    m_format.setChannelCount(1);
    m_format.setSampleSize(8);
    m_format.setCodec("audio/pcm");
    m_format.setByteOrder(QAudioFormat::LittleEndian);
    m_format.setSampleType(QAudioFormat::UnSignedInt);

    QAudioDeviceInfo info(QAudioDeviceInfo::defaultOutputDevice());
    if (!info.isFormatSupported(m_format)) {
        qWarning() << "Default format not supported - trying to use nearest";
        m_format = info.nearestFormat(m_format);
    }
    //delete m_audioOutput;
    m_audioOutput = 0;
    m_audioOutput = new QAudioOutput(m_device, m_format, this);
    //connect(m_audioOutput, SIGNAL(notify()), SLOT(notified()));
    connect(m_audioOutput, SIGNAL(stateChanged(QAudio::State)), SLOT(audioStateChanged(QAudio::State)));
    m_audioOutput->setBufferSize(1000);

    m_output = m_audioOutput->start();
//    int p = m_audioOutput->periodSize();
//    qWarning()<<p;
#endif
    return 1;
}

int CPObject::exitsound()
{
    return 1;
}


//FIXME: The piezo doesn't produce sounf for frequency < 1Khz
void CPObject::fillSoundBuffer(BYTE val)
{
    if (!soundEnabled) return;

#ifndef NO_SOUND
//    quint64 new_state;
    quint64 delta_state;
	 
    if ( (fillSoundBuffer_old_state == 0) ||
         (fillSoundBuffer_old_state > pTIMER->state)) {
        fillSoundBuffer_old_state = pTIMER->state;
    }

    if (getfrequency()==0) {
//        mainwindow->audioMutex.lock();
        audioBuff.append(val);
//        mainwindow->audioMutex.unlock();
        return;
    }

//    qWarning("freq:%i",getfrequency());
//	new_state = pTIMER->state;
    delta_state = pTIMER->state - fillSoundBuffer_old_state;
//    if (delta_state < 0) fillSoundBuffer_old_state=new_state;
	// Calculate nb of state to skip corresponding to the CPU frequency
    quint64 wait = ((pTIMER->CPUSpeed*getfrequency()) / SAMPLERATE );
//	fprintf(fp_tmp,"%s\n",tr("%1 : wait = %2  -  delta=%3  new:%4 - old:%5  ptimer:%6").arg(getName()).arg(wait).arg(delta_state).arg(new_state).arg(fillSoundBuffer_old_state).arg((int)pTIMER).toLocal8Bit().data());
    if (delta_state >= wait)
    {
        //AddLog(LOG_TEMP,tr("delta:%1   wait:%2").arg(delta_state).arg(wait));
//        mainwindow->audioMutex.lock();
        while ((pTIMER->state - fillSoundBuffer_old_state) >= wait)
        {
            audioBuff.append(val);

            fillSoundBuffer_old_state += wait;
            //delta_state -= wait;
        }

        int ps = m_audioOutput->periodSize();
        //AddLog(LOG_TEMP,tr("buff:%1   ps:%2").arg(audioBuff.size()).arg(ps));

        if (audioBuff.size() >= (2*ps)) {
            m_output->write(audioBuff.constData(),ps);
//            AddLog(LOG_TEMP,tr("audiobuffsize:%1  outbuffer:%2").arg(audioBuff.size()).arg(m_audioOutput->bufferSize()-m_audioOutput->bytesFree()));
            //audioBuff.clear();
            audioBuff.remove(0,ps);
        }
        else {
            //FIXME: buffer underrun
#if 0
            if((m_audioOutput->bufferSize()-m_audioOutput->bytesFree()) < ps) {
                AddLog(LOG_TEMP,tr("sound1 size:%1    free:%2   diff:%3   ps:%4").arg(m_audioOutput->bufferSize()).arg(m_audioOutput->bytesFree()).arg(m_audioOutput->bufferSize()-m_audioOutput->bytesFree()).arg(ps));
                QByteArray fill(ps-(m_audioOutput->bufferSize()-m_audioOutput->bytesFree()),0);
                m_output->write(fill.constData(),fill.size());
            }
#endif
        }
//        mainwindow->audioMutex.unlock();
    }
#endif
}



void CPObject::SwitchFrontBack(QPoint point) {
    if (Front)
    {
        // calculate the new origine
        int newposx = point.x() + (pos().x()-point.x())/4;
        int newposy = point.y() + (pos().y()-point.y())/4;

        QPropertyAnimation *animation = new QPropertyAnimation(this, "geometry");
        animation->setDuration(1000);
        animation->setStartValue(QRect(posx(),posy(),getDX()*mainwindow->zoom/100,getDY()*mainwindow->zoom/100));
        animation->setEndValue(QRect(newposx,newposy,getDX()*mainwindow->zoom/100/4,getDY()*mainwindow->zoom/100/4));
        animation->setEasingCurve(QEasingCurve::OutBounce);
        animation->start();
        this->setPosX(newposx);
        this->setPosY(newposy);

    }
    else
    {
        // calculate the new origine
        int newposx = point.x() + (pos().x()-point.x())*4;
        int newposy = point.y() + (pos().y()-point.y())*4;

//        QPropertyAnimation *animation = new QPropertyAnimation(this, "geometry");
//        animation->setDuration(1000);
//        animation->setStartValue(this->rect());
//        animation->setEndValue(QRect(newposx,newposy,Pc_DX,Pc_DY));
//        animation->setEasingCurve(QEasingCurve::OutBounce);
//        animation->start();
        setGeometry(newposx,newposy,getDX()*mainwindow->zoom/100,getDY()*mainwindow->zoom/100);
        this->setPosX(newposx);
        this->setPosY(newposy);
//            QPoint newpos = pos() - mainwindow->pos();
//            setParent(mainwindow);
//            move(newpos);

    }
    Front = ! Front;
}

bool CPObject::event(QEvent *event)
 {

     if (event->type() == QEvent::Gesture) {
         if (QGesture *swipe = (static_cast<QGestureEvent*>(event))->gesture(Qt::SwipeGesture)) {
             swipeTriggered(static_cast<QSwipeGesture *>(swipe));
             event->accept();
         }
         else {
             if (QGesture *tap = (static_cast<QGestureEvent*>(event))->gesture(Qt::TapAndHoldGesture)) {
                 const QPoint pos = (static_cast<QTapAndHoldGesture *>(tap))->position().toPoint();
                 //             qWarning()<< (static_cast<QTapAndHoldGesture *>(tap))->timeout()<<pos<<tap->gestureType()<<tap->state();
                 if (tap->state() == Qt::GestureStarted) {
                     QContextMenuEvent *cme = new QContextMenuEvent(
                                 QContextMenuEvent::Mouse,
                                 pos,
                                 (pos));
                     //contextMenuEvent(cme);
                     QApplication::sendEvent(this,cme);

                     setCursor(Qt::ArrowCursor);

                     event->accept();
                 }
             }
         }
         event->accept();
         return true;
     }
     return QWidget::event(event);
 }

void CPObject::swipeTriggered(QSwipeGesture *gesture)
{
    qWarning()<<"SWIPE:";
    if (gesture->state() == Qt::GestureFinished) {
        if (fullscreenMode) {
            if (gesture->horizontalDirection() == QSwipeGesture::Left) {
                // move to left;
                qWarning()<<"SWIPE LEFT";
            }
            else {
                // move to right
                qWarning()<<"SWIPE LEFT";
            }

            Refresh_Display = true;
            update();
        }
    }
}

void CPObject::wheelEvent(QWheelEvent *event) {

    QPoint point;
    if (Parent) {
        point = event->pos();
    }
    else {
        point = event->globalPos();
    }

    float delta = event->delta()/12;

    mainwindow->doZoom(point,delta);
    event->accept();

}

void CPObject::slotDoubleClick(QPoint pos) {
    if ((pKEYB) &&(pKEYB->KeyClick(pos))) {
//        qWarning()<<"keyclick";
        // Send thee mouseclick event twice
        QMouseEvent *e=new QMouseEvent(QEvent::MouseButtonPress, pos, Qt::LeftButton, Qt::LeftButton,Qt::NoModifier);
        QApplication::sendEvent(this, e);
        delete e;
        return;
    }


    if (mainwindow->zoom <= 100) {
        // Compute global rect
        QRectF rs = RectWithLinked();
        int rw= 100*mainwindow->centralwidget->rect().width()/rs.width();
        int rh= 100*mainwindow->centralwidget->rect().height()/rs.height();
        int r = MIN(rw,rh);
        if (r>100) {
            mainwindow->doZoom(pos,1,r-mainwindow->zoom);
            //move to upper left
            // Fetch all_object and move them
            rs = RectWithLinked();
            mainwindow->MoveAll(- rs.topLeft());
        }
    }
    else {
        mainwindow->doZoom(pos,-1,mainwindow->zoom-100);
    }

}

void CPObject::mouseDoubleClickEvent(QMouseEvent *event)
{
    QPoint pts(event->x() , event->y());
    slotDoubleClick(pts);
}


extern void Vibrate();
void CPObject::mousePressEvent(QMouseEvent *event)
{
//    qWarning()<<"CPObject::mousePressEvent"<<event;
    CViewObject::mousePressEvent(event);

    if (event->button() != Qt::LeftButton) {
        event->accept();
        return;
    }

    setFocus();

    if (event->modifiers() == Qt::MetaModifier) return;

    QPoint pts(event->x() , event->y());

//    if (dialogkeylist)
//    {
//        // look keyFound
//        if ( dialogkeylist->keyIter->Rect.contains(pts) )
//        {
//            // keydef found start drag mode
//            // Change mouse pointer
//            qWarning()<<"startKeyDrag";
//            setCursor(Qt::SizeAllCursor);
//            startKeyDrag = true;
//            KeyDrag = event->globalPos();
//            return;
//        }
//    }

    if (pKEYB)
    {
//        qWarning()<<"ok"
;        pKEYB->LastKey = pKEYB->KeyClick(pts);
        if (pKEYB->LastKey != 0)  {
            ungrabGesture(Qt::TapAndHoldGesture);
//            qWarning()<<"lastKey="<<pKEYB->LastKey;
        }


        pKEYB->lastMousePressedKey = pKEYB->LastKey;
        if (pKEYB->LastKey) pKEYB->keyPressedList.append(pKEYB->LastKey);

        switch (pKEYB->LastKey) {
        case K_OF :
            Vibrate();
            slotPower();
            grabGesture(Qt::TapAndHoldGesture);
            return;
            break;
        case K_BRK :
        case K_POW_ON :qWarning()<<"GOGO"; Vibrate();TurnON(); break;
        case K_POW_OFF: Vibrate();Power = false;TurnOFF();grabGesture(Qt::TapAndHoldGesture);return;break;
        case K_CLOSE: Vibrate();TurnCLOSE();break;
        }


        if (pKEYB->LastKey != 0)
        {
            ComputeKey(KEY_PRESSED,pKEYB->LastKey);
            Vibrate();
        }

        grabGesture(Qt::TapAndHoldGesture);
        if (pKEYB->LastKey != 0) {
            event->accept();
            return;
        }
    }

    // NO KEY CLICK Global pobject drag mode

//    _gestureHandler->handleEvent( event );

    // raise all connected object and then manage Z-order between them
    raise();
    QList<CPObject *> list;
    mainwindow->pdirectLink->findAllObj(this,&list);
    for (int i=0;i<list.size();i++) {
        list.at(i)->raise();
    }
    list.clear();

    manageStackPos(&list);
    emit stackPosChanged();

    if ( (parentWidget() != mainwindow->centralwidget) //mainwindow
         && (parentWidget() != 0))
    {
        QApplication::sendEvent(parentWidget(), event);
    }
    else
    {
//        if (!fullscreenMode)
        {
            setCursor(Qt::ClosedHandCursor);	// Change mouse pointer
            startPosDrag = true;
            PosDrag = event->globalPos();
        }
    }

    event->accept();
}


void CPObject::mouseMoveEvent( QMouseEvent * event )
{
//    if (!fullscreenMode)
    {
        //    _gestureHandler->handleEvent( event );
//        if (dialogkeylist)
//        {
//            if (startKeyDrag)
//            {
//                QPoint delta(event->globalPos() - KeyDrag);
//                dialogkeylist->keyIter->Rect.adjust(delta.x(),delta.y(),delta.x(),delta.y());
//                pKEYB->modified = true;
//                KeyDrag = event->globalPos();
//                emit updatedPObject(this);
//                Refresh_Display = true;
//                update();
//                event->accept();
//                return;
//            }
//        }

        if (startPosDrag)
        {
            // Move all conected objects
            QPoint delta(event->globalPos() - PosDrag);
            if (delta.manhattanLength() > 5) {
                if (fullscreenMode) {
                    delta.setY(0);
                    if (delta.x()>0) delta.setX(MIN(delta.x(),50-posx()));
                    if (delta.x()<0) delta.setX(MAX(delta.x(),mainwindow->centralwidget->width()- (posx()+width())));

                }
                MoveWithLinkedRel(delta);
                PosDrag = event->globalPos();
                Refresh_Display = true;
                update();
                event->accept();
                return;
            }
        }

#ifndef Q_OS_ANDROID
        if (pKEYB)
        {
            QPoint pts(event->x() , event->y());
            if (pKEYB->KeyClick(pts))
            {
                setCursor(Qt::PointingHandCursor);
                setToolTip(pKEYB->KeyString(pts));
                event->accept();
            }
            else
            {
                setCursor(Qt::ArrowCursor);
            }
        }
#endif

        if ( (parentWidget() != mainwindow->centralwidget)
             && (parentWidget() != 0))
        {
            QApplication::sendEvent(parentWidget(), event);
        }
    }
}

#define SNAPRANGE 20

QList<Cconnector *> CPObject::nearConnectors(Cconnector *refConnector,qint8 snaprange) {
    // Compare snap between the refConnector and all free object connectors
    QList<Cconnector *> retList;
    for (int i=0;i<ConnList.size();i++) {
        if (!mainwindow->pdirectLink->isLinked(ConnList.at(i)) &&
            Cconnector::arePluggable(ConnList.at(i),refConnector)) {
            Cconnector *c = ConnList.at(i);
            // ATTENTION : POSITIONN DE L'OBJECT + SNAP !!!!!!
            CPObject *p = refConnector->Parent;
            qreal range = QLineF(p->pos()+refConnector->getSnap()*mainwindow->zoom/100,this->pos()+c->getSnap()*mainwindow->zoom/100).length();
            if (range < snaprange) {
                retList.append(ConnList.at(i));
            }
        }
    }
    return retList;
}

void CPObject::mouseReleaseEvent(QMouseEvent *event)
{
//    _gestureHandler->handleEvent( event );
    // if a connector is free
    // if an object with free connector is "near"
    // propose to autolink


    // Fetch all object
    for (int k = 0; k < listpPObject.size(); k++)
    {
        // fetch all others objects FREE connectors
        if (listpPObject.at(k) != this) {
            // Fect object connectors
            for (int c=0; c < listpPObject.at(k)->ConnList.size(); c++) {
                if (!mainwindow->pdirectLink->isLinked(listpPObject.at(k)->ConnList.at(c))) {
                    // If not already linked
                    QList<Cconnector *> nearList = nearConnectors(listpPObject.at(k)->ConnList.at(c),SNAPRANGE);
                    for (int r=0; r<nearList.size();r++) {
//                        qWarning("pre box :%i",mainwindow);
#ifdef EMSCRIPTEN
                        QMessageBox *msgBox = new QMessageBox(this);
                         msgBox->setText(nearList.at(r)->Desc + " linked to ["+ listpPObject.at(k)->getName()+"]"+listpPObject.at(k)->ConnList.at(c)->Desc);
                         msgBox->show();
#else
                        if (ask(mainwindow, "Do you want to link those two materials ?\n"+
                                                  nearList.at(r)->Desc + "--> ["+ listpPObject.at(k)->getName()+"]"+listpPObject.at(k)->ConnList.at(c)->Desc,
                                                  2) == 1)
#endif
                        {
                            // The user clicked the Yes button or pressed Enter
                            // Connect

                            MoveWithLinkedRel(listpPObject.at(k)->pos() + listpPObject.at(k)->ConnList.at(c)->getSnap()*mainwindow->zoom/100 - pos() - nearList.at(r)->getSnap()*mainwindow->zoom/100);
                            mainwindow->pdirectLink->addLink(listpPObject.at(k)->ConnList.at(c),nearList.at(r),true);
                            QList<CPObject *> list;
                            listpPObject.at(k)->manageStackPos(&list);
                            emit stackPosChanged();
                        }
//                        qWarning("post box :%i",mainwindow);
                    }
                }
            }
        }
    }

    startKeyDrag = false;
    startPosDrag = false;
    setCursor(Qt::ArrowCursor);
    if (pKEYB) {
        pKEYB->keyPressedList.removeAll(pKEYB->lastMousePressedKey);
        ComputeKey(KEY_RELEASED,pKEYB->lastMousePressedKey);
        pKEYB->lastMousePressedKey = 0;
        pKEYB->LastKey = 0;
    }

    if ( (parentWidget() != mainwindow->centralwidget)
        && (parentWidget() != 0))
    {
        QApplication::sendEvent(parentWidget(), event);
    }
    event->accept();
}

void CPObject::raise() {
    QWidget::raise();
}

void CPObject::TurnCLOSE()
{

}

//FIXME: StackPosition managed by widget
//       If we want to be independent from QWidget, need to implement
//       stackUnder function

void CPObject::manageStackPos(QList<CPObject *> *l) {
    // fetch connectors connected
    // for each connector
    //      if male then stackunder and recursive
    //
    // Keep track of already computed objects
    if (l->contains(this)) return;
    l->append(this);

    for (int i=0;i < ConnList.size();i++) {
        Cconnector * conn = ConnList.at(i);
        Cconnector * conn2 = mainwindow->pdirectLink->Linked(conn);
        if (conn2) {
            CPObject * linkedPC = (CPObject *) (conn2->Parent);
            if (forceStackUnder) {
                linkedPC->stackUnder(this);
                stackUnder(linkedPC);
                linkedPC->manageStackPos(l);
            }
            else {
                if ((conn->getGender() == true)) {    // If Male stackUnder
                    linkedPC->stackUnder(this);
                    stackUnder(linkedPC);
                    linkedPC->manageStackPos(l);
                }
                else {
                    linkedPC->stackUnder(this);
                    linkedPC->manageStackPos(l);       // Doesn't work if we don't manage a queue
                }
            }
            if (forceStackOver) {
                linkedPC->stackUnder(this);
                linkedPC->manageStackPos(l);       // Doesn't work if we don't manage a queue
            }
            else {
                if ((conn->getGender() == true)) {    // If Male stackUnder
                    linkedPC->stackUnder(this);
                    stackUnder(linkedPC);
                    linkedPC->manageStackPos(l);
                }
                else {
                    linkedPC->stackUnder(this);
                    linkedPC->manageStackPos(l);       // Doesn't work if we don't manage a queue
                }
            }
        }
    }

}

bool CPObject::SaveSession_File(QXmlStreamWriter *xmlOut)
{
    Q_UNUSED(xmlOut)

    return true;
}

bool CPObject::LoadSession_File(QXmlStreamReader *)
{
    return true;
}

			
qreal CPObject::RangeFrom(CPObject * target)
{
	qreal range = QLineF(pos() + SnapPts,target->pos() + target->SnapPts).length();
    return range;
}


#define KEY(c)	( pKEYB->keyPressedList.contains(TOUPPER(c)) || pKEYB->keyPressedList.contains(c) || pKEYB->keyPressedList.contains(TOLOWER(c)))

void CPObject::ComputeKey(CPObject::KEYEVENT ke, int scancode)
{
    Q_UNUSED(ke)
    Q_UNUSED(scancode)

    if (KEY(K_SHARP11PINS)) {
        FluidLauncher *launcher = new FluidLauncher(mainwindow,
                                     QStringList()<<P_RES(":/pockemul/configExt.xml"),
                                     FluidLauncher::PictureFlowType,QString(),
                                     "Sharp_11");
        launcher->show();
    }
}

void CPObject::paintEvent(QPaintEvent *event)
{
#ifdef Q_OS_ANDROID
#define TRANSFORM Qt::FastTransformation
#else
#define TRANSFORM Qt::SmoothTransformation
#endif
    if (flipping) {
        CViewObject::paintEvent(event);
    }
    else {

        UpdateFinalImage();
        QPainter painter;

        painter.begin(this);
        QImage *_target = getViewImage(currentView);
        if (_target)
        {
            painter.drawImage(QPoint(0,0), _target->scaled(this->size(),Qt::IgnoreAspectRatio,TRANSFORM));
        }

        if (dialogkeylist)
        {
            painter.setPen(QPen(Qt::red));
            QList<QRect> _rectList = dialogkeylist->getkeyFoundRect();
            qWarning()<<"drawRect:"<<_rectList;

            for (int i=0; i<_rectList.count();i++) {
                painter.drawRect(_rectList.at(i));
            }
        }


        //    if (getfrequency()>0) {
        //        painter.drawText(10,100,QString("").setNum((int)rate)+"%");
        //    }
        painter.end();
    }

}

void CPObject::keyReleaseEvent(QKeyEvent * event )
{
//    if (event->isAutoRepeat()) return;

	if (!pKEYB) return;	// if no Keyboard then return;
    pKEYB->isShift = event->modifiers() &  Qt::ShiftModifier;//(QApplication::keyboardModifiers() == Qt::ShiftModifier);
    pKEYB->isCtrl = (QApplication::keyboardModifiers() == Qt::ControlModifier);

    int _key = mapKey(event);
    pKEYB->keyPressedList.removeAll(_key);
    ComputeKey(KEY_RELEASED,_key);

    pKEYB->LastKey = 0;

}

void CPObject::TurnON() {

}

void CPObject::TurnOFF() {}

int CPObject::mapKey(QKeyEvent * event) {
    int key = 0;
//    if ( (event->key() & 0x2000000) == 0x2000000 ) pKEYB->isShift = true;
//    if ( (event->key() & 0x4000000) == 0x4000000 ) pKEYB->isCtrl = true;
    switch (event->key() & 0x1FFFFFF) {
//        case K_SHIFT_DOWN_MOD:  pKEYB->isShift = true;  event->accept(); break;
//        case K_CTRL_DOWN_MOD:   pKEYB->isCtrl = true;   event->accept(); break;
//        case K_SHIFT_UP_MOD:    pKEYB->isShift = false; event->accept(); break;
//        case K_CTRL_UP_MOD:     pKEYB->isCtrl = false;  event->accept(); break;
        case Qt::Key_Shift:		key = K_SHT;	event->accept();	break;
        case Qt::Key_Control:	key = K_CTRL;	event->accept();	break;
        case Qt::Key_AltGr:     key = 0;        event->accept();    break;
        case Qt::Key_Alt:       key = 0;        event->accept();    break;
//        case Qt::Key_AsciiTilde:       key = 0;        event->accept();    break;
        case Qt::Key_Return:	key = K_RET;	event->accept();	break;
        case Qt::Key_Delete:	key = K_DEL;	event->accept();	break;
        case Qt::Key_Insert:	key = K_INS;	event->accept();	break;
        case Qt::Key_QuoteLeft: key = K_QUOTE;  event->accept();    break;
        case Qt::Key_Tab:       key = K_TAB;    event->accept();    break;
        case Qt::Key_Space:		key = ' ';		event->accept();	break;
        case Qt::Key_Period:	key = '.';		event->accept();	break;
        case Qt::Key_Plus:		key = '+';		event->accept();	break;
        case Qt::Key_Minus:		key = '-';		event->accept();	break;
        case Qt::Key_Asterisk:	key = '*';		event->accept();	break;
        case Qt::Key_Slash:		key = '/';		event->accept();	break;
        case Qt::Key_Equal:		key = '=';		event->accept();	break;
        case Qt::Key_QuoteDbl:	key = '"';		event->accept();	break;
        case Qt::Key_Semicolon:	key = ';';		event->accept();	break;
        case Qt::Key_Comma:		key = ',';		event->accept();	break;
        case Qt::Key_ParenLeft: key = '(';       event->accept();    break;
        case Qt::Key_ParenRight:key = ')';       event->accept();    break;
        case Qt::Key_Backspace:	key = K_BS;		event->accept();	break;
        case Qt::Key_Dollar:	key = '$';		event->accept();	break;
        case Qt::Key_Ampersand:	key = '&';		event->accept();	break;
        case Qt::Key_CapsLock:	key = K_SML;		event->accept();	break;
        case Qt::Key_Left:		key = K_LA;		event->accept();	break;
        case Qt::Key_Right:		key = K_RA;		event->accept();	break;
        case Qt::Key_Up:		key = K_UA;		event->accept();	break;
        case Qt::Key_Down:		key = K_DA;		event->accept();	break;
        case Qt::Key_F1:		key = K_F1;		event->accept();	break;
        case Qt::Key_F2:		key = K_F2;		event->accept();	break;
        case Qt::Key_F3:		key = K_F3;		event->accept();	break;
        case Qt::Key_F4:		key = K_F4;		event->accept();	break;
        case Qt::Key_F5:		key = K_F5;		event->accept();	break;
        case Qt::Key_F6:		key = K_F6;		event->accept();	break;
        case Qt::Key_F7:		key = K_F7;		event->accept();	break;
        case Qt::Key_F8:		key = K_CLR;		event->accept();	break;
        case Qt::Key_F9:		key = K_DEF;		event->accept();	break;
        case Qt::Key_F10:		key = K_MOD;		event->accept();	break;
        case Qt::Key_F11:		key = K_BRK;		event->accept();	break;
    default: key = event->key() & 0x1FFFFFF; event->accept();
//        qWarning()<<"key def:"<<key;
        }
//    if ( (event->key() >= 0x41) && (event->key() <= 0x5A) ) { key = event->key(); event->accept();	}
//    if ( (event->key() >= 0x30) && (event->key() <= 0x39) ) { key = event->key(); event->accept();	}

//    qWarning()<<"key:"<<key;
    return key;
}

void CPObject::keyPressEvent (QKeyEvent * event )
{

//    if (event->isAutoRepeat()) return;
	if (!pKEYB) return;	// if no Keyboard then return;

    pKEYB->isShift = event->modifiers() &  Qt::ShiftModifier;//QApplication::keyboardModifiers() == Qt::ShiftModifier);
    pKEYB->isCtrl = (QApplication::keyboardModifiers() == Qt::ControlModifier);


    pKEYB->LastKey = mapKey(event);

    if (pKEYB->LastKey>0) {
        // Add th key to Key pressed buffer
        if (!pKEYB->keyPressedList.contains(pKEYB->LastKey)) pKEYB->keyPressedList.append(pKEYB->LastKey);
        ComputeKey(KEY_PRESSED,pKEYB->LastKey);
    }
    else {
        event->ignore();
        QWidget::keyPressEvent(event);
    }
}

void CPObject::focusInEvent ( QFocusEvent *  )
{

}

void CPObject::focusOutEvent (QFocusEvent *)
{
}



void CPObject::contextMenuEvent ( QContextMenuEvent * event )
{
//    qWarning()<<"contextMenuEvent";
//    Vibrate();

    delete contextMenu;
    contextMenu = new QMenu(this);
    BuildContextMenu(contextMenu);

    contextMenu->popup(event->globalPos () );
    event->accept();
}

void CPObject::BuildContextMenu(QMenu * menu)
{
    Vibrate();
#ifdef Q_OS_ANDROID
    menu->addAction(tr("Fit width"),this,SLOT(maximizeWidth()));
    menu->addAction(tr("fit height"),this,SLOT(maximizeHeight()));
     menu->addAction(tr("Create desktop Shortcut"),this,SLOT(createShortcut()));
#endif

    if ( ( dynamic_cast<CpcXXXX *>(this) ) &&  ((CpcXXXX*)this)->pCPU)
    {
        menupocket = menu->addMenu(tr("Pocket"));
			menupocket->addAction(tr("Turn On"),this,SLOT(slotPower()));
            menupocket->addAction(tr("Reset"),this,SLOT(slotResetNow()));
            menupocket->addAction(tr("Reset (5s delay)"),this,SLOT(slotReset()));
            menupocket->addAction(tr("Hard Reset (5s delay)"),this,SLOT(slotHardReset()));
            //menupocket->addAction(tr("Detach"),this,SLOT(slotDetach()));
            menupocket->addSeparator();
            menupocket->addAction(tr("Load ..."),this,SLOT(slotLoadSession()));
            menupocket->addAction(tr("Save ..."),this,SLOT(slotSaveSession()));
    }

    menuext = NULL;
    for (int i=0;i<5;i++)
    {
        if (extensionArray[i])
        {
            if (!menuext)
                menuext = menu->addMenu(tr("Extensions"));
            menuext->addMenu(extensionArray[i]->Menu);
        }
    }

    menuconfig = menu->addMenu(tr("Configuration"));

    if ( dynamic_cast<CpcXXXX *>(this) &&  ((CpcXXXX*)this)->pCPU )
    {
        menucpuspeed = menuconfig->addMenu(tr("CPU Speed"));
        menucpuspeed->addAction(tr("100%"));
        menucpuspeed->addAction(tr("200%"));
        menucpuspeed->addAction(tr("300%"));
        menucpuspeed->addAction(tr("500%"));
        menucpuspeed->addAction(tr("Maximum"));

        connect(menucpuspeed, SIGNAL(triggered(QAction*)), this, SLOT(slotCpu(QAction*)));
    }

    if (pLCDC)
    {
        menulcd = menuconfig->addMenu(tr("LCD contrast"));
        menulcd->addAction(tr("0"));
        menulcd->addAction(tr("1"));
        menulcd->addAction(tr("2"));
        menulcd->addAction(tr("3"));
        menulcd->addAction(tr("4"));

        connect(menulcd, SIGNAL(triggered(QAction*)), this, SLOT(slotContrast(QAction*)));

    }
    if (pKEYB) {
        menuconfig->addAction(tr("Keyboard"),this,SLOT(KeyList()));
        menu->addAction(tr("Keyboard Simulator"),this,SLOT(VirtualKeyboard()));
    }

    if (!SlotList.isEmpty()) menuconfig->addAction(tr("Dump Memory"),this,SLOT(Dump()));

    if ( dynamic_cast<CpcXXXX *>(this) &&  ((CpcXXXX*)this)->pCPU  ) {

        menuconfig->addAction(tr("Debug"),this,SLOT(Dasm()));
    }

    menu->addAction(tr("New Post-it"),this,SLOT(Postit()));

    menuconfig->addSeparator();
    computeLinkMenu(menuconfig);
    computeUnLinkMenu(menuconfig);
    menu->addSeparator();
    computeWebLinksMenu(menu);
    menu->addSeparator();
    menu->addAction(tr("Close"),this,SLOT(slotExit()));

}

void CPObject::computeWebLinksMenu(QMenu * menu) {

    menuweblink = menu->addMenu(tr("Web Links"));

    menuDocument = menu->addMenu(tr("Documents"));
    connect(menuweblink, SIGNAL(triggered( QAction *)), mainwindow, SLOT(slotWebLink( QAction *)));
// FETCH XML FILE TO ADD MENU ACTIONS
#ifdef EMSCRIPTEN
    connect(menuDocument, SIGNAL(triggered(QAction*)), mainwindow, SLOT(slotWebLink(QAction*)));
    QFile fileRes(P_RES(":/pockemul/weblinks.xml"));
#else
    connect(menuDocument, SIGNAL(triggered(QAction*)), mainwindow, SLOT(slotDocument(QAction*)));
    // Does weblinks.xml exists locally ? if not generate one
//    QString weblinksFn = QApplication::applicationDirPath()+"/weblinks.xml";
//    if (!QFile::exists(weblinksFn)) {
//        QFile::copy(P_RES(":/pockemul/weblinks.xml"),weblinksFn);
//        QFile::setPermissions(weblinksFn,QFile::WriteOther);
//    }

    QFile fileRes(P_RES(":/pockemul/weblinks.xml"));
 #endif
    QXmlInputSource sourceRes(&fileRes);
    QXmlSimpleReader reader;
    reader.setContentHandler( new WebLinksParser(this) );

    reader.parse( sourceRes );

    fileRes.close();

}

void CPObject::insertLinkAction(LINKTYPE type,QString desc,QString link) {

    if (type == WEBLINK) {
        QLabel* label = new QLabel("<a href='"+link+"'>"+desc+"</a>", this);
        label->setOpenExternalLinks(true);

        QWidgetAction* a = new QWidgetAction(this);
        a->setDefaultWidget( label );
        menuweblink->addAction(a);
    }
    if (type == DOCUMENT) {
        QAction *a = menuDocument->addAction(desc);
        a->setData(link);
    }
}

void CPObject::computeLinkMenu(QMenu * menu)
{
    menulink = menu->addMenu(tr("Link"));
	connect(menulink, SIGNAL(triggered( QAction *)), mainwindow, SLOT(slotNewLink( QAction *)));    
	for (int i = 0;i < ConnList.size(); i++)
 	{
        QMenu * menuLocConn = 0;
        if (ConnList.size() == 1)
            menuLocConn = menulink;
        else
            menuLocConn = menulink->addMenu(ConnList.at(i)->Desc);
		
		for (int j = 0;j < listpPObject.size(); j++)
		{
			if (listpPObject.at(j) != this)
			{
                QMenu * menuAllPc = 0;//menuLocConn->addMenu(listpPObject.at(j)->getName());
				for (int k = 0; k < listpPObject.at(j)->ConnList.size(); k++)
                {
                    if (ConnList.at(i)->isPluggableWith(listpPObject.at(j)->ConnList.at(k))) {
                        if (menuAllPc==0)
                            menuAllPc = menuLocConn->addMenu(listpPObject.at(j)->getName());
                        QAction * actionDistConn = menuAllPc->addAction(listpPObject.at(j)->ConnList.at(k)->Desc);
                        actionDistConn->setData(tr("%1:%2").arg((long)ConnList.at(i)).arg((long)listpPObject.at(j)->ConnList.at(k)));
                    }
                }
            }
        }
	}	
}

void CPObject::computeUnLinkMenu(QMenu * menu)
{
    menuunlink = menu->addMenu(tr("Remove Link"));
    if(ConnList.size()>1) {
        QAction * actionLocAllConn = menuunlink->addAction("All");
        actionLocAllConn->setData(tr("A%1").arg((long)this));
        menuunlink->addSeparator();
    //connect(menuunlink, SIGNAL(triggered( QAction *)), mainwindow, SLOT(slotUnLinkAll( QAction *)));
    }
	for (int i = 0;i < ConnList.size(); i++)
 	{
        QAction * actionLocConn = menuunlink->addAction(ConnList.at(i)->Desc);
        actionLocConn->setData(tr("C%1").arg((long)ConnList.at(i)));
        connect(menuunlink, SIGNAL(triggered( QAction *)), mainwindow, SLOT(slotUnLink( QAction *)));
	}	
}




void CPObject::publish(Cconnector* newConn)
{
	ConnList.append(newConn);

}
void CPObject::remove(Cconnector* newConn)
{
    ConnList.removeAt(ConnList.indexOf(newConn));
}


void CPObject::slotExit(void)
{

	toDestroy = true;
}

bool CPObject::InitDisplay(void)
{
    CViewObject::InitDisplay();



    delete BackgroundImage;
    BackgroundImage = new QImage(*BackgroundImageBackup);
    delete FinalImage;
    FinalImage = new QImage(*BackgroundImageBackup);

    mask = QPixmap(BackGroundFname).scaled(getDX()*mainwindow->zoom/100,getDY()*mainwindow->zoom/100);
    setMask(mask.mask());

	return(1);
}

bool CPObject::UpdateFinalImage(void)
{

	QPainter painter;
	if ( (BackgroundImage) )
	{
        painter.begin(FinalImage);
        painter.drawImage(QPoint(0,0),*BackgroundImage);
        painter.end();

        if (dialogkeylist)
        {
            InitView(currentView);
            switch(currentView) {
            case TOPview:  painter.begin(TopImage); break;
            case LEFTview: painter.begin(LeftImage); break;
            case RIGHTview: painter.begin(RightImage); break;
            case BOTTOMview: painter.begin(BottomImage); break;
            case BACKview: painter.begin(BackImage); break;
            default: painter.begin(FinalImage); break;
            }
            painter.setPen(QPen(Qt::red));
            QList<QRect> _rectList = dialogkeylist->getkeyFoundRect();
//            qWarning()<<"drawRect:"<<_rectList;

            for (int i=0; i<dialogkeylist->lwKeys->count();i++) {
                if (dialogkeylist->lwKeys->item(i)->isSelected() &&
                    pKEYB->Keys.at(i).view == currentView ) {
                painter.fillRect(QRect(pKEYB->Keys.at(i).Rect.x()*internalImageRatio,
                                       pKEYB->Keys.at(i).Rect.y()*internalImageRatio,
                                       pKEYB->Keys.at(i).Rect.width()*internalImageRatio,
                                       pKEYB->Keys.at(i).Rect.height()*internalImageRatio),
                                 QBrush(Qt::red,Qt::BDiagPattern));
                }
            }
            painter.end();
        }


	}

    return true;
}

void CPObject::KeyList()
{
        dialogkeylist = new DialogKeyList(this);
        dialogkeylist->show();
}

void CPObject::setCpu(int newspeed)
{
	pTIMER->SetCPUspeed(newspeed);
}

void CPObject::slotCpu(QAction* action) {
    if (action->text() == tr("100%")) setCpu(1);
    if (action->text() == tr("200%")) setCpu(2);
    if (action->text() == tr("300%")) setCpu(3);
    if (action->text() == tr("500%")) setCpu(5);
    if (action->text() == tr("Maximum")) setCpu(1000);
}

void CPObject::slotContrast(QAction * action) {
    if (action->text() == tr("0")) pLCDC->Contrast(0);
    if (action->text() == tr("1")) pLCDC->Contrast(1);
    if (action->text() == tr("2")) pLCDC->Contrast(2);
    if (action->text() == tr("3")) pLCDC->Contrast(3);
    if (action->text() == tr("4")) pLCDC->Contrast(4);
}

void CPObject::slotPower()
{ 
    qWarning()<<"Slot POWER"<<Power;
    if (!Power) {
//        pKEYB->LastKey = K_POW_ON;
		TurnON();
    }
	else
        TurnOFF();

    pKEYB->LastKey = 0;
}

void CPObject::slotResetNow() {
    resetAt = pTIMER->state;

//    Reset();
}

void CPObject::slotReset() {
    resetAt = (pTIMER->CPUSpeed * getfrequency())*5 + pTIMER->state;

//    Reset();
}

void CPObject::slotHardReset() {
    hardresetAt = (pTIMER->CPUSpeed * getfrequency())*5 + pTIMER->state;

//    Reset();
}

void CPObject::slotLoadSession()
{
    ((CpcXXXX *)this)->LoadSession();
}

void CPObject::slotSaveSession()
{
    ((CpcXXXX *)this)->SaveSession();
}

void CPObject::Dump()
{
	dialogdump = new DialogDump(this);
	dialogdump->show();
}

void CPObject::Dasm()
{
    if (dialogdasm) dialogdasm->show();
}

void CPObject::Postit()
{
    mainwindow->LoadPocket("Post-it");
}

void CPObject::VirtualKeyboard()
{
    dialogVKeyboard = new DialogVKeyboard(this);
    dialogVKeyboard->show();
}

bool CPObject::getdisp_onRaised()
{
    return disp_onRaised;
}

void CPObject::setDisp_on(bool v)
{
    pLCDC->On = v;

    if (v && !disp_on) disp_onRaised=true;
    else disp_onRaised = false;
    disp_on = v;
}

bool CPObject::getDisp_on()
{
    return disp_on;
}




//////////////////////////////////////////////
// Save Memory to file						//
//  ENTRY :BYTE s=Slot No.(SLOT1-3)			//
//  RETURN:none								//
//////////////////////////////////////////////
void CPObject::Mem_Save(QFile *file,BYTE s)
{
    QDataStream out(file);
    out.writeRawData( (char *) &mem[SlotList[s].getAdr()],SlotList[s].getSize() * 1024 );
}

void CPObject::Mem_Save(QXmlStreamWriter *xmlOut,BYTE s,bool dumphex)
{
    xmlOut->writeStartElement("bank");
        switch(SlotList[s].getType()) {
        case CSlot::ROM: xmlOut->writeAttribute("type","ROM");
            xmlOut->writeAttribute("resID",SlotList[s].getResID());
            break;
        case CSlot::CUSTOM_ROM: xmlOut->writeAttribute("type","CUSTOM_ROM"); break;
        case CSlot::RAM: xmlOut->writeAttribute("type","RAM"); break;
        default: break;
        }
        xmlOut->writeAttribute("label",SlotList[s].getLabel());
        xmlOut->writeAttribute("empty",SlotList[s].isEmpty()?"true":"false");
        xmlOut->writeAttribute("id",QString("%1").arg(s));
        if (dumphex) {
            int size = SlotList[s].getSize() * 1024;
            int adr = SlotList[s].getAdr();
            QByteArray ba((char *) &mem[adr],size );
            //        xmlOut->writeTextElement("data",ba.toBase64());
            QString outHex = "\n";
            for (int a=0;a<size;a+=16) {
                outHex += QString("%1:").arg(a,6,16,QChar('0'))+ba.mid(a,16).toHex()+"\n";
            }
            xmlOut->writeTextElement("datahex",outHex);
        }
    xmlOut->writeEndElement();
}

void CPObject::Mem_Save(BYTE s)
{
    QFile file( SlotList[s].getFileName() );

    if (file.open(QIODevice::WriteOnly))
    {
        Mem_Save(&file,s);
        file.close();
        return;
    }
}

//////////////////////////////////////////////
// Load Memory from file					//
//  ENTRY :BYTE s=Slot No.(SLOT1-3)			//
//  RETURN:1:success, 0:error				//
//////////////////////////////////////////////
void CPObject::Mem_Load(QFile *file,BYTE s)
{
    QDataStream in(file);
    in.readRawData ((char *) &mem[SlotList[s].getAdr()],SlotList[s].getSize() * 1024 );
}

void CPObject::Mem_Load(QXmlStreamReader *xmlIn,BYTE s)
{
    if (xmlIn->readNextStartElement()) {
        if (xmlIn->name() == "bank" && xmlIn->attributes().value("id").toString().toInt() == s) {
            SlotList[s].setEmpty(xmlIn->attributes().value("empty").toString()=="true"?true:false);
            SlotList[s].setLabel(xmlIn->attributes().value("label").toString());

            QString _type = xmlIn->attributes().value("type").toString();
            if (_type=="ROM") {
                SlotList[s].setType(CSlot::ROM);
                SlotList[s].setResID(xmlIn->attributes().value("resID").toString());
                Mem_Load(s);
            }
            else {
                if (_type=="CUSTOM_ROM") SlotList[s].setType(CSlot::CUSTOM_ROM);
                if (_type=="RAM") SlotList[s].setType(CSlot::RAM);
                if (xmlIn->readNextStartElement() ) {
                    if (xmlIn->name()=="data")  {
                        QByteArray ba = QByteArray::fromBase64(xmlIn->readElementText().toLatin1());
                        memcpy((char *) &mem[SlotList[s].getAdr()],ba.data(),SlotList[s].getSize() * 1024);
                    }
                    if (xmlIn->name()=="datahex")  {

                        //                    MSG_ERROR(xmlIn->readElementText().replace(QRegExp("......:"),"").toLatin1())
                        QByteArray ba = QByteArray::fromHex(xmlIn->readElementText().replace(QRegExp("......:"),"").toLatin1());
                        memcpy((char *) &mem[SlotList[s].getAdr()],ba.data(),SlotList[s].getSize() * 1024);
                    }
                }
            }
        }
        xmlIn->skipCurrentElement();
    }
}

bool CPObject::Mem_Load(BYTE s)
{
    QFile file;

    if (SlotList[s].getFileName() == "EMPTY") return true;

    file.setFileName(QCoreApplication::applicationDirPath()+"/"+QFileInfo(SlotList[s].getFileName()).fileName());
//    qWarning()<<file.fileName();
    if (!SlotList[s].getFileName().isEmpty() && file.exists())
    {
//        qWarning()<<"ok";
        if (file.size() == (SlotList[s].getSize() * 1024) )
        {
            file.open(QIODevice::ReadOnly);
            Mem_Load(&file,s);
            AddLog(LOG_MASTER,tr("loaded from file : %1").arg(SlotList[s].getFileName()));
            return true;
        }
//		else
//			AddLog(LOG_MASTER,tr("FileSize[%1] = %2  instead of %3 ").arg(SlotName[s]).arg(file.size().arg((SlotSize[s] * 1024)));

    }
    else
    {
        file.setFileName(SlotList[s].getResID());
        if (file.exists())
        {
            file.open(QIODevice::ReadOnly);
            Mem_Load(&file,s);
            AddLog(LOG_MASTER,"Loaded from ressource");
            return true;
        }
    }
    return false;
}

bool CPObject::Mem_Load(qint32 adr, QByteArray data ) {

    QDataStream in(data);
    int p =data.size();
    in.readRawData ((char *) &mem[adr], p );
    return true;
}

void CPObject::manageBus(Cbus *_bus) {

    // write connector

    Set_Connector(_bus);

    // Execute all connected objetcs
    // WRITE the LINK BOX Connector
    mainwindow->pdirectLink->Cascade(this);

    if ((mainwindow->dialoganalogic) && mainwindow->dialoganalogic->capture()) {
        mainwindow->dialoganalogic->captureData();
    }
    // Read connector
    Get_Connector(_bus);

}


void CPObject::writeBus(Cbus *bus, UINT32 *d, UINT32 data) {
    busMutex.lock();

    bus->setWrite(true);
    bus->setAddr(*d);
    bus->setData((quint8)data);
    bus->setEnable(true);
    manageBus(bus);
    bus->setEnable(false);

    busMutex.unlock();
}

void CPObject::readBus(Cbus *bus,UINT32 *d,UINT32 *data) {
    busMutex.lock();

    bus->setWrite(false);
    bus->setAddr(*d);
    bus->setData(0xff);
    bus->setEnable(true);
//    qWarning()<<"ReadBus:"<<bus->toLog();
    manageBus(bus);
//    qWarning()<<"ReadBus after manage:"<<bus->toLog();
    *data = bus->getData();
    bus->setEnable(false);

    busMutex.unlock();
}
