/*
    Skelton for retropc emulator

    Author : Takeda.Toshiya
    Date   : 2010.08.03-

    [ mc6847 ]
*/
#include <QDebug>
#include <QPainter>
#include <QFile>

#include "mc6847.h"

#ifndef MC6847_VRAM_OFS
#define MC6847_VRAM_OFS	0
#endif

#define LIGHTGREEN	0
#define YELLOW		1
#define BLUE		2
#define RED		3
#define WHITE		4
#define CYAN		5
#define MAGENTA		6
#define ORANGE		7
#define BLACK		8
// text
#define GREEN		9
#define BEIGE		10
// phc20
#define GRAY		11

// from mess m6847.c
static const quint8 intfont[64 * 12] = {
    0x00, 0x00, 0x38, 0x44, 0x04, 0x34, 0x4C, 0x4C, 0x38, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x10, 0x28, 0x44, 0x44, 0x7C, 0x44, 0x44, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x78, 0x24, 0x24, 0x38, 0x24, 0x24, 0x78, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x38, 0x44, 0x40, 0x40, 0x40, 0x44, 0x38, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x78, 0x24, 0x24, 0x24, 0x24, 0x24, 0x78, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x7C, 0x40, 0x40, 0x70, 0x40, 0x40, 0x7C, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x7C, 0x40, 0x40, 0x70, 0x40, 0x40, 0x40, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x38, 0x44, 0x40, 0x40, 0x4C, 0x44, 0x38, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x44, 0x44, 0x44, 0x7C, 0x44, 0x44, 0x44, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x38, 0x10, 0x10, 0x10, 0x10, 0x10, 0x38, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x04, 0x04, 0x04, 0x04, 0x04, 0x44, 0x38, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x44, 0x48, 0x50, 0x60, 0x50, 0x48, 0x44, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x7C, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x44, 0x6C, 0x54, 0x54, 0x44, 0x44, 0x44, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x44, 0x44, 0x64, 0x54, 0x4C, 0x44, 0x44, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x38, 0x44, 0x44, 0x44, 0x44, 0x44, 0x38, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x78, 0x44, 0x44, 0x78, 0x40, 0x40, 0x40, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x38, 0x44, 0x44, 0x44, 0x54, 0x48, 0x34, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x78, 0x44, 0x44, 0x78, 0x50, 0x48, 0x44, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x38, 0x44, 0x40, 0x38, 0x04, 0x44, 0x38, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x7C, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x38, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x44, 0x44, 0x44, 0x28, 0x28, 0x10, 0x10, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x44, 0x44, 0x44, 0x44, 0x54, 0x6C, 0x44, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x44, 0x44, 0x28, 0x10, 0x28, 0x44, 0x44, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x44, 0x44, 0x28, 0x10, 0x10, 0x10, 0x10, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x7C, 0x04, 0x08, 0x10, 0x20, 0x40, 0x7C, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x38, 0x20, 0x20, 0x20, 0x20, 0x20, 0x38, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x40, 0x20, 0x10, 0x08, 0x04, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x38, 0x08, 0x08, 0x08, 0x08, 0x08, 0x38, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x10, 0x38, 0x54, 0x10, 0x10, 0x10, 0x10, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x10, 0x20, 0x7C, 0x20, 0x10, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x10, 0x10, 0x10, 0x10, 0x10, 0x00, 0x10, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x28, 0x28, 0x28, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x28, 0x28, 0x7C, 0x28, 0x7C, 0x28, 0x28, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x10, 0x3C, 0x50, 0x38, 0x14, 0x78, 0x10, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x60, 0x64, 0x08, 0x10, 0x20, 0x4C, 0x0C, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x20, 0x50, 0x50, 0x20, 0x54, 0x48, 0x34, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x10, 0x10, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x08, 0x10, 0x20, 0x20, 0x20, 0x10, 0x08, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x20, 0x10, 0x08, 0x08, 0x08, 0x10, 0x20, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x10, 0x54, 0x38, 0x38, 0x54, 0x10, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x10, 0x10, 0x7C, 0x10, 0x10, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x20, 0x40, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x7C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x04, 0x08, 0x10, 0x20, 0x40, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x38, 0x44, 0x4C, 0x54, 0x64, 0x44, 0x38, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x10, 0x30, 0x10, 0x10, 0x10, 0x10, 0x38, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x38, 0x44, 0x04, 0x38, 0x40, 0x40, 0x7C, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x38, 0x44, 0x04, 0x08, 0x04, 0x44, 0x38, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x08, 0x18, 0x28, 0x48, 0x7C, 0x08, 0x08, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x7C, 0x40, 0x78, 0x04, 0x04, 0x44, 0x38, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x38, 0x40, 0x40, 0x78, 0x44, 0x44, 0x38, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x7C, 0x04, 0x08, 0x10, 0x20, 0x40, 0x40, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x38, 0x44, 0x44, 0x38, 0x44, 0x44, 0x38, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x38, 0x44, 0x44, 0x3C, 0x04, 0x04, 0x38, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x10, 0x10, 0x20, 0x00, 0x00,
    0x00, 0x00, 0x08, 0x10, 0x20, 0x40, 0x20, 0x10, 0x08, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x7C, 0x00, 0x7C, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x20, 0x10, 0x08, 0x04, 0x08, 0x10, 0x20, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x38, 0x44, 0x04, 0x08, 0x10, 0x00, 0x10, 0x00, 0x00, 0x00
};

