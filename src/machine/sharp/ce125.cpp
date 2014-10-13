//TODO  Take a look on the difference with the CE-126p

#include <QPainter>

#include "ce125.h"
#include "ce152.h"

TransMap KeyMapce125tape[]={
    {1,	"EJECT/LOAD  ",	K_EJECT,34,234,	9},		//OK
    {2,	"PLAY        ",	K_PLAY,156,234,	9},		//OK
    {3,	"RECORD      ",	K_RECORD,194,234,9},		//OK
};
int KeyMapce125tapeLenght = 3;

TransMap KeyMapce125[]={
    {1,	"FEED  ",	K_PFEED,34,234,	9},		//OK
    {2,	"RMT ON",	K_RMT_ON,34,234,	9},
    {3,	"RMT OFF",	K_RMT_OFF,34,234,	9},
    {4,	"POWER ON",	K_POW_ON,34,234,	9},
    {5,	"POWER OFF",K_POW_OFF,34,234,	9}
};
int KeyMapce125Lenght = 5;


void Cce125tape::ComputeKey(void)
{

    if (pKEYB->LastKey == K_PLAY)
    {
        switch (mode)
        {
        case STOP:
        case LOAD:
            BackGroundFname	= playImage;
            InitDisplay();
            update();
            Play();
            break;
        default: break;
        }
    }

	if (pKEYB->LastKey == K_EJECT) 
	{
		
		switch (mode)
		{
		case EJECT: 
        case STOP : BackGroundFname	= P_RES(":/ext/ce-125eject.png");
					InitDisplay();
					update();
					if (LoadTape())
                        BackGroundFname	= loadImage;
					else
                        BackGroundFname	= P_RES(":/ext/ce-125tape.png");
					InitDisplay();
					update(); break;
        case LOAD : BackGroundFname	= P_RES(":/ext/ce-125eject.png");
					InitDisplay();
					update();
					if (LoadTape())
                        BackGroundFname	= loadImage;
					else
                        BackGroundFname	= P_RES(":/ext/ce-125tape.png");
					InitDisplay();
					update(); break;
		case RECORD:
        case PLAY : BackGroundFname	= loadImage;
					InitDisplay();
					update();
					StopPlay(); break;
		}
	}
	if (pKEYB->LastKey == K_RECORD) 
		{
            RecTape();
            BackGroundFname	= RECORD;
            InitDisplay();
            update();
		}
}

Cmp220::Cmp220(CPObject *parent):Cce125(parent)
{
    BackGroundFname	= P_RES(":/ext/mp-220.jpg");
    setcfgfname("mp220");
}

Cce125::Cce125(CPObject *parent):Cce126(parent)
{								//[constructor]
    BackGroundFname	= P_RES(":/ext/ce-125.png");
    setcfgfname("ce125");


    setDXmm(205);//Pc_DX_mm = 205;
    setDYmm(149);//Pc_DY_mm = 149;
    setDZmm(23);//Pc_DZ_mm = 23;

    setDX(731);//Pc_DX	= 731;
    setDY(532);//Pc_DY	= 532;
    SnapPts = QPoint(247,280);

    remove(pTAPECONNECTOR);
    setPaperPos(QRect(377,0,207,149));

    KeyMap		= KeyMapce125;
    KeyMapLenght= KeyMapce125Lenght;
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

    // PRINTER SWITCH
    painter.begin(FinalImage);
    painter.drawImage(282,235,BackgroundImageBackup->copy(282,235,30,20).mirrored(rmtSwitch,false));

    painter.end();

    Refresh_Display = true;
    emit updatedPObject(this);
    return true;
}

void Cce125::resizeEvent ( QResizeEvent * event ) {
    float ratio = (float)this->width()/this->getDX() ;

    QRect rect = QRect(0,0,pTAPE->getDX(),pTAPE->getDY());
    pTAPE->setGeometry(rect.x()*ratio,
              rect.y()*ratio,
              rect.width()*ratio,
              rect.height()*ratio);

    Cce126::resizeEvent(event);

}

bool Cce125::init(void)
{
	Cce126::init();
    pCONNECTOR->setSnap(QPoint(247,367));

	pTAPE = new Cce125tape( this );
	pTAPE->setParent ( this );
	pTAPE->init();
	pTAPE->InitDisplay();
	pTAPE->show();
	
	return true;
}

bool Cce125::exit(void)
{
    pTAPE->exit();
	delete pTAPE;
	
	Cce126::exit();
	return true;
}

bool Cce125::run(void)
{
	Cce126::run();
	
    pTAPE->pTAPECONNECTOR->Set_pin(3,(rmtSwitch ? GET_PIN(PIN_SEL1):true));
	pTAPE->pTAPECONNECTOR->Set_pin(2,GET_PIN(PIN_MT_OUT1));
	pTAPE->pTIMER = pTIMER;
	pTAPE->run();
	SET_PIN(PIN_MT_IN,pTAPE->pTAPECONNECTOR->Get_pin(1));
	return true;	
}


