#ifndef _LOG_H
#define _LOG_H

#define LOG_CONSOLE		0x00
#define LOG_CPU			0x01
#define LOG_KEYBOARD	0x02
#define LOG_MASTER		0x04
#define LOG_DISPLAY		0x08
#define LOG_TEMP		0x10
#define LOG_TAPE		0x20
#define LOG_SIO			0x40
#define LOG_PRINTER		0x80
#define LOG_TIME		0x0100
#define LOG_ROM			0x0200
#define LOG_RAM			0x0400
#define LOG_FUNC		0x0800
#define LOG_11PORT		0x1000
#define LOG_ANALOG		0x2000
#define LOG_SIMULATOR	0x4000
#define LOG_CANON   	0x8000

//extern void AddLog(int ,QString);
#include "dialoglog.h"
#include "mainwindowpockemul.h"
extern MainWindowPockemul *mainwindow;

#define AddLog(Level,msg) 	\
    { \
        if ( mainwindow->dialoglog) {\
            if ((Level) & mainwindow->dialoglog->LogLevel) {\
                mainwindow->SendSignal_AddLogItem(msg); \
            } \
            if ((Level) == 0) {\
                mainwindow->SendSignal_AddLogConsole(msg); \
            } \
        } \
    }



#endif
