#include <QDebug>
#include <QPainter>
#include <QTimer>

#include "Lcdc_hp15c.h"
#include "Lcdc_symb.h"

#define VOYAGER_DISPLAY_DIGITS 11


typedef struct
{
  int reg;
  int dig;
  int bit;
} voyager_segment_info_t;


// For each of 11 digits, we need segments a-g for the actual digit,
// segment h for the decimal point, segment i for the tail of the comma,
// and segment j for the annunciator.
voyager_segment_info_t voyager_display_map [11] [10] =
  {
    {  /* leftmost position has only segment g for a minus */
      { 0,  0, 0 }, { 0,  0, 0 }, { 0,  0, 0 }, { 0,  0, 0 }, { 0,  0, 0 },
      { 0,  0, 0 }, { 0, 11, 4 }, { 0,  0, 0 }, { 0,  0, 0 },
      { 0,  0, 0 }  // no annunciator
    },
    {
      { 0,  5, 2 }, { 0,  5, 8 }, { 0,  4, 8 }, { 0, 11, 8 }, { 0,  4, 4 },
      { 0,  5, 1 }, { 0,  5, 4 }, { 0,  9, 8 }, { 0,  9, 4 },
      { 0,  0, 0 }  // no annunciator - "*" for low bat in KML, but that's
                    // not controllable by the calculator microcode
    },
    {
      { 0,  6, 8 }, { 0,  7, 2 }, { 0,  6, 2 }, { 0,  4, 2 }, { 0,  6, 1 },
      { 0,  6, 4 }, { 0,  7, 1 }, { 0,  3, 8 }, { 0,  3, 4 },
      { 0,  4, 1 }  // USER annunciator
    },
    {
      { 0, 12, 8 }, { 0, 13, 2 }, { 0, 12, 2 }, { 0,  3, 2 }, { 0, 12, 1 },
      { 0, 12, 4 }, { 0, 13, 1 }, { 0, 13, 8 }, { 0, 13, 4 },
      { 0,  3, 1 }  // f annunciator
    },
    {
      { 0,  8, 2 }, { 0,  8, 8 }, { 0,  7, 8 }, { 0,  2, 2 }, { 0,  7, 4 },
      { 0,  8, 1 }, { 0,  8, 4 }, { 0,  9, 2 }, { 0,  9, 1 },
      { 0,  2, 1 }  // g annunciator
    },
    {
      { 0, 10, 8 }, { 0, 11, 2 }, { 0, 10, 2 }, { 0,  1, 8 }, { 0, 10, 1 },
      { 0, 10, 4 }, { 0, 11, 1 }, { 0,  2, 8 }, { 0,  2, 4 },
      { 0,  1, 4 }  // BEGIN annunciator
    },
    {
      { 1,  2, 8 }, { 1,  3, 2 }, { 1,  2, 2 }, { 1,  3, 8 }, { 1,  2, 1 },
      { 1,  2, 4 }, { 1,  3, 1 }, { 1,  4, 2 }, { 1,  4, 1 },
      { 1,  3, 4 }  // G annunciator (for GRAD, or overflow on 16C)
    },
    {
      { 1,  5, 2 }, { 1,  5, 8 }, { 1,  4, 8 }, { 1,  1, 8 }, { 1,  4, 4 },
      { 1,  5, 1 }, { 1,  5, 4 }, { 1,  6, 2 }, { 1,  6, 1 },
      { 1,  1, 4 }  // RAD annunciator
    },
    {
      { 1,  7, 2 }, { 1,  7, 8 }, { 1,  6, 8 }, { 1,  9, 8 }, { 1,  6, 4 },
      { 1,  7, 1 }, { 1,  7, 4 }, { 1,  9, 2 }, { 1,  9, 1 },
      { 1,  9, 4 }  // D.MY annunciator
    },
    {
      { 1, 11, 8 }, { 1, 12, 2 }, { 1, 11, 2 }, { 1,  8, 2 }, { 1, 11, 1 },
      { 1, 11, 4 }, { 1, 12, 1 }, { 1,  8, 8 }, { 1,  8, 4 },
      { 1,  8, 1 }  // C annunciator (Complex on 15C, Carry on 16C)
    },
    {
      { 1, 13, 2 }, { 1, 13, 8 }, { 1, 12, 8 }, { 1, 10, 2 }, { 1, 12, 4 },
      { 1, 13, 1 }, { 1, 13, 4 }, { 1, 10, 8 }, { 1, 10, 4 },
      { 1, 10, 1 }  // PRGM annunciator
    }
  };

