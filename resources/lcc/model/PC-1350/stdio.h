#ifndef STDIO_H
#define STDIO_H

// PC-1350

#include <__stdio.h>


char xram l_getchar_ret;
/*! 
 \brief 
 
 \fn getchar 
 \return byte 
*/
char getchar() {

#save
#asm

    CALL 0x0436

    JRCP lb_getchar_key
    RA
    JRP lb_getchar_ascii
lb_getchar_key:
    ADIA  0x03
    LIB   0x84
    CALL 0x0297
    IXL
lb_getchar_ascii:
    LIDP l_getchar_ret
    STD
#endasm

#restore
    return l_getchar_ret;
}

/*! 
 \brief Return 0xff if the On/Break key is pressed, 0x00 otherwise.
 
 \fn onbreak 
 \return byte 
*/
byte onbreak() {

    return __onbreak();
}

#endif // _STDIO_H

