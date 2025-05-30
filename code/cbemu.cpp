#include "cpu.cpp"

int
main (int argc, char *argv[])
{
  Memory mem;
  CPU cpu;
  Sint32 Cycles;

  cpu.Reset (mem);

  // Inline a little program
  cpu.A = 0x0;
  mem[0xFFFC] = INS_JMP_ABS;
  mem[0xFFFD] = 0x80;
  mem[0xFFFE] = 0x44;
  mem[0x4480] = INS_LDA_ABS;
  mem[0x4481] = 0x44;
  mem[0x4482] = 0x80;
  mem[0x8044] = 0x77;


  // when:
  Sint32 CyclesUsed = cpu.Execute (mem);
  CyclesUsed += cpu.Execute (mem);
  // End - inline program

  Cycles = cpu.Execute (mem);

  printf ("Registers \n\tA: %X \n\tX: %X \n\tY: %X\nCycles Used: %d\n", cpu.A,
          cpu.X, cpu.Y, Cycles);
  printf ("Flags:\n\tN\tV\tB\tD\tI\tZ\tC\n\t%x\t%x\t%x\t%x\t%x\t%x\t%x\n", cpu.N, cpu.V, cpu.B, cpu.D, cpu.I, cpu.Z, cpu.C);
  return 0;
}
