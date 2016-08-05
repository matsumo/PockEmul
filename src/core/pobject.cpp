// TODO: move MemSave and MemLoad ti CSlot object
// TODO: call slotPower with a signal Emit
// TODO: maximize bug if zoom > 100%

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
#include "renderView.h"

#include "bus.h"
#include "overlay.h"


extern QList<CPObject *> listpPObject;  /**< TODO: describe */
extern CrenderView* view;


extern MainWindowPockemul* mainwindow; /**< TODO: describe */
/**
 * @brief
 *
 * @param parent
 * @param msg
 * @param nbButton
 * @return int
 */
extern int ask(QWidget *parent,QString msg,int nbButton);
extern bool soundEnabled; /**< TODO: describe */

/**
 * @brief Base emulated object.
 *
 * @param parent
 */
CPObject::CPObject(CPObject *parent):CViewObject(parent)
{
    pPC = (CpcXXXX*) parent;
    Parent	= parent;
    toDestroy = false;


    pKEYB	= 0;
    pTIMER	= 0;
    pLCDC	= 0;
    bus     = 0;
    busMem  = 0;
    BackgroundImage = 0;

    flipping = false;
    currentView = FRONTview;

    currentOverlay = -1;
    resetFiredState = 0;

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
    mem             = 0;

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
#ifndef NO_SOUND
    m_audioOutput = 0;
#endif
    contextMenu=menupocket=menuext=menuconfig=menucpuspeed=menuAudioVolume=menulcd=menulink=menuunlink=menuweblink=menuDocument=0;
    ioFreq = 0;
    off =true;
    closed = false;
    resetAt = 0;
    hardresetAt = 0;
    hardreset = false;

//    grabGesture(Qt::TapAndHoldGesture);

    // ERROR MESSAGE
    connect( this,SIGNAL(msgError(QString)),mainwindow,SLOT(slotMsgError(QString)));
    setStyleSheet("background-color:white;color: black;selection-background-color: grey;");
}


/**
 * @brief
 *
 */
CPObject::~CPObject()
{
    if (dialogdasm) delete dialogdasm;


    //FIXME: When extension are connected , they share the timer with the host.
    //delete pTIMER;
    if (pLCDC) delete pLCDC;
    if (bus) delete bus;
    if (busMem) delete busMem;
	

    delete BackgroundImage;

    delete contextMenu;

//    delete extensionArray[0];
//    delete extensionArray[1];
//    delete extensionArray[2];
//    delete extensionArray[3];
//    delete extensionArray[4];

//    if (mem) free(mem);
}



/**
 * @brief serialize an emulated object to a QXmlStreamWriter
 *
 * @param xml
 * @param id
 */
