/*
    Skelton for retropc emulator

    Origin : MAME
    Author : Takeda.Toshiya
    Date   : 2010.08.10-

    [ Cm6502 ]
*/

#include <QDebug>

#include "m6502.h"
#include "pcxxxx.h"
#include "Inter.h"
#include "ui/cregsz80widget.h"
#include "Debug.h"
#include "Log.h"

#include "ops02.h"
//#include "opsc02.h"

void Cm6502::OP(quint8 code)
{
    int tmp;

    switch(code) {
    case 0x00: { CYCLES(7);                    BRK;            } break; /* 7 BRK */
    case 0x01: { CYCLES(6); RD_IDX;            ORA;            } break; /* 6 ORA IDX */
    case 0x02: { CYCLES(1);                    KIL;            } break; /* 1 KIL */
    case 0x03: { CYCLES(7); RD_IDX;    RD_EA;  SLO; WB_EA;     } break; /* 7 SLO IDX */
    case 0x04: { CYCLES(3); RD_ZPG;            NOP;            } break; /* 3 NOP ZPG */
    case 0x05: { CYCLES(3); RD_ZPG;            ORA;            } break; /* 3 ORA ZPG */
    case 0x06: { CYCLES(5); RD_ZPG;    RD_EA;  ASL; WB_EA;     } break; /* 5 ASL ZPG */
    case 0x07: { CYCLES(5); RD_ZPG;    RD_EA;  SLO; WB_EA;     } break; /* 5 SLO ZPG */
    case 0x08: { CYCLES(3); RD_DUM;            PHP;            } break; /* 3 PHP */
    case 0x09: { CYCLES(2); RD_IMM;            ORA;            } break; /* 2 ORA IMM */
    case 0x0a: { CYCLES(2); RD_DUM;    RD_ACC; ASL; WB_ACC;    } break; /* 2 ASL A */
    case 0x0b: { CYCLES(2); RD_IMM;            ANC;            } break; /* 2 ANC IMM */
    case 0x0c: { CYCLES(4); RD_ABS;            NOP;            } break; /* 4 NOP ABS */
    case 0x0d: { CYCLES(4); RD_ABS;            ORA;            } break; /* 4 ORA ABS */
    case 0x0e: { CYCLES(6); RD_ABS;    RD_EA;  ASL; WB_EA;     } break; /* 6 ASL ABS */
    case 0x0f: { CYCLES(6); RD_ABS;    RD_EA;  SLO; WB_EA;     } break; /* 6 SLO ABS */
    case 0x10: { CYCLES(2);                    BPL;            } break; /* 2-4 BPL REL */
    case 0x11: { CYCLES(5); RD_IDY_P;          ORA;            } break; /* 5 ORA IDY page penalty */
    case 0x12: { CYCLES(1);                    KIL;            } break; /* 1 KIL */
    case 0x13: { CYCLES(7); RD_IDY_NP; RD_EA;  SLO; WB_EA;     } break; /* 7 SLO IDY */
    case 0x14: { CYCLES(4); RD_ZPX;            NOP;            } break; /* 4 NOP ZPX */
    case 0x15: { CYCLES(4); RD_ZPX;            ORA;            } break; /* 4 ORA ZPX */
    case 0x16: { CYCLES(6); RD_ZPX;    RD_EA;  ASL; WB_EA;     } break; /* 6 ASL ZPX */
    case 0x17: { CYCLES(6); RD_ZPX;    RD_EA;  SLO; WB_EA;     } break; /* 6 SLO ZPX */
    case 0x18: { CYCLES(2); RD_DUM;            CLC;            } break; /* 2 CLC */
    case 0x19: { CYCLES(4); RD_ABY_P;          ORA;            } break; /* 4 ORA ABY page penalty */
    case 0x1a: { CYCLES(2); RD_DUM;            NOP;            } break; /* 2 NOP */
    case 0x1b: { CYCLES(7); RD_ABY_NP; RD_EA;  SLO; WB_EA;     } break; /* 7 SLO ABY */
    case 0x1c: { CYCLES(4); RD_ABX_P;          NOP;            } break; /* 4 NOP ABX page penalty */
    case 0x1d: { CYCLES(4); RD_ABX_P;          ORA;            } break; /* 4 ORA ABX page penalty */
    case 0x1e: { CYCLES(7); RD_ABX_NP; RD_EA;  ASL; WB_EA;     } break; /* 7 ASL ABX */
    case 0x1f: { CYCLES(7); RD_ABX_NP; RD_EA;  SLO; WB_EA;     } break; /* 7 SLO ABX */
    case 0x20: { CYCLES(6);                    JSR;            } break; /* 6 JSR */
    case 0x21: { CYCLES(6); RD_IDX;            AND;            } break; /* 6 AND IDX */
    case 0x22: { CYCLES(1);                    KIL;            } break; /* 1 KIL */
    case 0x23: { CYCLES(7); RD_IDX;    RD_EA;  RLA; WB_EA;     } break; /* 7 RLA IDX */
    case 0x24: { CYCLES(3); RD_ZPG;            BIT;            } break; /* 3 BIT ZPG */
    case 0x25: { CYCLES(3); RD_ZPG;            AND;            } break; /* 3 AND ZPG */
    case 0x26: { CYCLES(5); RD_ZPG;    RD_EA;  ROL; WB_EA;     } break; /* 5 ROL ZPG */
    case 0x27: { CYCLES(5); RD_ZPG;    RD_EA;  RLA; WB_EA;     } break; /* 5 RLA ZPG */
    case 0x28: { CYCLES(4); RD_DUM;            PLP;            } break; /* 4 PLP */
    case 0x29: { CYCLES(2); RD_IMM;            AND;            } break; /* 2 AND IMM */
    case 0x2a: { CYCLES(2); RD_DUM;    RD_ACC; ROL; WB_ACC;    } break; /* 2 ROL A */
    case 0x2b: { CYCLES(2); RD_IMM;            ANC;            } break; /* 2 ANC IMM */
    case 0x2c: { CYCLES(4); RD_ABS;            BIT;            } break; /* 4 BIT ABS */
    case 0x2d: { CYCLES(4); RD_ABS;            AND;            } break; /* 4 AND ABS */
    case 0x2e: { CYCLES(6); RD_ABS;    RD_EA;  ROL; WB_EA;     } break; /* 6 ROL ABS */
    case 0x2f: { CYCLES(6); RD_ABS;    RD_EA;  RLA; WB_EA;     } break; /* 6 RLA ABS */
    case 0x30: { CYCLES(2);                    BMI;            } break; /* 2-4 BMI REL */
    case 0x31: { CYCLES(5); RD_IDY_P;          AND;            } break; /* 5 AND IDY page penalty */
    case 0x32: { CYCLES(1);                    KIL;            } break; /* 1 KIL */
    case 0x33: { CYCLES(7); RD_IDY_NP; RD_EA;  RLA; WB_EA;     } break; /* 7 RLA IDY */
    case 0x34: { CYCLES(4); RD_ZPX;            NOP;            } break; /* 4 NOP ZPX */
    case 0x35: { CYCLES(4); RD_ZPX;            AND;            } break; /* 4 AND ZPX */
    case 0x36: { CYCLES(6); RD_ZPX;    RD_EA;  ROL; WB_EA;     } break; /* 6 ROL ZPX */
    case 0x37: { CYCLES(6); RD_ZPX;    RD_EA;  RLA; WB_EA;     } break; /* 6 RLA ZPX */
    case 0x38: { CYCLES(2); RD_DUM;            SEC;            } break; /* 2 SEC */
    case 0x39: { CYCLES(4); RD_ABY_P;          AND;            } break; /* 4 AND ABY page penalty */
    case 0x3a: { CYCLES(2); RD_DUM;            NOP;            } break; /* 2 NOP */
    case 0x3b: { CYCLES(7); RD_ABY_NP; RD_EA;  RLA; WB_EA;     } break; /* 7 RLA ABY */
    case 0x3c: { CYCLES(4); RD_ABX_P;          NOP;            } break; /* 4 NOP ABX page penalty */
    case 0x3d: { CYCLES(4); RD_ABX_P;          AND;            } break; /* 4 AND ABX page penalty */
    case 0x3e: { CYCLES(7); RD_ABX_NP; RD_EA;  ROL; WB_EA;     } break; /* 7 ROL ABX */
    case 0x3f: { CYCLES(7); RD_ABX_NP; RD_EA;  RLA; WB_EA;     } break; /* 7 RLA ABX */
    case 0x40: { CYCLES(6);                    RTI;            } break; /* 6 RTI */
    case 0x41: { CYCLES(6); RD_IDX;            EOR;            } break; /* 6 EOR IDX */
    case 0x42: { CYCLES(1);                    KIL;            } break; /* 1 KIL */
    case 0x43: { CYCLES(7); RD_IDX;    RD_EA;  SRE; WB_EA;     } break; /* 7 SRE IDX */
    case 0x44: { CYCLES(3); RD_ZPG;            NOP;            } break; /* 3 NOP ZPG */
    case 0x45: { CYCLES(3); RD_ZPG;            EOR;            } break; /* 3 EOR ZPG */
    case 0x46: { CYCLES(5); RD_ZPG;    RD_EA;  LSR; WB_EA;     } break; /* 5 LSR ZPG */
    case 0x47: { CYCLES(5); RD_ZPG;    RD_EA;  SRE; WB_EA;     } break; /* 5 SRE ZPG */
    case 0x48: { CYCLES(3); RD_DUM;            PHA;            } break; /* 3 PHA */
    case 0x49: { CYCLES(2); RD_IMM;            EOR;            } break; /* 2 EOR IMM */
    case 0x4a: { CYCLES(2); RD_DUM;    RD_ACC; LSR; WB_ACC;    } break; /* 2 LSR A */
    case 0x4b: { CYCLES(2); RD_IMM;            ASR; WB_ACC;    } break; /* 2 ASR IMM */
    case 0x4c: { CYCLES(3); EA_ABS;            JMP;            } break; /* 3 JMP ABS */
    case 0x4d: { CYCLES(4); RD_ABS;            EOR;            } break; /* 4 EOR ABS */
    case 0x4e: { CYCLES(6); RD_ABS;    RD_EA;  LSR; WB_EA;     } break; /* 6 LSR ABS */
    case 0x4f: { CYCLES(6); RD_ABS;    RD_EA;  SRE; WB_EA;     } break; /* 6 SRE ABS */
    case 0x50: { CYCLES(2);                    BVC;            } break; /* 2-4 BVC REL */
    case 0x51: { CYCLES(5); RD_IDY_P;          EOR;            } break; /* 5 EOR IDY page penalty */
    case 0x52: { CYCLES(1);                    KIL;            } break; /* 1 KIL */
    case 0x53: { CYCLES(7); RD_IDY_NP; RD_EA;  SRE; WB_EA;     } break; /* 7 SRE IDY */
    case 0x54: { CYCLES(4); RD_ZPX;            NOP;            } break; /* 4 NOP ZPX */
    case 0x55: { CYCLES(4); RD_ZPX;            EOR;            } break; /* 4 EOR ZPX */
    case 0x56: { CYCLES(6); RD_ZPX;    RD_EA;  LSR; WB_EA;     } break; /* 6 LSR ZPX */
    case 0x57: { CYCLES(6); RD_ZPX;    RD_EA;  SRE; WB_EA;     } break; /* 6 SRE ZPX */
    case 0x58: { CYCLES(2); RD_DUM;            CLI;            } break; /* 2 CLI */
    case 0x59: { CYCLES(4); RD_ABY_P;          EOR;            } break; /* 4 EOR ABY page penalty */
    case 0x5a: { CYCLES(2); RD_DUM;            NOP;            } break; /* 2 NOP */
    case 0x5b: { CYCLES(7); RD_ABY_NP; RD_EA;  SRE; WB_EA;     } break; /* 7 SRE ABY */
    case 0x5c: { CYCLES(4); RD_ABX_P;          NOP;            } break; /* 4 NOP ABX page penalty */
    case 0x5d: { CYCLES(4); RD_ABX_P;          EOR;            } break; /* 4 EOR ABX page penalty */
    case 0x5e: { CYCLES(7); RD_ABX_NP; RD_EA;  LSR; WB_EA;     } break; /* 7 LSR ABX */
    case 0x5f: { CYCLES(7); RD_ABX_NP; RD_EA;  SRE; WB_EA;     } break; /* 7 SRE ABX */
    case 0x60: { CYCLES(6);                    RTS;            } break; /* 6 RTS */
    case 0x61: { CYCLES(6); RD_IDX;            ADC;            } break; /* 6 ADC IDX */
    case 0x62: { CYCLES(1);                    KIL;            } break; /* 1 KIL */
    case 0x63: { CYCLES(7); RD_IDX;    RD_EA;  RRA; WB_EA;     } break; /* 7 RRA IDX */
    case 0x64: { CYCLES(3); RD_ZPG;            NOP;            } break; /* 3 NOP ZPG */
    case 0x65: { CYCLES(3); RD_ZPG;            ADC;            } break; /* 3 ADC ZPG */
    case 0x66: { CYCLES(5); RD_ZPG;    RD_EA;  ROR; WB_EA;     } break; /* 5 ROR ZPG */
    case 0x67: { CYCLES(5); RD_ZPG;    RD_EA;  RRA; WB_EA;     } break; /* 5 RRA ZPG */
    case 0x68: { CYCLES(4); RD_DUM;            PLA;            } break; /* 4 PLA */
    case 0x69: { CYCLES(2); RD_IMM;            ADC;            } break; /* 2 ADC IMM */
    case 0x6a: { CYCLES(2); RD_DUM;    RD_ACC; ROR; WB_ACC;    } break; /* 2 ROR A */
    case 0x6b: { CYCLES(2); RD_IMM;            ARR; WB_ACC;    } break; /* 2 ARR IMM */
    case 0x6c: { CYCLES(5); EA_IND;            JMP;            } break; /* 5 JMP IND */
    case 0x6d: { CYCLES(4); RD_ABS;            ADC;            } break; /* 4 ADC ABS */
    case 0x6e: { CYCLES(6); RD_ABS;    RD_EA;  ROR; WB_EA;     } break; /* 6 ROR ABS */
    case 0x6f: { CYCLES(6); RD_ABS;    RD_EA;  RRA; WB_EA;     } break; /* 6 RRA ABS */
    case 0x70: { CYCLES(2);                    BVS;            } break; /* 2-4 BVS REL */
    case 0x71: { CYCLES(5); RD_IDY_P;          ADC;            } break; /* 5 ADC IDY page penalty */
    case 0x72: { CYCLES(1);                    KIL;            } break; /* 1 KIL */
    case 0x73: { CYCLES(7); RD_IDY_NP; RD_EA;  RRA; WB_EA;     } break; /* 7 RRA IDY */
    case 0x74: { CYCLES(4); RD_ZPX;            NOP;            } break; /* 4 NOP ZPX */
    case 0x75: { CYCLES(4); RD_ZPX;            ADC;            } break; /* 4 ADC ZPX */
    case 0x76: { CYCLES(6); RD_ZPX;    RD_EA;  ROR; WB_EA;     } break; /* 6 ROR ZPX */
    case 0x77: { CYCLES(6); RD_ZPX;    RD_EA;  RRA; WB_EA;     } break; /* 6 RRA ZPX */
    case 0x78: { CYCLES(2); RD_DUM;            SEI;            } break; /* 2 SEI */
    case 0x79: { CYCLES(4); RD_ABY_P;          ADC;            } break; /* 4 ADC ABY page penalty */
    case 0x7a: { CYCLES(2); RD_DUM;            NOP;            } break; /* 2 NOP */
    case 0x7b: { CYCLES(7); RD_ABY_NP; RD_EA;  RRA; WB_EA;     } break; /* 7 RRA ABY */
    case 0x7c: { CYCLES(4); RD_ABX_P;          NOP;            } break; /* 4 NOP ABX page penalty */
    case 0x7d: { CYCLES(4); RD_ABX_P;          ADC;            } break; /* 4 ADC ABX page penalty */
    case 0x7e: { CYCLES(7); RD_ABX_NP; RD_EA;  ROR; WB_EA;     } break; /* 7 ROR ABX */
    case 0x7f: { CYCLES(7); RD_ABX_NP; RD_EA;  RRA; WB_EA;     } break; /* 7 RRA ABX */
    case 0x80: { CYCLES(2); RD_IMM;            NOP;            } break; /* 2 NOP IMM */
    case 0x81: { CYCLES(6);                    STA; WR_IDX;    } break; /* 6 STA IDX */
    case 0x82: { CYCLES(2); RD_IMM;            NOP;            } break; /* 2 NOP IMM */
    case 0x83: { CYCLES(6);                    SAX; WR_IDX;    } break; /* 6 SAX IDX */
    case 0x84: { CYCLES(3);                    STY; WR_ZPG;    } break; /* 3 STY ZPG */
    case 0x85: { CYCLES(3);                    STA; WR_ZPG;    } break; /* 3 STA ZPG */
    case 0x86: { CYCLES(3);                    STX; WR_ZPG;    } break; /* 3 STX ZPG */
    case 0x87: { CYCLES(3);                    SAX; WR_ZPG;    } break; /* 3 SAX ZPG */
    case 0x88: { CYCLES(2); RD_DUM;            DEY;            } break; /* 2 DEY */
    case 0x89: { CYCLES(2); RD_IMM;            NOP;            } break; /* 2 NOP IMM */
    case 0x8a: { CYCLES(2); RD_DUM;            TXA;            } break; /* 2 TXA */
    case 0x8b: { CYCLES(2); RD_IMM;            AXA;            } break; /* 2 AXA IMM */
    case 0x8c: { CYCLES(4);                    STY; WR_ABS;    } break; /* 4 STY ABS */
    case 0x8d: { CYCLES(4);                    STA; WR_ABS;    } break; /* 4 STA ABS */
    case 0x8e: { CYCLES(4);                    STX; WR_ABS;    } break; /* 4 STX ABS */
    case 0x8f: { CYCLES(4);                    SAX; WR_ABS;    } break; /* 4 SAX ABS */
    case 0x90: { CYCLES(2);                    BCC;            } break; /* 2-4 BCC REL */
    case 0x91: { CYCLES(6);                    STA; WR_IDY_NP; } break; /* 6 STA IDY */
    case 0x92: { CYCLES(1);                    KIL;            } break; /* 1 KIL */
    case 0x93: { CYCLES(5); EA_IDY_NP;         SAH; WB_EA;     } break; /* 5 SAH IDY */
    case 0x94: { CYCLES(4);                    STY; WR_ZPX;    } break; /* 4 STY ZPX */
    case 0x95: { CYCLES(4);                    STA; WR_ZPX;    } break; /* 4 STA ZPX */
    case 0x96: { CYCLES(4);                    STX; WR_ZPY;    } break; /* 4 STX ZPY */
    case 0x97: { CYCLES(4);                    SAX; WR_ZPY;    } break; /* 4 SAX ZPY */
    case 0x98: { CYCLES(2); RD_DUM;            TYA;            } break; /* 2 TYA */
    case 0x99: { CYCLES(5);                    STA; WR_ABY_NP; } break; /* 5 STA ABY */
    case 0x9a: { CYCLES(2); RD_DUM;            TXS;            } break; /* 2 TXS */
    case 0x9b: { CYCLES(5); EA_ABY_NP;         SSH; WB_EA;     } break; /* 5 SSH ABY */
    case 0x9c: { CYCLES(5); EA_ABX_NP;         SYH; WB_EA;     } break; /* 5 SYH ABX */
    case 0x9d: { CYCLES(5);                    STA; WR_ABX_NP; } break; /* 5 STA ABX */
    case 0x9e: { CYCLES(5); EA_ABY_NP;         SXH; WB_EA;     } break; /* 5 SXH ABY */
    case 0x9f: { CYCLES(5); EA_ABY_NP;         SAH; WB_EA;     } break; /* 5 SAH ABY */
    case 0xa0: { CYCLES(2); RD_IMM;            LDY;            } break; /* 2 LDY IMM */
    case 0xa1: { CYCLES(6); RD_IDX;            LDA;            } break; /* 6 LDA IDX */
    case 0xa2: { CYCLES(2); RD_IMM;            LDX;            } break; /* 2 LDX IMM */
    case 0xa3: { CYCLES(6); RD_IDX;            LAX;            } break; /* 6 LAX IDX */
    case 0xa4: { CYCLES(3); RD_ZPG;            LDY;            } break; /* 3 LDY ZPG */
    case 0xa5: { CYCLES(3); RD_ZPG;            LDA;            } break; /* 3 LDA ZPG */
    case 0xa6: { CYCLES(3); RD_ZPG;            LDX;            } break; /* 3 LDX ZPG */
    case 0xa7: { CYCLES(3); RD_ZPG;            LAX;            } break; /* 3 LAX ZPG */
    case 0xa8: { CYCLES(2); RD_DUM;            TAY;            } break; /* 2 TAY */
    case 0xa9: { CYCLES(2); RD_IMM;            LDA;            } break; /* 2 LDA IMM */
    case 0xaa: { CYCLES(2); RD_DUM;            TAX;            } break; /* 2 TAX */
    case 0xab: { CYCLES(2); RD_IMM;            OAL;            } break; /* 2 OAL IMM */
    case 0xac: { CYCLES(4); RD_ABS;            LDY;            } break; /* 4 LDY ABS */
    case 0xad: { CYCLES(4); RD_ABS;            LDA;            } break; /* 4 LDA ABS */
    case 0xae: { CYCLES(4); RD_ABS;            LDX;            } break; /* 4 LDX ABS */
    case 0xaf: { CYCLES(4); RD_ABS;            LAX;            } break; /* 4 LAX ABS */
    case 0xb0: { CYCLES(2);                    BCS;            } break; /* 2-4 BCS REL */
    case 0xb1: { CYCLES(5); RD_IDY_P;          LDA;            } break; /* 5 LDA IDY page penalty */
    case 0xb2: { CYCLES(1);                    KIL;            } break; /* 1 KIL */
    case 0xb3: { CYCLES(5); RD_IDY_P;          LAX;            } break; /* 5 LAX IDY page penalty */
    case 0xb4: { CYCLES(4); RD_ZPX;            LDY;            } break; /* 4 LDY ZPX */
    case 0xb5: { CYCLES(4); RD_ZPX;            LDA;            } break; /* 4 LDA ZPX */
    case 0xb6: { CYCLES(4); RD_ZPY;            LDX;            } break; /* 4 LDX ZPY */
    case 0xb7: { CYCLES(4); RD_ZPY;            LAX;            } break; /* 4 LAX ZPY */
    case 0xb8: { CYCLES(2); RD_DUM;            CLV;            } break; /* 2 CLV */
    case 0xb9: { CYCLES(4); RD_ABY_P;          LDA;            } break; /* 4 LDA ABY page penalty */
    case 0xba: { CYCLES(2); RD_DUM;            TSX;            } break; /* 2 TSX */
    case 0xbb: { CYCLES(4); RD_ABY_P;          AST;            } break; /* 4 AST ABY page penalty */
    case 0xbc: { CYCLES(4); RD_ABX_P;          LDY;            } break; /* 4 LDY ABX page penalty */
    case 0xbd: { CYCLES(4); RD_ABX_P;          LDA;            } break; /* 4 LDA ABX page penalty */
    case 0xbe: { CYCLES(4); RD_ABY_P;          LDX;            } break; /* 4 LDX ABY page penalty */
    case 0xbf: { CYCLES(4); RD_ABY_P;          LAX;            } break; /* 4 LAX ABY page penalty */
    case 0xc0: { CYCLES(2); RD_IMM;            CPY;            } break; /* 2 CPY IMM */
    case 0xc1: { CYCLES(6); RD_IDX;            CMP;            } break; /* 6 CMP IDX */
    case 0xc2: { CYCLES(2); RD_IMM;            NOP;            } break; /* 2 NOP IMM */
    case 0xc3: { CYCLES(7); RD_IDX;    RD_EA;  DCP; WB_EA;     } break; /* 7 DCP IDX */
    case 0xc4: { CYCLES(3); RD_ZPG;            CPY;            } break; /* 3 CPY ZPG */
    case 0xc5: { CYCLES(3); RD_ZPG;            CMP;            } break; /* 3 CMP ZPG */
    case 0xc6: { CYCLES(5); RD_ZPG;    RD_EA;  DEC; WB_EA;     } break; /* 5 DEC ZPG */
    case 0xc7: { CYCLES(5); RD_ZPG;    RD_EA;  DCP; WB_EA;     } break; /* 5 DCP ZPG */
    case 0xc8: { CYCLES(2); RD_DUM;            INY;            } break; /* 2 INY */
    case 0xc9: { CYCLES(2); RD_IMM;            CMP;            } break; /* 2 CMP IMM */
    case 0xca: { CYCLES(2); RD_DUM;            DEX;            } break; /* 2 DEX */
    case 0xcb: { CYCLES(2); RD_IMM;            ASX;            } break; /* 2 ASX IMM */
    case 0xcc: { CYCLES(4); RD_ABS;            CPY;            } break; /* 4 CPY ABS */
    case 0xcd: { CYCLES(4); RD_ABS;            CMP;            } break; /* 4 CMP ABS */
    case 0xce: { CYCLES(6); RD_ABS;    RD_EA;  DEC; WB_EA;     } break; /* 6 DEC ABS */
    case 0xcf: { CYCLES(6); RD_ABS;    RD_EA;  DCP; WB_EA;     } break; /* 6 DCP ABS */
    case 0xd0: { CYCLES(2);                    BNE;            } break; /* 2-4 BNE REL */
    case 0xd1: { CYCLES(5); RD_IDY_P;          CMP;            } break; /* 5 CMP IDY page penalty */
    case 0xd2: { CYCLES(1);                    KIL;            } break; /* 1 KIL */
    case 0xd3: { CYCLES(7); RD_IDY_NP; RD_EA;  DCP; WB_EA;     } break; /* 7 DCP IDY */
    case 0xd4: { CYCLES(4); RD_ZPX;            NOP;            } break; /* 4 NOP ZPX */
    case 0xd5: { CYCLES(4); RD_ZPX;            CMP;            } break; /* 4 CMP ZPX */
    case 0xd6: { CYCLES(6); RD_ZPX;    RD_EA;  DEC; WB_EA;     } break; /* 6 DEC ZPX */
    case 0xd7: { CYCLES(6); RD_ZPX;    RD_EA;  DCP; WB_EA;     } break; /* 6 DCP ZPX */
    case 0xd8: { CYCLES(2); RD_DUM;            CLD;            } break; /* 2 CLD */
    case 0xd9: { CYCLES(4); RD_ABY_P;          CMP;            } break; /* 4 CMP ABY page penalty */
    case 0xda: { CYCLES(2); RD_DUM;            NOP;            } break; /* 2 NOP */
    case 0xdb: { CYCLES(7); RD_ABY_NP; RD_EA;  DCP; WB_EA;     } break; /* 7 DCP ABY */
    case 0xdc: { CYCLES(4); RD_ABX_P;          NOP;            } break; /* 4 NOP ABX page penalty */
    case 0xdd: { CYCLES(4); RD_ABX_P;          CMP;            } break; /* 4 CMP ABX page penalty */
    case 0xde: { CYCLES(7); RD_ABX_NP; RD_EA;  DEC; WB_EA;     } break; /* 7 DEC ABX */
    case 0xdf: { CYCLES(7); RD_ABX_NP; RD_EA;  DCP; WB_EA;     } break; /* 7 DCP ABX */
    case 0xe0: { CYCLES(2); RD_IMM;            CPX;            } break; /* 2 CPX IMM */
    case 0xe1: { CYCLES(6); RD_IDX;            SBC;            } break; /* 6 SBC IDX */
    case 0xe2: { CYCLES(2); RD_IMM;            NOP;            } break; /* 2 NOP IMM */
    case 0xe3: { CYCLES(7); RD_IDX;    RD_EA;  ISB; WB_EA;     } break; /* 7 ISB IDX */
    case 0xe4: { CYCLES(3); RD_ZPG;            CPX;            } break; /* 3 CPX ZPG */
    case 0xe5: { CYCLES(3); RD_ZPG;            SBC;            } break; /* 3 SBC ZPG */
    case 0xe6: { CYCLES(5); RD_ZPG;    RD_EA;  INC; WB_EA;     } break; /* 5 INC ZPG */
    case 0xe7: { CYCLES(5); RD_ZPG;    RD_EA;  ISB; WB_EA;     } break; /* 5 ISB ZPG */
    case 0xe8: { CYCLES(2); RD_DUM;            INX;            } break; /* 2 INX */
    case 0xe9: { CYCLES(2); RD_IMM;            SBC;            } break; /* 2 SBC IMM */
    case 0xea: { CYCLES(2); RD_DUM;            NOP;            } break; /* 2 NOP */
    case 0xeb: { CYCLES(2); RD_IMM;            SBC;            } break; /* 2 SBC IMM */
    case 0xec: { CYCLES(4); RD_ABS;            CPX;            } break; /* 4 CPX ABS */
    case 0xed: { CYCLES(4); RD_ABS;            SBC;            } break; /* 4 SBC ABS */
    case 0xee: { CYCLES(6); RD_ABS;    RD_EA;  INC; WB_EA;     } break; /* 6 INC ABS */
    case 0xef: { CYCLES(6); RD_ABS;    RD_EA;  ISB; WB_EA;     } break; /* 6 ISB ABS */
    case 0xf0: { CYCLES(2);                    BEQ;            } break; /* 2-4 BEQ REL */
    case 0xf1: { CYCLES(5); RD_IDY_P;          SBC;            } break; /* 5 SBC IDY page penalty */
    case 0xf2: { CYCLES(1);                    KIL;            } break; /* 1 KIL */
    case 0xf3: { CYCLES(7); RD_IDY_NP; RD_EA;  ISB; WB_EA;     } break; /* 7 ISB IDY */
    case 0xf4: { CYCLES(4); RD_ZPX;            NOP;            } break; /* 4 NOP ZPX */
    case 0xf5: { CYCLES(4); RD_ZPX;            SBC;            } break; /* 4 SBC ZPX */
    case 0xf6: { CYCLES(6); RD_ZPX;    RD_EA;  INC; WB_EA;     } break; /* 6 INC ZPX */
    case 0xf7: { CYCLES(6); RD_ZPX;    RD_EA;  ISB; WB_EA;     } break; /* 6 ISB ZPX */
    case 0xf8: { CYCLES(2); RD_DUM;            SED;            } break; /* 2 SED */
    case 0xf9: { CYCLES(4); RD_ABY_P;          SBC;            } break; /* 4 SBC ABY page penalty */
    case 0xfa: { CYCLES(2); RD_DUM;            NOP;            } break; /* 2 NOP */
    case 0xfb: { CYCLES(7); RD_ABY_NP; RD_EA;  ISB; WB_EA;     } break; /* 7 ISB ABY */
    case 0xfc: { CYCLES(4); RD_ABX_P;          NOP;            } break; /* 4 NOP ABX page penalty */
    case 0xfd: { CYCLES(4); RD_ABX_P;          SBC;            } break; /* 4 SBC ABX page penalty */
    case 0xfe: { CYCLES(7); RD_ABX_NP; RD_EA;  INC; WB_EA;     } break; /* 7 INC ABX */
    case 0xff: { CYCLES(7); RD_ABX_NP; RD_EA;  ISB; WB_EA;     } break; /* 7 ISB ABX */
    default: break;
    }
}

