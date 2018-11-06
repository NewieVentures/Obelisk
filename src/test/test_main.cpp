#include "CppUTest/CommandLineTestRunner.h"
#include "CppUTest/TestRegistry.h"
#include "CppUTestExt/MockSupportPlugin.h"
#include "CppUTest/SimpleString.h"
#include "String.h"

#include <iostream>
using namespace std;

MockSupportPlugin mockPlugin;

class StringComparator : public MockNamedValueComparator {
  public:
  virtual bool isEqual(const void* object1, const void* object2) {
    return object1 == object2;
  }

  virtual SimpleString valueToString(const void* object) {
    return StringFromOrNull(((char*)object));
  }
};

StringComparator stringComparator;

int main(int ac, char** av)
{
  mockPlugin.installComparator("String", stringComparator);
  TestRegistry::getCurrentRegistry()->installPlugin(&mockPlugin);

  return CommandLineTestRunner::RunAllTests(ac, av);
}