void CPObject::serialize(QXmlStreamWriter *xml,int id) {
    xml->writeStartElement("object");
        xml->writeAttribute("name", getName());
        xml->writeAttribute("id", QString("%1").arg(id));
        xml->writeAttribute("front",Front?"true":"false");
        xml->writeAttribute("power",Power?"true":"false");
        xml->writeAttribute("visible",isVisible()?"true":"false");
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


/**
 * @brief Return the enclosure Rect for the current emulated object, included all linked objects.
 *
 * @return QRectF
 */
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

/**
 * @brief
 *
 * @param name
 * @param param
 */
extern void m_addShortcut(QString name,QString param);

/**
 * @brief
 *
 */
void CPObject::createShortcut() {
#ifdef Q_OS_ANDROID
    m_addShortcut(getName(),"-r \""+getName()+"\"");
#endif
}

/**
 * @brief
 *
 */
void CPObject::maximizeWidth()
{
    if (mainwindow->zoom <= 1) {
        // Compute global rect
        QRectF rs = RectWithLinked();
        float rw= mainwindow->centralwidget->rect().width()/rs.width();
        float rh= mainwindow->centralwidget->rect().height()/rs.height();
        int r = MAX(rw,rh);
        if (r>1) {
            mainwindow->doZoom(QPoint(0,0),1,r-mainwindow->zoom);
            //move to upper left
            // Fetch all_object and move them
            rs = RectWithLinked();
            mainwindow->MoveAll(- rs.topLeft());
            fullscreenMode = true;
//            grabGesture(Qt::SwipeGesture);
//            qWarning()<<"GrabGesture SwipeGesture";
        }
    }
    else {
        mainwindow->doZoom(QPoint(0,0),-1,mainwindow->zoom-1);
        fullscreenMode = false;
//        ungrabGesture(Qt::SwipeGesture);
//        qWarning()<<"unGrab Gesture SwipeGesture";
    }
}

/**
 * @brief
 *
 */
void CPObject::maximizeHeight()
{
    if (mainwindow->zoom <= 1) {
        // Compute global rect
        QRectF rs = RectWithLinked();
        int rw= mainwindow->centralwidget->rect().width()/rs.width();
        int rh= mainwindow->centralwidget->rect().height()/rs.height();
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
        mainwindow->doZoom(QPoint(0,0),-1,mainwindow->zoom-1);
        fullscreenMode = false;
    }
}

/**
 * @brief Find recursively all connected object for a specific object.
 *
 * @param search the object to start the search from.
 * @param liste a list of all linked objects.
 */
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

/**
 * @brief Move the object with all linked to a an absolute position
 *
 * @param p the destination position
 */
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

/**
 * @brief
 *
 * @param p
 */
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

/**
 * @brief Set the fequency of the emulated object.
 *
 * PockEmul execute the funtion run() at the defined frequency.
 * Set the value to 0 for a passive object. The function run()  of a passive object will be executed
 * after the run() function of the master object.
 *
 * @param f Frequency in Hz
 * @sa getfrequency
 */
void CPObject::setfrequency(int f)
{
    frequency = f;
    if (pTIMER) {
        pTIMER->state = pTIMER->currentState();
    }
}



/**
 * @brief
 *
 * @return bool
 */
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

/**
 * @brief
 *
 * @return bool
 */
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

/**
 * @brief
 *
 * @param step
 * @return quint64
 */
quint64 CPObject::runRange(quint64 step) {
    if (pTIMER) {

        quint64 t = pTIMER->state;

        while (/*!off &&*/ ((pTIMER->state - t) < step)) {
            run();

//            if (pLCDC) {
//                pLCDC->disp();
//                if (pLCDC->Refresh) {
//                    Refresh_Display = true;
//                }
//            }

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

/**
 * @brief
 *
 * @return bool
 */
bool CPObject::run(void){

    if (KEY(K_RESET)) {
        Reset();
    }


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
//                        MoveWithLinked(QPoint(10,0));//centralwidget->rect().width() - CurrentpPC->width()*zoom,0));
//                        update();
//                    }
//                }
//                else {
//                    // move right
//                    if ((posx()+width()) > mainwindow->centralWidget()->rect().width()) {
//                        MoveWithLinked(QPoint( -10,0));//CurrentpPC->width()*zoom - centralwidget->rect().width(),0));
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


//    if ((hardresetAt>0) && (pTIMER->state >= hardresetAt)) {
//        qWarning()<<"HardReset";
//        // FIXME: cannot call init from CPU Thread. Use a SIGNAL
//        hardreset = true;
//        init();
//        Reset();
//        hardresetAt = 0;
//    }


   return true;
}

#define SAMPLERATE 8000
#define BUFFLEN 500

#ifndef NO_SOUND
/**
 * @brief
 *
 * @param state
 */
void CPObject::audioStateChanged(QAudio::State state)
{
    if (state == QAudio::IdleState) {

    }
    //AddLog(LOG_TEMP,tr("state = %1").arg(state))
//    qWarning() << "state = " << state;
}
#endif
/**
 * @brief
 *
 * @return int
 */
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

    m_audioOutput = 0;
    m_audioOutput = new QAudioOutput(m_device, m_format, this);
    //connect(m_audioOutput, SIGNAL(notify()), SLOT(notified()));
    connect(m_audioOutput, SIGNAL(stateChanged(QAudio::State)), SLOT(audioStateChanged(QAudio::State)));
    m_audioOutput->setBufferSize(1000);

    m_output = m_audioOutput->start();
    m_audioOutput->setVolume(0.25);
//    int p = m_audioOutput->periodSize();
//    qWarning()<<p;
#endif
    return 1;
}

/**
 * @brief
 *
 * @return int
 */
int CPObject::exitsound()
{
    return 1;
}


//FIXME: The piezo doesn't produce sounf for frequency < 1Khz
/**
 * @brief
 *
 * @param val
 */
void CPObject::fillSoundBuffer(BYTE val)
{
    if (!soundEnabled) return;
    if (!m_output) return;

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

        int ps = m_audioOutput->periodSize();
        if (audioBuff.size() <= (2*ps)) {
            while ((pTIMER->state - fillSoundBuffer_old_state) >= wait)
            {
                audioBuff.append(val);
//qWarning()<<val;
                fillSoundBuffer_old_state += wait;
                //delta_state -= wait;
            }
        }
        else {
            fillSoundBuffer_old_state = pTIMER->state;
        }

        //AddLog(LOG_TEMP,tr("buff:%1   ps:%2").arg(audioBuff.size()).arg(ps));

        if (audioBuff.size() >= (ps)) {
            qint64 _bytescount = m_output->write(audioBuff.constData(),ps);
//            AddLog(LOG_TEMP,tr("audiobuffsize:%1  outbuffer:%2").arg(audioBuff.size()).arg(m_audioOutput->bufferSize()-m_audioOutput->bytesFree()));
            //audioBuff.clear();
            audioBuff.remove(0,_bytescount);
        }
        else {
            //FIXME: buffer underrun
#if 1
            if((m_audioOutput->bufferSize()-m_audioOutput->bytesFree()) < ps) {
                AddLog(LOG_TEMP,tr("sound1 size:%1    free:%2   diff:%3   ps:%4").arg(m_audioOutput->bufferSize()).arg(m_audioOutput->bytesFree()).arg(m_audioOutput->bufferSize()-m_audioOutput->bytesFree()).arg(ps));
                QByteArray fill(2*ps-(m_audioOutput->bufferSize()-m_audioOutput->bytesFree()),0);
                m_output->write(fill.constData(),fill.size());
            }
#endif
        }
//        mainwindow->audioMutex.unlock();
    }
#endif
}



/**
 * @brief
 *
 * @param point
 */
void CPObject::SwitchFrontBack(QPoint point) {
    if (Front)
    {
        // calculate the new origine
        int newposx = point.x() + (pos().x()-point.x())/4;
        int newposy = point.y() + (pos().y()-point.y())/4;

        QPropertyAnimation *animation = new QPropertyAnimation(this, "geometry");
        animation->setDuration(1000);
        animation->setStartValue(QRect(posx(),posy(),getDX()*mainwindow->zoom,getDY()*mainwindow->zoom));
        animation->setEndValue(QRect(newposx,newposy,getDX()*mainwindow->zoom/4,getDY()*mainwindow->zoom/4));
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
        setGeometry(newposx,newposy,getDX()*mainwindow->zoom,getDY()*mainwindow->zoom);
        this->setPosX(newposx);
        this->setPosY(newposy);
//            QPoint newpos = pos() - mainwindow->pos();
//            setParent(mainwindow);
//            move(newpos);

    }
    Front = ! Front;
}

/**
 * @brief
 *
 * @param event
 * @return bool
 */
bool CPObject::event(QEvent *event)
 {

//     if (event->type() == QEvent::Gesture) {
////         if (QGesture *swipe = (static_cast<QGestureEvent*>(event))->gesture(Qt::SwipeGesture)) {
////             swipeTriggered(static_cast<QSwipeGesture *>(swipe));
////             event->accept();
////         }
////         else
//         {
//             if (QGesture *tap = (static_cast<QGestureEvent*>(event))->gesture(Qt::TapAndHoldGesture)) {
//                 const QPoint pos = (static_cast<QTapAndHoldGesture *>(tap))->position().toPoint();
//                 //             qWarning()<< (static_cast<QTapAndHoldGesture *>(tap))->timeout()<<pos<<tap->gestureType()<<tap->state();
//                 if (tap->state() == Qt::GestureStarted) {
//                     QContextMenuEvent *cme = new QContextMenuEvent(
//                                 QContextMenuEvent::Mouse,
//                                 pos,
//                                 (pos));
//                     //contextMenuEvent(cme);
//                     QApplication::sendEvent(this,cme);

//                     setCursor(Qt::ArrowCursor);

//                     event->accept();
//                 }
//             }
//         }
//         event->accept();
//         return true;
//     }
     return QWidget::event(event);
 }

/**
 * @brief
 *
 * @param gesture
 */
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

/**
 * @brief
 *
 * @param event
 */
void CPObject::wheelEvent(QWheelEvent *event) {
qWarning()<<"Weel";
    QPoint point;
    if (Parent) {
        point = event->pos();
    }
    else {
        point = event->globalPos();
    }

    float delta = event->delta()/12;

    mainwindow->doZoom(point,delta>0?1.1:0.9);
    event->accept();

}

/**
 * @brief
 *
 * @param pos
 */
void CPObject::maximize(QPoint pos) {

    // Compute global rect
    QRectF rs = RectWithLinked();
    float rw= mainwindow->centralwidget->rect().width()/(rs.width());
    float rh= mainwindow->centralwidget->rect().height()/(rs.height());
    float r=0;
    if ((rw>1) && (rh>1)) r = MIN(rw,rh);
    else if ((rw>1) && (rh<=1)) r = rh;
    else if ((rw<=1) && (rh>1)) r = rw;
    else if ((rw<=1) && (rh<=1)) r = MAX(rw,rh);

    mainwindow->doZoom(pos,r);
    //move to upper left
    // Fetch all_object and move them
    rs = RectWithLinked();
    mainwindow->MoveAll(- rs.topLeft());
}

/**
 * @brief
 *
 * @param pos
 */
void CPObject::minimize(QPoint pos) {

    mainwindow->doZoom(pos,1.0/mainwindow->zoom);
}

/**
 * @brief
 *
 * @param pos
 */
void CPObject::slotDoubleClick(QPoint pos) {
    if ((pKEYB) &&(pKEYB->KeyClick(pos))) {
//        qWarning()<<"keyclick";
        // Send thee mouseclick event twice
        QMouseEvent event(QEvent::MouseButtonPress, pos, Qt::LeftButton, Qt::LeftButton,Qt::NoModifier);
        QApplication::sendEvent(this, &event);

        return;
    }


    if (mainwindow->zoom <= 1) {
        // Compute global rect
        QRectF rs = RectWithLinked();
        float rw= mainwindow->centralwidget->rect().width()/rs.width();
        float rh= mainwindow->centralwidget->rect().height()/rs.height();
        float r = MIN(rw,rh);
        if (r>1) {
            mainwindow->doZoom(pos,r/mainwindow->zoom);
            //move to upper left
            // Fetch all_object and move them
            rs = RectWithLinked();
            mainwindow->MoveAll(- rs.topLeft());
        }
    }
    else {
        mainwindow->doZoom(pos,1.0/mainwindow->zoom);
    }

}

/**
 * @brief
 *
 * @param event
 */
void CPObject::mouseDoubleClickEvent(QMouseEvent *event)
{
    CViewObject::mouseDoubleClickEvent(event);

    QPoint pts(event->x() , event->y());
    slotDoubleClick(pts);
}


/**
 * @brief
 *
 */
extern void Vibrate();
/**
 * @brief
 *
 * @param event
 */
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

    if (pKEYB)
    {
        pKEYB->LastKey = pKEYB->KeyClick(pts);
        if (pKEYB->LastKey != 0)  {
//            ungrabGesture(Qt::TapAndHoldGesture);
//            qWarning()<<"lastKey="<<pKEYB->LastKey;
        }

        pKEYB->lastMousePressedKey = pKEYB->LastKey;
        if (pKEYB->LastKey) {
            pKEYB->keyPressedList.insert(pKEYB->LastKey,pTIMER ? pTIMER->state:0);
            Refresh_Display = true;
        }

        switch (pKEYB->LastKey) {
        case K_OF :
            Vibrate();
            slotPower();
            return;
            break;
        case K_BRK :
        case K_POW_ON :qWarning()<<"GOGO"; Vibrate();TurnON(); break;
        case K_POW_OFF: Vibrate();
            Power = false;
            mainwindow->saveAll = YES;
            TurnOFF();
            mainwindow->saveAll = ASK;
            return;
            break;
        case K_CLOSE: Vibrate();TurnCLOSE();break;
        }

        if (pKEYB->LastKey != 0)
        {
            ComputeKey(KEY_PRESSED,pKEYB->LastKey,event);
            Vibrate();
        }

        if (pKEYB->LastKey != 0) {
            event->accept();
            return;
        }
    }

    // NO KEY CLICK Global pobject drag mode

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
//            setCursor(Qt::ClosedHandCursor);	// Change mouse pointer
            startPosDrag = true;
            PosDrag = event->globalPos();
        }
    }

    event->accept();
}