inline void Cm6502::update_irq()
{
//    if (fp_log) fprintf(fp_log,"\n INT update_irq\n");
AddLog(LOG_CONSOLE,"updateIRQ\n");
    if(!(P & F_I)) {
        if (fp_log) fprintf(fp_log,"\n INT update_irq GO\n");

        AddLog(LOG_CONSOLE,"updateIRQ GOGOGO\n");
        EAD = IRQ_VEC;
        CYCLES(2);
        PUSH(PCH);
        PUSH(PCL);
        PUSH(P & ~F_B);
        P |= F_I;
        PCL = RDMEM(EAD);
        PCH = RDMEM(EAD + 1);
        CallSubLevel++;
        // call back the cpuintrf to let it clear the line
        //d_pic->intr_reti();
        irq_state = false;
        halt = false;
        AddLog(LOG_CONSOLE,"CPU RUNNING\n");
//        qWarning()<<"CPU RUNNING";
    }
    pending_irq = false;
}

// main

bool Cm6502::init(void)
{
    Check_Log();
    pDEBUG->init();
    A = X = Y = P = 0;
    SPD = EAD = ZPD = PCD = 0;
    return true;
}

bool Cm6502::exit()
{
    return true;
}

void Cm6502::Reset(void)
{
    if (fp_log) fprintf(fp_log,"\nRESET\n");
    PCL = RDMEM(RST_VEC);
    PCH = RDMEM(RST_VEC + 1);
    SPD = 0x01ff;
    P = F_T | F_I | F_Z | F_B | (P & F_D);

    icount = 0;
    pending_irq = after_cli = false;
    irq_state = nmi_state = so_state = false;
    halt = false;
    CallSubLevel = 0;
    AddLog(LOG_CONSOLE,"CPU RUNNING\n");
//    qWarning()<<"CPU RUNNING";
}