bool MC6847::init()
{
    // semigraphics pattern
    for(int i = 0; i < 16; i++) {
        for(int j = 0; j < 6; j++) {
            sg4[i * 12 + j] = ((i & 0x08) ? 0xf0 : 0) | ((i & 0x04) ? 0x0f : 0);
        }
        for(int j = 6; j < 12; j++) {
            sg4[i * 12 + j] = ((i & 0x02) ? 0xf0 : 0) | ((i & 0x01) ? 0x0f : 0);
        }
    }
    for(int i = 0; i < 64; i++) {
        for(int j = 0; j < 4; j++) {
            sg6[i * 12 + j] = ((i & 0x20) ? 0xf0 : 0) | ((i & 0x10) ? 0x0f : 0);
        }
        for(int j = 4; j < 8; j++) {
            sg6[i * 12 + j] = ((i & 0x08) ? 0xf0 : 0) | ((i & 0x04) ? 0x0f : 0);
        }
        for(int j = 8; j < 12; j++) {
            sg6[i * 12 + j] = ((i & 0x02) ? 0xf0 : 0) | ((i & 0x01) ? 0x0f : 0);
        }
    }

    // pc pallete
    palette_pc[LIGHTGREEN].setRgb(184,255,181);
    palette_pc[RED       ].setRgb(254, 65,105);
    palette_pc[YELLOW    ].setRgb(252,253,153);
    palette_pc[BLUE      ].setRgb(116, 41,255);
    palette_pc[WHITE     ].setRgb(241,229,232);
    palette_pc[CYAN      ].setRgb(124,210,213);
    palette_pc[MAGENTA   ].setRgb(254,113,255);
    palette_pc[ORANGE    ].setRgb(254,112, 35);
    palette_pc[BLACK     ].setRgb(  0,  0,  0);
    palette_pc[GREEN     ].setRgb( 22,134, 10);
    palette_pc[BEIGE     ].setRgb(255,198,170);
    palette_pc[GRAY      ].setRgb( 32, 32, 32);

    // register event
//    register_vline_event(this);
//    update_timing(CPU_CLOCKS, FRAMES_PER_SEC, LINES_PER_FRAME);

    displaySL = 0;
    return true;
}

void MC6847::reset()
{
    vsync = hsync = disp = true;
    displaySL = 0;
}

