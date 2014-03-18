#ifndef __INTERNAL_H
#define __INTERNAL_H

#define __PC_1250__	0
#define __PC_1251__	0
#define __PC_1255__	0
#define __PC_1260__	0
#define __PC_1261__	0
#define __PC_1262__	0
#define __PC_1350__	0
#define __PC_1360__	0
#define __PC_1403__	0
#define __PC_1475__	0

// CPU registers

#define REG_I	    0x00               // index register
#define REG_J	    0x01               // index register
#define REG_A	    0x02               // accumulator
#define REG_B	    0x03               // accumulator
#define REG_XL	    0x04               // LSB of adress pointer
#define REG_XH	    0x05               // MSB of adress pointer
#define REG_YL	    0x06               // LSB of adress pointer
#define REG_YH	    0x07               // MSB of adress pointer
#define REG_K	    0x08               // counter
#define REG_L	    0x09               // counter
#define REG_M	    0x0A               // counter
#define REG_N	    0x0B               // counter

// Ports

#define PORT_A_adr  92
#define PORT_B_adr  93
#define PORT_F_adr  94
#define PORT_C_adr  95

// Port C mask

#define PORT_C_DISPLAY   0x01         // display on=1/off
#define PORT_C_CNT_RESET 0x02         // counter reset=1
#define PORT_C_CPU_HALT  0x04         // CPU halt=1
#define PORT_C_PWR_DOWN  0x08         // power down=1
#define PORT_C_BEEP_FREQ 0x10           // beeper frequency 2kHz/4kHz=1 if bit 5 set
                                        // Xout on=1/off if bit 5 not set
#define PORT_C_BEEP_CTRL 0x20
#define PORT_C_XIN_ENBL  0x40         // Xin readable with TEST if set

// TEST mask

#define TEST_CNT_512     0x01         // 512 ms counter
#define TEST_CNT_2       0x02         // 2 ms counter
#define TEST_BRK         0x08         // break key
#define TEST_RESET       0x40         // hard reset
#define TEST_XIN         0x80         // Xin port


byte _reg_i at 0;
byte _reg_j at 1;
byte _reg_a at 2;
byte _reg_b at 3;
//word _regw_ab at 2;
byte _reg_xl at 4;
byte _reg_xh at 5;
//word _regw_x at 4;
byte _reg_yl at 6;
byte _reg_yh at 7;
//word _regw_y at 6;
byte _reg_k at 8;
byte _reg_l at 9;
byte _reg_m at 10;
byte _reg_n at 11;




#endif // __INTERNAL_H
