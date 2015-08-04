#include <QDebug>
#include <QPainter>
#include <QTimer>

#include "Lcdc_ti57.h"
#include "Lcdc_symb.h"
#include "ti/ti57.h"


void Clcdc_ti57::disp_symb(void)
{
}

Clcdc_ti57::Clcdc_ti57(CPObject *parent, QRect _lcdRect, QRect _symbRect, QString _lcdfname, QString _symbfname):
    Clcdc(parent,_lcdRect,_symbRect,_lcdfname,_symbfname){						//[constructor]
    Color_Off.setRgb(
                (int) (111*contrast),
                (int) (117*contrast),
                (int) (108*contrast));

    blinkTimer = new QTimer(this);
    connect(blinkTimer,SIGNAL(timeout()),this,SLOT(blink()));
    blinkState = false;

}

void Clcdc_ti57::disp(void)
{
    bool error = false;

    Refresh = false;

    if (!ready) return;
    if (!redraw) {
        if (!updated) return;
    }
    redraw = false;
    updated = false;
    Refresh= true;

    QPainter painter(LcdImage);
    painter.setCompositionMode(QPainter::CompositionMode_Source);
//    painter.setPen(Qt::transparent);
//    painter.setBrush(Qt::transparent);
    painter.fillRect(LcdImage->rect(),Qt::transparent);

    QFont font;
    font.setPixelSize(30);
    painter.setFont(font);

    QString s = ((Cti57 *)pPC)->Display();
    // Check for ERROR
    if (s.startsWith('E')) {
//        qWarning()<<"ERROR";
        s.remove(0,1);  // Remove the error
        // Start Blinking timer
        if (!blinkTimer->isActive()) {
            blinkTimer->start(500);
            blinkState = true;
//            qWarning()<<"start timer";
        }
        error = true;
    }
    else {
        blinkTimer->stop();
    }

//    qWarning()<<"display:"<<s;

#define YOFFSET 5

    if (!error || blinkState) {
        //        qWarning()<<"print text";
        painter.setPen(QColor(255,0,0));
#if 0
        painter.drawText(LcdImage->rect(),Qt::AlignCenter,s);
#else
        int pos = 0;
        int charSpace = 18;
        for (int i =0;i<s.length();i++) {
            int code = s.at(i).digitValue();
            if (s.at(i) == '-') code = 16;
            if (s.at(i) == '.') code = 17;
            if (s.at(i) == ' ') pos+=charSpace;

            if (code == 17) {
                QRect rect(pos- charSpace + 10,12,1,1);
                painter.drawRect(rect);
            }
            else
                if (code >=0) {

                    QRect rect(pos,0,8,13);

                    Clcd7::draw(code,&painter,rect,1,0);
                    pos += charSpace;
                }
        }
#endif
    }
    //On=true;
    Refresh = true;
    painter.end();
}

void Clcdc_ti57::blink() {


    blinkState = !blinkState;
//qWarning()<<"blink timer "<<blinkState;
    updated = true;
}


Clcd7::Clcd7()
{
    // Set initial values
    gap		= 1;
    width	= 0;
    oldnum = num = 0;

    low		= 0;
    high	= 9;
    firstpaint = true;
    seg1= seg2= seg3= seg4= seg5= seg6= seg7 = false;

}



Clcd7::~Clcd7()
{
}

/////////////////////////////////////////////////////////////////////////////
// Clcd7 message handlers

void Clcd7::draw(UINT8 code, QPainter *painter, QRect rect, int width, int gap = 1)