void MC6847::write_signal(int id, quint32 data, quint32 mask)
{
    switch(id) {
    case SIG_MC6847_AG:
        ag = ((data & mask) != 0);
        break;
    case SIG_MC6847_AS:
        as = ((data & mask) != 0);
        break;
    case SIG_MC6847_INTEXT:
        intext = ((data & mask) != 0);
        break;
    case SIG_MC6847_GM:
        gm = (gm & ~mask) | (data & mask);
        break;
    case SIG_MC6847_CSS:
        css = ((data & mask) != 0);
        break;
    case SIG_MC6847_INV:
        inv = ((data & mask) != 0);
        break;
    }
}

void MC6847::update_timing(int new_clocks, double new_frames_per_sec, int new_lines_per_frame)
{
    // this should be called before vline event
    tWHS = (int)((double)new_clocks / new_frames_per_sec / (double)new_lines_per_frame * 16.5 / 227.5 + 0.5);
}

void MC6847::event_vline(int v, int clock)
{
    // vsync
    set_vsync(v > 32);	// 32/262

    // hsync
    set_hsync(false);
//    register_event_by_clock(this, 0, tWHS, false, NULL);
}

void MC6847::event_callback(int event_id, int err)
{
    set_hsync(true);
}

void MC6847::set_vsync(bool val)
{
    if(vsync != val) {
//        write_signals(&outputs_vsync, val ? 0xffffffff : 0);
        vsync = val;
        set_disp(vsync && hsync);
    }
}

void MC6847::set_hsync(bool val)
{
    if(hsync != val) {
//        write_signals(&outputs_hsync, val ? 0xffffffff : 0);
        hsync = val;
        set_disp(vsync && hsync);
    }
}

void MC6847::set_disp(bool val)
{
    if(disp != val) {
//        if(d_cpu != NULL) {
//            d_cpu->write_signal(SIG_CPU_BUSREQ, val ? 1 : 0, 1);
//        }
        disp = val;
    }
}

void MC6847::load_font_image(QString path)
{
    // external font
#if 0
    FILEIO* fio = new FILEIO();
    if(fio->Fopen(path, FILEIO_READ_BINARY)) {
        fio->Fread(extfont, sizeof(extfont), 1);
        fio->Fclose();
    }
    delete fio;
#endif
}

void MC6847::load_font_bin(QString path)
{
    // external font
    QFile file;
    file.setFileName(path);
    if (file.exists())
    {
        file.open(QIODevice::ReadOnly);
        QDataStream in(&file);
        in.readRawData ((char *) extfont,256 * 16 );
        file.close();
    }

}

void MC6847::draw_screen()
{
    // render screen
    if(ag) {
        // graphics mode
        switch(gm) {
        case 0: draw_cg(4, 3); break;	//  64x 64
        case 1: draw_rg(2, 3); break;	// 128x 64
        case 2: draw_cg(2, 3); break;	// 128x 64
        case 3: draw_rg(2, 2); break;	// 128x 96
        case 4: draw_cg(2, 2); break;	// 128x 96
        case 5: draw_rg(2, 1); break;	// 128x192
        case 6: draw_cg(2, 1); break;	// 128x192
        case 7: draw_rg(1, 1); break;	// 256x192
        }
    } else {
        // alphanumerics / semigraphics
        draw_alpha();
    }

    // copy to screen
    QPainter painter(screenImage);

    for(int y = 0; y < 192; y++) {
//        scrntype* dest = emu->(y);
        for(int x = 0; x < 256; x++) {
            painter.setPen( palette_pc[screen[computeSL(y)][x]]);
            painter.drawPoint( x, y);
        }
    }
    painter.end();
}

int MC6847::computeSL(int y) {
    int _res = y;
    _res += displaySL;
    if (_res<0) _res+=192;
    if (_res>191) _res -=192;

    return _res;

}

