/**
* @file microunit.h
* @author Sebastiao Salvador de Miranda (ssm)
* @copyright Copyright (c) 2016, Sebastiao Salvador de Miranda.
*            All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are
* met:
*
* (1) Redistributions of source code must retain the above copyright
* notice, this list of conditions and the following disclaimer.
*
* (2) Redistributions in binary form must reproduce the above copyright
* notice, this list of conditions and the following disclaimer in
* the documentation and/or other materials provided with the
* distribution.
*
* (3) The name of the author may not be used to
* endorse or promote products derived from this software without
* specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
* IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
* INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
* STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
* IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*
* @brief Tiny library for cpp unit testing. Should work on any c++11 compiler.
*
* Simply include this header in your test implementation file (e.g., main.cpp)
* and call microunit::UnitTester::Run() in the function main(). To register
* a new unit test case, use the macro UNIT (See the example below). Inside the
* test case body, you can use the following macros to control the result
* of the test.
*
* @li PASS() : Pass the test and return.
* @li FAIL() : Fail the test and return.
* @li ASSERT_TRUE(condition) : If the condition does not hold, fail and return.
* @li ASSERT_FALSE(condition) : If the condition holds, fail and return.
*
* @code{.cpp}
*  UNIT(Test_Two_Plus_Two) {
*    ASSERT_TRUE(2 + 2 == 4);
*  };
*  // ...
*  int main(){
*    return microunit::UnitTester::Run() ? 0 : -1;
*  }
* @endcode
*/

#ifndef _MICROUNIT_MICROUNIT_H_
#define _MICROUNIT_MICROUNIT_H_
#include <map>
#include <string>
#include <vector>
#include <iostream>
#define MICROUNIT_SEPARATOR "----------------------------------------"\
                            "----------------------------------------"
namespace microunit {
/**
* @brief Result of a unit test.
*/
struct UnitFunctionResult {
  bool success{ true };
};

/**
* @brief Unit test function type.
*/
typedef void(*UnitFunction)(UnitFunctionResult*);

/**
* @brief Main class for unit test management. This class is a singleton
*        and maintains a list of all registered unit test cases.
*/
class UnitTester {
public:
  ~UnitTester() {};
  UnitTester(const UnitTester&) = delete;
  UnitTester(UnitTester&&) = delete;

  /**
  * @brief Run all the registered unit test cases.
  * @returns True if all tests pass, false otherwise.
  */
  static bool Run() {
    std::vector<std::string> failures;

    // Iterate all registered unit tests
    for (auto& unit : Instance().unitfunction_map_) {
      std::cout << MICROUNIT_SEPARATOR << std::endl;
      std::cout << "[    ] Test case '" << unit.first << "'" << std::endl;

      // Run the unit test
      UnitFunctionResult result;
      unit.second(&result);

      std::cout << (result.success ? "[    ] Success" :
                    "[!!!!] Failure") << std::endl;
      if (!result.success)
        failures.push_back(unit.first);
    }
    std::cout
      << MICROUNIT_SEPARATOR << std::endl
      << MICROUNIT_SEPARATOR << std::endl;

    // Output result summary
    if (failures.empty()) {
      std::cout << "[    ] All tests passed" << std::endl;
      std::cout << MICROUNIT_SEPARATOR << std::endl;
      return true;
    } else {
      std::cout << "[!!!!] Failed " << failures.size()
        << " test cases:" << std::endl;
      for (const auto& failure : failures) {
        std::cout << "> " << failure << std::endl;
      }
      std::cout << MICROUNIT_SEPARATOR << std::endl;
      return false;
    }
  }

  /**
  * @brief Register a unit test case function. In regular library client usage,
  *        this doesn't need to be called, and the macro UNIT should be used
  *        instead.
  * @param [in] name  Name of the unit test case.
  * @param [in] function  Pointer to unit test case function.
  * @returns True if all tests pass, false otherwise.
  */
  static void RegisterFunction(const std::string &name,
                               UnitFunction function) {
    Instance().unitfunction_map_.emplace(name, function);
  }

  /**
  * @brief Helper class to register a unit test in construction time. This is
  *        used to call RegisterFunction in the construction of a static
  *        helper object. Used by the REGISTER_UNIT macro, which in turn is
  *        used by the UNIT macro.
  * @returns True if all tests pass, false otherwise.
  */
  class Registrator {
  public:
    Registrator(const std::string &name,
                UnitFunction function) {
      UnitTester::RegisterFunction(name, function);
    };
    Registrator(const Registrator&) = delete;
    Registrator(Registrator&&) = delete;
    ~Registrator() {};
  };

private:
  UnitTester() {};
  static UnitTester& Instance() {
    static UnitTester instance;
    return instance;
  }
  std::map<std::string, UnitFunction> unitfunction_map_;
};
}

#define MACROCAT_NEXP(A, B) A ## B
#define MACROCAT(A, B) MACROCAT_NEXP(A, B)

/**
* @brief Register a unit function using a helper static Registrator object.
*/
#define REGISTER_UNIT(FUNCTION)                                                \
  static microunit::UnitTester::Registrator                                    \
  MACROCAT(MICROUNIT_REGISTRATION, __COUNTER__)(#FUNCTION, FUNCTION);

/**
* @brief Define a unit function body. This macro is the one which should be used
*        by client code to define unit test cases.
* @code{.cpp}
*  UNIT(Test_Two_Plus_Two) {
*    ASSERT_TRUE(2 + 2 == 4);
*  };
* @endcode
*/
#define UNIT(FUNCTION)                                                         \
void FUNCTION(microunit::UnitFunctionResult*);                                 \
REGISTER_UNIT(FUNCTION);                                                       \
void FUNCTION(microunit::UnitFunctionResult *__microunit_testresult)            

/**
* @brief Pass the test and return from the test case.
*/
#define PASS() {                                                               \
__microunit_testresult->success = true;                                        \
return;                                                                        \
}

/**
* @brief Fail the test and return from the test case.
*/
#define FAIL() {                                                               \
std::cout << "[    ] Test failed " << std::endl;                               \
__microunit_testresult->success = false;                                       \
return;                                                                        \
}

/**
* @brief Check a particular test condition. If the condition does not hold,
*        fail the test and return.
*/
#define ASSERT_TRUE(condition) if(!(condition)) {                              \
std::cout << "[    ] Test Assert failed: " #condition << std::endl;            \
FAIL();                                                                        \
}

/**
* @brief Check a particular test condition. If the condition holds, fail the 
*        test and return.
*/
#define ASSERT_FALSE(condition) if((condition)) {                              \
std::cout << "[    ] Test Assert failed: " #condition << std::endl;            \
FAIL();                                                                        \
}
#endif