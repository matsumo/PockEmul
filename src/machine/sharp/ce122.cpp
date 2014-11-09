#include <QPainter>
#include <QImage>
#include <QDebug>

#include "ce122.h"
#include "Connect.h"
#include "Keyb.h"
#include "watchpoint.h"
#include "Log.h"
#include "paperwidget.h"

Cce122::Cce122(CPObject *parent):Cce126(parent)
{								//[constructor]
    BackGroundFname	= P_RES(":/ext/ce-122.png");
    setcfgfname("ce122");
    setDX(1002);//Pc_DX	= 282;
    setDY(330);//Pc_DY	= 95;
    setDXmm(282);
    setDYmm(95);

    setPaperPos(QRect(90,0,158,151));

    delete pKEYB; pKEYB=new Ckeyb(this,"ce122.map");

    Power=true;
    printSwitch = true;

}

bool Cce122::init(void)
{
    Cce126::init();

    setfrequency( 0);
    remove(pCONNECTOR); delete pCONNECTOR;
    pCONNECTOR	= new Cconnector(this,9,0,Cconnector::Sharp_9,"Connector 9 pins",true,QPoint(372,107));	publish(pCONNECTOR);

    WatchPoint.add(&pCONNECTOR_value,64,9,this,"Standard 11pins connector");
    WatchPoint.add(&pTAPECONNECTOR_value,64,2,this,"Line In / Rec");

    AddLog(LOG_PRINTER,tr("PRT initializing..."));

    if(pKEYB)	pKEYB->init();
    if(pTIMER)	pTIMER->init();

    // Create CE-122 Paper Image
    // The final paper image is 207 x 149 at (277,0) for the ce125
    ce126buf	= new QImage(QSize(130, 3000),QImage::Format_ARGB32);
    ce126display= new QImage(QSize(130, 149),QImage::Format_ARGB32);
    margin = 5;



    charTable = new QImage(P_RES(":/ext/ce122table.bmp"));

//	bells	 = new QSound("ce.wav");

// Create a paper widget

    paperWidget = new CpaperWidget(PaperPos(),ce126buf,this);
    paperWidget->updated = true;
    paperWidget->show();

    // Fill it blank
    clearPaper();



    return true;
}


bool Cce122::run(void)
{



    char c = (pCONNECTOR->Get_values()>>1) & 0xFF;

    if (c>0) {
//        qWarning()<<"CE122:RECEIVED:"<<QString(c)<<"-"<<(int)c<<"--y="<<top;
        if (c=='\n') {
            c=0x0d;
//            qWarning()<<"CR RECEIVED";
        }
        RefreshCe126(c);

        pCONNECTOR->Set_values(0);
    }

    if (Power && printSwitch) {
        pCONNECTOR->Set_pin(1,true);
    }
    else pCONNECTOR->Set_pin(1,false);


    return true;
}

void Cce122::ComputeKey(KEYEVENT ke,int scancode)
{
    Q_UNUSED(ke)
    Q_UNUSED(scancode)


    if (pKEYB->LastKey == K_PFEED) {
        RefreshCe126(0x0d);
    }
    if (pKEYB->LastKey == K_RMT_ON) {
        rmtSwitch = true;
        emit updatedPObject(this);
        update();
    }
    if (pKEYB->LastKey == K_RMT_OFF) {
        rmtSwitch = false;
        emit updatedPObject(this);
        update();
    }
    if (pKEYB->LastKey == K_PRINT_ON) {
        printSwitch = true;
        emit updatedPObject(this);
        update();
    }
    if (pKEYB->LastKey == K_PRINT_OFF) {
        printSwitch = false;
        emit updatedPObject(this);
        update();
    }
}

bool Cce122::UpdateFinalImage(void) {
    Cprinter::UpdateFinalImage();

    // Draw switch by 180° rotation
    QPainter painter;

    // PRINTER SWITCH
    painter.begin(FinalImage);

    painter.drawImage(257,257,BackgroundImageBackup->copy(257,257,22,30).mirrored(false,Power));
    painter.drawImage(203,257,BackgroundImageBackup->copy(203,257,22,30).mirrored(false,printSwitch));
    painter.end();

    Refresh_Display = true;
    emit updatedPObject(this);
    return true;
}

void Cce122::TurnON()
{
    Power = true;
    update();
    emit updatedPObject(this);
}

void Cce122::TurnOFF()
{
    Power = false;
    update();
    emit updatedPObject(this);
}
