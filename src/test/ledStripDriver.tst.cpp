#include <CppUTest/TestHarness.h>
#include <CppUTestExt/MockSupport.h>
#include <cstring>

#include "StringFrom.h"
#include "colour.h"
#include "ledStripDriver.h"

#define MAX_LEDS 10

static const Colour COLOUR_RED(255, 0, 0);
static const Colour COLOUR_GREEN(0, 255, 0);

static const uint8_t COLOURS_PER_LED = 3;

static uint8_t values[MAX_LEDS * COLOURS_PER_LED];
static uint8_t *lastValuesWritten;

static void writeValueStub(uint8_t *values, uint32_t length) {
    memcpy(lastValuesWritten, values, length);
}

static LedStripDriver *driver;

static const led_strip_config_t CONFIG_LEDS_1 = {
    .numLeds = 1,
    .writeValueFn = writeValueStub,
    .resolutionMs = 1,
};

static const led_strip_config_t CONFIG_LEDS_3 = {
    .numLeds = 3,
    .writeValueFn = writeValueStub,
    .resolutionMs = 1,
};

static void verify_colours(const Colour *expected, uint8_t *values, uint32_t len) {
    for (uint32_t i=0; i<len; i++) {
      uint32_t index = i * 3;
      CHECK_EQUAL(*expected, Colour(values[index+1], values[index+0], values[index+2]));
    }
}

static void verify_colour(const Colour *expected, uint8_t *values) {
  verify_colours(expected, values, 1);
}

/***********************************************************************************************
 * Common code
 **********************************************************************************************/

TEST_GROUP(LedStripDriverCommonTestGroup)
{
    void setup() {
        const uint32_t valuesLength = MAX_LEDS * COLOURS_PER_LED;

        driver = new LedStripDriver((led_strip_config_t*)&CONFIG_LEDS_3);
        lastValuesWritten = new uint8_t[valuesLength];
        memset(lastValuesWritten, 0, valuesLength);
        memset(values, 0, valuesLength);
    }

    void teardown() {
        delete driver;
        delete[] lastValuesWritten;
    }
};

TEST(LedStripDriverCommonTestGroup, counterIncrementsByResolution)
{
    led_strip_state_t state = { .counter = 0 };

    driver->onTimerFired(&state, values);

    LONGS_EQUAL(CONFIG_LEDS_3.resolutionMs, state.counter);
}

/***********************************************************************************************
 * Pulse pattern
 **********************************************************************************************/

TEST_GROUP(LedStripDriverPulseTestGroup)
{
    void setup() {
        const uint32_t valuesLength = MAX_LEDS * COLOURS_PER_LED;

        driver = new LedStripDriver((led_strip_config_t*)&CONFIG_LEDS_3);
        lastValuesWritten = new uint8_t[valuesLength];
        memset(lastValuesWritten, 0, valuesLength);
        memset(values, 0, valuesLength);
    }

    void teardown() {
        delete driver;
        delete[] lastValuesWritten;
    }
};

TEST(LedStripDriverPulseTestGroup, writesOnValueForPulseWhenCounterLessThanOnTime)
{
    const Colour& COLOUR_ON = COLOUR_RED;

    driver->pattern(Pattern::pulse)
          ->period(10)
          ->dutyCycle(50)
          ->colourOn((Colour*)&COLOUR_ON)
          ->colourOff((Colour*)&COLOUR_GREEN);

    led_strip_state_t state = {
      .counter = 0,
      .dutyCycle = 50,
    };
    driver->onTimerFired(&state, values);

    verify_colour(&COLOUR_ON, lastValuesWritten);
}

TEST(LedStripDriverPulseTestGroup, writesOffValueForPulseWhenCounterGreaterThanOnTime)
{
    const Colour& COLOUR_OFF = COLOUR_GREEN;

    driver->pattern(Pattern::pulse)
          ->period(10)
          ->dutyCycle(50)
          ->colourOn((Colour*)&COLOUR_RED)
          ->colourOff((Colour*)&COLOUR_OFF);

    led_strip_state_t state = { .counter = 5 };
    driver->onTimerFired(&state, values);

    verify_colour(&COLOUR_OFF, lastValuesWritten);
}