void Clcdc_hp15c::voyager_display_update (nut_reg_t *nut_reg,voyager_display_reg_t *display)
{

  int digit;
  int segment;

  for (digit = 0; digit < VOYAGER_DISPLAY_DIGITS; digit++)
    {
      display->display_segments [digit] = 0;
      if (display->enable &&
      ((! display->blink) || (display->blink_state)))
    {
      for (segment = 0; segment <= 9; segment++)
        {
          int vreg = voyager_display_map [digit][segment].reg;
          int vdig = voyager_display_map [digit][segment].dig;
          int vbit = voyager_display_map [digit][segment].bit;
          if (vbit && (nut_reg->ram [9 + vreg][vdig] & vbit))
        {
          if (segment < 9)
            display->display_segments [digit] |= (1 << segment);
          else
            display->display_segments [digit] |= SEGMENT_ANN;
        }
        }
    }
    }

  if (display->blink)
    {
      display->blink_count--;
      if (! display->blink_count)
    {
      display->blink_state ^= 1;
      display->blink_count = VOYAGER_DISPLAY_BLINK_DIVISOR;
    }
    }
}

void Clcdc_hp15c::disp_symb(void)
{
    delete pPC->SymbImage;
    pPC->SymbImage = pPC->CreateImage(QSize(pPC->Lcd_Symb_DX, pPC->Lcd_Symb_DY),pPC->SymbFname,false,false,0);
    QPainter painter(pPC->SymbImage);

    painter.setCompositionMode(QPainter::CompositionMode_Source);
//    if (! (hp41->DIS_ANNUN_REG&0x0800)) {painter.fillRect(  0,0,18,8,Qt::transparent); }// bat
//    if (! (hp41->DIS_ANNUN_REG&0x0400)) {painter.fillRect( 24,0,24,8,Qt::transparent); }// user
//    if (! (hp41->DIS_ANNUN_REG&0x0200)) {painter.fillRect( 54,0, 6,8,Qt::transparent); }// g
//    if (! (hp41->DIS_ANNUN_REG&0x0100)) {painter.fillRect( 60,0,18,8,Qt::transparent); }// rad
//    if (! (hp41->DIS_ANNUN_REG&0x0080)) {painter.fillRect( 84,0,30,8,Qt::transparent); }// shift
//    if (! (hp41->DIS_ANNUN_REG&0x0040)) {painter.fillRect(120,0, 6,8,Qt::transparent); }// 0
//    if (! (hp41->DIS_ANNUN_REG&0x0020)) {painter.fillRect(126,0, 6,8,Qt::transparent); }// 1
//    if (! (hp41->DIS_ANNUN_REG&0x0010)) {painter.fillRect(132,0, 6,8,Qt::transparent); }// 2
//    if (! (hp41->DIS_ANNUN_REG&0x0008)) {painter.fillRect(138,0, 6,8,Qt::transparent); }// 3
//    if (! (hp41->DIS_ANNUN_REG&0x0004)) {painter.fillRect(144,0, 6,8,Qt::transparent); }// 4
//    if (! (hp41->DIS_ANNUN_REG&0x0002)) {painter.fillRect(156,0,24,8,Qt::transparent); }// prgm
//    if (! (hp41->DIS_ANNUN_REG&0x0001)) {painter.fillRect(186,0,30,8,Qt::transparent); }// alpha

    painter.end();
}

Clcdc_hp15c::Clcdc_hp15c(CPObject *parent)	: Clcdc(parent){						//[constructor]
    Color_Off.setRgb(
                (int) (111*contrast),
                (int) (117*contrast),
                (int) (108*contrast));

}

bool Clcdc_hp15c::init()
{
    nutcpu = (CHPNUT*)(pPC->pCPU);
    info = (voyager_display_reg_t*) malloc(sizeof(voyager_display_reg_t));
    voyager_display_init_ops(nutcpu->reg,info);

    // 157x182
    seg[0] = pPC->CreateImage(QSize(),P_RES(":/hp15c/a.png"));
    seg[1] = pPC->CreateImage(QSize(),P_RES(":/hp15c/b.png"));
    seg[2] = pPC->CreateImage(QSize(),P_RES(":/hp15c/c.png"));
    seg[3] = pPC->CreateImage(QSize(),P_RES(":/hp15c/d.png"));
    seg[4] = pPC->CreateImage(QSize(),P_RES(":/hp15c/e.png"));
    seg[5] = pPC->CreateImage(QSize(),P_RES(":/hp15c/f.png"));
    seg[6] = pPC->CreateImage(QSize(),P_RES(":/hp15c/g.png"));
    seg[7] = pPC->CreateImage(QSize(),P_RES(":/hp15c/h.png"));
    seg[8] = pPC->CreateImage(QSize(),P_RES(":/hp15c/i.png"));
    minus  = pPC->CreateImage(QSize(),P_RES(":/hp15c/minus.png"));
    ann_user=pPC->CreateImage(QSize(),P_RES(":/hp15c/ann_user.png"));
    ann_f  = pPC->CreateImage(QSize(),P_RES(":/hp15c/ann_f.png"));
    ann_g  = pPC->CreateImage(QSize(),P_RES(":/hp15c/ann_g.png"));
    ann_begin  = pPC->CreateImage(QSize(),P_RES(":/hp15c/ann_begin.png"));
    ann_ggrad  = pPC->CreateImage(QSize(),P_RES(":/hp15c/ann_ggrad.png"));
    ann_rad  = pPC->CreateImage(QSize(),P_RES(":/hp15c/ann_rad.png"));
    ann_dmy  = pPC->CreateImage(QSize(),P_RES(":/hp15c/ann_dmy.png"));
    ann_c  = pPC->CreateImage(QSize(),P_RES(":/hp15c/ann_c.png"));
    ann_prgm  = pPC->CreateImage(QSize(),P_RES(":/hp15c/ann_prgm.png"));

    Clcdc::init();
    return true;
}

