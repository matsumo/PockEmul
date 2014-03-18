#ifndef INTERNAL_H
#define INTERNAL_H

#include <__internal.h>

// SHARP PC-1350
#define __PC_1350__		1

#define __PC_RESONATOR_768K__
#define __PC_DEFAULT_ORG__      0x4000
#define __PC_HAS_MULTI_LINE_LCD__

#define __DISP_CH_COLS__    24
#define __DISP_CH_ROWS__    4
#define __DISP_GR_COLS__    150
#define __DISP_GR_ROWS__    32


#define __MEM_LCD_L1C1__   0x7000
#define __MEM_LCD_L1C2__   0x7200
#define __MEM_LCD_L1C3__   0x7400
#define __MEM_LCD_L1C4__   0x7600
#define __MEM_LCD_L1C5__   0x7800
#define __MEM_LCD_L2C1__   0x7040
#define __MEM_LCD_L2C2__   0x7240
#define __MEM_LCD_L2C3__   0x7440
#define __MEM_LCD_L2C4__   0x7640
#define __MEM_LCD_L2C5__   0x7840
#define __MEM_LCD_L3C1__   0x701E
#define __MEM_LCD_L3C2__   0x721E
#define __MEM_LCD_L3C3__   0x741E
#define __MEM_LCD_L3C4__   0x761E
#define __MEM_LCD_L3C5__   0x781E
#define __MEM_LCD_L4C1__   0x705E
#define __MEM_LCD_L4C2__   0x725E
#define __MEM_LCD_L4C3__   0x745E
#define __MEM_LCD_L4C4__   0x765E
#define __MEM_LCD_L4C5__   0x785E

#define __LCD_COL_WIDTH__   6  // 5 on systems without full graphic screen like PC-1403
#define __LCD_CHAR_WIDTH__  5
#define __MEM_CHARTAB__     0x7FF0
#define __MEM_CHARTAB_OFF__ 0
#define __INVERS_CHARTAB__


#endif // INTERNAL_H
