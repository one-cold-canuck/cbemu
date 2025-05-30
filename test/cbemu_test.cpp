#include "../code/cpu.cpp"
#include <gtest/gtest.h>

class cbemuTest : public testing::Test
{
public:
  Memory mem;
  CPU cpu;

  virtual void
  SetUp ()
  {
    cpu.Reset (mem);
  }
  virtual void
  TearDown ()
  {
  }
};

static void
VerifyUnmodifiedFlags (CPU cpu, CPU cpuCopy)
{
  EXPECT_EQ (cpu.V, cpuCopy.V);
  EXPECT_EQ (cpu.D, cpuCopy.D);
  EXPECT_EQ (cpu.I, cpuCopy.I);
  EXPECT_EQ (cpu.C, cpuCopy.C);
  EXPECT_EQ (cpu.B, cpuCopy.B);
}

TEST_F (cbemuTest, LDAImmediate)
{

  // given:
  mem[0xFFFC] = INS_LDA_IM;
  mem[0xFFFD] = 0x77;
  CPU cpuCopy = cpu;
  constexpr Sint32 EXPECTED_CYCLES = 2;

  // when:
  Sint32 CyclesUsed = cpu.Execute (mem);

  EXPECT_EQ (cpu.A, 0x77);
  EXPECT_EQ (CyclesUsed, EXPECTED_CYCLES);
  EXPECT_FALSE (cpu.Z);
  EXPECT_FALSE (cpu.N);
  VerifyUnmodifiedFlags (cpu, cpuCopy);
}

TEST_F (cbemuTest, LDAZeroPage)
{

  // given:
  mem[0xFFFC] = INS_LDA_ZP;
  mem[0xFFFD] = 0x42;
  mem[0x0042] = 0x37;
  CPU cpuCopy = cpu;
  constexpr Sint32 EXPECTED_CYCLES = 3;

  // when:
  Sint32 CyclesUsed = cpu.Execute (mem);

  // then:
  EXPECT_EQ (cpu.A, 0x37);
  EXPECT_EQ (CyclesUsed, EXPECTED_CYCLES);
  EXPECT_FALSE (cpu.Z);
  EXPECT_FALSE (cpu.N);
  VerifyUnmodifiedFlags (cpu, cpuCopy);
}

TEST_F (cbemuTest, LDAZeroPageX)
{

  // given:
  cpu.X = 0x02;
  mem[0xFFFC] = INS_LDA_ZPX;
  mem[0xFFFD] = 0x04;
  mem[0x0006] = 0x37;
  CPU cpuCopy = cpu;
  constexpr Sint32 EXPECTED_CYCLES = 4;

  // when:
  Sint32 CyclesUsed = cpu.Execute (mem);

  EXPECT_EQ (cpu.A, 0x37);
  EXPECT_EQ (CyclesUsed, EXPECTED_CYCLES);
  EXPECT_FALSE (cpu.Z);
  EXPECT_FALSE (cpu.N);
  VerifyUnmodifiedFlags (cpu, cpuCopy);
}

TEST_F (cbemuTest, LDAZeroPageXBoundaryCheck)
{

  // given:
  cpu.X = 0xFF;
  mem[0xFFFC] = INS_LDA_ZPX;
  mem[0xFFFD] = 0x80;
  mem[0x007F] = 0x37;
  CPU cpuCopy = cpu;
  constexpr Sint32 EXPECTED_CYCLES = 4;

  // when:
  Sint32 CyclesUsed = cpu.Execute (mem);

  EXPECT_EQ (cpu.A, 0x37);
  EXPECT_EQ (CyclesUsed, EXPECTED_CYCLES);
  EXPECT_FALSE (cpu.Z);
  EXPECT_FALSE (cpu.N);
  VerifyUnmodifiedFlags (cpu, cpuCopy);
}

TEST_F (cbemuTest, LDAImmediateZeroValue)
{

  // given:
  cpu.A = 0x44;
  mem[0xFFFC] = INS_LDA_IM;
  mem[0xFFFD] = 0x0;
  CPU cpuCopy = cpu;
  constexpr Sint32 EXPECTED_CYCLES = 2;

  // when:
  Sint32 CyclesUsed = cpu.Execute (mem);

  // then:
  EXPECT_EQ (cpu.A, 0x0);
  EXPECT_EQ (CyclesUsed, EXPECTED_CYCLES);
  EXPECT_TRUE (cpu.Z);
  EXPECT_FALSE (cpu.N);
  VerifyUnmodifiedFlags (cpu, cpuCopy);
}

