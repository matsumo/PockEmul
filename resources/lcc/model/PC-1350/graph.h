//TODO add screen inverter
#ifndef GRAPH_H
#define GRAPH_H

#include <__graph.h>

byte xram _grfx_graph at 0x6F62;
word xram _grfx_x1 at 0x6F63;
word xram _grfx_y1 at 0x6F65;
word xram _grfx_x2 at 0x6F67;
word xram _grfx_y2 at 0x6F69;

/*! 
 \brief [ROM CALL PC-1350]draw a line
 
 \fn line 
 \param x1 
 \param y1 
 \param x2 
 \param y2 
 \param lineflag GSET, GRESET, GINV, GBOX, GBOXF
*/
line(word x1,word y1, word x2, word y2,byte lineflag) {
    _grfx_x1 = x1;
    _grfx_y1 = y1;
    _grfx_x2 = x2;
    _grfx_y2 = y2;

    _grfx_graph = lineflag;
#save
#asm
    LIJ     01
    CALL    0xEAEB
#endasm
#restore
}



/*! 
 \brief [ROM CALL PC-1350] set or reset a pixel
 USE PS_PSET instead which is platform independent.
 
 \fn pset 
 \param px1 
 \param py1 
 \param psetflag GSET,GRESET,GINV
*/
pset(word px1,word py1,byte psetflag) {
    _grfx_x1 = px1;
    _grfx_y1 = py1;

    _grfx_graph = psetflag;

#save
#asm
    LIJ     01
    CALL    0xEF24
#endasm
#restore
}


/*! 
 \brief [ROM CALL PC-1350] clear screen
 USE PS_CLRSCR instead which is platform independent.
 
 \fn cls 
*/
cls() {
    _grfx_graph = 1;
#save
#asm
    LIJ     01
    CALL    0xDBA8
#endasm
#restore
}

#endif // GRAPH_H
