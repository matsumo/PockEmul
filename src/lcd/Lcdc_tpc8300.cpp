#include <QPainter>
#include <QDebug>

#include "common.h"
#include "pcxxxx.h"
#include "cpu.h"
#include "tpc8300.h"
#include "upd16434.h"


#include "Lcdc_tpc8300.h"
#include "Log.h"

Clcdc_tpc8300::Clcdc_tpc8300(CPObject *parent )	: Clcdc(parent){						//[constructor]
    Color_Off.setRgb(
                        (int) (101*contrast),
                        (int) (99*contrast),
                        (int) (92*contrast));
}





void Clcdc_tpc8300::disp_symb(void)
{

    Clcdc::disp_symb();
}


void Clcdc_tpc8300::disp(void)
{

    Ctpc8300 *tpc8300 = (Ctpc8300*)pPC;

    BYTE b;

    Refresh = false;


    if (!ready) return;
    if (!tpc8300->upd16434[0]) return;
//qWarning()<<"disp";
    disp_symb();

    QPainter painter(pPC->LcdImage);

    for (int i = 0 ; i<3; i++)
    {

        if (tpc8300->upd16434[i]->updated) {
            Refresh = true;
//            qWarning()<<"update:"<<i;
            //AddLog(LOG_DISPLAY,"DISP");
            tpc8300->upd16434[i]->updated = false;
            for (int j = 0; j< 0x28;j++)
            {
                BYTE data = tpc8300->upd16434[i]->info.imem[0x27 - j];
                for (b=0;b<8;b++)
                {
                    painter.setPen(((data>>b)&0x01)? Color_On : Color_Off);
                    painter.drawPoint(j + j/5 + i*48,b + (b==7));
                }
            }
            for (int j = 0; j< 0x28;j++)
            {
                BYTE data = tpc8300->upd16434[i]->info.imem[0x67-j];
                for (b=0;b<8;b++)
                {
                    painter.setPen(((data>>b)&0x01)? Color_On : Color_Off);
                    painter.drawPoint(j + j/5 + i*48,b + (b==7)+12);
                }
            }
        }
    }

    redraw = 0;
    painter.end();
}