TEST_F (cbemuTest, LDAAbsolute)
{

  // given:
  mem[0xFFFC] = INS_LDA_ABS;
  mem[0xFFFD] = 0x80;
  mem[0xFFFE] = 0x44; // 0x4480
  mem[0x4480] = 0x77;
  constexpr Sint32 EXPECTED_CYCLES = 4;
  CPU cpuCopy = cpu;
  //
  // when:
  Sint32 CyclesUsed = cpu.Execute (mem);

  EXPECT_EQ (cpu.A, 0x77);
  EXPECT_EQ (CyclesUsed, EXPECTED_CYCLES);
  EXPECT_FALSE (cpu.Z);
  EXPECT_FALSE (cpu.N);
  VerifyUnmodifiedFlags (cpu, cpuCopy);
}
TEST_F (cbemuTest, LDAAbsoluteX)
{

  // given:
  cpu.X = 1;
  mem[0xFFFC] = INS_LDA_ABX;
  mem[0xFFFD] = 0x80;
  mem[0xFFFE] = 0x44; // 0x4480
  mem[0x4481] = 0x77;
  constexpr Sint32 EXPECTED_CYCLES = 4;
  CPU cpuCopy = cpu;

  // when:
  Sint32 CyclesUsed = cpu.Execute (mem);

  // then:
  EXPECT_EQ (cpu.A, 0x77);
  EXPECT_EQ (CyclesUsed, EXPECTED_CYCLES);
  EXPECT_FALSE (cpu.Z);
  EXPECT_FALSE (cpu.N);
  VerifyUnmodifiedFlags (cpu, cpuCopy);
}

TEST_F (cbemuTest, LDAAbsoluteXBoundary)
{

  // given:
  cpu.X = 0xFF;
  mem[0xFFFC] = INS_LDA_ABX;
  mem[0xFFFD] = 0x80;
  mem[0xFFFE] = 0x44; // 0x4480
  mem[0x457F] = 0x77;
  constexpr Sint32 EXPECTED_CYCLES = 5;
  CPU cpuCopy = cpu;

  // when:
  Sint32 CyclesUsed = cpu.Execute (mem);

  // then:
  EXPECT_EQ (cpu.A, 0x77);
  EXPECT_EQ (CyclesUsed, EXPECTED_CYCLES);
  EXPECT_FALSE (cpu.Z);
  EXPECT_FALSE (cpu.N);
  VerifyUnmodifiedFlags (cpu, cpuCopy);
}

TEST_F (cbemuTest, LDAAbsoluteY)
{

  // given:
  cpu.Y = 1;
  mem[0xFFFC] = INS_LDA_ABY;
  mem[0xFFFD] = 0x80;
  mem[0xFFFE] = 0x44; // 0x4480
  mem[0x4481] = 0x77;
  constexpr Sint32 EXPECTED_CYCLES = 4;
  CPU cpuCopy = cpu;

  // when:
  Sint32 CyclesUsed = cpu.Execute (mem);

  // then:
  EXPECT_EQ (cpu.A, 0x77);
  EXPECT_EQ (CyclesUsed, EXPECTED_CYCLES);
  EXPECT_FALSE (cpu.Z);
  EXPECT_FALSE (cpu.N);
  VerifyUnmodifiedFlags (cpu, cpuCopy);
}

TEST_F (cbemuTest, LDAAbsoluteYBoundary)
{

  // given:
  cpu.Y = 0xFF;
  mem[0xFFFC] = INS_LDA_ABY;
  mem[0xFFFD] = 0x80;
  mem[0xFFFE] = 0x44; // 0x4480
  mem[0x457F] = 0x77;
  constexpr Sint32 EXPECTED_CYCLES = 5;
  CPU cpuCopy = cpu;

  // when:
  Sint32 CyclesUsed = cpu.Execute (mem);

  // then:
  EXPECT_EQ (cpu.A, 0x77);
  EXPECT_EQ (CyclesUsed, EXPECTED_CYCLES);
  EXPECT_FALSE (cpu.Z);
  EXPECT_FALSE (cpu.N);
  VerifyUnmodifiedFlags (cpu, cpuCopy);
}

TEST_F (cbemuTest, LDAIndirectX)
{

  // given:
  cpu.X = 4;
  mem[0xFFFC] = INS_LDA_IDX;
  mem[0xFFFD] = 0x02;
  mem[0x0006] = 0x00;
  mem[0x0007] = 0x80;
  mem[0x8000] = 0x77;
  constexpr Sint32 EXPECTED_CYCLES = 6;
  CPU cpuCopy = cpu;

  // when:
  Sint32 CyclesUsed = cpu.Execute (mem);

  // then:
  EXPECT_EQ (cpu.A, 0x77);
  EXPECT_EQ (CyclesUsed, EXPECTED_CYCLES);
  EXPECT_FALSE (cpu.Z);
  EXPECT_FALSE (cpu.N);
  VerifyUnmodifiedFlags (cpu, cpuCopy);
}

