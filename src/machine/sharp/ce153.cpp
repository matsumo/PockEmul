
#include <QPainter>
#include <QResource>
#include <QMouseEvent>

#include "common.h"
#include "ce153.h"
#include "lh5810.h"
#include "buspc1500.h"
#include "Inter.h"
#include "Connect.h"
#include "Keyb.h"

#include "mainwindowpockemul.h"
extern MainWindowPockemul *mainwindow;

Cce153::Cce153(CPObject *parent):CPObject(parent)
{
    Q_UNUSED(parent)

    //[constructor]
    BackGroundFname	= P_RES(":/pc1500/ce153.png");
    setcfgfname(QString("ce153"));

    setDXmm(240);//Pc_DX_mm = 240;
    setDYmm(174);//Pc_DY_mm = 174;
    setDZmm(13);//Pc_DZ_mm = 13;

    setDX(831);//Pc_DX	= 960;
    setDY(602);//Pc_DY	= 320;


    pTIMER		= new Ctimer(this);

    pLH5810   = new CLH5810(this);
//    pKEYB->setMap("ce150.map");

    bus = new CbusPc1500();

    keyPressed = 0xff;
    dragging = false;

}

Cce153::~Cce153() {
    delete pCONNECTOR;
    delete bus;
    delete pLH5810;

}

bool Cce153::init(void)
{
    CPObject::init();

    setfrequency( 0);
    pCONNECTOR	= new Cconnector(this,60,0,Cconnector::Sharp_60,"Connector 60 pins",true,QPoint(388,72));	publish(pCONNECTOR);

    if(pKEYB)	pKEYB->init();
    if(pTIMER)	pTIMER->init();
    pLH5810->init();

    return(1);

}

#define REVERSE8BITS(x) ((((x) * 0x0802LU & 0x22110LU) | ((x) * 0x8020LU & 0x88440LU)) * 0x10101LU >> 16)

#define PA		(pLH5810->lh5810.r_opa)
#define PB		(pLH5810->lh5810.r_opb)
#define PC		(pLH5810->lh5810.r_opc)

bool Cce153::run(void)
{

    bus->fromUInt64(pCONNECTOR->Get_values());

    quint16 addr = bus->getAddr();



    if (bus->isEnable() &&
        bus->isME1() &&
        (addr >= 0x8000) &&
        (addr <= 0x800f) &&
        (bus->isWrite()) )
    {
//        qWarning()<<"Write "<<QString("%1").arg(addr,4,16,QChar('0'))<<"="<<QString("%1").arg(bus->getData(),2,16,QChar('0'));
        lh5810_write();
    }


    if (keyPressed != 0xff) {
        quint8 _c = REVERSE8BITS(PC);
        quint8 _b = REVERSE8BITS(PB);
        quint16 strobe = ((_b & 0x3F)<<8) | _c ;
        if (strobe & (1 << (keyPressed >>4))) {
            quint16 _result = ( 1 << (keyPressed & 0x0f));
            PA =  (_result >> 3) & 0xff;
            PA |= (_result & 0x04) ? 0x80: 0;

            PB = (PB & 0xfc) |
                    ((_result&0x01)? 0x02:0x00) |
                    ((_result&0x02)? 0x01: 0x00);
        }
        else {
            PA = 0;
            PB &= 0xfc;
        }
    }

    pLH5810->step();

    if (bus->isEnable() &&
            !bus->isWrite() &&
            bus->isME1() &&
            (addr >= 0x8000) &&
            (addr <= 0x800f)) {
        lh5810_read();
//        if ((addr==0x800e) && (bus->getData()!=0))
//            qWarning()<<"Read "<<QString("%1").arg(addr,4,16,QChar('0'))<<"="<<QString("%1").arg(bus->getData(),2,16,QChar('0'));
    }

    bus->setEnable(false);
    pCONNECTOR->Set_values(bus->toUInt64());

    return true;
}

void Cce153::mouseDoubleClickEvent(QMouseEvent *event) {
    float _zoom = mainwindow->zoom;
    QPoint pts(event->x() , event->y());
    QRect kbdZone(118*_zoom,170*_zoom,636*_zoom,382*_zoom);

    if (kbdZone.contains(pts)) {
        QMouseEvent *e=new QMouseEvent(QEvent::MouseButtonPress, pts, Qt::LeftButton, Qt::LeftButton,Qt::NoModifier);
        QApplication::sendEvent(this, e);
        delete e;
        return;
    }
    else {
        CPObject::mouseDoubleClickEvent(event);
    }
}

