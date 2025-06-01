#ifndef CPU_H

#include <stdint.h>

typedef unsigned char Byte;
typedef unsigned short Word;

typedef uint32_t Uint32;
typedef int32_t Sint32;

typedef struct CPU CPU;
typedef struct Memory Memory;

/* Instructions */

/* Read instructions */
static constexpr Byte INS_LDA_IM = 0xA9;
static constexpr Byte INS_LDA_ZP = 0xA5;
static constexpr Byte INS_LDA_ZPX = 0xB5;
static constexpr Byte INS_LDA_ABS = 0xAD;
static constexpr Byte INS_LDA_ABX = 0xBD;
static constexpr Byte INS_LDA_ABY = 0xB9;
static constexpr Byte INS_LDA_IDX = 0xA1;
static constexpr Byte INS_LDA_IDY = 0xB1;
static constexpr Byte INS_LDX_IM = 0xA2;
static constexpr Byte INS_LDX_ZP = 0xA6;
static constexpr Byte INS_LDX_ZPY = 0xB6;
static constexpr Byte INS_LDX_ABS = 0xAE;
static constexpr Byte INS_LDX_ABY = 0xBE;
static constexpr Byte INS_LDY_IM = 0xA0;
static constexpr Byte INS_LDY_ZP = 0xA4;
static constexpr Byte INS_LDY_ZPX = 0xB4;
static constexpr Byte INS_LDY_ABS = 0xAC;
static constexpr Byte INS_LDY_ABX = 0xBC;

/* Write Instructions */
static constexpr Byte INS_STA_ZP = 0x85;
static constexpr Byte INS_STA_ZPX = 0x95;
static constexpr Byte INS_STA_ABS = 0x8D;
static constexpr Byte INS_STA_ABX = 0x9D;
static constexpr Byte INS_STA_ABY = 0x99;
static constexpr Byte INS_STA_IDX = 0x81;
static constexpr Byte INS_STA_IDY = 0x91;
static constexpr Byte INS_STX_ZP = 0x86;
static constexpr Byte INS_STX_ZPY = 0x96;
static constexpr Byte INS_STX_ABS = 0x8E;
static constexpr Byte INS_STY_ZP = 0x84;
static constexpr Byte INS_STY_ZPX = 0x94;
static constexpr Byte INS_STY_ABS = 0x8C;

/* Program Flow */
static constexpr Byte INS_JSR = 0x20;
static constexpr Byte INS_JMP_ABS = 0x4C;
static constexpr Byte INS_JMP_IND = 0x6C;

#define CPU_H
#endif // !CPU_H