void MC6847::draw_cg(int xofs, int yofs)
{
    quint8 color = css ? 4 : 0;
    int ofs = 0;

    for(int y = 0; y < 192; y += yofs) {
        for(int x = 0; x < 256; x += xofs * 4) {
            quint8 data = vram_ptr[ofs];
            if(++ofs >= vram_size) {
                ofs = 0;
            }
            quint8* dest = &screen[y][x];

            if(xofs == 4) {
                dest[ 0] = dest[ 1] = dest[ 2] = dest[ 3] = color | ((data >> 6) & 3);
                dest[ 4] = dest[ 5] = dest[ 6] = dest[ 7] = color | ((data >> 4) & 3);
                dest[ 8] = dest[ 9] = dest[10] = dest[11] = color | ((data >> 2) & 3);
                dest[12] = dest[13] = dest[14] = dest[15] = color | ((data >> 0) & 3);
            } else {
                dest[0] = dest[1] = color | ((data >> 6) & 3);
                dest[2] = dest[3] = color | ((data >> 4) & 3);
                dest[4] = dest[5] = color | ((data >> 2) & 3);
                dest[6] = dest[7] = color | ((data >> 0) & 3);
            }
        }
        if(yofs >= 2) {
            memcpy(screen[y + 1], screen[y], 256);
            if(yofs >= 3) {
                memcpy(screen[y + 2], screen[y], 256);
            }
        }
    }
}

void MC6847::draw_rg(int xofs, int yofs)
{
    static const quint8 color_table[4] = {
        GREEN, LIGHTGREEN, BLACK, WHITE
    };
    static const quint8 color_table2[4] = {
        BLACK, BLACK, CYAN, WHITE
    };
    static const quint8 color_table3[4] = {
        BLACK, ORANGE, BLACK, WHITE
    };
    quint8 color = css ? 2 : 0;
    int ofs = 0;

    for(int y = 0; y < 192; y += yofs) {
        for(int x = 0; x < 256; x += xofs * 8) {
            quint8 data = vram_ptr[ofs];
            if(++ofs >= vram_size) {
                ofs = 0;
            }
            quint8* dest = &screen[y][x];

            if(xofs == 2) {
                dest[ 0] = dest[ 1] = color_table[color | ((data >> 7) & 1)];
                dest[ 2] = dest[ 3] = color_table[color | ((data >> 6) & 1)];
                dest[ 4] = dest[ 5] = color_table[color | ((data >> 5) & 1)];
                dest[ 6] = dest[ 7] = color_table[color | ((data >> 4) & 1)];
                dest[ 8] = dest[ 9] = color_table[color | ((data >> 3) & 1)];
                dest[10] = dest[11] = color_table[color | ((data >> 2) & 1)];
                dest[12] = dest[13] = color_table[color | ((data >> 1) & 1)];
                dest[14] = dest[15] = color_table[color | ((data >> 0) & 1)];
            } else if(css) {
                // color bleed in black/white pattern
                dest[0] = color_table2[(data >> 6) & 3];
                dest[1] = color_table3[(data >> 6) & 3];
                dest[2] = color_table2[(data >> 4) & 3];
                dest[3] = color_table3[(data >> 4) & 3];
                dest[4] = color_table2[(data >> 2) & 3];
                dest[5] = color_table3[(data >> 2) & 3];
                dest[6] = color_table2[(data >> 0) & 3];
                dest[7] = color_table3[(data >> 0) & 3];
            } else {
                dest[0] = color_table[(data >> 7) & 1];
                dest[1] = color_table[(data >> 6) & 1];
                dest[2] = color_table[(data >> 5) & 1];
                dest[3] = color_table[(data >> 4) & 1];
                dest[4] = color_table[(data >> 3) & 1];
                dest[5] = color_table[(data >> 2) & 1];
                dest[6] = color_table[(data >> 1) & 1];
                dest[7] = color_table[(data >> 0) & 1];
            }
        }
        if(yofs >= 2) {
            memcpy(screen[y + 1], screen[y], 256);
            if(yofs >= 3) {
                memcpy(screen[y + 2], screen[y], 256);
            }
        }
    }
}