TEST(LedStripDriverPulseTestGroup, resetsCounterWhenEqualToPeriod)
{
    const Colour& COLOUR_ON = COLOUR_RED;
    const uint32_t PERIOD_MS = 10;

    driver->pattern(Pattern::pulse)
          ->period(PERIOD_MS)
          ->dutyCycle(50)
          ->colourOn((Colour*)&COLOUR_ON)
          ->colourOff((Colour*)&COLOUR_GREEN);

    led_strip_state_t state = { .counter = PERIOD_MS };
    driver->onTimerFired(&state, values);

    CHECK(state.counter < PERIOD_MS);
}

TEST(LedStripDriverPulseTestGroup, writesOnValueForPulseWhenCounterEqualToPeriod)
{
    const Colour& COLOUR_ON = COLOUR_RED;
    const uint32_t PERIOD_MS = 10;

    driver->pattern(Pattern::pulse)
          ->period(PERIOD_MS)
          ->dutyCycle(50)
          ->colourOn((Colour*)&COLOUR_ON)
          ->colourOff((Colour*)&COLOUR_GREEN);

    led_strip_state_t state = {
      .counter = PERIOD_MS,
      .dutyCycle = 50,
    };

    driver->onTimerFired(&state, values);

    verify_colour(&COLOUR_ON, lastValuesWritten);
}

TEST(LedStripDriverPulseTestGroup, dutyCycleIncrementsByCorrectAmountAfterPeriod)
{
    const Colour& COLOUR_ON = COLOUR_RED;
    const uint32_t PERIOD_MS = 100;

    driver->pattern(Pattern::pulse)
          ->period(PERIOD_MS)
          ->dutyCycle(20)
          ->colourOn((Colour*)&COLOUR_ON)
          ->colourOff((Colour*)&COLOUR_GREEN);

    led_strip_state_t state = {
      .counter = CONFIG_LEDS_1.resolutionMs * 10,
      .dutyDirection = 1,
      .dutyCycle = 1,
    };

    driver->onTimerFired(&state, values);

    DOUBLES_EQUAL(51, state.dutyCycle, 0.01);
}

TEST(LedStripDriverPulseTestGroup, dutyCycleCappedAt99)
{
    const Colour& COLOUR_ON = COLOUR_RED;
    const uint32_t PERIOD_MS = 100;

    driver->pattern(Pattern::pulse)
          ->period(PERIOD_MS)
          ->dutyCycle(50)
          ->colourOn((Colour*)&COLOUR_ON)
          ->colourOff((Colour*)&COLOUR_GREEN);

    led_strip_state_t state = {
      .counter = CONFIG_LEDS_1.resolutionMs * 10,
      .dutyDirection = 1,
      .dutyCycle = 98,
    };

    driver->onTimerFired(&state, values);

    DOUBLES_EQUAL(99, state.dutyCycle, 0.01);
}

TEST(LedStripDriverPulseTestGroup, dutyCycleDirectionReversesAfterReachingMaximum)
{
    const Colour& COLOUR_ON = COLOUR_RED;
    const uint32_t PERIOD_MS = 100;

    driver->pattern(Pattern::pulse)
          ->period(PERIOD_MS)
          ->dutyCycle(50)
          ->colourOn((Colour*)&COLOUR_ON)
          ->colourOff((Colour*)&COLOUR_GREEN);

    led_strip_state_t state = {
      .counter = CONFIG_LEDS_1.resolutionMs * 10,
      .dutyDirection = 1,
      .dutyCycle = 98,
    };

    driver->onTimerFired(&state, values);

    CHECK_EQUAL(-1, state.dutyDirection);
}

TEST(LedStripDriverPulseTestGroup, dutyCycleDecrementsByCorrectAmountAfterPeriod)
{
    const Colour& COLOUR_ON = COLOUR_RED;
    const uint32_t PERIOD_MS = 100;

    driver->pattern(Pattern::pulse)
          ->period(PERIOD_MS)
          ->dutyCycle(20)
          ->colourOn((Colour*)&COLOUR_ON)
          ->colourOff((Colour*)&COLOUR_GREEN);

    led_strip_state_t state = {
      .counter = CONFIG_LEDS_1.resolutionMs * 10,
      .dutyDirection = -1,
      .dutyCycle = 99,
    };

    driver->onTimerFired(&state, values);

    DOUBLES_EQUAL(86.5, state.dutyCycle, 0.01);
}