Cm6502::Cm6502(CPObject *parent)	: CCPU(parent)
{				//[constructor]

    pDEBUG = new Cdebug_m6502(parent);

    fn_log="m6502.log";

    regwidget = (CregCPU*) new Cregsz80Widget(0,this);
}

void Cm6502::step(void) {
    run_one_opecode();
}

int Cm6502::run(int clock)
{
    // return now if BUSREQ
    if(busreq) {
        icount = 0;
        return 1;
    }

    // run cpu
    if(clock == -1) {
        // run only one opcode
        icount = 0;
        run_one_opecode();
        return -icount;
    }
    else {
        // run cpu while given clocks
        icount += clock;
        int first_icount = icount;

        while(icount > 0 && !busreq) {
            run_one_opecode();
        }
        int passed_icount = first_icount - icount;
        if(busreq && icount > 0) {
            icount = 0;
        }
        return passed_icount;
    }
}

void Cm6502::run_one_opecode()
{
    // if an irq is pending, take it now
    if(nmi_state) {
        EAD = NMI_VEC;
        CYCLES(2);
        PUSH(PCH);
        PUSH(PCL);
        PUSH(P & ~F_B);
        P |= F_I;	// set I flag
        PCL = RDMEM(EAD);
        PCH = RDMEM(EAD + 1);
        CallSubLevel++;
        nmi_state = false;
        halt=false;
        AddLog(LOG_CONSOLE,"CPU RUNNING\n");
//        qWarning()<<"CPU RUNNING";
        if (fp_log) fprintf(fp_log,"\n INT NMI newpc:%04x\n",PCW);
    }
    else if(pending_irq) {
        if (halt) Reset();
        else update_irq();
    }
    if (halt) {
        CYCLES(50);
        P &= ~F_I;
        return;
    }

    prev_pc = pc.w.l;
    quint8 code = RDOP();
    OP(code);

    // check if the I flag was just reset (interrupts enabled)
    if(after_cli) {
        after_cli = false;
        if(irq_state) {
            if (fp_log) fprintf(fp_log,"\n INT after_cli irq\n");

            pending_irq = true;
        }
    }
    else if(pending_irq) {
        update_irq();
    }
}

