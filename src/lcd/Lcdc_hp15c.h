#ifndef LCDC_HP15C_H
#define LCDC_HP15C_H

#include <QTimer>

#include "Lcdc.h"
#include "hpnut.h"
#include "pcxxxx.h"

class CHPNUT;

#define MAX_DIGIT_POSITION 15    /* Classic, Topcat, 67, maybe 19C */
/* Digit positions are numbered left to right, starting with 0. */

#define MAX_SEGMENT 18           /* 41C: 14 segment char,
                                          3 segments punctuation,
                              1 annunciator */

/* Segments are stored as a bitmap, with the LSB being segment A.
   See comments at the end of this header file. */
typedef quint32 segment_bitmap_t;


/*
 * Display segments:
 *
 * For seven-segment displays, by convention the segments are labeled 'a'
 * through 'g'.  We designate the '.' and ',' as 'h' and 'i', respectively.
 *
 *     aaa
 *    f   b
 *    f   b
 *    f   b
 *     ggg
 *    e   c
 *    e   c
 *    e   c  hh
 *     ddd   hh
 *          ii
 *         ii
 *
 * Not all calculators have the comma.  Some calculators, particularly the
 * classic series, put the '.' inside the seven segments, and dedicate a
 * full digit position to the radix mark.
 *
 * For fourteen-segment displays, by convention the segments are
 * labeled 'a' through 'n'.  We designate '.' and ',' as 'o' and 'p',
 * respectively.  The second dot for the ':' is 'q'.  The segment
 * designations do not match the HP 1LA4 documentation, but rather are
 * based on commercial 14-segment displays such as the Noritake
 * AH1616A 14-segment VFD (though that does not have the second dot
 * for the colon).
 *
 *     aaaaaaa
 *    fl  i  kb  qq
 *    f l i k b  qq
 *    f  lik  b
 *     ggg hhh
 *    e  njm  c
 *    e n j m c
 *    en  j  mc  oo
 *     ddddddd   oo
 *              pp
 *             pp
 *
 */


#define SEGMENTS_PERIOD (1 << 14)
#define SEGMENTS_COMMA  ((1 << 14) | (1 << 15))
#define SEGMENTS_COLON  ((1 << 14) | (1 << 16))

#define SEGMENT_ANN (1 << 17)

#define VOYAGER_DISPLAY_BLINK_DIVISOR 150

typedef struct
{
  bool enable;
  int count;

  bool blink;
  bool blink_state;
  int blink_count;
  segment_bitmap_t *char_gen;

  int display_digits;
  segment_bitmap_t display_segments [MAX_DIGIT_POSITION];
} voyager_display_reg_t;

class Clcdc_hp15c:public Clcdc{
    Q_OBJECT

public:
    virtual bool	init(void);		// initialize
    void disp(void);				//display LCDC data to screen
    void disp_symb(void);
    const char*	GetClassName(){ return("Clcdc_hp15c");}


    Clcdc_hp15c(CPObject *parent, QRect _lcdRect, QRect _symbRect, QString _lcdfname=QString(), QString _symbfname=QString());
    virtual ~Clcdc_hp15c();

    CHPNUT *nutcpu;

    voyager_display_reg_t *info;
    segment_bitmap_t display_segments_backup [MAX_DIGIT_POSITION];

    static void voyager_op_display_off(nut_reg_t *nut_reg, int opcode);
    static void voyager_op_display_toggle(nut_reg_t *nut_reg, int opcode);
    static void voyager_op_display_blink(nut_reg_t *nut_reg, int opcode);
    static void voyager_display_init_ops(nut_reg_t *nut_reg, voyager_display_reg_t *display);
    static void voyager_display_reset(voyager_display_reg_t *display);
    static void voyager_display_update(nut_reg_t *nut_reg,voyager_display_reg_t *display);

private:
    QImage *seg[10],*minus,*ann_user,*ann_f,*ann_g,*ann_begin,*ann_ggrad,*ann_rad,*ann_dmy,*ann_c,*ann_prgm;


};


#endif // LCDC_HP15C_H