/**
 * @brief
 *
 * @param event
 */
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
                setCursor(NONEdir != borderClick(event->pos()) ? Qt::OpenHandCursor : Qt::ArrowCursor);
            }
        }
        else {
            setCursor(NONEdir != borderClick(event->pos()) ? Qt::OpenHandCursor : Qt::ArrowCursor);
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

/**
 * @brief Search all connector near a specific connector
 *
 * @param refConnector The base connector
 * @param snaprange
 * @return QList<Cconnector *> List of all connectors near the refConnector.
 */
QList<Cconnector *> CPObject::nearConnectors(Cconnector *refConnector,qint8 snaprange) {
    // Compare snap between the refConnector and all free object connectors
    QList<Cconnector *> retList;
    for (int i=0;i<ConnList.size();i++) {
        if (!mainwindow->pdirectLink->isLinked(ConnList.at(i)) &&
            Cconnector::arePluggable(ConnList.at(i),refConnector)) {
            Cconnector *c = ConnList.at(i);
            // ATTENTION : POSITIONN DE L'OBJECT + SNAP !!!!!!
            CPObject *p = refConnector->Parent;
            qreal range = QLineF(p->pos()+refConnector->getSnap()*mainwindow->zoom,this->pos()+c->getSnap()*mainwindow->zoom).length();
            if (range < snaprange) {
                retList.append(ConnList.at(i));
            }
        }
    }
    return retList;
}

/**
 * @brief
 *
 * @param event
 */
void CPObject::mouseReleaseEvent(QMouseEvent *event)
{

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

                            MoveWithLinkedRel(listpPObject.at(k)->pos() + listpPObject.at(k)->ConnList.at(c)->getSnap()*mainwindow->zoom - pos() - nearList.at(r)->getSnap()*mainwindow->zoom);
                            mainwindow->pdirectLink->addLink(listpPObject.at(k)->ConnList.at(c),nearList.at(r),true);
                            QList<CPObject *> list;
                            listpPObject.at(k)->manageStackPos(&list);
                            emit stackPosChanged();
                            emit ObjLinkedTo(nearList.at(r),listpPObject.at(k)->ConnList.at(c));
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
        QPoint pts(event->x() , event->y());
        int _releasedKey = TOUPPER(pKEYB->KeyClick(pts));
        pKEYB->keyPressedList.remove(_releasedKey); //pKEYB->lastMousePressedKey);
        ComputeKey(KEY_RELEASED,_releasedKey,event);
        pKEYB->lastMousePressedKey = 0;
        pKEYB->LastKey = 0;
        Refresh_Display = true;
    }

    if ( (parentWidget() != mainwindow->centralwidget)
        && (parentWidget() != 0))
    {
        QApplication::sendEvent(parentWidget(), event);
    }
    event->accept();
}