void Cce153::mouseMoveEvent(QMouseEvent *event) {
#if 0

#else
    if (!dragging) {
        CPObject::mouseMoveEvent(event);
        return;
    }

    float _zoom = mainwindow->zoom;
    QPoint pts(event->x() , event->y());
    QRect kbdZone(118*_zoom,170*_zoom,636*_zoom,382*_zoom);

    if (kbdZone.contains(pts)) {
        int x = pts.x() - kbdZone.x();
        int y = pts.y() - kbdZone.y();
        int col = x / (45*_zoom);
        int row = y / (38*_zoom);
        keyPressed = ((col & 0xf) << 4 ) | (row & 0xf);
        event->accept();
        return;
    }else {
        keyPressed = 0xff;
        PA = 0;
        PB &= 0xfc;

    }
#endif
}

void Cce153::mousePressEvent(QMouseEvent *event)
{
    float _zoom = mainwindow->zoom;
    QPoint pts(event->x() , event->y());
    QRect kbdZone(118*_zoom,170*_zoom,636*_zoom,382*_zoom);

    if (kbdZone.contains(pts)) {
        int x = pts.x() - kbdZone.x();
        int y = pts.y() - kbdZone.y();
        int col = x / (45*_zoom);
        int row = y / (38*_zoom);
        keyPressed = ((col & 0xf) << 4 ) | (row & 0xf);
//        qWarning()<<"Key pressed:"<<QString("%1").arg(keyPressed,2,16,QChar('0'));
        event->accept();
        dragging =true;
        return;
    }
    else {
        dragging = false;
        CPObject::mousePressEvent(event);
    }
}

void Cce153::mouseReleaseEvent(QMouseEvent *event)
{
    dragging = false;
    keyPressed = 0xff;
    PA = 0;
    PB &= 0xfc;
    CPObject::mouseReleaseEvent(event);
}

bool Cce153::SaveSession_File(QXmlStreamWriter *xmlOut)
{
    pLH5810->save_internal(xmlOut);
    return true;
}

bool Cce153::LoadSession_File(QXmlStreamReader *xmlIn)
{
    pLH5810->Load_Internal(xmlIn);
    return true;
}

bool Cce153::exit(void)
{

    CPObject::exit();

    return true;
}


INLINE bool Cce153::lh5810_write(void)
{
    switch (bus->getAddr()) {
    case 0x8008: pLH5810->SetReg(CLH5810::OPC,bus->getData()); break;
    case 0x8009: pLH5810->SetReg(CLH5810::G  ,bus->getData()); break;
    case 0x800A: pLH5810->SetReg(CLH5810::MSK,bus->getData()); break;
    case 0x800B: pLH5810->SetReg(CLH5810::IF ,bus->getData()); break;
    case 0x800C: pLH5810->SetReg(CLH5810::DDA,bus->getData()); break;
    case 0x800D: pLH5810->SetReg(CLH5810::DDB,bus->getData()); break;
    case 0x800E: pLH5810->SetReg(CLH5810::OPA,bus->getData()); break;
    case 0x800F: pLH5810->SetReg(CLH5810::OPB,bus->getData()); break;
    default:  break;
    }

    return true;
}


INLINE bool Cce153::lh5810_read(void)
{
    switch (bus->getAddr()) {
    case 0x8008: bus->setData( pLH5810->GetReg(CLH5810::OPC) ); break;
    case 0x8009: bus->setData( pLH5810->GetReg(CLH5810::G)); break;
    case 0x800A: bus->setData( pLH5810->GetReg(CLH5810::MSK)); break;
    case 0x800B: bus->setData( pLH5810->GetReg(CLH5810::IF)); break;
    case 0x800C: bus->setData( pLH5810->GetReg(CLH5810::DDA)); break;
    case 0x800D: bus->setData( pLH5810->GetReg(CLH5810::DDB)); break;
    case 0x800E: bus->setData( pLH5810->GetReg(CLH5810::OPA)); break;
    case 0x800F: bus->setData( pLH5810->GetReg(CLH5810::OPB)); break;
    default: /*bus->setData(0x00);*/ break;
    }

    return true;
}