TEST_F (cbemuTest, LDAIndirectY)
{

  // given:
  cpu.Y = 0x04;
  mem[0xFFFC] = INS_LDA_IDY;
  mem[0xFFFD] = 0x02;
  mem[0x0002] = 0x00; // 0x4480
  mem[0x0003] = 0x80;
  mem[0x8004] = 0x77;
  constexpr Sint32 EXPECTED_CYCLES = 5;
  CPU cpuCopy = cpu;

  // when:
  Sint32 CyclesUsed = cpu.Execute (mem);

  // then:
  EXPECT_EQ (cpu.A, 0x77);
  EXPECT_EQ (CyclesUsed, EXPECTED_CYCLES);
  EXPECT_FALSE (cpu.Z);
  EXPECT_FALSE (cpu.N);
  VerifyUnmodifiedFlags (cpu, cpuCopy);
}

TEST_F (cbemuTest, LDAAIndirectYBoundary)
{

  // given:
  cpu.Y = 0xFF;
  mem[0xFFFC] = INS_LDA_IDY;
  mem[0xFFFD] = 0x02;
  mem[0x0002] = 0x80;
  mem[0x0003] = 0x44;
  mem[0x457F] = 0x77;
  constexpr Sint32 EXPECTED_CYCLES = 6;
  CPU cpuCopy = cpu;

  // when:
  Sint32 CyclesUsed = cpu.Execute (mem);

  // then:
  EXPECT_EQ (cpu.A, 0x77);
  EXPECT_EQ (CyclesUsed, EXPECTED_CYCLES);
  EXPECT_FALSE (cpu.Z);
  EXPECT_FALSE (cpu.N);
  VerifyUnmodifiedFlags (cpu, cpuCopy);
}

TEST_F (cbemuTest, LDXImmediate)
{

  // given:
  mem[0xFFFC] = INS_LDX_IM;
  mem[0xFFFD] = 0x77;
  CPU cpuCopy = cpu;
  constexpr Sint32 EXPECTED_CYCLES = 2;

  // when:
  Sint32 CyclesUsed = cpu.Execute (mem);

  EXPECT_EQ (cpu.X, 0x77);
  EXPECT_EQ (CyclesUsed, EXPECTED_CYCLES);
  EXPECT_FALSE (cpu.Z);
  EXPECT_FALSE (cpu.N);
  VerifyUnmodifiedFlags (cpu, cpuCopy);
}

TEST_F (cbemuTest, LDXZeroPage)
{

  // given:
  mem[0xFFFC] = INS_LDX_ZP;
  mem[0xFFFD] = 0x42;
  mem[0x0042] = 0x37;
  CPU cpuCopy = cpu;
  constexpr Sint32 EXPECTED_CYCLES = 3;

  // when:
  Sint32 CyclesUsed = cpu.Execute (mem);

  // then:
  EXPECT_EQ (cpu.X, 0x37);
  EXPECT_EQ (CyclesUsed, EXPECTED_CYCLES);
  EXPECT_FALSE (cpu.Z);
  EXPECT_FALSE (cpu.N);
  VerifyUnmodifiedFlags (cpu, cpuCopy);
}

TEST_F (cbemuTest, LDXZeroPageY)
{

  // given:
  cpu.Y = 0x04;
  mem[0xFFFC] = INS_LDX_ZPY;
  mem[0xFFFD] = 0x06;
  mem[0x000A] = 0x37;
  CPU cpuCopy = cpu;
  constexpr Sint32 EXPECTED_CYCLES = 4;

  // when:
  Sint32 CyclesUsed = cpu.Execute (mem);

  EXPECT_EQ (cpu.X, 0x37);
  EXPECT_EQ (CyclesUsed, EXPECTED_CYCLES);
  EXPECT_FALSE (cpu.Z);
  EXPECT_FALSE (cpu.N);
  VerifyUnmodifiedFlags (cpu, cpuCopy);
}

TEST_F (cbemuTest, LDXZeroPageYBoundaryCheck)
{

  // given:
  cpu.Y = 0xFF;
  mem[0xFFFC] = INS_LDX_ZPY;
  mem[0xFFFD] = 0x80;
  mem[0x007F] = 0x37;
  CPU cpuCopy = cpu;
  constexpr Sint32 EXPECTED_CYCLES = 4;

  // when:
  Sint32 CyclesUsed = cpu.Execute (mem);

  EXPECT_EQ (cpu.X, 0x37);
  EXPECT_EQ (CyclesUsed, EXPECTED_CYCLES);
  EXPECT_FALSE (cpu.Z);
  EXPECT_FALSE (cpu.N);
  VerifyUnmodifiedFlags (cpu, cpuCopy);
}

