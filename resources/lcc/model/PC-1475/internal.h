#ifndef INTERNAL_H
#define INTERNAL_H

#include <__internal.h>

#define __PC_1475__		1

#define __PC_RESONATOR_768K__
#define __PC_DEFAULT_ORG__      0x8000

#define __DISP_CH_COLS__    24
#define __DISP_CH_ROWS__    2
#define __DISP_GR_COLS__    120
#define __DISP_GR_ROWS__    14

#define __MEM_LCD_L1C1__    0x2800
#define __MEM_LCD_L1C2__    0x2A00
#define __MEM_LCD_L2C1__    0x2840
#define __MEM_LCD_L2C2__    0x2A40

#define __LCD_CHAR_WIDTH__  5
#define __LCD_COL_WIDTH__   5
#define __MEM_CHARTAB__     0x4173
#define __MEM_CHARTAB_OFF__ 0
//#define __INVERS_CHARTAB__

#define __PC_IS_BANKSWITCHED__
#define __MEM_BANKSWITCH__		0x3400
#define __MEM_BS_IND_CHARTAB__	1

#endif // INTERNAL_H