/**
 * @brief
 *
 */
void CPObject::raise() {
    QWidget::raise();
}

/**
 * @brief
 *
 */
void CPObject::TurnCLOSE()
{

}

//FIXME: StackPosition managed by widget
//       If we want to be independent from QWidget, need to implement
//       stackUnder function

/**
 * @brief
 *
 * @param l
 */
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

/**
 * @brief
 *
 * @param xmlOut
 * @return bool
 */
bool CPObject::SaveSession_File(QXmlStreamWriter *xmlOut)
{
    Q_UNUSED(xmlOut)

    return true;
}

/**
 * @brief
 *
 * @param
 * @return bool
 */
bool CPObject::LoadSession_File(QXmlStreamReader *)
{
    return true;
}


#define KEY(c)	( pKEYB->keyPressedList.contains(TOUPPER(c)) || pKEYB->keyPressedList.contains(c) || pKEYB->keyPressedList.contains(TOLOWER(c)))

/**
 * @brief
 *
 * @param ke
 * @param scancode
 * @param event
 */
void CPObject::ComputeKey(CPObject::KEYEVENT ke, int scancode, QMouseEvent *event)
{
    Q_UNUSED(ke)
    Q_UNUSED(scancode)
    Q_UNUSED(event)

    if ((currentView==LEFTview) && KEY(K_SHARP11PINS)) {
#if 1
        view->pickExtensionConnector("Sharp_11");
#else
        FluidLauncher *launcher = new FluidLauncher(mainwindow,
                                     QStringList()<<P_RES(":/pockemul/configExt.xml"),
                                     FluidLauncher::PictureFlowType,QString(),
                                     "Sharp_11");
        launcher->show();
#endif
    }
}