TEST_F (cbemuTest, LDXAbsolute)
{

  // given:
  mem[0xFFFC] = INS_LDX_ABS;
  mem[0xFFFD] = 0x80;
  mem[0xFFFE] = 0x44; // 0x4480
  mem[0x4480] = 0x77;
  constexpr Sint32 EXPECTED_CYCLES = 4;
  CPU cpuCopy = cpu;
  //
  // when:
  Sint32 CyclesUsed = cpu.Execute (mem);

  EXPECT_EQ (cpu.X, 0x77);
  EXPECT_EQ (CyclesUsed, EXPECTED_CYCLES);
  EXPECT_FALSE (cpu.Z);
  EXPECT_FALSE (cpu.N);
  VerifyUnmodifiedFlags (cpu, cpuCopy);
}

TEST_F (cbemuTest, LDXAbsoluteY)
{

  // given:
  cpu.Y = 1;
  mem[0xFFFC] = INS_LDX_ABY;
  mem[0xFFFD] = 0x80;
  mem[0xFFFE] = 0x44; // 0x4480
  mem[0x4481] = 0x77;
  constexpr Sint32 EXPECTED_CYCLES = 4;
  CPU cpuCopy = cpu;

  // when:
  Sint32 CyclesUsed = cpu.Execute (mem);

  // then:
  EXPECT_EQ (cpu.X, 0x77);
  EXPECT_EQ (CyclesUsed, EXPECTED_CYCLES);
  EXPECT_FALSE (cpu.Z);
  EXPECT_FALSE (cpu.N);
  VerifyUnmodifiedFlags (cpu, cpuCopy);
}

TEST_F (cbemuTest, LDXAbsoluteYBoundary)
{

  // given:
  cpu.Y = 0xFF;
  mem[0xFFFC] = INS_LDX_ABY;
  mem[0xFFFD] = 0x80;
  mem[0xFFFE] = 0x44; // 0x4480
  mem[0x457F] = 0x77;
  constexpr Sint32 EXPECTED_CYCLES = 5;
  CPU cpuCopy = cpu;

  // when:
  Sint32 CyclesUsed = cpu.Execute (mem);

  // then:
  EXPECT_EQ (cpu.X, 0x77);
  EXPECT_EQ (CyclesUsed, EXPECTED_CYCLES);
  EXPECT_FALSE (cpu.Z);
  EXPECT_FALSE (cpu.N);
  VerifyUnmodifiedFlags (cpu, cpuCopy);
}
TEST_F (cbemuTest, LDYImmediate)
{

  // given:
  mem[0xFFFC] = INS_LDY_IM;
  mem[0xFFFD] = 0x77;
  CPU cpuCopy = cpu;
  constexpr Sint32 EXPECTED_CYCLES = 2;

  // when:
  Sint32 CyclesUsed = cpu.Execute (mem);

  EXPECT_EQ (cpu.Y, 0x77);
  EXPECT_EQ (CyclesUsed, EXPECTED_CYCLES);
  EXPECT_FALSE (cpu.Z);
  EXPECT_FALSE (cpu.N);
  VerifyUnmodifiedFlags (cpu, cpuCopy);
}

TEST_F (cbemuTest, LDYZeroPage)
{

  // given:
  mem[0xFFFC] = INS_LDY_ZP;
  mem[0xFFFD] = 0x42;
  mem[0x0042] = 0x37;
  CPU cpuCopy = cpu;
  constexpr Sint32 EXPECTED_CYCLES = 3;

  // when:
  Sint32 CyclesUsed = cpu.Execute (mem);

  // then:
  EXPECT_EQ (cpu.Y, 0x37);
  EXPECT_EQ (CyclesUsed, EXPECTED_CYCLES);
  EXPECT_FALSE (cpu.Z);
  EXPECT_FALSE (cpu.N);
  VerifyUnmodifiedFlags (cpu, cpuCopy);
}

TEST_F (cbemuTest, LDYZeroPageX)
{

  // given:
  cpu.X = 0x04;
  mem[0xFFFC] = INS_LDY_ZPX;
  mem[0xFFFD] = 0x06;
  mem[0x000A] = 0x37;
  CPU cpuCopy = cpu;
  constexpr Sint32 EXPECTED_CYCLES = 4;

  // when:
  Sint32 CyclesUsed = cpu.Execute (mem);

  EXPECT_EQ (cpu.Y, 0x37);
  EXPECT_EQ (CyclesUsed, EXPECTED_CYCLES);
  EXPECT_FALSE (cpu.Z);
  EXPECT_FALSE (cpu.N);
  VerifyUnmodifiedFlags (cpu, cpuCopy);
}

