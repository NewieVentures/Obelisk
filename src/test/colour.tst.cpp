#include <CppUTest/TestHarness.h>
#include <CppUTest/SimpleString.h>
#include <CppUTestExt/MockSupport.h>

#include "StringFrom.h"
#include "colour.h"


TEST_GROUP(ColourTestGroup)
{
    void teardown()
    {
    }
};

TEST(ColourTestGroup, returnsBlackForInvalidString)
{
    Colour c = Colour("saohetu");

    BYTES_EQUAL((uint8_t)0, c.getRed());
    BYTES_EQUAL((uint8_t)0, c.getGreen());
    BYTES_EQUAL((uint8_t)0, c.getBlue());
}

TEST(ColourTestGroup, returnsBlackForInvalidLength)
{
    Colour c = Colour("#1122334");

    BYTES_EQUAL((uint8_t)0, c.getRed());
    BYTES_EQUAL((uint8_t)0, c.getGreen());
    BYTES_EQUAL((uint8_t)0, c.getBlue());
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