/**
 * @brief
 *
 */
void CPObject::RefreshDisplay()
{
    Refresh_Display = true;
    update();
}

/**
 * @brief
 *
 * @param action
 */
void CPObject::Overlay(QAction * action)
{
    currentOverlay = action->data().toInt();
    Refresh_Display = true;
}

/**
 * @brief
 *
 * @param event
 */
void CPObject::paintEvent(QPaintEvent *event)
{
#ifdef Q_OS_ANDROID
#define TRANSFORM Qt::SmoothTransformation
//#define TRANSFORM Qt::FastTransformation
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

/**
 * @brief
 *
 * @param event
 */
void CPObject::keyReleaseEvent(QKeyEvent * event )
{
//    if (event->isAutoRepeat()) return;

	if (!pKEYB) return;	// if no Keyboard then return;
    pKEYB->isShift = event->modifiers() &  Qt::ShiftModifier;//(QApplication::keyboardModifiers() == Qt::ShiftModifier);
    pKEYB->isCtrl = (QApplication::keyboardModifiers() == Qt::ControlModifier);

    int _key = mapKey(event);
    pKEYB->keyPressedList.remove(_key);
    ComputeKey(KEY_RELEASED,_key);

    pKEYB->LastKey = 0;
    Refresh_Display = true;

}

/**
 * @brief
 *
 */
void CPObject::TurnON() {

}

/**
 * @brief
 *
 */
void CPObject::TurnOFF() {}

/**
 * @brief
 *
 * @param event
 * @return int
 */
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
        case Qt::Key_F8:		key = K_F8;		event->accept();	break;
        case Qt::Key_F9:		key = K_F9;		event->accept();	break;
        case Qt::Key_F10:		key = K_F0;		event->accept();	break;
        case Qt::Key_F11:		key = K_BRK;		event->accept();	break;
    default: key = event->key() & 0x1FFFFFF; event->accept();
//        qWarning()<<"key def:"<<key;
        }
//    if ( (event->key() >= 0x41) && (event->key() <= 0x5A) ) { key = event->key(); event->accept();	}
//    if ( (event->key() >= 0x30) && (event->key() <= 0x39) ) { key = event->key(); event->accept();	}

//    qWarning()<<"key:"<<key;
    return key;
}

/**
 * @brief
 *
 * @param event
 */
void CPObject::keyPressEvent (QKeyEvent * event )
{

//    if (event->isAutoRepeat()) return;
	if (!pKEYB) return;	// if no Keyboard then return;

    pKEYB->isShift = event->modifiers() &  Qt::ShiftModifier;//QApplication::keyboardModifiers() == Qt::ShiftModifier);
    pKEYB->isCtrl = (QApplication::keyboardModifiers() == Qt::ControlModifier);


    pKEYB->LastKey = mapKey(event);

    if (pKEYB->LastKey>0) {
        // Add th key to Key pressed buffer
        if (!pKEYB->keyPressedList.contains(TOUPPER(pKEYB->LastKey))) {
            pKEYB->keyPressedList.insert(TOUPPER(pKEYB->LastKey),pTIMER?pTIMER->state:0);
            Refresh_Display = true;
        }
        ComputeKey(KEY_PRESSED,pKEYB->LastKey);
    }
    else {
        event->ignore();
        QWidget::keyPressEvent(event);
    }
}

/**
 * @brief
 *
 * @param
 */
void CPObject::focusInEvent ( QFocusEvent *  )
{

}

/**
 * @brief
 *
 * @param
 */
void CPObject::focusOutEvent (QFocusEvent *)
{
}


/**
 * @brief
 *
 * @param event
 */
void CPObject::contextMenuEvent ( QContextMenuEvent * event )
{
//    qWarning()<<"contextMenuEvent";
//    Vibrate();

    delete contextMenu;
    contextMenu = new QMenu(this);
    BuildContextMenu(contextMenu);

//    contextMenu->setStyleSheet("QMenu { font-size:36px; color:white; left: 20px; background-color:qlineargradient(x1:0, y1:0, x2:0, y2:1, stop: 0 #cccccc, stop: 1 #333333);}");
    contextMenu->popup(event->globalPos () );
    event->accept();
}

