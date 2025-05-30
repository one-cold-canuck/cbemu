#ifndef DEBUG

#endif // !DEBUG
#include <stdint.h>

typedef unsigned char Byte;
typedef unsigned short Word;

typedef uint32_t Uint32;
typedef int32_t Sint32;

typedef struct CPU CPU;
typedef struct Memory Memory;

/* Instructions */

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
/* Program Flow */
static constexpr Byte INS_JSR = 0x20;
static constexpr Byte INS_JMP_ABS = 0x4C;
static constexpr Byte INS_JMP_IND = 0x6C;
