#ifndef INTERNAL_H
#define INTERNAL_H

#include <__internal.h>

// SHARP PC-1350
#define __PC_1360__		1

#define __PC_RESONATOR_768K__
#define __PC_DEFAULT_ORG__      0x8000
#define __PC_HAS_MULTI_LINE_LCD__

#define __DISP_CH_COLS__    24
#define __DISP_CH_ROWS__    4
#define __DISP_GR_COLS__    150
#define __DISP_GR_ROWS__    32


#define __MEM_LCD_L1C1__   0x2800
#define __MEM_LCD_L1C2__   0x2A00
#define __MEM_LCD_L1C3__   0x2C00
#define __MEM_LCD_L1C4__   0x2E00
#define __MEM_LCD_L1C5__   0x3000
#define __MEM_LCD_L2C1__   0x2840
#define __MEM_LCD_L2C2__   0x2A40
#define __MEM_LCD_L2C3__   0x2C40
#define __MEM_LCD_L2C4__   0x2E40
#define __MEM_LCD_L2C5__   0x3040
#define __MEM_LCD_L3C1__   0x281E
#define __MEM_LCD_L3C2__   0x2A1E
#define __MEM_LCD_L3C3__   0x2C1E
#define __MEM_LCD_L3C4__   0x2E1E
#define __MEM_LCD_L3C5__   0x301E
#define __MEM_LCD_L4C1__   0x285E
#define __MEM_LCD_L4C2__   0x2A5E
#define __MEM_LCD_L4C3__   0x2C5E
#define __MEM_LCD_L4C4__   0x2E5E
#define __MEM_LCD_L4C5__   0x305E

#define __LCD_COL_WIDTH__   6  // 5 on systems without full graphic screen like PC-1403
#define __LCD_CHAR_WIDTH__  5
#define __MEM_CHARTAB__     0x40EB
#define __MEM_CHARTAB_OFF__ 0
//#define __INVERS_CHARTAB__

#define __PC_IS_BANKSWITCHED__
#define __MEM_BANKSWITCH__		0x3400
#define __MEM_BS_IND_CHARTAB__	1


#endif // INTERNAL_H
