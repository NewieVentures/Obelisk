#include <CppUTest/TestHarness.h>
#include <CppUTest/SimpleString.h>
#include <CppUTestExt/MockSupport.h>

#include "StringFrom.h"
#include "colour.h"
#include <stdexcept>


TEST_GROUP(ColourTestGroup)
{
  void teardown()
  {
  }
};

TEST(ColourTestGroup, isValidReturnsFalseForInvalidString)
{
  Colour colour = Colour("saohetu");
  CHECK_FALSE(colour.isValid());
}

TEST(ColourTestGroup, isValidReturnsFalseForInvalidLength)
{
  Colour colour = Colour("#1122334");
  CHECK_FALSE(colour.isValid());
}

TEST(ColourTestGroup, isValidReturnsTrueForValidColourString)
{
  Colour colour = Colour("#112233");
  CHECK(colour.isValid());
}

TEST(ColourTestGroup, returnsCorrectRedValueFromString)
{
  Colour c = Colour("#11FFA5");
  BYTES_EQUAL((uint8_t)17, c.getRed());
}

TEST(ColourTestGroup, returnsCorrectGreenValueFromString)
{
  Colour c = Colour("#00FEA5");
  BYTES_EQUAL((uint8_t)254, c.getGreen());
}

TEST(ColourTestGroup, returnsCorrectBlueValueFromString)
{
  Colour c = Colour("#00FFA5");
  BYTES_EQUAL((uint8_t)165, c.getBlue());
}

TEST(ColourTestGroup, comparesValuesForEquality)
{
  Colour c1 = Colour("#00FFA5");
  Colour c2 = Colour("#00FFA5");

  CHECK_EQUAL(c1, c2);
}

TEST(ColourTestGroup, toStringReturnsCorrectResult)
{
  uint8_t red = 0xA5;
  uint8_t green = 0x21;
  uint8_t blue = 0;
  String expected = String("#A52100");

  Colour colour = Colour(red, green, blue);

  STRCMP_EQUAL(expected, colour.toString());
}