TEST(LedStripDriverPulseTestGroup, dutyCycleMinLimitOf1)
{
    const Colour& COLOUR_ON = COLOUR_RED;
    const uint32_t PERIOD_MS = 100;

    driver->pattern(Pattern::pulse)
          ->period(PERIOD_MS)
          ->dutyCycle(50)
          ->colourOn((Colour*)&COLOUR_ON)
          ->colourOff((Colour*)&COLOUR_GREEN);

    led_strip_state_t state = {
      .counter = CONFIG_LEDS_1.resolutionMs * 10,
      .dutyDirection = -1,
      .dutyCycle = 2,
    };

    driver->onTimerFired(&state, values);

    DOUBLES_EQUAL(1, state.dutyCycle, 0.01);
}

TEST(LedStripDriverPulseTestGroup, dutyCycleDirectionReversesAfterReachingMinimum)
{
    const Colour& COLOUR_ON = COLOUR_RED;
    const uint32_t PERIOD_MS = 100;

    driver->pattern(Pattern::pulse)
          ->period(PERIOD_MS)
          ->dutyCycle(50)
          ->colourOn((Colour*)&COLOUR_ON)
          ->colourOff((Colour*)&COLOUR_GREEN);

    led_strip_state_t state = {
      .counter = CONFIG_LEDS_1.resolutionMs * 10,
      .dutyDirection = -1,
      .dutyCycle = 2,
    };

    driver->onTimerFired(&state, values);

    CHECK_EQUAL(1, state.dutyDirection);
}

TEST(LedStripDriverPulseTestGroup, writesOnValueForAllLeds)
{
    const Colour& COLOUR_ON = COLOUR_RED;

    LedStripDriver drv((led_strip_config_t*)&CONFIG_LEDS_3);

    drv.pattern(Pattern::pulse)
      ->period(100)
      ->dutyCycle(50)
      ->colourOn((Colour*)&COLOUR_ON)
      ->colourOff((Colour*)&COLOUR_GREEN);

    led_strip_state_t state = {
      .counter = 0,
      .dutyDirection = 1,
      .dutyCycle = 50,
    };

    drv.onTimerFired(&state, values);

    verify_colours(&COLOUR_ON, lastValuesWritten, 3);
}

TEST(LedStripDriverPulseTestGroup, writesOffValueForAllLeds)
{
    const Colour& COLOUR_OFF = COLOUR_GREEN;

    LedStripDriver drv((led_strip_config_t*)&CONFIG_LEDS_3);

    drv.pattern(Pattern::pulse)
      ->period(100)
      ->dutyCycle(50)
      ->colourOn((Colour*)&COLOUR_RED)
      ->colourOff((Colour*)&COLOUR_OFF);

    led_strip_state_t state = {
      .counter = 5,
      .dutyDirection = 1,
      .dutyCycle = 50,
    };

    drv.onTimerFired(&state, values);

    verify_colours(&COLOUR_OFF, lastValuesWritten, 3);
}

/***********************************************************************************************
 * Blink pattern
 **********************************************************************************************/
TEST_GROUP(LedStripDriverBlinkTestGroup)
{
    void setup() {
        const uint32_t valuesLength = MAX_LEDS * COLOURS_PER_LED;

        driver = new LedStripDriver((led_strip_config_t*)&CONFIG_LEDS_3);
        lastValuesWritten = new uint8_t[valuesLength];
        memset(lastValuesWritten, 0, valuesLength);
        memset(values, 0, valuesLength);
    }

    void teardown() {
        delete driver;
        delete[] lastValuesWritten;
    }
};

TEST(LedStripDriverBlinkTestGroup, writesOnValueForAllLedsWithCorrectColour)
{
    const Colour& COLOUR_ON = COLOUR_RED;

    driver->pattern(Pattern::blink)
          ->period(10)
          ->dutyCycle(50)
          ->colourOn((Colour*)&COLOUR_ON)
          ->colourOff((Colour*)&COLOUR_GREEN);

    led_strip_state_t state = {
      .counter = 0,
    };

    driver->onTimerFired(&state, values);

    verify_colours(&COLOUR_ON, lastValuesWritten, 3);
}