#define MC6847_ATTR_OFS		1
//#define MC6847_ATTR_AG		0x80
//#define MC6847_ATTR_AS		0x40
#define MC6847_ATTR_INTEXT	0x02
//#define MC6847_ATTR_GM0		0x10
//#define MC6847_ATTR_GM1		0x08
//#define MC6847_ATTR_GM2		0x04
//#define MC6847_ATTR_CSS		0x02
#define MC6847_ATTR_INV		0x08



void MC6847::draw_alpha()
{
//    qWarning()<<"drawAlpha";
    int ofs = 0;

    for(int y = 0; y < 192; y += 12) {
        for(int x = 0; x < 256; x += 8) {
            quint8 data = vram_ptr[2*ofs + MC6847_ATTR_OFS];
#ifdef MC6847_ATTR_OFS
            quint8 attr = vram_ptr[2*ofs ];
#endif

            if(++ofs >= vram_size) {
                ofs = 0;
            }
//            qWarning()<<"ofs="<<ofs;

            // vram data bits may be connected to mode signals
            bool as2 = as;
            bool intext2 = intext;
            bool css2 = css;
            bool inv2 = inv;
#ifdef MC6847_VRAM_AS
            as2 = ((data & MC6847_VRAM_AS) != 0);
#endif
#ifdef MC6847_VRAM_INTEXT
            intext2 = ((data & MC6847_VRAM_INTEXT) != 0);
#endif
#ifdef MC6847_VRAM_CSS
            css2 = ((data & MC6847_VRAM_CSS) != 0);
#endif
#ifdef MC6847_VRAM_INV
            inv2 = ((data & MC6847_VRAM_INV) != 0);
#endif
#ifdef MC6847_ATTR_OFS
#ifdef MC6847_ATTR_AS
            as2 = ((attr & MC6847_ATTR_AS) != 0);
#endif
#ifdef MC6847_ATTR_INTEXT
            intext2 = ((attr & MC6847_ATTR_INTEXT) != 0);
#endif
#ifdef MC6847_ATTR_CSS
            css2 = ((attr & MC6847_ATTR_CSS) != 0);
#endif
#ifdef MC6847_ATTR_INV
            inv2 = ((attr & MC6847_ATTR_INV) != 0);
#endif
#endif
            quint8 *pattern;
            quint8 col_fore, col_back;
            if(!as2) {
                if(intext2) {
                    // external alphanumerics
                    pattern = &extfont[0x800-16 * (data+1)];
                } else {
                    // internal alphanumerics
                    pattern = (quint8 *)(&intfont[12 * (data & 0x3f)]);
                }
                // note: we need to overwrite the color table by each driver
                static const quint8 color_table[6] = {
                    LIGHTGREEN, GREEN, BEIGE, RED, GREEN, BLACK
                };
                int col = (css2 ? 2 : 0) | (inv2 ? 1 : 0);
                col_fore = color_table[col];
                col_back = color_table[col ^ 1];
            }
            else {
                if(intext2) {
                    // semiggraphics 6
                    pattern = &sg6[12 * (data & 0x3f)];
                    col_fore = (css2 ? 4 : 0) | ((data >> 6) & 3);
                } else {
                    // semiggraphics 4
                    pattern = &sg4[12 * (data & 0x0f)];
                    col_fore = (data >> 4) & 7;
                }
                col_back = BLACK;
            }
            for(int l = 0; l < 12; l++) {
                quint8 pat = pattern[l];
                quint8* dest = &screen[y + l][x];

                dest[0] = (pat & 0x80) ? col_fore : col_back;
                dest[1] = (pat & 0x40) ? col_fore : col_back;
                dest[2] = (pat & 0x20) ? col_fore : col_back;
                dest[3] = (pat & 0x10) ? col_fore : col_back;
                dest[4] = (pat & 0x08) ? col_fore : col_back;
                dest[5] = (pat & 0x04) ? col_fore : col_back;
                dest[6] = (pat & 0x02) ? col_fore : col_back;
                dest[7] = (pat & 0x01) ? col_fore : col_back;
            }
        }
    }
//    qWarning()<<"end::DrawAlpha";
}


