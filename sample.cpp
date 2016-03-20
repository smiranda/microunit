#include "microunit.h"

int Double(int n) {
  return 2 * n;
}
int DoubleFlawed(int n) {
  return n < 100 ? 2 * n : 3 * n;
}

UNIT(Test_Two_Plus_Two) {
  ASSERT_TRUE(2 + 2 == 4);
};

UNIT(Test_Flawed_Two_Plus_Two) {
  ASSERT_TRUE(2 + 2 == 3);
};

UNIT(Test_Double) {
  for (int i = 0; i < 1000; ++i) {
    if (Double(i) != 2 * i) {
      FAIL();
    }
  }
};

UNIT(Test_Double_Flawed) {
  for (int i = 0; i < 1000; ++i) {
    if (DoubleFlawed(i) != 2 * i) {
      FAIL();
    }
  }
};

int main() {
  microunit::UnitTester::Run();
}