TEST(LedStripDriverBlinkTestGroup, writesOffValueForAllLedsWithCorrectColourAfterPeriod)
{
    const Colour& COLOUR_OFF = COLOUR_GREEN;

    driver->pattern(Pattern::blink)
          ->period(10)
          ->dutyCycle(50)
          ->colourOn((Colour*)&COLOUR_RED)
          ->colourOff((Colour*)&COLOUR_OFF);

    led_strip_state_t state = {
      .counter = 5,
    };

    driver->onTimerFired(&state, values);

    verify_colours(&COLOUR_OFF, lastValuesWritten, 3);
}

TEST(LedStripDriverBlinkTestGroup, resetsCounterAfterPeriod)
{
  const uint32_t PERIOD_MS = 10;

    driver->pattern(Pattern::blink)
          ->period(PERIOD_MS)
          ->dutyCycle(50)
          ->colourOn((Colour*)&COLOUR_RED)
          ->colourOff((Colour*)&COLOUR_GREEN);

    led_strip_state_t state = {
      .counter = PERIOD_MS,
    };

    driver->onTimerFired(&state, values);

    LONGS_EQUAL(1, state.counter);
}

/***********************************************************************************************
 * Colour pattern
 **********************************************************************************************/
TEST_GROUP(LedStripDriverColourTestGroup)
{
    void setup() {
        const uint32_t valuesLength = MAX_LEDS * COLOURS_PER_LED;

        driver = new LedStripDriver((led_strip_config_t*)&CONFIG_LEDS_3);
        lastValuesWritten = new uint8_t[valuesLength];
        memset(lastValuesWritten, 0, valuesLength);
        memset(values, 0, valuesLength);
    }

    void teardown() {
        delete driver;
        delete[] lastValuesWritten;
    }
};

TEST(LedStripDriverColourTestGroup, writesValueForAllLedsWithCorrectColour)
{
    const Colour& COLOUR = COLOUR_RED;

    driver->pattern(Pattern::colour)
          ->colourOn((Colour*)&COLOUR);

    led_strip_state_t state = {
      .counter = 0,
    };

    driver->onTimerFired(&state, values);

    verify_colours(&COLOUR, lastValuesWritten, 3);
}

/***********************************************************************************************
 * Strobe pattern
 **********************************************************************************************/
TEST_GROUP(LedStripDriverStrobeTestGroup)
{
    void setup() {
        const uint32_t valuesLength = MAX_LEDS * COLOURS_PER_LED;

        driver = new LedStripDriver((led_strip_config_t*)&CONFIG_LEDS_3);
        lastValuesWritten = new uint8_t[valuesLength];
        memset(lastValuesWritten, 0, valuesLength);
        memset(values, 0, valuesLength);
    }

    void teardown() {
        delete driver;
        delete[] lastValuesWritten;
    }
};

TEST(LedStripDriverStrobeTestGroup, writesOnValueForAllLedsWithCorrectColour)
{
  const Colour& COLOUR_ON = COLOUR_RED;
  const Colour& COLOUR_OFF = COLOUR_GREEN;

  driver->pattern(Pattern::strobe)
    ->period(10)
    ->colourOn((Colour*)&COLOUR_ON)
    ->colourOff((Colour*)&COLOUR_OFF);

  led_strip_state_t state = {
    .counter = 0,
  };

  driver->onTimerFired(&state, values);

  verify_colours(&COLOUR_ON, lastValuesWritten, 3);
}

TEST(LedStripDriverStrobeTestGroup, writesOffValueForAllLedsWithCorrectColour)
{
  const Colour& COLOUR_ON = COLOUR_RED;
  const Colour& COLOUR_OFF = COLOUR_GREEN;
  const uint32_t PERIOD_MS = 10;

  driver->pattern(Pattern::strobe)
    ->period(PERIOD_MS)
    ->colourOn((Colour*)&COLOUR_ON)
    ->colourOff((Colour*)&COLOUR_OFF);

  led_strip_state_t state = {
    .counter = PERIOD_MS / 2,
  };

  driver->onTimerFired(&state, values);

  verify_colours(&COLOUR_OFF, lastValuesWritten, 3);
}

