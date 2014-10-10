#include <QPainter>
#include "common.h"
#include "pcxxxx.h"

#include "Lcdc_fp200.h"
#include "Lcdc_symb.h"
#include "Log.h"
#include "cpu.h"

UINT8 FP200_CarDef[256][8]={
{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},     // 0x00
{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},     // 0x01
{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},     // 0x02
{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},     // 0x03
{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},     // 0x04
{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},     // 0x05
{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},     // 0x06
{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},     // 0x07
{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},     // 0x08
{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},     // 0x09
{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},     // 0x0A
{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},     // 0x0B
{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},     // 0x0C
{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},     // 0x0D
{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},     // 0x0E
{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},     // 0x0F
{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},     // 0x10
{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},     // 0x11
{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},     // 0x12
{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},     // 0x13
{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},     // 0x14
{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},     // 0x15
{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},     // 0x16
{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},     // 0x17
{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},     // 0x18
{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},     // 0x19
{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},     // 0x1A
{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},     // 0x1B
{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},     // 0x1C
{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},     // 0x1D
{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},     // 0x1E
{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},     // 0x1F
{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},     // 0x20
{0x20,0x20,0x20,0x20,0x20,0x00,0x20,0x00},     // 0x21
{0x50,0x50,0x50,0x00,0x00,0x00,0x00,0x00},     // 0x22
{0x50,0x50,0xF8,0x50,0xF8,0x50,0x50,0x00},     // 0x23
{0x20,0x78,0xA0,0x70,0x28,0xF0,0x20,0x00},     // 0x24
{0xC0,0xC8,0x10,0x20,0x40,0x98,0x18,0x00},     // 0x25
{0x40,0xA0,0xA0,0x40,0xA8,0x90,0x68,0x00},     // 0x26
{0x10,0x20,0x00,0x00,0x00,0x00,0x00,0x00},     // 0x27
{0x10,0x20,0x40,0x40,0x40,0x20,0x10,0x00},     // 0x28
{0x40,0x20,0x10,0x10,0x10,0x20,0x40,0x00},     // 0x29
{0x00,0x20,0xA8,0x70,0xA8,0x20,0x00,0x00},     // 0x2A
{0x00,0x20,0x20,0xF8,0x20,0x20,0x00,0x00},     // 0x2B
{0x00,0x00,0x00,0x00,0x60,0x20,0x40,0x00},     // 0x2C
{0x00,0x00,0x00,0xF8,0x00,0x00,0x00,0x00},     // 0x2D
{0x00,0x00,0x00,0x00,0x00,0x60,0x60,0x00},     // 0x2E
{0x00,0x08,0x10,0x20,0x40,0x80,0x00,0x00},     // 0x2F
{0x70,0x88,0x98,0xA8,0xC8,0x88,0x70,0x00},     // 0x30
{0x20,0x60,0x20,0x20,0x20,0x20,0x70,0x00},     // 0x31
{0x70,0x88,0x08,0x30,0x40,0x80,0xF8,0x00},     // 0x32
{0x70,0x88,0x08,0x30,0x08,0x88,0x70,0x00},     // 0x33
{0x10,0x30,0x50,0x90,0xF8,0x10,0x10,0x00},     // 0x34
{0xF8,0x80,0xF0,0x08,0x08,0x88,0x70,0x00},     // 0x35
{0x30,0x40,0x80,0xF0,0x88,0x88,0x70,0x00},     // 0x36
{0xF8,0x08,0x10,0x20,0x40,0x40,0x40,0x00},     // 0x37
{0x70,0x88,0x88,0x70,0x88,0x88,0x70,0x00},     // 0x38
{0x70,0x88,0x88,0x78,0x08,0x10,0x60,0x00},     // 0x39
{0x00,0x60,0x60,0x00,0x60,0x60,0x00,0x00},     // 0x3A
{0x00,0x60,0x60,0x00,0x60,0x20,0x40,0x00},     // 0x3B
{0x10,0x20,0x40,0x80,0x40,0x20,0x10,0x00},     // 0x3C
{0x00,0x00,0xF8,0x00,0xF8,0x00,0x00,0x00},     // 0x3D
{0x40,0x20,0x10,0x08,0x10,0x20,0x40,0x00},     // 0x3E
{0x70,0x88,0x08,0x10,0x20,0x00,0x20,0x00},     // 0x3F
{0x70,0x88,0xB8,0xA8,0xB8,0x80,0x78,0x00},     // 0x40
{0x70,0x88,0x88,0xF8,0x88,0x88,0x88,0x00},     // 0x41
{0xF0,0x88,0x88,0xF0,0x88,0x88,0xF0,0x00},     // 0x42
{0x70,0x88,0x80,0x80,0x80,0x88,0x70,0x00},     // 0x43
{0xE0,0x90,0x88,0x88,0x88,0x90,0xE0,0x00},     // 0x44
{0xF8,0x80,0x80,0xF0,0x80,0x80,0xF8,0x00},     // 0x45
{0xF8,0x80,0x80,0xF0,0x80,0x80,0x80,0x00},     // 0x46
{0x70,0x88,0x80,0xB8,0x88,0x98,0x68,0x00},     // 0x47
{0x88,0x88,0x88,0xF8,0x88,0x88,0x88,0x00},     // 0x48
{0x70,0x20,0x20,0x20,0x20,0x20,0x70,0x00},     // 0x49
{0x38,0x10,0x10,0x10,0x10,0x90,0x60,0x00},     // 0x4A
{0x88,0x90,0xA0,0xC0,0xA0,0x90,0x88,0x00},     // 0x4B
{0x80,0x80,0x80,0x80,0x80,0x80,0xF8,0x00},     // 0x4C
{0x88,0xD8,0xA8,0xA8,0x88,0x88,0x88,0x00},     // 0x4D
{0x88,0x88,0xC8,0xA8,0x98,0x88,0x88,0x00},     // 0x4E
{0x70,0x88,0x88,0x88,0x88,0x88,0x70,0x00},     // 0x4F
{0xF0,0x88,0x88,0xF0,0x80,0x80,0x80,0x00},     // 0x50
{0x70,0x88,0x88,0x88,0xA8,0x90,0x68,0x00},     // 0x51
{0xF0,0x88,0x88,0xF0,0xA0,0x90,0x88,0x00},     // 0x52
{0x70,0x88,0x80,0x70,0x08,0x88,0x70,0x00},     // 0x53
{0xF8,0x20,0x20,0x20,0x20,0x20,0x20,0x00},     // 0x54
{0x88,0x88,0x88,0x88,0x88,0x88,0x70,0x00},     // 0x55
{0x88,0x88,0x88,0x88,0x88,0x50,0x20,0x00},     // 0x56
{0x88,0x88,0x88,0x88,0xA8,0xA8,0x50,0x00},     // 0x57
{0x88,0x88,0x50,0x20,0x50,0x88,0x88,0x00},     // 0x58
{0x88,0x88,0x50,0x20,0x20,0x20,0x20,0x00},     // 0x59
{0xF8,0x08,0x10,0x20,0x40,0x80,0xF8,0x00},     // 0x5A
{0x70,0x40,0x40,0x40,0x40,0x40,0x70,0x00},     // 0x5B
{0x88,0x50,0x20,0xF8,0x20,0xF8,0x20,0x00},     // 0x5C
{0x70,0x10,0x10,0x10,0x10,0x10,0x70,0x00},     // 0x5D
{0x20,0x50,0x88,0x00,0x00,0x00,0x00,0x00},     // 0x5E
{0x00,0x00,0x00,0x00,0x00,0x00,0xF8,0x00},     // 0x5F
{0x40,0x20,0x00,0x00,0x00,0x00,0x00,0x00},     // 0x60
{0x00,0x00,0x70,0x08,0x78,0x88,0x78,0x00},     // 0x61
{0x80,0x80,0xF0,0x88,0x88,0x88,0xF0,0x00},     // 0x62
{0x00,0x00,0x70,0x80,0x80,0x80,0x70,0x00},     // 0x63
{0x08,0x08,0x78,0x88,0x88,0x88,0x78,0x00},     // 0x64
{0x00,0x00,0x70,0x88,0xF8,0x80,0x78,0x00},     // 0x65
{0x18,0x20,0x78,0x20,0x20,0x20,0x20,0x00},     // 0x66
{0x00,0x00,0x78,0x88,0x78,0x08,0xF0,0x00},     // 0x67
{0x00,0x80,0x80,0xF0,0x88,0x88,0x88,0x00},     // 0x68
{0x00,0x20,0x00,0x60,0x20,0x20,0x70,0x00},     // 0x69
{0x10,0x00,0x30,0x10,0x10,0x10,0x60,0x00},     // 0x6A
{0x40,0x40,0x48,0x50,0x60,0x50,0x48,0x00},     // 0x6B
{0x60,0x20,0x20,0x20,0x20,0x20,0x70,0x00},     // 0x6C
{0x00,0x00,0x50,0xA8,0xA8,0xA8,0xA8,0x00},     // 0x6D
{0x00,0x00,0xB0,0x48,0x48,0x48,0x48,0x00},     // 0x6E
{0x00,0x00,0x70,0x88,0x88,0x88,0x70,0x00},     // 0x6F
{0x00,0x00,0xF0,0x88,0xF0,0x80,0x80,0x00},     // 0x70
{0x00,0x00,0x78,0x88,0x78,0x08,0x08,0x00},     // 0x71
{0x00,0x00,0x58,0x60,0x40,0x40,0x40,0x00},     // 0x72
{0x00,0x00,0x78,0x80,0x70,0x08,0xF0,0x00},     // 0x73
{0x20,0x20,0x78,0x20,0x20,0x20,0x18,0x00},     // 0x74
{0x00,0x00,0x90,0x90,0x90,0x90,0x68,0x00},     // 0x75
{0x00,0x00,0x88,0x88,0x88,0x50,0x20,0x00},     // 0x76
{0x00,0x00,0x88,0x88,0xA8,0xA8,0x50,0x00},     // 0x77
{0x00,0x00,0xC8,0x30,0x20,0x60,0x98,0x00},     // 0x78
{0x00,0x00,0x88,0x88,0x78,0x08,0x70,0x00},     // 0x79
{0x00,0x00,0xF8,0x10,0x20,0x40,0xF8,0x00},     // 0x7A
{0x10,0x20,0x20,0x40,0x20,0x20,0x10,0x00},     // 0x7B
{0x20,0x20,0x20,0x00,0x20,0x20,0x20,0x00},     // 0x7C
{0x40,0x20,0x20,0x10,0x20,0x20,0x40,0x00},     // 0x7D
{0x40,0xA8,0x10,0x00,0x00,0x00,0x00,0x00},     // 0x7E
{0x20,0x00,0x20,0x40,0x80,0x88,0x70,0x00},     // 0x7F
{0x00,0x20,0x70,0xF8,0xF8,0x20,0x70,0x00},     // 0x80
{0x00,0xD8,0xF8,0xF8,0x70,0x70,0x20,0x00},     // 0x81
{0x00,0x70,0x70,0xF8,0xF8,0x20,0x70,0x00},     // 0x82
{0x00,0x20,0x70,0xF8,0xF8,0x70,0x20,0x00},     // 0x83
{0x00,0x70,0x88,0x88,0x88,0x70,0x00,0x00},     // 0x84
{0x00,0x70,0xF8,0xF8,0xF8,0x70,0x00,0x00},     // 0x85
{0x50,0x00,0x70,0x88,0xF8,0x88,0x88,0x00},     // 0x86
{0x20,0x50,0x20,0x70,0x88,0xF8,0x88,0x00},     // 0x87
{0x50,0x00,0x70,0x08,0x78,0x88,0x78,0x00},     // 0x88
{0x40,0x20,0x70,0x08,0x78,0x88,0x78,0x00},     // 0x89
{0x20,0x88,0x70,0x08,0x78,0x88,0x78,0x00},     // 0x8A
{0x10,0x20,0x70,0x08,0x78,0x88,0x78,0x00},     // 0x8B
{0x20,0x20,0x70,0x08,0x78,0x88,0x78,0x00},     // 0x8C
{0x70,0x08,0x78,0x88,0x78,0x00,0xF8,0x00},     // 0x8D
{0x50,0x00,0x70,0x20,0x20,0x20,0x70,0x00},     // 0x8E
{0x50,0x00,0x00,0x60,0x20,0x20,0x70,0x00},     // 0x8F
{0x40,0x20,0x00,0x60,0x20,0x20,0x70,0x00},     // 0x90
{0x20,0x88,0x00,0x60,0x20,0x20,0x70,0x00},     // 0x91
{0x10,0x20,0x00,0x60,0x20,0x20,0x70,0x00},     // 0x92
{0x50,0x00,0x88,0x88,0x88,0x88,0x70,0x00},     // 0x93
{0x50,0x00,0x90,0x90,0x90,0x90,0x68,0x00},     // 0x94
{0x40,0x20,0x90,0x90,0x90,0x90,0x68,0x00},     // 0x95
{0x20,0x88,0x00,0x90,0x90,0x90,0x68,0x00},     // 0x96
{0x10,0x20,0x90,0x90,0x90,0x90,0x68,0x00},     // 0x97
{0x10,0x20,0xF8,0x80,0xF0,0x80,0xF8,0x00},     // 0x98
{0x50,0x00,0x70,0x88,0xF8,0x80,0x78,0x00},     // 0x99
{0x40,0x20,0x70,0x88,0xF8,0x80,0x78,0x00},     // 0x9A
{0x20,0x88,0x70,0x88,0xF8,0x80,0x78,0x00},     // 0x9B
{0x10,0x20,0x70,0x88,0xF8,0x80,0x78,0x00},     // 0x9C
{0x50,0x00,0x70,0x88,0x88,0x88,0x70,0x00},     // 0x9D
{0x50,0x00,0x00,0x70,0x88,0x88,0x70,0x00},     // 0x9E
{0x40,0x20,0x00,0x70,0x88,0x88,0x70,0x00},     // 0x9F
{0x38,0x20,0x20,0x20,0xA0,0x60,0x20,0x00},     // 0xA0
{0x00,0x00,0x00,0x00,0x20,0x50,0x20,0x00},     // 0xA1
{0x70,0x40,0x40,0x40,0x40,0x40,0x40,0x00},     // 0xA2
{0x10,0x10,0x10,0x10,0x10,0x10,0x70,0x00},     // 0xA3
{0x00,0x00,0x00,0x00,0x40,0x20,0x10,0x00},     // 0xA4
{0x00,0x00,0x00,0x00,0x20,0x70,0x20,0x00},     // 0xA5
{0xF8,0x08,0xF8,0x08,0x08,0x10,0x20,0x00},     // 0xA6
{0x00,0x00,0xF8,0x08,0x30,0x20,0x40,0x00},     // 0xA7
{0x00,0x00,0x18,0x60,0xA0,0x20,0x20,0x00},     // 0xA8
{0x00,0x00,0x20,0xF8,0x88,0x10,0x20,0x00},     // 0xA9
{0x00,0x00,0xF8,0x20,0x20,0x20,0xF8,0x00},     // 0xAA
{0x00,0x00,0x10,0xF8,0x30,0x50,0x90,0x00},     // 0xAB
{0x00,0x00,0x40,0xF8,0x48,0x50,0x40,0x00},     // 0xAC
{0x00,0x00,0x70,0x10,0x10,0x10,0xF8,0x00},     // 0xAD
{0x00,0x00,0xF0,0x10,0xF0,0x10,0xF0,0x00},     // 0xAE
{0x00,0x00,0xA8,0xA8,0x08,0x10,0x20,0x00},     // 0xAF
{0x00,0x00,0x00,0x70,0x00,0x00,0x00,0x00},     // 0xB0
{0xF8,0x08,0x28,0x30,0x20,0x20,0x40,0x00},     // 0xB1
{0x08,0x10,0x30,0x50,0x90,0x10,0x10,0x00},     // 0xB2
{0x20,0xF8,0x88,0x08,0x08,0x10,0x20,0x00},     // 0xB3
{0xF8,0x20,0x20,0x20,0x20,0x20,0xF8,0x00},     // 0xB4
{0x10,0xF8,0x10,0x30,0x50,0x90,0x10,0x00},     // 0xB5
{0x40,0xF8,0x48,0x48,0x48,0x48,0x98,0x00},     // 0xB6
{0x20,0xF8,0x20,0x20,0xF8,0x20,0x20,0x00},     // 0xB7
{0x78,0x48,0x88,0x08,0x08,0x10,0x20,0x00},     // 0xB8
{0x40,0x78,0x90,0x10,0x10,0x20,0x40,0x00},     // 0xB9
{0xF8,0x08,0x08,0x08,0x08,0x08,0xF8,0x00},     // 0xBA
{0x50,0xF8,0x50,0x50,0x10,0x20,0x40,0x00},     // 0xBB
{0xC0,0x00,0xC8,0x08,0x08,0x10,0xE0,0x00},     // 0xBC
{0xF8,0x08,0x08,0x10,0x20,0x50,0x88,0x00},     // 0xBD
{0x40,0xF8,0x48,0x40,0x40,0x40,0x38,0x00},     // 0xBE
{0x88,0x48,0x08,0x08,0x08,0x10,0x20,0x00},     // 0xBF
{0x78,0x48,0x88,0x38,0x08,0x10,0x20,0x00},     // 0xC0
{0xF8,0x20,0xF8,0x20,0x20,0x40,0x80,0x00},     // 0xC1
{0xA8,0xA8,0xA8,0x08,0x08,0x10,0x20,0x00},     // 0xC2
{0x70,0x00,0xF8,0x20,0x20,0x40,0x80,0x00},     // 0xC3
{0x40,0x40,0x40,0x60,0x50,0x40,0x40,0x00},     // 0xC4
{0x20,0xF8,0x20,0x20,0x20,0x40,0x80,0x00},     // 0xC5
{0x70,0x00,0x00,0x00,0x00,0x00,0xF8,0x00},     // 0xC6
{0xF8,0x08,0x08,0x50,0x20,0x50,0x88,0x00},     // 0xC7
{0x20,0xF8,0x10,0x20,0x70,0xA8,0x20,0x00},     // 0xC8
{0x08,0x08,0x08,0x08,0x10,0x20,0x40,0x00},     // 0xC9
{0x50,0x50,0x50,0x48,0x48,0x48,0x88,0x00},     // 0xCA
{0x80,0x80,0xF8,0x80,0x80,0x80,0x78,0x00},     // 0xCB
{0xF8,0x08,0x08,0x08,0x10,0x20,0x40,0x00},     // 0xCC
{0x40,0xA0,0xA0,0x90,0x10,0x08,0x08,0x00},     // 0xCD
{0x20,0xF8,0x20,0xA8,0xA8,0xA8,0x20,0x00},     // 0xCE
{0xF8,0x08,0x08,0x50,0x20,0x10,0x10,0x00},     // 0xCF
{0xF8,0x00,0x00,0xF8,0x00,0x00,0xF8,0x00},     // 0xD0
{0x20,0x20,0x40,0x40,0x88,0x88,0xF8,0x00},     // 0xD1
{0x08,0x08,0x50,0x20,0x30,0x48,0x80,0x00},     // 0xD2
{0xF8,0x40,0xF8,0x40,0x40,0x40,0x38,0x00},     // 0xD3
{0x40,0xF8,0x48,0x50,0x40,0x40,0x40,0x00},     // 0xD4
{0x70,0x10,0x10,0x10,0x10,0x10,0xF8,0x00},     // 0xD5
{0xF8,0x08,0x08,0xF8,0x08,0x08,0xF8,0x00},     // 0xD6
{0x70,0x00,0xF8,0x08,0x08,0x10,0x20,0x00},     // 0xD7
{0x88,0x88,0x88,0x88,0x08,0x10,0x20,0x00},     // 0xD8
{0x20,0xA0,0xA0,0xA0,0xA0,0xA8,0xB0,0x00},     // 0xD9
{0x80,0x80,0x80,0x88,0x90,0xA0,0xC0,0x00},     // 0xDA
{0xF8,0x88,0x88,0x88,0x88,0x88,0xF8,0x00},     // 0xDB
{0xF8,0x88,0x88,0x08,0x08,0x10,0x20,0x00},     // 0xDC
{0xC0,0x00,0x08,0x08,0x08,0x10,0xE0,0x00},     // 0xDD
{0x40,0x20,0x80,0x40,0x00,0x00,0x00,0x00},     // 0xDE
{0x40,0xA0,0x40,0x00,0x00,0x00,0x00,0x00},     // 0xDF
{0x20,0x88,0x00,0x70,0x88,0x88,0x70,0x00},     // 0xE0
{0x10,0x20,0x00,0x70,0x88,0x88,0x70,0x00},     // 0xE1
{0x00,0x70,0x88,0x88,0x70,0x00,0xF8,0x00},     // 0xE2
{0x50,0x00,0x88,0x50,0x20,0x40,0x80,0x00},     // 0xE3
{0x70,0x88,0x80,0x88,0x70,0x20,0x40,0x00},     // 0xE4
{0x00,0x70,0x80,0x80,0x70,0x20,0x40,0x00},     // 0xE5
{0x70,0x00,0x88,0xC8,0xA8,0x98,0x88,0x00},     // 0xE6
{0x70,0x00,0xB0,0x48,0x48,0x48,0x48,0x00},     // 0xE7
{0xF8,0x88,0x80,0x80,0x80,0x80,0x80,0x00},     // 0xE8
{0xF8,0x88,0x40,0x20,0x40,0x88,0xF8,0x00},     // 0xE9
{0xF8,0x88,0x88,0x88,0x88,0x88,0x88,0x00},     // 0xEA
{0x70,0x88,0x88,0x88,0x88,0x50,0xD8,0x00},     // 0xEB
{0x00,0x68,0x90,0x90,0x90,0x68,0x00,0x00},     // 0xEC
{0x60,0x90,0xB0,0x88,0x88,0xF0,0x80,0x00},     // 0xED
{0x00,0xC8,0x30,0x20,0x20,0x20,0x20,0x00},     // 0xEE
{0x30,0x48,0xA0,0x90,0x90,0x60,0x00,0x00},     // 0xEF
{0x00,0x70,0x80,0x70,0x80,0x70,0x00,0x00},     // 0xF0
{0x08,0x78,0xA0,0x90,0x90,0x60,0x00,0x00},     // 0xF1
{0x60,0x90,0xF8,0x90,0x90,0x60,0x00,0x00},     // 0xF2
{0x00,0x98,0xA0,0xC0,0xA0,0x98,0x00,0x00},     // 0xF3
{0x40,0x20,0x20,0x20,0x50,0x98,0x00,0x00},     // 0xF4
{0x00,0x90,0x90,0x90,0xE8,0x80,0x80,0x00},     // 0xF5
{0x00,0x70,0x88,0x88,0xB0,0x80,0x80,0x00},     // 0xF6
{0x00,0xF8,0x50,0x50,0x50,0x98,0x00,0x00},     // 0xF7
{0x00,0xF8,0x40,0x40,0x48,0x30,0x00,0x00},     // 0xF8
{0x20,0x70,0xA8,0xA8,0x70,0x20,0x20,0x00},     // 0xF9
{0x00,0xC8,0x30,0x20,0x20,0x60,0x98,0x00},     // 0xFA
{0x00,0x98,0x88,0xA8,0xA8,0xD8,0x00,0x00},     // 0xFB
{0x00,0x88,0x88,0x90,0xA0,0xC0,0x00,0x00},     // 0xFC
{0x18,0x20,0x70,0x20,0x60,0xA0,0x58,0x00},     // 0xFD
{0x20,0x70,0xA8,0xA0,0xA8,0x70,0x20,0x00},     // 0xFE
{0x00,0x20,0x00,0xF8,0x00,0x20,0x00,0x00}};    // 0xFF



