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

/* Program Flow */
static constexpr Byte INS_JSR = 0x20;