void Cm6502::write_signal(int id, quint32 data, quint32 mask)
{
#if 1
    bool state = ((data & mask) != 0);

    if(id == SIG_CPU_NMI) {
        nmi_state = state;
    }
    else if(id == SIG_CPU_IRQ) {
        irq_state = state;
        if(state) {
            pending_irq = true;
            AddLog(LOG_CONSOLE,"pending IRQ\n");
        }
    }
    else if(id == SIG_CPU_OVERFLOW) {
        if(so_state && !state) {
            P |= F_V;
        }
        so_state = state;
    }
    else if(id == SIG_CPU_BUSREQ) {
        busreq = ((data & mask) != 0);
    }
#endif
}

inline BYTE Cm6502::read_data8( UINT32 address)
{
    return (((CpcXXXX *)pPC)->Get_8(address));
}
inline void Cm6502::write_data8( UINT32 address, BYTE value)
{
    ((CpcXXXX *)pPC)->Set_8(address,value);
}

UINT32 Cm6502::get_PC(void)
{
    return(PCW);
}

void Cm6502::set_PC(UINT32 newpc)
{
    PCW = newpc;
}

void Cm6502::Regs_Info(UINT8 Type)
{
    switch(Type)
    {
    case 0:			// Monitor Registers Dialog
        sprintf(Regs_String,	"PC:%.4x A:%02X\nX:%02X Y:%02X\nP:%02X SPD:%04X\n%s%s%s%s%s%s%s%s",
                            PCW,A,X,Y,P,SPD,
                P&F_N ? "N":".",
                P&F_V ? "V":".",
                P&F_T ? "T":".",
                P&F_B ? "B":".",
                P&F_D ? "D":".",
                P&F_I ? "I":".",
                P&F_Z ? "Z":".",
                P&F_C ? "C":"."


                );
        break;
    case 1:			// For Log File
        sprintf(Regs_String,	"PC:%.4x A:%02X X:%02X Y:%02X P:%02X SPD:%04X  %s%s%s%s%s%s%s%s",
                            PCW,A,X,Y,P,SPD,
                P&F_N ? "N":".",
                P&F_V ? "V":".",
                P&F_T ? "T":".",
                P&F_B ? "B":".",
                P&F_D ? "D":".",
                P&F_I ? "I":".",
                P&F_Z ? "Z":".",
                P&F_C ? "C":"."


                );
        break;
    }

}
