#include "cpu.cpp"

int main(int argc, char *argv[])
{
    Memory mem;
    CPU cpu;
    Sint32 Cycles;

    cpu.Reset(mem);
    
    // Inline a little program
    mem[0xFFFC] = INS_LDA_ABS;
    mem[0xFFFD] = 0x42;
    mem[0xFFFE] = 0x42;
    mem[0x4242] = 0x77;
    // End - inline program

    Cycles = cpu.Execute(mem, 4);

    printf("Registers \n\tA: %X \n\tX: %X \n\tY: %X\nCycles Used: %d\n", cpu.A, cpu.X, cpu.Y, Cycles);
    return 0;
}