TEST(LedStripDriverStrobeTestGroup, counterResetAfterPeriod)
{
  const Colour& COLOUR_ON = COLOUR_RED;
  const Colour& COLOUR_OFF = COLOUR_GREEN;
  const uint32_t PERIOD_MS = 10;

  driver->pattern(Pattern::strobe)
    ->period(PERIOD_MS)
    ->colourOn((Colour*)&COLOUR_ON)
    ->colourOff((Colour*)&COLOUR_OFF);

  led_strip_state_t state = {
    .counter = PERIOD_MS,
  };

  driver->onTimerFired(&state, values);

  LONGS_EQUAL(1, state.counter);
}

/***********************************************************************************************
 * Progress pattern
 **********************************************************************************************/
TEST_GROUP(LedStripDriverProgressTestGroup)
{
    void setup() {
        const uint32_t valuesLength = MAX_LEDS * COLOURS_PER_LED;

        driver = new LedStripDriver((led_strip_config_t*)&CONFIG_LEDS_3);
        lastValuesWritten = new uint8_t[valuesLength];
        memset(lastValuesWritten, 0, valuesLength);
        memset(values, 0, valuesLength);
    }

    void teardown() {
        delete driver;
        delete[] lastValuesWritten;
    }
};

TEST(LedStripDriverProgressTestGroup, writesCorrectInitialValue)
{
  const Colour& COLOUR_ON = COLOUR_RED;
  const Colour& COLOUR_OFF = COLOUR_GREEN;

  driver->pattern(Pattern::progress)
    ->initial(1)
    ->direction(Direction::forward)
    ->colourOn((Colour*)&COLOUR_ON)
    ->colourOff((Colour*)&COLOUR_OFF);

  led_strip_state_t state = {
    .counter = 0,
  };

  driver->onTimerFired(&state, values);

  verify_colour(&COLOUR_ON, lastValuesWritten);
  verify_colours(&COLOUR_OFF, &lastValuesWritten[3], 2);
}

TEST(LedStripDriverProgressTestGroup, incrementsProgressValueAfterDelay)
{
  const uint32_t DELAY_MS = 10;

  driver->pattern(Pattern::progress)
    ->delay(DELAY_MS);

  led_strip_state_t state = {
    .counter = DELAY_MS,
    .progress = 0,
  };

  driver->onTimerFired(&state, values);

  LONGS_EQUAL(1, state.progress);
}

TEST(LedStripDriverProgressTestGroup, resetsCounterAfterDelay)
{
  const uint32_t DELAY_MS = 10;

  driver->pattern(Pattern::progress)
    ->delay(DELAY_MS);

  led_strip_state_t state = {
    .counter = DELAY_MS,
    .progress = 0,
  };

  driver->onTimerFired(&state, values);

  LONGS_EQUAL(1, state.counter);
}

TEST(LedStripDriverProgressTestGroup, writesCorrectValue)
{
  const Colour& COLOUR_ON = COLOUR_RED;
  const Colour& COLOUR_OFF = COLOUR_GREEN;
  const uint32_t PROGRESS = 2;

  driver->pattern(Pattern::progress)
    ->initial(0)
    ->direction(Direction::forward)
    ->colourOn((Colour*)&COLOUR_ON)
    ->colourOff((Colour*)&COLOUR_OFF);

  led_strip_state_t state = {
    .counter = 0,
    .progress = PROGRESS,
  };

  driver->onTimerFired(&state, values);

  verify_colour(&COLOUR_ON, lastValuesWritten);
  verify_colours(&COLOUR_OFF, &lastValuesWritten[PROGRESS * COLOURS_PER_LED], 1);
}

TEST(LedStripDriverProgressTestGroup, resetsProgressAfterAllLedsOn)
{
  const Colour& COLOUR_ON = COLOUR_RED;
  const Colour& COLOUR_OFF = COLOUR_GREEN;
  const uint32_t INITIAL = 1;
  const uint32_t PROGRESS = CONFIG_LEDS_3.numLeds;
  const uint32_t DELAY_MS = 10;

  driver->pattern(Pattern::progress)
    ->initial(INITIAL)
    ->delay(DELAY_MS)
    ->direction(Direction::forward)
    ->colourOn((Colour*)&COLOUR_ON)
    ->colourOff((Colour*)&COLOUR_OFF);

  led_strip_state_t state = {
    .counter = DELAY_MS,
    .progress = PROGRESS,
  };

  driver->onTimerFired(&state, values);

  LONGS_EQUAL(0, state.progress);
}
