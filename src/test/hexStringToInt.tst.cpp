#include <CppUTest/TestHarness.h>
#include <CppUTestExt/MockSupport.h>

#include "hexStrToInt.h"
#include <iostream>
#include <stdlib.h>
#include <stdexcept>

TEST_GROUP(HexStringToIntTestGroup) {

};

TEST(HexStringToIntTestGroup, returnsCorrectResultForSingleLowercaseHexChar)
{
    BYTES_EQUAL((uint8_t)15, hexStrToInt("f"));
}

TEST(HexStringToIntTestGroup, returnsCorrectResultForSingleUppercaseHexChar)
{
    BYTES_EQUAL((uint8_t)10, hexStrToInt("A"));
}

TEST(HexStringToIntTestGroup, returnsCorrectResultForSingleNumber)
{
    BYTES_EQUAL((uint8_t)0, hexStrToInt("0"));
}

TEST(HexStringToIntTestGroup, returnsCorrectResultForTwoChars)
{
    BYTES_EQUAL((uint8_t)245, hexStrToInt("F5"));
}
