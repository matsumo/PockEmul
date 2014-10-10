#include <QDebug>
#include <QPainter>
#include <QTimer>

#include "Lcdc_hp41.h"
#include "Lcdc_symb.h"

const byte ASCIItoLCD[128]=
    {
    0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,
    0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,
    0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x2a,0x2b,0xa3,0x2d,0xa2,0x2f,    // exceptions ,.
    0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0xa1,0x3a,0x3c,0x3d,0x3e,0x3f,    // exceptions :;
    0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,
    0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,
    0x60,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6a,0x6b,0x6c,0x6d,0x6e,0x6f,
    0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x78,0x79,0x7a,0x7b,0x3a,0x7d,0x3a,0x3a,    // exceptions |~
    };

QByteArray ConvertASCIItoLCD(QByteArray *ba)
  {
    QByteArray out;
  for (int i=0;i<ba->size();i++)
    {
//    if (ba->at(i) < 0x80)
        out.append(ba->at(i));
//    if ((byte)(ba.at(i)<0x80))
//      out[i]= out.at(i)+0x20;
    }
  return out;
  }


void Clcdc_hp41::disp_symb(void)
{
    delete SymbImage;
    SymbImage = pPC->CreateImage(symbRect.size(),SymbFname,false,false,0);
    QPainter painter(SymbImage);

    painter.setCompositionMode(QPainter::CompositionMode_Source);
    if (! (hp41->DIS_ANNUN_REG&0x0800)) {painter.fillRect(  0,0,18,8,Qt::transparent); }// bat
    if (! (hp41->DIS_ANNUN_REG&0x0400)) {painter.fillRect( 24,0,24,8,Qt::transparent); }// user
    if (! (hp41->DIS_ANNUN_REG&0x0200)) {painter.fillRect( 54,0, 6,8,Qt::transparent); }// g
    if (! (hp41->DIS_ANNUN_REG&0x0100)) {painter.fillRect( 60,0,18,8,Qt::transparent); }// rad
    if (! (hp41->DIS_ANNUN_REG&0x0080)) {painter.fillRect( 84,0,30,8,Qt::transparent); }// shift
    if (! (hp41->DIS_ANNUN_REG&0x0040)) {painter.fillRect(120,0, 6,8,Qt::transparent); }// 0
    if (! (hp41->DIS_ANNUN_REG&0x0020)) {painter.fillRect(126,0, 6,8,Qt::transparent); }// 1
    if (! (hp41->DIS_ANNUN_REG&0x0010)) {painter.fillRect(132,0, 6,8,Qt::transparent); }// 2
    if (! (hp41->DIS_ANNUN_REG&0x0008)) {painter.fillRect(138,0, 6,8,Qt::transparent); }// 3
    if (! (hp41->DIS_ANNUN_REG&0x0004)) {painter.fillRect(144,0, 6,8,Qt::transparent); }// 4
    if (! (hp41->DIS_ANNUN_REG&0x0002)) {painter.fillRect(156,0,24,8,Qt::transparent); }// prgm
    if (! (hp41->DIS_ANNUN_REG&0x0001)) {painter.fillRect(186,0,30,8,Qt::transparent); }// alpha

    painter.end();
}

Clcdc_hp41::Clcdc_hp41(CPObject *parent, QRect _lcdRect, QRect _symbRect, QString _lcdfname, QString _symbfname):
    Clcdc(parent,_lcdRect,_symbRect,_lcdfname,_symbfname){						//[constructor]
    Color_Off.setRgb(
                (int) (111*contrast),
                (int) (117*contrast),
                (int) (108*contrast));

    hp41 = (Chp41*) parent;
    hp41cpu = hp41->hp41cpu;
}

void Clcdc_hp41::disp(void)
{

    Refresh = false;

    if (!ready) return;
    if (!updated) return;
    //    qWarning()<<"disp";
    updated = false;
    Refresh= true;
//    delete LcdImage;
//    LcdImage = pPC->CreateImage(QSize(pPC->Lcd_DX, pPC->Lcd_DY),pPC->LcdFname,false,false,0);



    QPainter painter(LcdImage);
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    painter.fillRect(LcdImage->rect(),Qt::transparent);

    QFont font;
    font.setFamily("HP41 Character Set");
    font.setPixelSize(20);
    painter.setFont(font);
    QByteArray ba;
    ba[0]=0x1;
    ba[1]=0x2;
    ba[2]=0x3;
    ba[3]=0x4;
    ba[4]=0x5;
    ba[5]=0x6;
    ba[6]=0x7;
    ba[7]=0x8;
    ba[8]=0x9;
    ba[9]=0xa;
    ba[10]=0xb;
    ba[11]=0xc;

    ba[12]=0xd;

//    ba = ConvertASCIItoLCD(GetLCD());

//    painter.drawText(LcdImage->rect(),ba);
    painter.drawText(LcdImage->rect(),GetLCD());

    disp_symb();

//    qWarning()<<"Display:"<<GetLCD()<<" : ";

    Refresh = true;
    redraw = 0;
    painter.end();
}



/***************************/
// Get LCD string from registers based on current font mapping.
// Always 24 chars long, Could have 0 as a char value
/***************************/
QString Clcdc_hp41::GetLCD()
  {

  char ch;
  char punct;
  QString out;


  for (int i=0;i<12;i++)
    {
    // chr= cbbaaaa = 0x00 - 0x7f
    ch=(hp41->DIS_C_REG[11-i]<<6) | ((hp41->DIS_B_REG[11-i]&0x03)<<4) | hp41->DIS_A_REG[11-i];
    punct=(hp41->DIS_B_REG[11-i]&0x0c)>>2;
//    if (eFont==eFontLCD4)
//      *(ptr++)=ch;
//    else
//    qWarning()<<"char:"<<ch;
    if (ch==0x20) out.append(ch);
    else if (ch==0x2E) out.append(0xFE);
    else if ((ch>=0x00) && (ch<=0x1F)) out.append(0x40 + ch);
    else if ((ch>=0x20) && (ch<=0x3F)) out.append(ch);
    else if ((ch>=0x40) && (ch<=0x4F)) out.append(ch+0x19);
//    else if ((ch>=0x61) && (ch<=0x6F)) out.append(ch - 0x40);
    switch (punct)
      {
      case 0:     // none
//        out.append(QChar(0xF020));
        break;
      case 1:     // period
        out.append(0x2e);
        break;
      case 2:     // colon
        out.append(0x3a);
        break;
      case 3:     // comma
        out.append(0x2c);
        break;
      }
//    if (UseAltPunc)     // slightly narrower punc chars
//      *ptr+=4;
//    ptr++;
    }
  return out;
  }
