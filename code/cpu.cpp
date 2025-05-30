#include "cpu.h"
#include <stdint.h>
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
    Data[0x00] = 0xFF;
    Data[0x01] = 0x07;
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
    Cycles += 2;
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
     * initialised and BASIC ; cold start routine is activated
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
     * JMP ($A000)                     ; direct to BASIC cold start via vector
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
    Cycles++;
    return (Data);
  }

  Word
  FetchWord (Memory &memory, Sint32 &Cycles)
  {

    // 6502 is little endian
    Word Data = memory[PC];
    PC++;

    Cycles++;
    Data |= (memory[PC] << 8);
    PC++;

    Cycles++;

    // TODO: Handle byte swap for Big Endian
    return Data;
  }

  Byte
  ReadByte (Memory &memory, Byte Address, Sint32 &Cycles)
  {
    Byte Data = memory[Address];
    Cycles++;
    return (Data);
  }

  Byte
  ReadByte (Memory &memory, Word Address, Sint32 &Cycles)
  {
    Byte Data = memory[Address];
    Cycles++;
    return (Data);
  }

  Word
  ReadWord (Memory &memory, Byte Address, Sint32 &Cycles)
  {
    Word Data = memory[Address];
    Cycles++;
    Data = memory[Address + 1] << 8;
    Cycles++;
    return (Data);
  }

  void
  SetStatusFlag (Byte &Value)
  {
    Z = (Value == 0);
    N = (Value & 0b10000000) > 0;
  }

  Word
  GetWordAddress (Byte LoByte, Byte HiByte)
  {
    // Could switch here for Big Endian
    Word Address = LoByte + (HiByte << 8);
    return Address;
  }

  Sint32
  Execute (Memory &memory)
  {
    Sint32 Cycles = 0;

    Byte instruction = FetchByte (memory, Cycles); // One cycle
    switch (instruction)
      {
      /****************************************
       * Immediate Addressing
       ***************************************

        #  address R/W description
       --- ------- --- ------------------------------------------
        1    PC     R  fetch opcode, increment PC
        2    PC     R  fetch value, increment PC
       */
      case INS_LDA_IM:
        {
          Byte Value = FetchByte (memory, Cycles);
          A = Value;
          SetStatusFlag (A);
        }
        break;
      case INS_LDX_IM:
        {
          Byte Value = FetchByte (memory, Cycles);
          X = Value;
          SetStatusFlag (X);
        }
        break;
      case INS_LDY_IM:
        {
          Byte Value = FetchByte (memory, Cycles);
          Y = Value;
          SetStatusFlag (Y);
        }
        break;

      /****************************************
       * Absolute Addressing
       ****************************************
      JMP

        #  address R/W description
       --- ------- --- -------------------------------------------------
        1    PC     R  fetch opcode, increment PC
        2    PC     R  fetch low address byte, increment PC
        3    PC     R  copy low address byte to PCL, fetch high address
                       byte to PCH

      Read instructions (LDA, LDX, LDY, EOR, AND, ORA, ADC, SBC, CMP, BIT,
                        LAX, NOP)

        #  address R/W description
       --- ------- --- ------------------------------------------
        1    PC     R  fetch opcode, increment PC
        2    PC     R  fetch low byte of address, increment PC
        3    PC     R  fetch high byte of address, increment PC
        4  address  R  read from effective address

      Read-Modify-Write instructions (ASL, LSR, ROL, ROR, INC, DEC,
                                     SLO, SRE, RLA, RRA, ISB, DCP)

        #  address R/W description
       --- ------- --- ------------------------------------------
        1    PC     R  fetch opcode, increment PC
        2    PC     R  fetch low byte of address, increment PC
        3    PC     R  fetch high byte of address, increment PC
        4  address  R  read from effective address
        5  address  W  write the value back to effective address,
                       and do the operation on it
        6  address  W  write the new value to effective address

      Write instructions (STA, STX, STY, SAX)

        #  address R/W description
       --- ------- --- ------------------------------------------
        1    PC     R  fetch opcode, increment PC
        2    PC     R  fetch low byte of address, increment PC
        3    PC     R  fetch high byte of address, increment PC
        4  address  W  write register to effective address
       */
      case INS_LDA_ABS:
        {
          Byte LoByte = FetchByte (memory, Cycles);
          Byte HiByte = FetchByte (memory, Cycles);

          Word Address = GetWordAddress (LoByte, HiByte);

          A = ReadByte (memory, Address, Cycles);

          SetStatusFlag (A);
        }
        break;
      case INS_LDX_ABS:
        {
          Byte LoByte = FetchByte (memory, Cycles);
          Byte HiByte = FetchByte (memory, Cycles);

          Word Address = GetWordAddress (LoByte, HiByte);
          X = ReadByte (memory, Address, Cycles);
          SetStatusFlag (X);
        }
        break;
      case INS_LDY_ABS:
        {
          Byte LoByte = FetchByte (memory, Cycles);
          Byte HiByte = FetchByte (memory, Cycles);

          Word Address = GetWordAddress (LoByte, HiByte);
          Y = ReadByte (memory, Address, Cycles);
          SetStatusFlag (Y);
        }
        break;
/*
        #  address R/W description
       --- ------- --- -------------------------------------------------
        1    PC     R  fetch opcode, increment PC
        2    PC     R  fetch low address byte, increment PC
        3    PC     R  copy low address byte to PCL, fetch high address
                       byte to PCH
*/
      case INS_JMP_ABS:
        {
          Byte LoByte = FetchByte (memory, Cycles);
          Byte HiByte = FetchByte (memory, Cycles);

          PC = GetWordAddress(LoByte, HiByte);
          printf("PC = %x\n", PC);
          Cycles ++;
          
        }
        break;
      /****************************************
       * Absolute Indexed Addressing
       ***************************************
       Read instructions (LDA, LDX, LDY, EOR, AND, ORA, ADC, SBC, CMP, BIT,
                        LAX, LAE, SHS, NOP)

        #   address  R/W description
       --- --------- --- ------------------------------------------
        1     PC      R  fetch opcode, increment PC
        2     PC      R  fetch low byte of address, increment PC
        3     PC      R  fetch high byte of address,
                         add index register to low address byte,
                         increment PC
        4  address+I* R  read from effective address,
                         fix the high byte of effective address
        5+ address+I  R  re-read from effective address

        Notes: I denotes either index register (X or Y).

              * The high byte of the effective address may be invalid
                at this time, i.e. it may be smaller by $100.

              + This cycle will be executed only if the effective address
                was invalid during cycle #4, i.e. page boundary was crossed.

      Read-Modify-Write instructions (ASL, LSR, ROL, ROR, INC, DEC,
                                     SLO, SRE, RLA, RRA, ISB, DCP)

        #   address  R/W description
       --- --------- --- ------------------------------------------
        1    PC       R  fetch opcode, increment PC
        2    PC       R  fetch low byte of address, increment PC
        3    PC       R  fetch high byte of address,
                         add index register X to low address byte,
                         increment PC
        4  address+X* R  read from effective address,
                         fix the high byte of effective address
        5  address+X  R  re-read from effective address
        6  address+X  W  write the value back to effective address,
                         and do the operation on it
        7  address+X  W  write the new value to effective address

       Notes: * The high byte of the effective address may be invalid
                at this time, i.e. it may be smaller by $100.

      Write instructions (STA, STX, STY, SHA, SHX, SHY)

        #   address  R/W description
       --- --------- --- ------------------------------------------
        1     PC      R  fetch opcode, increment PC
        2     PC      R  fetch low byte of address, increment PC
        3     PC      R  fetch high byte of address,
                         add index register to low address byte,
                         increment PC
        4  address+I* R  read from effective address,
                         fix the high byte of effective address
        5  address+I  W  write to effective address

        Notes: I denotes either index register (X or Y).

              * The high byte of the effective address may be invalid
                at this time, i.e. it may be smaller by $100. Because
                the processor cannot undo a write to an invalid
                address, it always reads from the address first.
       */
      case INS_LDA_ABX:
        {
          Byte LoByte = FetchByte (memory, Cycles);
          Byte HiByte = FetchByte (memory, Cycles);

          Word Address = GetWordAddress (LoByte, HiByte);
          Byte AddrHiByte = Address >> 8;

          Address += X;
          Byte AddrAfterHiByte = Address >> 8;

          A = ReadByte (memory, Address, Cycles);
          SetStatusFlag (A);
          if (AddrHiByte != AddrAfterHiByte)
            {
              Cycles++;
            }
        }
        break;
      case INS_LDA_ABY:
        {

          Byte LoByte = FetchByte (memory, Cycles);
          Byte HiByte = FetchByte (memory, Cycles);

          Word Address = GetWordAddress (LoByte, HiByte);
          Byte AddrHiByte = Address >> 8;

          Address += Y;
          Byte AddrAfterHiByte = Address >> 8;

          A = ReadByte (memory, Address, Cycles);
          SetStatusFlag (A);
          if (AddrHiByte != AddrAfterHiByte)
            {
              Cycles++;
            }
        }
        break;
      case INS_LDX_ABY:
        {

          Byte LoByte = FetchByte (memory, Cycles);
          Byte HiByte = FetchByte (memory, Cycles);

          Word Address = GetWordAddress (LoByte, HiByte);
          Byte AddrHiByte = Address >> 8;

          Address += Y;
          Byte AddrAfterHiByte = Address >> 8;

          X = ReadByte (memory, Address, Cycles);
          SetStatusFlag (X);
          if (AddrHiByte != AddrAfterHiByte)
            {
              Cycles++;
            }
        }
        break;
      case INS_LDY_ABX:
        {
          Byte LoByte = FetchByte (memory, Cycles);
          Byte HiByte = FetchByte (memory, Cycles);

          Word Address = GetWordAddress (LoByte, HiByte);
          Byte AddrHiByte = Address >> 8;

          Address += X;
          Byte AddrAfterHiByte = Address >> 8;

          Y = ReadByte (memory, Address, Cycles);
          SetStatusFlag (Y);
          if (AddrHiByte != AddrAfterHiByte)
            {
              Cycles++;
            }
        }
        break;

      /****************************************
       * Zero Page Addressing
       ***************************************

       Read instructions (LDA, LDX, LDY, EOR, AND, ORA, ADC, SBC, CMP, BIT,
                        LAX, NOP)

        #  address R/W description
       --- ------- --- ------------------------------------------
        1    PC     R  fetch opcode, increment PC
        2    PC     R  fetch address, increment PC
        3  address  R  read from effective address

      Read-Modify-Write instructions (ASL, LSR, ROL, ROR, INC, DEC,
                                     SLO, SRE, RLA, RRA, ISB, DCP)

        #  address R/W description
       --- ------- --- ------------------------------------------
        1    PC     R  fetch opcode, increment PC
        2    PC     R  fetch address, increment PC
        3  address  R  read from effective address
        4  address  W  write the value back to effective address,
                       and do the operation on it
        5  address  W  write the new value to effective address

      Write instructions (STA, STX, STY, SAX)

        #  address R/W description
       --- ------- --- ------------------------------------------
        1    PC     R  fetch opcode, increment PC
        2    PC     R  fetch address, increment PC
        3  address  W  write register to effective address
       */
      case INS_LDA_ZP:
        {
          Byte Address = FetchByte (memory, Cycles);
          A = ReadByte (memory, Address, Cycles);
          SetStatusFlag (A);
        }
        break;
      case INS_LDX_ZP:
        {
          Byte Address = FetchByte (memory, Cycles);
          X = ReadByte (memory, Address, Cycles);
          SetStatusFlag (X);
        }
        break;
      case INS_LDY_ZP:
        {
          Byte Address = FetchByte (memory, Cycles);
          Y = ReadByte (memory, Address, Cycles);
          SetStatusFlag (Y);
        }
        break;

        /****************************************
         * Zero Page Indexed Addressing
         ***************************************
         Read instructions (LDA, LDX, LDY, EOR, AND, ORA, ADC, SBC, CMP, BIT,
                          LAX, NOP)

          #   address  R/W description
         --- --------- --- ------------------------------------------
          1     PC      R  fetch opcode, increment PC
          2     PC      R  fetch address, increment PC
          3   address   R  read from address, add index register to it
          4  address+I* R  read from effective address

          Notes: I denotes either index register (X or Y).

                * The high byte of the effective address is always zero,
                  i.e. page boundary crossings are not handled.

        Read-Modify-Write instructions (ASL, LSR, ROL, ROR, INC, DEC,
                                       SLO, SRE, RLA, RRA, ISB, DCP)

          #   address  R/W description
         --- --------- --- ---------------------------------------------
          1     PC      R  fetch opcode, increment PC
          2     PC      R  fetch address, increment PC
          3   address   R  read from address, add index register X to it
          4  address+X* R  read from effective address
          5  address+X* W  write the value back to effective address,
                           and do the operation on it
          6  address+X* W  write the new value to effective address

          Note: * The high byte of the effective address is always zero,
                 i.e. page boundary crossings are not handled.

        Write instructions (STA, STX, STY, SAX)

          #   address  R/W description
         --- --------- --- -------------------------------------------
          1     PC      R  fetch opcode, increment PC
          2     PC      R  fetch address, increment PC
          3   address   R  read from address, add index register to it
          4  address+I* W  write to effective address

          Notes: I denotes either index register (X or Y).

                * The high byte of the effective address is always zero,
                  i.e. page boundary crossings are not handled.
        */

      case INS_LDA_ZPX:
        {
          Byte Address = FetchByte (memory, Cycles);
          Address = (Address + X) & 0x00FF;
          Cycles++;
          A = ReadByte (memory, Address, Cycles);
          SetStatusFlag (A);
        }
        break;
      case INS_LDX_ZPY:
        {
          Byte Address = FetchByte (memory, Cycles);
          Address += Y;
          Cycles++;
          X = ReadByte (memory, Address, Cycles);
          SetStatusFlag (X);
        }
        break;
      case INS_LDY_ZPX:
        {
          Byte Address = FetchByte (memory, Cycles);
          Address += X;
          Cycles++;
          Y = ReadByte (memory, Address, Cycles);
          SetStatusFlag (Y);
        }
        break;

        /*************************************************************
         *Relative addressing (BCC, BCS, BNE, BEQ, BPL, BMI, BVC, BVS)
         *************************************************************

          #   address  R/W description
         --- --------- --- ---------------------------------------------
          1     PC      R  fetch opcode, increment PC
          2     PC      R  fetch operand, increment PC
          3     PC      R  Fetch opcode of next instruction,
                           If branch is taken, add operand to PCL.
                           Otherwise increment PC.
          4+    PC*     R  Fetch opcode of next instruction.
                           Fix PCH. If it did not change, increment PC.
          5!    PC      R  Fetch opcode of next instruction,
                           increment PC.

          Notes: The opcode fetch of the next instruction is included to
                this diagram for illustration purposes. When determining
                real execution times, remember to subtract the last
                cycle.

                * The high byte of Program Counter (PCH) may be invalid
                  at this time, i.e. it may be smaller or bigger by $100.

                + If branch is taken, this cycle will be executed.

                ! If branch occurs to different page, this cycle will be
                  executed. */

        /****************************************
         * Indexed Indirect Addressing
         ***************************************
        Read instructions (LDA, ORA, EOR, AND, ADC, CMP, SBC, LAX)

          #    address   R/W description
         --- ----------- --- ------------------------------------------
          1      PC       R  fetch opcode, increment PC
          2      PC       R  fetch pointer address, increment PC
          3    pointer    R  read from the address, add X to it
          4   pointer+X   R  fetch effective address low
          5  pointer+X+1  R  fetch effective address high
          6    address    R  read from effective address

         Note: The effective address is always fetched from zero page,
               i.e. the zero page boundary crossing is not handled.

        Read-Modify-Write instructions (SLO, SRE, RLA, RRA, ISB, DCP)

          #    address   R/W description
         --- ----------- --- ------------------------------------------
          1      PC       R  fetch opcode, increment PC
          2      PC       R  fetch pointer address, increment PC
          3    pointer    R  read from the address, add X to it
          4   pointer+X   R  fetch effective address low
          5  pointer+X+1  R  fetch effective address high
          6    address    R  read from effective address
          7    address    W  write the value back to effective address,
                             and do the operation on it
          8    address    W  write the new value to effective address

          Note: The effective address is always fetched from zero page,
               i.e. the zero page boundary crossing is not handled.

        Write instructions (STA, SAX)

          #    address   R/W description
         --- ----------- --- ------------------------------------------
          1      PC       R  fetch opcode, increment PC
          2      PC       R  fetch pointer address, increment PC
          3    pointer    R  read from the address, add X to it
          4   pointer+X   R  fetch effective address low
          5  pointer+X+1  R  fetch effective address high
          6    address    W  write to effective address

          Note: The effective address is always fetched from zero page,
               i.e. the zero page boundary crossing is not handled. */

      case INS_LDA_IDX:
        {
          Byte Address = FetchByte (memory, Cycles);
          Address += X;

          Byte LoByte = ReadByte (memory, Address, Cycles);
          Address += 1;
          Byte HiByte = ReadByte (memory, Address, Cycles);

          Word TargetAddress = GetWordAddress (LoByte, HiByte);
          Cycles += 1;
          A = ReadByte (memory, TargetAddress, Cycles);
          SetStatusFlag (A);
        }
        break;

        /****************************************
         * Indirect Indexed Addressing
         ***************************************

        Read instructions (LDA, EOR, AND, ORA, ADC, SBC, CMP)

          #    address   R/W description
         --- ----------- --- ------------------------------------------
          1      PC       R  fetch opcode, increment PC
          2      PC       R  fetch pointer address, increment PC
          3    pointer    R  fetch effective address low
          4   pointer+1   R  fetch effective address high,
                             add Y to low byte of effective address
          5   address+Y*  R  read from effective address,
                             fix high byte of effective address
          6+  address+Y   R  read from effective address

          Notes: The effective address is always fetched from zero page,
                i.e. the zero page boundary crossing is not handled.

                * The high byte of the effective address may be invalid
                  at this time, i.e. it may be smaller by $100.

                + This cycle will be executed only if the effective address
                  was invalid during cycle #5, i.e. page boundary was crossed.

        Read-Modify-Write instructions (SLO, SRE, RLA, RRA, ISB, DCP)

          #    address   R/W description
         --- ----------- --- ------------------------------------------
          1      PC       R  fetch opcode, increment PC
          2      PC       R  fetch pointer address, increment PC
          3    pointer    R  fetch effective address low
          4   pointer+1   R  fetch effective address high,
                             add Y to low byte of effective address
          5   address+Y*  R  read from effective address,
                             fix high byte of effective address
          6   address+Y   R  read from effective address
          7   address+Y   W  write the value back to effective address,
                             and do the operation on it
          8   address+Y   W  write the new value to effective address

          Notes: The effective address is always fetched from zero page,
                i.e. the zero page boundary crossing is not handled.

                * The high byte of the effective address may be invalid
                  at this time, i.e. it may be smaller by $100.

         Write instructions (STA, SHA)

          #    address   R/W description
         --- ----------- --- ------------------------------------------
          1      PC       R  fetch opcode, increment PC
          2      PC       R  fetch pointer address, increment PC
          3    pointer    R  fetch effective address low
          4   pointer+1   R  fetch effective address high,
                             add Y to low byte of effective address
          5   address+Y*  R  read from effective address,
                             fix high byte of effective address
          6   address+Y   W  write to effective address

          Notes: The effective address is always fetched from zero page,
                i.e. the zero page boundary crossing is not handled.

                * The high byte of the effective address may be invalid
                  at this time, i.e. it may be smaller by $100.
         */

      case INS_LDA_IDY:
        {
          Byte Address = FetchByte (memory, Cycles);

          Byte LoByte = ReadByte (memory, Address, Cycles);
          Address += 1;
          Byte HiByte = ReadByte (memory, Address, Cycles);

          Word TargetAddress = GetWordAddress (LoByte, HiByte);
          TargetAddress += Y;

          Byte AddrAfterHiByte = TargetAddress >> 8;

          A = ReadByte (memory, TargetAddress, Cycles);
          SetStatusFlag (A);
          if (AddrAfterHiByte != HiByte)
            {
              Cycles++;
            }
        }
        break;

      /**************************************************
       * Program flow / Stack Instructions
       * ***********************************************/
      case INS_JSR:
        {
          Byte LoByte = FetchByte (memory, Cycles);
          Byte HiByte = FetchByte (memory, Cycles);

          Word Address = GetWordAddress (LoByte, HiByte);
          memory.WriteWord (PC - 1, SP, Cycles);
          SP -= 1;
          PC = Address;
          Cycles++;
        }
        break;

      /****************************************
       * Absolute Indirect Addressing (JMP)
       ****************************************

        #   address  R/W description
       --- --------- --- ------------------------------------------
        1     PC      R  fetch opcode, increment PC
        2     PC      R  fetch pointer address low, increment PC
        3     PC      R  fetch pointer address high, increment PC
        4   pointer   R  fetch low address to latch
        5  pointer+1* R  fetch PCH, copy latch to PCL

         Note: * The PCH will always be fetched from the same page
               than PCL, i.e. page boundary crossing is not handled.
      */
      case INS_JMP_IND:
        {
        }
        break;
      default:
        {
          printf ("Operation not handled %d\n", instruction);
        };
      }

    return Cycles;
  }
};
