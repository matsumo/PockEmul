//TODO  Take a look on the difference with the CE-126p

#include <QDebug>

#include <QPainter>

#include "ce125.h"
#include "ce152.h"


Cce125tape::Cce125tape(CPObject *parent)	: Cce152(parent)
{									//[constructor]
    BackGroundFname	= P_RES(":/ext/ce-125tape.png");
    playImage = P_RES(":/ext/ce-125play.png");
    recordImage=P_RES(":/ext/ce-125record.png");
    loadImage = P_RES(":/ext/ce-125load.png");
    setDX(249);
    setDY(299);

    pKEYB		= new Ckeyb(this,"ce125tape.map");
}

void Cce125tape::ComputeKey(KEYEVENT ke,int scancode)
{
    Q_UNUSED(ke)
    Q_UNUSED(scancode)


    if (scancode == K_PLAY)
    {
        switch (mode)
        {
        case STOP:
        case LOAD:
            BackGroundFname	= playImage;
            InitDisplay();
            update();
            Refresh_Display = true;
            emit updatedPObject(this);
            Play();
            break;
        default: break;
        }
    }

    if (scancode == K_EJECT)
    {
qWarning()<<"EJECT";
        switch (mode)
        {
        case EJECT:
        case STOP : BackGroundFname	= P_RES(":/ext/ce-125eject.png");
                    InitDisplay();
                    update();
                    Refresh_Display = true;
                    emit updatedPObject(this);
                    if (LoadTape())
                        BackGroundFname	= loadImage;
                    else
                        BackGroundFname	= P_RES(":/ext/ce-125tape.png");
                    InitDisplay();
                    update();
                    Refresh_Display = true;
                    emit updatedPObject(this);
            break;
        case LOAD : BackGroundFname	= P_RES(":/ext/ce-125eject.png");
                    InitDisplay();
                    update();
                    Refresh_Display = true;
                    emit updatedPObject(this);
                    if (LoadTape())
                        BackGroundFname	= loadImage;
                    else
                        BackGroundFname	= P_RES(":/ext/ce-125tape.png");
                    InitDisplay();
                    update();
                    Refresh_Display = true;
                    emit updatedPObject(this);
                    break;
        case RECORD:
        case PLAY : BackGroundFname	= loadImage;
                    InitDisplay();
                    update();
                    Refresh_Display = true;
                    emit updatedPObject(this);
                    StopPlay(); break;
        }
    }
    if (scancode == K_RECORD)
        {
            RecTape();
            BackGroundFname	= RECORD;
            InitDisplay();
            update();
            Refresh_Display = true;
        }
}

Cmp220::Cmp220(CPObject *parent):Cce125(parent)
{
    BackGroundFname	= P_RES(":/ext/mp-220.jpg");
    setcfgfname("mp220");
}

Cce125::Cce125(CPObject *parent):Cce126(parent),
    Tape(this)
{								//[constructor]
    BackGroundFname	= P_RES(":/ext/ce-125.png");
    setcfgfname("ce125");


    setDXmm(205);
    setDYmm(149);
    setDZmm(23);

    setDX(731);
    setDY(532);
    SnapPts = QPoint(247,280);

    remove(pTAPECONNECTOR);
    setPaperPos(QRect(377,0,207,149));

    delete pKEYB; pKEYB=new Ckeyb(this,"ce125.map");
}

Cce125::~Cce125() {
    //FIXME: Crash when close
//    delete pTAPE;
}

bool Cce125::UpdateFinalImage(void) {
    Cprinter::UpdateFinalImage();

    // Draw switch by 180° rotation
    QPainter painter;
    painter.begin(FinalImage);

    // Draw Tape
    painter.drawImage(0,0,Tape.FinalImage->scaled(Tape.size()*internalImageRatio));

    // PRINTER SWITCH
    painter.drawImage(282,235,BackgroundImageBackup->copy(282,235,30,20).mirrored(rmtSwitch,false));

    painter.end();

    emit updatedPObject(this);
    return true;
}

void Cce125::ComputeKey(CPObject::KEYEVENT ke, int scancode)
{
    qWarning()<<"ComputeKey"<<ke<<scancode<<pKEYB->LastKey;
    Cce126::ComputeKey(ke,scancode);

    Tape.ComputeKey(ke,scancode);
    if (Tape.Refresh_Display) {
        Tape.Refresh_Display = false;
        Refresh_Display = true;
    }
}

void Cce125::resizeEvent ( QResizeEvent * event ) {
    float ratio = (float)this->width()/this->getDX() ;

    QRect rect = QRect(0,0,Tape.getDX(),Tape.getDY());
    Tape.setGeometry(rect.x()*ratio,
              rect.y()*ratio,
              rect.width()*ratio,
              rect.height()*ratio);

    Cce126::resizeEvent(event);

}

bool Cce125::init(void)
{
	Cce126::init();
    pCONNECTOR->setSnap(QPoint(247,367));

    Tape.init();
    Tape.InitDisplay();
    Tape.hide();
//    Tape.show();
	
	return true;
}

bool Cce125::exit(void)
{
    Tape.exit();
	
	Cce126::exit();
	return true;
}

bool Cce125::run(void)
{
	Cce126::run();
	
    Tape.pTAPECONNECTOR->Set_pin(3,(rmtSwitch ? GET_PIN(PIN_SEL1):true));
    Tape.pTAPECONNECTOR->Set_pin(2,GET_PIN(PIN_MT_OUT1));
    Tape.pTIMER = pTIMER;
    Tape.run();
    SET_PIN(PIN_MT_IN,Tape.pTAPECONNECTOR->Get_pin(1));
	return true;	
}


