#ifndef HD44780_H
#define HD44780_H


#include <QObject>
#include "common.h"

class CpcXXXX;

/***************************************************************************

        Hitachi HD44780 LCD controller

***************************************************************************/

typedef void (*hd44780_pixel_update_func)(QPainter *painter, UINT8 line, UINT8 pos, UINT8 y, UINT8 x, int state,QColor Color_On,QColor Color_Off);
#define HD44780_PIXEL_UPDATE(name) void name(QPainter *painter, UINT8 line, UINT8 pos, UINT8 y, UINT8 x, int state,QColor Color_On,QColor Color_Off)

typedef struct {
    UINT8       m_lines;          // number of lines
    UINT8       m_chars;          // chars for line
    hd44780_pixel_update_func m_pixel_update_func; // pixel update callback

    bool        m_busy_flag;      // busy flag
    UINT8       m_ddram[0x80];    // internal display data RAM
    UINT8       m_cgram[0x40];    // internal chargen RAM
    UINT8 *     m_cgrom;          // internal chargen ROM
    qint8       m_ac;             // address counter
    UINT8       m_dr;             // data register
    UINT8       m_ir;             // instruction register
    UINT8       m_active_ram;     // DDRAM or CGRAM
    bool        m_display_on;     // display on/off
    bool        m_cursor_on;      // cursor on/off
    bool        m_blink_on;       // blink on/off
    bool        m_shift_on;       // shift on/off
    UINT8       m_disp_shift;     // display shift
    qint8       m_direction;      // auto increment/decrement
    UINT8       m_data_len;       // interface data length 4 or 8 bit
    UINT8       m_num_line;       // number of lines
    UINT8       m_char_size;      // char size 5x8 or 5x10
    bool        m_blink;
    bool        m_first_cmd;
    int         m_rs_state;
    int         m_rw_state;
    bool        m_nibble;
    int         m_charset_type;
} HD44780info;

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// ======================> hd44352_interface

struct hd44780_interface
{
    /*devcb_write_line*/	int m_on_cb;		// ON line
};

// ======================> hd44352_device

class CHD44780 : public QObject
{
public:
    CpcXXXX *pPC;
    // construction/destruction
    explicit CHD44780(QString fnCharSet,QObject *parent = 0);

    void set_pixel_update_cb(hd44780_pixel_update_func _cb) { info.m_pixel_update_func = _cb; }
    // device interface
    QString fncharset;
    UINT8 data_read();
    void data_write(UINT8 data);
    UINT8 control_read();
    void control_write(UINT8 data);
    HD44780info *getInfo();

    quint64 on_timer_rate;

    UINT32 screen_update(QPainter *painter,QColor color_ON,QColor color_OFF);

    void pixel_update(QPainter *painter, UINT8 line, UINT8 pos, UINT8 y, UINT8 x, int state,QColor color_ON,QColor color_OFF);
    void set_charset_type(int type);

    // device-level overrides
    bool	init(void);						//initialize
    bool	exit(void);						//end
    void	Reset(void);
    bool	step(void);



private:

    UINT8 get_char(UINT16 pos);



    HD44780info info;
    UINT8 charset[0x1000];

    void set_busy_flag(UINT16 usec);
    void update_ac(int direction);
    void update_nibble(int rs, int rw);
    void shift_display(int direction);
//    void pixel_update(bitmap_ind16 &bitmap, UINT8 line, UINT8 pos, UINT8 y, UINT8 x, int state);


    enum        { DDRAM, CGRAM };

};




#endif // HD44780_H
