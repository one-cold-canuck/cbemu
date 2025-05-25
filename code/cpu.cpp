#include "cpu.h"
#include <stdio.h>

struct Memory
{
  static constexpr Uint32 MAX_MEM = 1024 * 64;
  Byte Data[MAX_MEM];

  /* Initializes memory to 0 */
  void
  Initialize ()
  {
    for (Uint32 i = 0; i < MAX_MEM; i++)
      {
        Data[i] = 0;
      }
  }

  /** Read one byte */
  Byte
  operator[] (Uint32 Address) const
  {
    // TODO: Assert that Address < MAX_MEM
    return Data[Address];
  }

  /** Write one byte */
  Byte &
  operator[] (Uint32 Address)
  {
    // TODO: Assert that Address < MAX_MEM
    return Data[Address];
  }

  /** Write two bytes (one Word) */
  void
  WriteWord (Word Value, Byte Address, Sint32 &Cycles)
  {
    Data[Address] = Value & 0xFF;
    Data[Address - 1] = (Value >> 8);
    Cycles -= 2;
  }
};

struct CPU
{
  Word PC; // Program Counter
  Byte SP; // Stack Pointer - Starts at 0x01FF, grows downard to 0x0100

  Byte P; // processor Status  = Bits are: NV-BDIZC

  Byte A; // Accumulator
  Byte Y; // Y Index Register
  Byte X; // X Index Register

  Byte N : 1; // Negative flag
  Byte V : 1; // Overflow flag
  Byte B : 1; // Break flag
  Byte D : 1; // Decimal mode flag
  Byte I : 1; // Interrupt disable flag
  Byte Z : 1; // Zero flag;
  Byte C : 1; // Carry flag;

  void
  Reset (Memory &memory)
  {
    /* TODO: Implement MOS 6510 System Reset routine
     * From documentation:
     * ; Reset vector (Kernel address $FFFC) points here.
     * ;
     * ; If cartridge is detected then cartridge cold start routine is
     * activated. ; If no cartridge is detected then I/O and memory are
     * initialised and BASIC cold start routine is ; activated
     *
     * FCE2  A2 FF     LDX #$FF        ;
     * FCE4  78        SEI             ; set interrupt disable
     * FCE5  9A        TXS             ; transfer .X to stack
     * FCE6  D8        CLD             ; clear decimal flag
     * FCE7  20 02 FD  JSR $FD02       ; check for cart
     * FCEA  D0 03     BNE $FCEF       ; .Z=0? then no cart detected
     * FCEC  6C 00 80  JMP ($8000)     ; direct to cartridge cold start via
     * vector FCEF  8E 16 D0  STX $D016       ; sets bit 5 (MCM) off, bit 3 (38
     * cols) off FCF2  20 A3 FD  JSR $FDA3       ; initialise I/O FCF5  20 50
     * FD  JSR $FD50       ; initialise memory FCF8  20 15 FD  JSR $FD15 ; set
     * I/O vectors ($0314..$0333) to kernel defaults FCFB  20 5B FF  JSR $FF5B
     * ; more initialising... mostly set system IRQ to correct value and start
     * FCFE  58        CLI             ; clear interrupt flag FCFF  6C 00 A0
     * JMP ($A000)     ; direct to BASIC cold start via vector
     */

    PC = 0xFFFC;
    SP = 0xFF;

    A = X = Y = 0;
    N = V = B = D = I = Z = C = 0;
    memory.Initialize ();
  }

  Byte
  FetchByte (Memory &memory, Sint32 &Cycles)
  {
    Byte Data = memory[PC];
    PC++;
    Cycles--;
    return (Data);
  }

  Word
  FetchWord (Memory &memory, Sint32 &Cycles)
  {

    // 6502 is little endian
    Word Data = memory[PC];
    PC++;

    Cycles--;
    Data |= (memory[PC] << 8);
    PC++;

    Cycles--;

    // TODO: Handle byte swap for Big Endian
    return Data;
  }

  Byte
  ReadByte (Memory &memory, Byte Address, Sint32 &Cycles)
  {
    Byte Data = memory[Address];
    Cycles--;
    return (Data);
  }

 Byte ReadByte (Memory &memory, Word Address, Sint32 &Cycles){
    Byte Data = memory[Address];
    Cycles--;
    return (Data);
  }

  void
  SetStatusFlagLDA ()
  {
    Z = (A == 0);
    N = (A & 0b10000000) > 0;
  }

  Sint32
  Execute (Memory &memory, Sint32 Cycles)
  {
    const Sint32 CyclesRequested = Cycles;
    while (Cycles > 0)
      {

        Byte instruction = FetchByte (memory, Cycles);

        switch (instruction)
          {
          /****************************************
           * LDA Instructions
           ****************************************/
          case INS_LDA_IM:
            {
              Byte Value = FetchByte (memory, Cycles);
              A = Value;
              SetStatusFlagLDA ();
            }
            break;
          case INS_LDA_ZP:
            {
              Byte ZeroPageAddress = FetchByte (memory, Cycles);
              A = ReadByte (memory, ZeroPageAddress, Cycles);
              SetStatusFlagLDA ();
            }
            break;
          case INS_LDA_ZPX:
            {
              Byte ZeroPageAddress = FetchByte (memory, Cycles);
              ZeroPageAddress = (ZeroPageAddress + X) & 0x00FF;
              Cycles--;
              A = ReadByte (memory, ZeroPageAddress, Cycles);
              SetStatusFlagLDA ();
            }
            break;
          case INS_LDA_ABS:
            {
              Word AbsAddress = FetchWord (memory, Cycles);
              A = ReadByte (memory, AbsAddress, Cycles);
            }
            break;
          case INS_JSR:
            {
              Word JumpAddress = FetchWord (memory, Cycles);
              memory.WriteWord (PC - 1, SP, Cycles);
              SP -= 1;
              PC = JumpAddress;
              Cycles--;
            }
            break;
          default:
            {
              printf ("Operation not handled %d\n", instruction);
            };
          }
      }
    return (CyclesRequested - Cycles);
  }
};
