#ifndef MC6847_H
#define MC6847_H

/*
    Skelton for retropc emulator

    Author : Takeda.Toshiya
    Date   : 2010.08.03-

    [ mc6847 ]
*/

#include "common.h"

#define SIG_MC6847_AG		0
#define SIG_MC6847_AS		1
#define SIG_MC6847_INTEXT	2
#define SIG_MC6847_GM		3
#define SIG_MC6847_CSS		4
#define SIG_MC6847_INV		5

class MC6847
{
private:

    // output signals
//    outputs_t outputs_vsync;
//    outputs_t outputs_hsync;

    quint8 extfont[256 * 16];
    quint8 sg4[16 * 12];
    quint8 sg6[64 * 12];
    quint8 screen[192][256];
    quint8 *vram_ptr;
    int vram_size;
    QColor palette_pc[16];

    bool ag, as;
    bool intext;
    quint8 gm;
    bool css, inv;

    bool vsync, hsync, disp;
    int tWHS;

    void set_vsync(bool val);
    void set_hsync(bool val);
    void set_disp(bool val);
    void draw_cg(int xofs, int yofs);
    void draw_rg(int xofs, int yofs);
    void draw_alpha();

public:
    MC6847() { //VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {
//        d_cpu = NULL;
        ag = as = intext = css = inv = false;
        gm = 0;
//        init_output_signals(&outputs_vsync);
//        init_output_signals(&outputs_hsync);
    }
    ~MC6847() {}

    // common functions
    bool init();
    void reset();
    void write_signal(int id, quint32 data, quint32 mask);
    void event_vline(int v, int clock);
    void event_callback(int event_id, int err);
    void update_timing(int new_clocks, double new_frames_per_sec, int new_lines_per_frame);

    // unique functions

//    void set_context_vsync(DEVICE* device, int id, uint32 mask) {
////        register_output_signal(&outputs_vsync, device, id, mask);
//    }
//    void set_context_hsync(DEVICE* device, int id, uint32 mask) {
////        register_output_signal(&outputs_hsync, device, id, mask);
//    }
    void set_vram_ptr(quint8* ptr, int size) {
        vram_ptr = ptr; vram_size = size;
    }
    void load_font_image(QString path);
    void draw_screen();
};



#endif // MC6847_H
