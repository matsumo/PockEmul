#ifndef HPNUTD_H
#define HPNUTD_H

#include "Debug.h"

class Chpnut;

class Cdebug_hpnut:public Cdebug{
    Q_OBJECT
public:
    quint32 DisAsm_1(quint32 adr);			//disasm 1 line to Buffer

        Cdebug_hpnut(CCPU *parent);
        virtual ~Cdebug_hpnut(){}

        Chpnut *phpnut;

        static const char *nut_arith_mnem [32];
        static const char *nut_field_mnem [8];
        static const int tmap [16];
        static const char *nut_op30 [16];
        static const char *nut_op20 [16];
        static const char *nut_op18 [16];
        static const char *nut_op00 [16];

        static int nut_disassemble_inst(int addr, int op1, int op2, char *buf, int len);
        static int nut_disassemble_arith(int op1, char *buf, int len);
        static int nut_disassemble_00(int op1, int op2, char *buf, int len);
        static int nut_disassemble_long_branch(int op1, int op2, char *buf, int len);
        static int nut_disassemble_short_branch(int addr, int op1, char *buf, int len);
};



#endif // HPNUTD_H