/**
 * @brief Build the contect menu.
 *
 * @param menu The context menu is built inside the QMenu pointed by menu
 */
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
            menupocket->addAction(tr("Detach"),this,SLOT(slotDetach()));
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
#ifndef NO_SOUND
    if (m_audioOutput )
    {
        menuAudioVolume = menuconfig->addMenu(tr("Sound volume"));
        QAction *_a;
        _a = new QAction(tr("Mute"),this);
        _a->setCheckable(true);
        _a->setChecked(m_audioOutput->volume()==0);
        menuAudioVolume->addAction(_a);

        _a = new QAction(tr("25%"),this);
        _a->setCheckable(true);
        _a->setChecked(m_audioOutput->volume()==0.25);
        menuAudioVolume->addAction(_a);

        _a = new QAction(tr("50%"),this);
        _a->setCheckable(true);
        _a->setChecked(m_audioOutput->volume()==0.5);
        menuAudioVolume->addAction(_a);

        _a = new QAction(tr("75%"),this);
        _a->setCheckable(true);
        _a->setChecked(m_audioOutput->volume()==0.75);
        menuAudioVolume->addAction(_a);

        _a = new QAction(tr("100%"),this);
        _a->setCheckable(true);
        _a->setChecked(m_audioOutput->volume()==1);
        menuAudioVolume->addAction(_a);

        connect(menuAudioVolume, SIGNAL(triggered(QAction*)), this, SLOT(slotAudioVolume(QAction*)));
    }
#endif
    if (pKEYB) {
        menuconfig->addAction(tr("Keyboard"),this,SLOT(KeyList()));
        menu->addAction(tr("Keyboard Simulator"),this,SLOT(VirtualKeyboard()));
    }

    if (!SlotList.isEmpty()) menuconfig->addAction(tr("Dump Memory"),this,SLOT(Dump()));

    if ( dynamic_cast<CpcXXXX *>(this) &&  ((CpcXXXX*)this)->pCPU  ) {

        menuconfig->addAction(tr("Debug"),this,SLOT(Dasm()));
    }

    menu->addAction(tr("New Post-it"),this,SLOT(Postit()));

    if (overlays.count()>0) {
        QMenu *menuOverlay = menu->addMenu(tr("Overlays"));

        menuOverlay->addAction("None")->setData(-1);
        menuOverlay->addSeparator();
        for (int i=0;i<overlays.count();i++) {
            menuOverlay->addAction(overlays[i]->Title)->setData(i);

        }
        //    QAction *_act = menuOverlay->addAction("Load from file...");
        connect(menuOverlay, SIGNAL(triggered(QAction*)), this, SLOT(Overlay(QAction*)));
    }

    menuconfig->addSeparator();


    computeLinkMenu(menuconfig);
    computeUnLinkMenu(menuconfig);
    menu->addSeparator();
    computeWebLinksMenu(menu);
    menu->addSeparator();
    menu->addAction(tr("Close"),this,SLOT(slotExit()));

}

/**
 * @brief Build the WebLinks menu based on the weblinks.xml file
 *
 * @param menu
 *
 * @todo Add the capability to fetch a personal weblink.xml file.
 */
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

/**
 * @brief
 *
 * @param type
 * @param desc
 * @param link
 */
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

/**
 * @brief
 *
 * @param menu
 */
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
                        actionDistConn->setData(tr("%1:%2").arg((qlonglong)ConnList.at(i)).arg((qlonglong)listpPObject.at(j)->ConnList.at(k)));
                    }
                }
            }
        }
	}	
}

/**
 * @brief
 *
 * @param menu
 */
void CPObject::computeUnLinkMenu(QMenu * menu)
{
    menuunlink = menu->addMenu(tr("Remove Link"));
    if(ConnList.size()>1) {
        QAction * actionLocAllConn = menuunlink->addAction("All");
        actionLocAllConn->setData(tr("A%1").arg((qlonglong)this));
        menuunlink->addSeparator();
    //connect(menuunlink, SIGNAL(triggered( QAction *)), mainwindow, SLOT(slotUnLinkAll( QAction *)));
    }
	for (int i = 0;i < ConnList.size(); i++)
 	{
        QAction * actionLocConn = menuunlink->addAction(ConnList.at(i)->Desc);
        actionLocConn->setData(tr("C%1").arg((qlonglong)ConnList.at(i)));
        connect(menuunlink, SIGNAL(triggered( QAction *)), mainwindow, SLOT(slotUnLink( QAction *)));
	}	
}




/**
 * @brief Register the connector in the Global Connector Management System
 *
 * @param newConn
 *
 * @sa remove
 */
void CPObject::publish(Cconnector* newConn)
{
	ConnList.append(newConn);

}
/**
 * @brief Remove a connector from the Global Connector Management System
 *
 * @param newConn
 *
 * @sa publish
 */
