#include <QDebug>

#include "hd66108.h"



CHD66108::CHD66108(CpcXXXX *parent)
{
    pPC = parent;

    for (int i = 0 ; i < 0x200 ; i++)
    {
        info.imem[i] = 0;
    }

    info.on_off = 0;
    info.displaySL = 0;
    info.Xadr = 0;
    info.Yadr = 0;
    info.status = 0;


    updated = true;
}

CHD66108::~CHD66108() {

}

UINT8 CHD66108::readVram( int p)
{

    updated=true;
    if(p & 1)
        return 0;
    else
        return reg;
}

void CHD66108::pset(UINT8 *vram, int x, int y, int pix)
{
//    qWarning()<<"pset:"<<x<<y<<pix;
    if(pix)
        vram[y * (LCD_WIDTH / 8) + (x / 8)] |= (0x80 >> (x % 8));
    else
        vram[y * (LCD_WIDTH / 8) + (x / 8)] &= ~(0x80 >> (x % 8));
}

void CHD66108::writeVram( int p, UINT8 v)
{

    int x, y;

    updated=true;
    if(p & 1) {

//        qWarning()<<"mode:"<<(reg&7);
        switch(reg & 7) {
        case 0:
            if(fcr & 0x40) {
                if(xar < VRAM_WIDTH && yar < VRAM_HEIGHT) {
                    ram[xar][yar]=v;
                    if (6 <= xar) {
                        x = (yar / 32) * 96 + (xar - 6) * 6;
                        y = (yar % 32);
                        pset(vram, x + 0, y, v & 0x20);
                        pset(vram, x + 1, y, v & 0x10);
                        pset(vram, x + 2, y, v & 0x08);
                        pset(vram, x + 3, y, v & 0x04);
                        pset(vram, x + 4, y, v & 0x02);
                        pset(vram, x + 5, y, v & 0x01);
                    }
                    else {
//                        qWarning()<<xar<<yar<<v;
                    }

                }
            } else {
                if(xar < VRAM_WIDTH && yar < VRAM_HEIGHT) {
                    ram[xar][yar]=v;
                    if (4 <= xar) {
                        x = (yar / 32) * 96 + (xar - 4) * 8;
                        y = (yar % 32);
                        pset(vram, x - 4, y, v & 0x80);
                        pset(vram, x - 3, y, v & 0x40);
                        pset(vram, x - 2, y, v & 0x20);
                        pset(vram, x - 1, y, v & 0x10);
                        pset(vram, x + 0, y, v & 0x08);
                        pset(vram, x + 1, y, v & 0x04);
                        pset(vram, x + 2, y, v & 0x02);
                        pset(vram, x + 3, y, v & 0x01);
                    }
                    else {
//                        qWarning()<<xar<<yar<<v;
                    }

                }
            }
            if(fcr & 0x80)
                xar++;
            else
                yar++;
            break;
        case 1:
            xar = v;
            /*
            printf("x=%d,y=%d\n", z1->lcd.xar, z1->lcd.yar);
            */
            break;
        case 2:
            yar = v;
            /*
            printf("x=%d,y=%d\n", z1->lcd.xar, z1->lcd.yar);
            */
            break;
        case 3:
            fcr = v;
            break;
        case 4:
            break;
        case 5:
            break;
        }
    } else
        reg = v;



}


void CHD66108::Load_Internal(QXmlStreamReader *xmlIn)
{
    if (xmlIn->readNextStartElement()) {
        if ( (xmlIn->name()=="cpu") &&
             (xmlIn->attributes().value("model").toString() == "hd66108")) {
            QByteArray ba_reg = QByteArray::fromBase64(xmlIn->attributes().value("registers").toString().toLatin1());
            memcpy((char *) &info,ba_reg.data(),sizeof(info));
        }
        xmlIn->skipCurrentElement();
    }
}

void CHD66108::save_internal(QXmlStreamWriter *xmlOut)
{
    xmlOut->writeStartElement("cpu");
        xmlOut->writeAttribute("model","hd66108");
        QByteArray ba_reg((char*)&info,sizeof(info));
        xmlOut->writeAttribute("registers",ba_reg.toBase64());
    xmlOut->writeEndElement();
}



bool CHD66108::init()
{
//    memset(&vram[0],0x00,sizeof(vram));
//    reg=fcr=xar=yar=0;
    return true;
}


void CHD66108::Reset()
{
    memset(&vram[0],0x00,sizeof(vram));
    reg=fcr=xar=yar=0;
}
