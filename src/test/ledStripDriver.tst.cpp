#include <CppUTest/TestHarness.h>
#include <CppUTestExt/MockSupport.h>
#include <cstring>

#include "StringFrom.h"
#include "colour.h"
#include "ledStripDriver.h"

// class LedStripDriverSpy : public LedStripDriver {
//     public:
//     LedStripDriverSpy(led_strip_config_t *config) : LedStripDriver(config) {
//     };
// };

static const Colour COLOUR_RED(255, 0, 0);
static const Colour COLOUR_GREEN(0, 255, 0);

static const uint8_t COLOURS_PER_LED = 3;

static uint8_t *lastValuesWritten;

static void writeValueStub(uint8_t *values, uint32_t length) {
    memcpy(lastValuesWritten, values, length);
}

static LedStripDriver *driver;

static const led_strip_config_t CONFIG_LEDS_1 = {
    .num_leds = 1,
    .write_value_fn = writeValueStub,
    .resolution_ms = 1,
};

// static void verify_colour(const Colour *expected, uint8_t *values) {
//     const Colour expectedColour = Colour(expected->getRed(), expected->getGreen(), expected->getBlue());
//     const Colour actualColour = Colour(values[0], values[1], values[2]);
//     // const Colour& actual = actualColour;

//     const Colour& c1 = expectedColour;
//     const Colour& c2 = actualColour;

//     CHECK_EQUAL(c1, c2);
//     // BYTES_EQUAL((uint8_t)expected->getRed(), actual[0]);
//     // BYTES_EQUAL((uint8_t)expected->getGreen(), actual[1]);
//     // BYTES_EQUAL((uint8_t)expected->getBlue(), actual[2]);
// }

TEST_GROUP(LedStripDriverTestGroup)
{
    void setup() {
        const led_strip_config_t *CONFIG = &CONFIG_LEDS_1;
        const uint32_t valuesLength = CONFIG->num_leds * COLOURS_PER_LED;

        driver = new LedStripDriver((led_strip_config_t*)&CONFIG_LEDS_1);
        lastValuesWritten = new uint8_t[valuesLength];
        memset(lastValuesWritten, 0, valuesLength);
    }

    void teardown() {
        delete driver;
        delete[] lastValuesWritten;
    }
};
/*
TEST(LedStripDriverTestGroup, writesOnValueForPulseWhenCounterLessThanOnTime)
{
    const Colour& COLOUR_ON = COLOUR_RED;

    driver->pattern(Pattern::pulse)
          ->period(10)
          ->dutyCycle(50)
          ->colourOn((Colour*)&COLOUR_ON)
          ->colourOff((Colour*)&COLOUR_GREEN);

    led_strip_state_t state = { .counter = 0 };
    driver->onTimerFired(&state);

    verify_colour(&COLOUR_ON, lastValuesWritten);
}

TEST(LedStripDriverTestGroup, writesOffValueForPulseWhenCounterGreaterThanOnTime)
{
    const Colour& COLOUR_OFF = COLOUR_GREEN;

    driver->pattern(Pattern::pulse)
          ->period(10)
          ->dutyCycle(50)
          ->colourOn((Colour*)&COLOUR_RED)
          ->colourOff((Colour*)&COLOUR_OFF);

    led_strip_state_t state = { .counter = 5 };
    driver->onTimerFired(&state);

    verify_colour(&COLOUR_OFF, lastValuesWritten);
}
*/