Clcdc_fp200::Clcdc_fp200(CPObject *parent, QRect _lcdRect, QRect _symbRect, QString _lcdfname, QString _symbfname):
    Clcdc(parent,_lcdRect,_symbRect,_lcdfname,_symbfname){						//[constructor]
    Color_Off.setRgb(
                        (int) (118*contrast),
                        (int) (149*contrast),
                        (int) (136*contrast));
    X = Y = 0;
    Status = 0;
    memset((char*)mem_video,0,sizeof(mem_video));
    text = true;

}

void Clcdc_fp200::Write(quint8 side, quint8 val) {
    quint8 offset = (side == 1 ? 0 : 10);
    if (Status == 1) {

        if (text) {
            AffCar(X+offset,Y,val);
            updated = true;
        }
        else {
            AddLog(LOG_DISPLAY,tr("W Video[%1,%2]=[%3]\n").arg(X+offset,2,16,QChar('0')).arg(Y,2,16,QChar('0')).arg(val,2,16,QChar('0')));
            mem_video[X+offset][Y] = val;
            updated = true;
        }
    }
    else if (Status== 0x0b) {
        switch (val) {

        case 0x40: // Graphic Mode ???
            text = false; break;
        case 0x50: // Scroll 1 line ???
            displaySL[side - 1] = Y;
            if (displaySL[side - 1] > 63) displaySL[side - 1] = 0;
            AddLog(LOG_DISPLAY,tr("%1:DSL=[%2,%3]\n").arg(side).arg(displaySL[0],2,16,QChar('0')).arg(displaySL[1],2,16,QChar('0')));
            updated = true;
            break;
        case 0x60: // ???????? text mode ?
            text = true;
            break;
        default: text = true;
            break;
        }


    }

}