{
bool seg1, seg2, seg3, seg4, seg5, seg6, seg7;


    // Start from all reset
    seg1 = seg2 = seg3 = seg4 = seg5 = seg6 = seg7 = false;

    // Coding of segments:
    //				  1
    //			 ---------
    //		4	|    3    | 6
    //			 ---------
    //		5	|         | 7
    //			 ---------
    //               2

    // Set the ones that need to be set:
    switch(code)
        {
        case 0: seg1 = seg2 = seg4 = seg5 = seg6 = seg7 = true;	break;
        case 1:	seg6= seg7 = true; break;
        case 2: seg1 = seg6 = seg3 = seg5 = seg2 = true; break;
        case 3: seg1 = seg6 = seg3 = seg7 = seg2 = true; break;
        case 4: seg4  = seg3 = seg6 = seg7 = true;	break;
        case 5: seg1  = seg4 = seg3 = seg7 = seg2 = true; break;
        case 6: seg1 = seg4  = seg5 = seg2 = seg7 = seg3 = true; break;
        case 7: seg1  = seg6  = seg7 = true; break;
        case 8: seg1 = seg2 = seg3 = seg4 = seg5 = seg6 = seg7 = true; break;
        case 9: seg1 = seg3 = seg4 = seg6 = seg7 = seg2 = true; break;
        case 10: seg1 = seg4 = seg5 = seg3 = seg6 = seg7 = true; break;  // A
        case 11: seg4 = seg5 = seg2 = seg3 = seg7 = true; break;  // b
        case 12: seg3 = seg5 = seg2 = true; break;  // c
        case 13: seg3 = seg5 = seg2 = seg7 = seg6 = true; break;  // d
        case 14: seg1 = seg4 = seg3 = seg5 = seg2 = true; break;  // e
        case 15: seg1 = seg4 = seg3 = seg5 = true; break;  // f
        case 16: seg3 = true; break;  // -
        case 17: seg1 = seg4 = seg5 = seg2 = true; break;  // C
    }


    int vhalf = rect.height() / 2;

    painter->translate(rect.left(),0);
    QPolygon poly;

    ///////////////////////////////////////////////////
    ///    0---------------------------1
    //        3--------------------2
    //

    poly << QPoint(0,0)
         << QPoint(rect.width(),0)
         << QPoint(rect.width() - width,width)
         << QPoint(width,width);

    if(seg1)
        painter->drawPolygon(poly);

    ///////////////////////////////////////////////////
    //        0--------------------1
    ///    3---------------------------2

    poly.clear();
    poly << QPoint(width, rect.height() - width )
         << QPoint(rect.width() - width,rect.height() - width )
         << QPoint(rect.width(),rect.height())
         << QPoint(0,rect.height());

    if(seg2)
        painter->drawPolygon(poly);

    ///////////////////////////////////////////////////
    //        0--------------------1
    ///    5---------------------------2
    ///        4--------------------3
    //
    poly.clear();
    poly << QPoint(width, vhalf - width/2 + gap)
         << QPoint(rect.width() - width, vhalf - width/2 + gap)
         << QPoint(rect.width(), vhalf)
         << QPoint(rect.width() - width, vhalf + width/2 + gap)
         << QPoint(width, vhalf + width/2 + gap)
         << QPoint(0, vhalf + gap);

    if(seg3)
        painter->drawPolygon(poly);


    //////////////////////////////////////////////////
    //	 0
    //  |  3
    //  | |
    //  |  2
    //   1

    poly.clear();
    poly << QPoint(0,gap)
         << QPoint(0,vhalf)
         << QPoint(width,vhalf - (width/2))
         << QPoint( width, width + gap);

    if(seg4)
        painter->drawPolygon(poly);

    poly.clear();
    poly << QPoint(0,vhalf + gap + gap)
         << QPoint(0,vhalf + vhalf -  gap)
         << QPoint(width,vhalf - (width/2) + vhalf -  gap - width/2)
         << QPoint( width, width + gap + vhalf - width/2 + gap);

    if(seg5)
        painter->drawPolygon(poly);

    //////////////////////////////////////////////////
    //	        3
    //      0  |
    //       | |
    //      1  |
    //          2
    poly.clear();
    poly << QPoint(rect.width() - (width), (width)  + gap)
         << QPoint(rect.width() - (width), vhalf - (width/2))
         << QPoint(rect.width(),vhalf - gap)
         << QPoint(rect.width(),gap);

    if(seg6)
        painter->drawPolygon(poly);

    poly.clear();
    poly << QPoint(rect.width() - (width), (width)  + gap + vhalf - width/2 + gap)
         << QPoint(rect.width() - (width), vhalf - (width/2) + vhalf - width/2 - gap)
         << QPoint(rect.width(),vhalf - gap + vhalf)
         << QPoint(rect.width(),vhalf + gap);

    if(seg7)
        painter->drawPolygon(poly);


    painter->translate(-rect.left(),0);
    // Just for testing
    //dc.SetBkMode(TRANSPARENT );
    //CString str; str.Format("%d", num);
    //rc.bottom = rc.bottom / 2;
    //dc.DrawText(str, &rc, DT_CENTER | DT_SINGLELINE | DT_VCENTER);

    // Do not call CWnd::OnPaint() for painting messages
}