void Clcdc_hp15c::disp(void)
{
    bool error = false;

    Refresh = false;

    if (!ready) return;
    if (!updated) return;
//    qWarning()<<"disp";


    voyager_display_update(nutcpu->reg,info);


    updated = false;
    Refresh= true;
    delete pPC->LcdImage;
    pPC->LcdImage = pPC->CreateImage(QSize(160*11, 260),QString());

    QPainter painter(pPC->LcdImage);
//painter.setCompositionMode(QPainter::CompositionMode_Source);
#if 1
    int x = 0;
    int y = 0;
    if (info->display_segments[0] & (1<<6)) {
        painter.drawImage(x,y,minus->copy(1,1,157,182));
    }
    for (int digit=1 ; digit < 11; digit++) {
        x = 160 * digit - 30;
        y = 0;
        segment_bitmap_t segments = info->display_segments[digit];

        for (int i = 0; i < 9; i++) {
            if (segments & (1 << i))
            {
                // pre-rendered images extracted from a single template in
                // image file and scaled down
                painter.drawImage(x,y,seg[i]->copy(1,1,157,182));

            }
        }
        if (segments & (1 << 17)) {
            switch (digit) {
            case 2: painter.drawImage(215,190,*ann_user); break;
            case 3: painter.drawImage(480,190,*ann_f); break;
            case 4: painter.drawImage(580,190,*ann_g); break;
            case 5: painter.drawImage(680,190,*ann_begin); break;
            case 6: painter.drawImage(980,190,*ann_ggrad); break;
            case 7: painter.drawImage(1020,190,*ann_rad); break;
            case 8: painter.drawImage(1175,190,*ann_dmy); break;
            case 9: painter.drawImage(1410,190,*ann_c); break;
            case 10:painter.drawImage(1550,190,*ann_prgm); break;
            }
        }
    }


#else
    painter.drawImage(0,0,seg[0]->copy(1,1,157,182));

#endif
//    disp_symb();

//    qWarning()<<"Display:"<<GetLCD()<<" : ";

    Refresh = true;
    redraw = 0;
    painter.end();
}

void Clcdc_hp15c::voyager_op_display_off (nut_reg_t *nut_reg, int opcode)
{
    Q_UNUSED(opcode)
qWarning()<<"display off";
    voyager_display_reg_t *display = ((Clcdc_hp15c*)nut_reg->pPC->pLCDC)->info;

    display->enable = 0;
    display->blink = 0;
    display->count = 2;
    // Don't change immediately, as the next instruction might be a
    // display toggle.
}


void Clcdc_hp15c::voyager_op_display_toggle (nut_reg_t *nut_reg, int opcode)
{
    Q_UNUSED(opcode)

    voyager_display_reg_t *display = ((Clcdc_hp15c*)nut_reg->pPC->pLCDC)->info;

  display->enable = ! display->enable;
  display->count = 0;  // force immediate display update
}


void Clcdc_hp15c::voyager_op_display_blink (nut_reg_t *nut_reg, int opcode)
{
    Q_UNUSED(opcode)

    voyager_display_reg_t *display = ((Clcdc_hp15c*)nut_reg->pPC->pLCDC)->info;

    display->enable = 1;
    display->blink = 1;
    display->blink_state = 1;
    display->blink_count = VOYAGER_DISPLAY_BLINK_DIVISOR;
    display->count = 0;  // force immediate display update
}


void Clcdc_hp15c::voyager_display_init_ops (nut_reg_t *nut_reg,voyager_display_reg_t *display)
{

  display->display_digits = VOYAGER_DISPLAY_DIGITS;
  nut_reg->op_fcn [0x030] = voyager_op_display_blink;
  nut_reg->op_fcn [0x2e0] = voyager_op_display_off;
  nut_reg->op_fcn [0x320] = voyager_op_display_toggle;
}


void Clcdc_hp15c::voyager_display_reset (voyager_display_reg_t *display)
{
  display->enable = 0;
  display->blink = 0;
  display->count = 0;
}