void CPObject::remove(Cconnector* newConn)
{
    ConnList.removeAt(ConnList.indexOf(newConn));
}


/**
 * @brief
 *
 */
void CPObject::slotExit(void)
{
	toDestroy = true;
}

/**
 * @brief
 *
 * @return bool
 */
bool CPObject::InitDisplay(void)
{
    CViewObject::InitDisplay();

    delete BackgroundImage;
    BackgroundImage = new QImage(*BackgroundImageBackup);
    delete FinalImage;
    FinalImage = new QImage(*BackgroundImageBackup);

    mask = QPixmap(BackGroundFname).scaled(getDX()*mainwindow->zoom,getDY()*mainwindow->zoom);
    setMask(mask.mask());

	return(1);
}

/**
 * @brief
 *
 * @return bool
 */
bool CPObject::UpdateFinalImage(void)
{

	QPainter painter;
	if ( (BackgroundImage) )
	{

        delete FinalImage;
        FinalImage = new QImage(*BackgroundImage);



        // Draw Overlay
        if ( (currentOverlay >=0) && (currentOverlay < overlays.count()) ) {
            painter.begin(FinalImage);

            int x = overlays[currentOverlay]->overlayRect.x() * internalImageRatio;
            int y = overlays[currentOverlay]->overlayRect.y() * internalImageRatio;
            int z	= (int) (overlays[currentOverlay]->overlayRect.width()  * internalImageRatio);
            int t	= (int) (overlays[currentOverlay]->overlayRect.height() * internalImageRatio);
            painter.drawImage(QRect(x,y,z,t),
                              overlays[currentOverlay]->overlayImage.scaled(z,t,Qt::IgnoreAspectRatio,TRANSFORM));
            painter.end();
        }


        InitView(currentView);

        if (dialogkeylist)
        {    
            switch(currentView) {
            case TOPview:  painter.begin(TopImage); break;
            case LEFTview: painter.begin(LeftImage); break;
            case RIGHTview: painter.begin(RightImage); break;
            case BOTTOMview: painter.begin(BottomImage); break;
            case BACKview:
            case BACKviewREV: painter.begin(BackImage); break;
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


        // DRAW DELAYED KEYS
        // Fetch keypressedList and for each delayed keys
        // show message
        QString _msg ="";
        QMapIterator<int, quint64> i(pKEYB->keyPressedList);
        while (i.hasNext()) {
            i.next();
            // Check if this key is delayed
            CKey _key = pKEYB->getKey(TOUPPER(i.key()));
            int _delay = _key.delay;
            if ( _delay > 0) {
                // Check timing
                quint64 _stick = i.value();
                quint64 _elapsed = pTIMER->msElapsed(_stick);
//                qWarning()<<"delay"<<_delay<<"stick"<<_stick<<"elapsed"<<_elapsed;

                if (_elapsed <= (_delay*1000)) {
                    // Draw text
                    if (!_msg.isEmpty()) _msg+= "\n";
                    _msg = QString(_key.Description+" in %1s").arg(_delay - _elapsed/1000);
                }
            }
        }
//        qWarning()<<"msg"<<_msg;
        if (!_msg.isEmpty()) {
            switch(currentView) {
            case TOPview:  painter.begin(TopImage); break;
            case LEFTview: painter.begin(LeftImage); break;
            case RIGHTview: painter.begin(RightImage); break;
            case BOTTOMview: painter.begin(BottomImage); break;
            case BACKview:
            case BACKviewREV: painter.begin(BackImage); break;
            default: painter.begin(FinalImage); break;
            }
            painter.setPen(QPen(Qt::red));
            QFont _font = painter.font();

            _font.setPointSize(25);
            _font.setBold(true);
            painter.setFont(_font);

            QRect _rect(0,0,getDX()*internalImageRatio,getDY()*internalImageRatio);
            float factor = _rect.width() / painter.fontMetrics().width(_msg);
             if ((factor < 1) || (factor > 1.25))
             {
              QFont f = painter.font();
              f.setPointSizeF(f.pointSizeF()*factor);
              painter.setFont(f);
             }
            painter.drawText(_rect,Qt::AlignCenter,_msg);
            painter.end();
        }

	}


    return true;
}

/**
 * @brief
 *
 */
void CPObject::KeyList()
{
        dialogkeylist = new DialogKeyList(this);
        dialogkeylist->show();
}

/**
 * @brief
 *
 * @param newspeed
 */
void CPObject::setCpu(int newspeed)
{
	pTIMER->SetCPUspeed(newspeed);
}

/**
 * @brief
 *
 * @param action
 */
void CPObject::slotCpu(QAction* action) {
    if (action->text() == tr("100%")) setCpu(1);
    if (action->text() == tr("200%")) setCpu(2);
    if (action->text() == tr("300%")) setCpu(3);
    if (action->text() == tr("500%")) setCpu(5);
    if (action->text() == tr("Maximum")) setCpu(1000);
}

/**
 * @brief
 *
 * @param action
 */
void CPObject::slotAudioVolume(QAction * action) {
#ifndef NO_SOUND
    if (action->text() == tr("Mute")) m_audioOutput->setVolume(0);
    if (action->text() == tr("25%")) m_audioOutput->setVolume(0.25);
    if (action->text() == tr("50%")) m_audioOutput->setVolume(0.5);
    if (action->text() == tr("75%")) m_audioOutput->setVolume(0.75);
    if (action->text() == tr("100%")) m_audioOutput->setVolume(1);
#endif
}

/**
 * @brief
 *
 * @param action
 */
void CPObject::slotContrast(QAction * action) {
    if (action->text() == tr("0")) pLCDC->Contrast(0);
    if (action->text() == tr("1")) pLCDC->Contrast(1);
    if (action->text() == tr("2")) pLCDC->Contrast(2);
    if (action->text() == tr("3")) pLCDC->Contrast(3);
    if (action->text() == tr("4")) pLCDC->Contrast(4);
}

/**
 * @brief
 *
 */
void CPObject::slotPower()
{ 
    qWarning()<<"Slot POWER"<<Power;
    if (!Power) {
//        pKEYB->LastKey = K_POW_ON;
		TurnON();
    }
    else {
        mainwindow->saveAll = YES;
        TurnOFF();
        mainwindow->saveAll = ASK;
    }
    pKEYB->LastKey = 0;
}

/**
 * @brief
 *
 */
void CPObject::slotResetNow() {
    resetAt = pTIMER->state;

}

/**
 * @brief
 *
 */
void CPObject::slotReset() {
    resetAt = (pTIMER->CPUSpeed * getfrequency())*5 + pTIMER->state;

}

/**
 * @brief
 *
 */
void CPObject::slotHardReset() {
    hardresetAt = (pTIMER->CPUSpeed * getfrequency())*5 + pTIMER->state;

}

/**
 * @brief
 *
 */
void CPObject::slotDetach()
{
    if (parentWidget()==0) {
        setParent(mainwindow->centralWidget());
    }
    else {
        setParent(0);
    }
    show();
}

/**
 * @brief
 *
 */
void CPObject::slotLoadSession()
{
    ((CpcXXXX *)this)->LoadSession();
}

/**
 * @brief
 *
 */
void CPObject::slotSaveSession()
{
    ((CpcXXXX *)this)->SaveSession();
}

/**
 * @brief
 *
 */
void CPObject::Dump()
{
	dialogdump = new DialogDump(this);
	dialogdump->show();
}

/**
 * @brief
 *
 */
void CPObject::Dasm()
{
    if (dialogdasm) dialogdasm->show();
}

/**
 * @brief
 *
 */
void CPObject::Postit()
{
    mainwindow->LoadPocket("Post-it");
}

/**
 * @brief
 *
 */
void CPObject::VirtualKeyboard()
{
    dialogVKeyboard = new DialogVKeyboard(this);
    dialogVKeyboard->show();
}

/**
 * @brief
 *
 * @return bool
 */
bool CPObject::getdisp_onRaised()
{
    return disp_onRaised;
}

/**
 * @brief
 *
 * @param v
 */
void CPObject::setDisp_on(bool v)
{


    pLCDC->On = v;
    if (!v) {
        pLCDC->disp();
        if (pLCDC->Refresh) {
            Refresh_Display = true;
        }
    }

    if (v && !disp_on) disp_onRaised=true;
    else disp_onRaised = false;
    disp_on = v;
}

/**
 * @brief
 *
 * @return bool
 */
bool CPObject::getDisp_on()
{
    return disp_on;
}




//////////////////////////////////////////////
// Save Memory to file						//
//  ENTRY :BYTE s=Slot No.(SLOT1-3)			//
//  RETURN:none								//
//////////////////////////////////////////////
/**
 * @brief
 *
 * @param file
 * @param s
 */
void CPObject::Mem_Save(QFile *file,BYTE s)
{
    QDataStream out(file);
    out.writeRawData( (char *) &mem[SlotList[s].getAdr()],SlotList[s].getSize() * 1024 );
}

/**
 * @brief
 *
 * @param xmlOut
 * @param s
 * @param dumphex
 */
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

/**
 * @brief
 *
 * @param s
 */
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
/**
 * @brief
 *
 * @param file
 * @param s
 */
void CPObject::Mem_Load(QFile *file,BYTE s)
{
    QDataStream in(file);
    in.readRawData ((char *) &mem[SlotList[s].getAdr()],SlotList[s].getSize() * 1024 );
}

/**
 * @brief
 *
 * @param xmlIn
 * @param s
 */
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

/**
 * @brief
 *
 * @param s
 * @return bool
 */
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

/**
 * @brief
 *
 * @param adr
 * @param data
 * @return bool
 */
bool CPObject::Mem_Load(qint32 adr, QByteArray data ) {

    QDataStream in(data);
    int p =data.size();
    in.readRawData ((char *) &mem[adr], p );
    return true;
}

/**
 * @brief
 *
 * @param _bus
 */
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


/**
 * @brief
 *
 * @param bus
 * @param d
 * @param data
 */
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

/**
 * @brief
 *
 * @param bus
 * @param d
 * @param data
 */
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

//    if (bus->getRESET()) {
//        qWarning()<<"RESET";
//        Reset();
//    }

}