INLINE int Clcdc_fp200::computeSL(int side,int ord)
{
    int y = ord;
    y -= displaySL[side - 1];
    if (y <0) y += 64;
    return y;
}

quint8 Clcdc_fp200::Read(quint8 side)
{
    quint8 offset = (side == 1 ? 0 : 10);
    quint8 val = mem_video[X + offset][Y];
    //Y++;
    return val;
}

void Clcdc_fp200::disp_symb(void)
{
    Clcdc::disp_symb();
}

bool Clcdc_fp200::init()
{

    QFile file;
    file.setFileName(":/fp200/chr.bin");
    file.open(QIODevice::ReadOnly);
    QDataStream in(&file);
    in.readRawData ((char *) &charset,0x800 );

    displaySL[0] = 0;//-16;
    displaySL[1] = 0;//-16;

    Clcdc::init();

    return true;
}

void Clcdc_fp200::disp(void)
{

    BYTE b;

    Refresh = false;

    if (!ready) return;
    if (!updated) return;

    updated = false;

    Refresh = true;

    QPainter painter(LcdImage);

//    if (((Ce500 *)pPC)->pHD61102_2->info.on_off) {
        for (int i = 0 ; i < 64; i++)
        {
            for (int li = 0 ; li < 20 ; li++)
            {

                quint8 data = mem_video[li][ i ];
                for (b=0; b<8;b++)
                {
                    //if (((data>>b)&0x01) && (pPC->pCPU->fp_log)) fprintf(pPC->pCPU->fp_log,"PSET [%i,%i]\n",i,j*8+b);
                    painter.setPen( ((data>>b)&0x01) ? Color_On : Color_Off );
                    quint8 y = computeSL((li<10)?1:2,i);
                    painter.drawPoint( li*8+b,y );
                }
            }
        }




    redraw = 0;
    painter.end();
}


void Clcdc_fp200::AffCar(UINT8 x, UINT8 y, UINT8 Car)
{


    if (Car>0) {
        AddLog (LOG_DISPLAY,tr("Draw char (%1) at %2,%3\n").arg(Car,2,16,QChar('0')).arg(x).arg(y));
        if (pPC->pCPU->fp_log) fprintf(pPC->pCPU->fp_log,"\nDraw char (%c) at %i,%i\n",Car,x,y);
    }
    for (int P_y=0;P_y<8;P_y++)
    {
        quint8 b = FP200_CarDef[Car][P_y] ;
        b = (b * 0x0202020202ULL & 0x010884422010ULL) % 1023;       // reverse bits order

        mem_video[x][y+P_y] = b<<1 ;
    }

    updated = true;
}
