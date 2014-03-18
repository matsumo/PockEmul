#ifndef STDIO_H
#define STDIO_H

// PC-1255

#include <__stdio.h>




/*! 
 \brief Return 0xff if the On/Break key is pressed, 0x00 otherwise.
 
 \fn onbreak 
 \return byte 
*/
byte onbreak() {

    return __onbreak();
}

#endif // _STDIO_H
