#include <CppUTest/TestHarness.h>
#include <CppUTestExt/MockSupport.h>
#include "argParser.h"

using namespace argParser;

#define NUM_ARGS_MAX 5

String output[NUM_ARGS_MAX];

const ArgInfo ARG_INFO_PERIOD_MS = {
  .type = ARG_TYPE_NUMBER,
  .min = 1,
  .max = 1000,
};

const ArgInfo ARG_INFO_COLOUR = {
  .type = ARG_TYPE_COLOUR,
};

const ArgInfo ARGS_INFO_STROBE[] = {
  ARG_INFO_PERIOD_MS,
  ARG_INFO_COLOUR,
};

const ArgConfig ARG_CONFIG_STROBE = {
  .info = ARGS_INFO_STROBE,
  .length = 2,
};

TEST_GROUP(ArgParserTestGroup)
{
    void setup() {
      //clear results array
      for (uint32_t i=0; i<NUM_ARGS_MAX; i++) {
        output[i] = "";
      }
    }

    void teardown() {

    }
};

TEST(ArgParserTestGroup, returnsSuccessForValidArguments)
{
  LONGS_EQUAL(RET_VAL_SUC,
              parseAndValidateArgs(output, (ArgConfig*)&ARG_CONFIG_STROBE, "1000,#FF0000"));
}

TEST(ArgParserTestGroup, returnsErrorForNoArgs)
{
  LONGS_EQUAL(RET_VAL_TOO_FEW_ARGS,
              parseAndValidateArgs(output, (ArgConfig*)&ARG_CONFIG_STROBE,""));
}

TEST(ArgParserTestGroup, returnsErrorForOneArg)
{
  LONGS_EQUAL(RET_VAL_TOO_FEW_ARGS,
              parseAndValidateArgs(output, (ArgConfig*)&ARG_CONFIG_STROBE,"1000"));
}

TEST(ArgParserTestGroup, returnsErrorForOneArgWithComma)
{
  LONGS_EQUAL(RET_VAL_TOO_FEW_ARGS,
              parseAndValidateArgs(output, (ArgConfig*)&ARG_CONFIG_STROBE,"1000,"));
}

TEST(ArgParserTestGroup, returnsErrorForTooManyArgs)
{
  LONGS_EQUAL(RET_VAL_TOO_MANY_ARGS,
              parseAndValidateArgs(output, (ArgConfig*)&ARG_CONFIG_STROBE,"1000,#000000,0"));
}

TEST(ArgParserTestGroup, returnsErrorForInvalidFirstArgType)
{
  LONGS_EQUAL(RET_VAL_INVALID_ARG,
              parseAndValidateArgs(output, (ArgConfig*)&ARG_CONFIG_STROBE,"abcd,#000000"));
}

TEST(ArgParserTestGroup, returnsErrorForInvalidSecondArgType)
{
  LONGS_EQUAL(RET_VAL_INVALID_ARG,
              parseAndValidateArgs(output, (ArgConfig*)&ARG_CONFIG_STROBE,"1000,12"));
}

TEST(ArgParserTestGroup, returnsErrorForNumberBelowMinimum)
{
  LONGS_EQUAL(RET_VAL_INVALID_ARG,
              parseAndValidateArgs(output, (ArgConfig*)&ARG_CONFIG_STROBE, "0,#FF0000"));
}

TEST(ArgParserTestGroup, returnsErrorForNumberAboveMaximum)
{
  LONGS_EQUAL(RET_VAL_INVALID_ARG,
              parseAndValidateArgs(output, (ArgConfig*)&ARG_CONFIG_STROBE, "1001,#FF0000"));
}

TEST(ArgParserTestGroup, writesParsedArgsToOutputCorrectly)
{
  const String ARG_PERIOD("1000");
  const String ARG_COLOUR("#FF0000");
  const String ARGS = ARG_PERIOD + String(",") + ARG_COLOUR;

  parseAndValidateArgs(output, (ArgConfig*)&ARG_CONFIG_STROBE, ARGS);

  STRCMP_EQUAL(ARG_PERIOD, output[0]);
  STRCMP_EQUAL(ARG_COLOUR, output[1]);
}
