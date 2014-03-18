#ifndef INTERNAL_H
#define INTERNAL_H

#include <__internal.h>

#define __PC_1250__		1

#define __PC_RESONATOR_576K__
#define __PC_DEFAULT_ORG__      0xC030

//#define __PC_HAS_MULTI_LINE_LCD__
#define __DISP_CH_COLS__    24
#define __DISP_CH_ROWS__    1
#define __DISP_GR_COLS__    120
#define __DISP_GR_ROWS__    7

#define __MEM_LCD_L1C1__    0xF800
#define __MEM_LCD_L1C2__    0xF840

#define __LCD_CHAR_WIDTH__  5
#define __LCD_COL_WIDTH__   5
#define __MEM_CHARTAB__     0x4414
//#define __INVERS_CHARTAB__

#endif // INTERNAL_H
