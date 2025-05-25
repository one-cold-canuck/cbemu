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
VerifyUnmodifiedFlagsFromLDA (CPU cpu, CPU cpuCopy)
{
  EXPECT_EQ (cpu.V, cpuCopy.V);
  EXPECT_EQ (cpu.D, cpuCopy.D);
  EXPECT_EQ (cpu.I, cpuCopy.I);
  EXPECT_EQ (cpu.C, cpuCopy.C);
  EXPECT_EQ (cpu.B, cpuCopy.B);
}

TEST_F (cbemuTest, LDAImmediateCanLoadAValueIntoTheARegister)
{

  // given:
  mem[0xFFFC] = INS_LDA_IM;
  mem[0xFFFD] = 0x77;

  // when:
  CPU cpuCopy = cpu;
  Sint32 CyclesUsed = cpu.Execute (mem, 2);

  EXPECT_EQ (cpu.A, 0x77);
  EXPECT_EQ (CyclesUsed, 2);
  EXPECT_FALSE (cpu.Z);
  EXPECT_FALSE (cpu.N);
  VerifyUnmodifiedFlagsFromLDA (cpu, cpuCopy);
}

TEST_F (cbemuTest, LDAZeroPageCanLoadAValueIntoTheARegister)
{

  // given:
  mem[0xFFFC] = INS_LDA_ZP;
  mem[0xFFFD] = 0x42;
  mem[0x0042] = 0x37;

  // when:
  CPU cpuCopy = cpu;
  Sint32 CyclesUsed = cpu.Execute (mem, 3);

  // then:
  EXPECT_EQ (cpu.A, 0x37);
  EXPECT_EQ (CyclesUsed, 3);
  EXPECT_FALSE (cpu.Z);
  EXPECT_FALSE (cpu.N);
  VerifyUnmodifiedFlagsFromLDA (cpu, cpuCopy);
}

TEST_F (cbemuTest, LDAZeroPageLoadARegisterWrap)
{

  // given:
  cpu.X = 0xFF;
  mem[0xFFFC] = INS_LDA_ZPX;
  mem[0xFFFD] = 0x80;
  mem[0x007F] = 0x37;

  // when:
  CPU cpuCopy = cpu;
  Sint32 CyclesUsed = cpu.Execute (mem, 4);

  EXPECT_EQ (cpu.A, 0x37);
  EXPECT_EQ (CyclesUsed, 4);
  EXPECT_FALSE (cpu.Z);
  EXPECT_FALSE (cpu.N);
  VerifyUnmodifiedFlagsFromLDA (cpu, cpuCopy);
}