TEST_F (cbemuTest, LDYZeroPageXBoundaryCheck)
{

  // given:
  cpu.X = 0xFF;
  mem[0xFFFC] = INS_LDY_ZPX;
  mem[0xFFFD] = 0x80;
  mem[0x007F] = 0x37;
  CPU cpuCopy = cpu;
  constexpr Sint32 EXPECTED_CYCLES = 4;

  // when:
  Sint32 CyclesUsed = cpu.Execute (mem);

  EXPECT_EQ (cpu.Y, 0x37);
  EXPECT_EQ (CyclesUsed, EXPECTED_CYCLES);
  EXPECT_FALSE (cpu.Z);
  EXPECT_FALSE (cpu.N);
  VerifyUnmodifiedFlags (cpu, cpuCopy);
}

TEST_F (cbemuTest, LDYAbsolute)
{

  // given:
  mem[0xFFFC] = INS_LDY_ABS;
  mem[0xFFFD] = 0x80;
  mem[0xFFFE] = 0x44; // 0x4480
  mem[0x4480] = 0x77;
  constexpr Sint32 EXPECTED_CYCLES = 4;
  CPU cpuCopy = cpu;
  //
  // when:
  Sint32 CyclesUsed = cpu.Execute (mem);

  EXPECT_EQ (cpu.Y, 0x77);
  EXPECT_EQ (CyclesUsed, EXPECTED_CYCLES);
  EXPECT_FALSE (cpu.Z);
  EXPECT_FALSE (cpu.N);
  VerifyUnmodifiedFlags (cpu, cpuCopy);
}

TEST_F (cbemuTest, LDYAbsoluteX)
{

  // given:
  cpu.X = 1;
  mem[0xFFFC] = INS_LDY_ABX;
  mem[0xFFFD] = 0x80;
  mem[0xFFFE] = 0x44; // 0x4480
  mem[0x4481] = 0x77;
  constexpr Sint32 EXPECTED_CYCLES = 4;
  CPU cpuCopy = cpu;

  // when:
  Sint32 CyclesUsed = cpu.Execute (mem);

  // then:
  EXPECT_EQ (cpu.Y, 0x77);
  EXPECT_EQ (CyclesUsed, EXPECTED_CYCLES);
  EXPECT_FALSE (cpu.Z);
  EXPECT_FALSE (cpu.N);
  VerifyUnmodifiedFlags (cpu, cpuCopy);
}

TEST_F (cbemuTest, LDYAbsoluteXBoundary)
{

  // given:
  cpu.X = 0xFF;
  mem[0xFFFC] = INS_LDY_ABX;
  mem[0xFFFD] = 0x80;
  mem[0xFFFE] = 0x44; // 0x4480
  mem[0x457F] = 0x77;
  constexpr Sint32 EXPECTED_CYCLES = 5;
  CPU cpuCopy = cpu;

  // when:
  Sint32 CyclesUsed = cpu.Execute (mem);

  // then:
  EXPECT_EQ (cpu.Y, 0x77);
  EXPECT_EQ (CyclesUsed, EXPECTED_CYCLES);
  EXPECT_FALSE (cpu.Z);
  EXPECT_FALSE (cpu.N);
  VerifyUnmodifiedFlags (cpu, cpuCopy);
}

TEST_F (cbemuTest, JMPAbsolutePCContainsAddress){
  // given:
  cpu.A = 0x0;
  mem[0xFFFC] = INS_JMP_ABS;
  mem[0xFFFD] = 0x80;
  mem[0xFFFE] = 0x44;
  mem[0x4480] = INS_LDA_ABS;
  mem[0x4481] = 0x44;
  mem[0x4482] = 0x80;
  mem[0x8044] = 0x77;

  constexpr Sint32 EXPECTED_CYCLES = 7;
  CPU cpuCopy = cpu;

  // when:
  Sint32 CyclesUsed = cpu.Execute (mem);
  CyclesUsed += cpu.Execute (mem);

  // then:
  EXPECT_EQ (cpu.PC, 0x4483);
  EXPECT_EQ (cpu.A, 0x77);
  EXPECT_FALSE (cpu.Z);
  EXPECT_FALSE (cpu.N);
  VerifyUnmodifiedFlags (cpu, cpuCopy);
}
