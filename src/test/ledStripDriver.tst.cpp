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

static void verify_colours(Colour *expected,
                           uint8_t *values,
                           uint32_t len,
                           uint32_t offset = 0) {
    for (uint32_t i=offset; i<len; i++) {
      uint32_t index = i * 3;
      Colour actual = Colour(values[index+1], values[index+0], values[index+2]);
      String message;

      message = message.format("Incorrect colour at index %d, expected: %s, actual: %s ",
                               len, expected->toString().c_str(), actual.toString().c_str());

      CHECK_TEXT(*expected == actual,
                 message);
    }
}

static void verify_colour(Colour *expected, uint8_t *values) {
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

    verify_colour((Colour*)&COLOUR_ON, lastValuesWritten);
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

    verify_colour((Colour*)&COLOUR_OFF, lastValuesWritten);
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

    verify_colour((Colour*)&COLOUR_ON, lastValuesWritten);
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

    verify_colours((Colour*)&COLOUR_ON, lastValuesWritten, 3);
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

    verify_colours((Colour*)&COLOUR_OFF, lastValuesWritten, 3);
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

    verify_colours((Colour*)&COLOUR_ON, lastValuesWritten, 3);
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

    verify_colours((Colour*)&COLOUR_OFF, lastValuesWritten, 3);
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

    verify_colours((Colour*)&COLOUR, lastValuesWritten, 3);
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

  verify_colours((Colour*)&COLOUR_ON, lastValuesWritten, 3);
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

  verify_colours((Colour*)&COLOUR_OFF, lastValuesWritten, 3);
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
    ->initialValue(1)
    ->colourOn((Colour*)&COLOUR_ON)
    ->colourOff((Colour*)&COLOUR_OFF);

  led_strip_state_t state = {
    .counter = 0,
  };

  driver->onTimerFired(&state, values);

  verify_colour((Colour*)&COLOUR_ON, lastValuesWritten);
  verify_colours((Colour*)&COLOUR_OFF, &lastValuesWritten[3], 2);
}

TEST(LedStripDriverProgressTestGroup, incrementsProgressValueAfterDelay)
{
  const uint32_t DELAY_MS = 10;

  driver->pattern(Pattern::progress)
    ->incDelay(DELAY_MS);

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
    ->incDelay(DELAY_MS);

  led_strip_state_t state = {
    .counter = DELAY_MS,
    .progress = 0,
  };

  driver->onTimerFired(&state, values);

  LONGS_EQUAL(1, state.counter);
}

TEST(LedStripDriverProgressTestGroup, writesCorrectValueForForwardDirection)
{
  const Colour& COLOUR_ON = COLOUR_RED;
  const Colour& COLOUR_OFF = COLOUR_GREEN;
  const uint32_t PROGRESS = 2;

  driver->pattern(Pattern::progress)
    ->initialValue(0)
    ->progressDirection(Direction::forward)
    ->colourOn((Colour*)&COLOUR_ON)
    ->colourOff((Colour*)&COLOUR_OFF);

  led_strip_state_t state = {
    .counter = 0,
    .progress = PROGRESS,
  };

  driver->onTimerFired(&state, values);

  verify_colour((Colour*)&COLOUR_ON, lastValuesWritten);
  verify_colours((Colour*)&COLOUR_OFF, &lastValuesWritten[PROGRESS * COLOURS_PER_LED], 1);
}

TEST(LedStripDriverProgressTestGroup, resetsProgressAfterFinalValueAndResetDelay)
{
  const Colour& COLOUR_ON = COLOUR_RED;
  const Colour& COLOUR_OFF = COLOUR_GREEN;
  const uint32_t FINAL = 2;
  const uint32_t DELAY_MS = 10;
  const uint32_t RESET_DELAY_MS = 3;

  driver->pattern(Pattern::progress)
    ->finalValue(FINAL)
    ->incDelay(DELAY_MS)
    ->resetDelay(RESET_DELAY_MS)
    ->colourOn((Colour*)&COLOUR_ON)
    ->colourOff((Colour*)&COLOUR_OFF);

  led_strip_state_t state = {
    .counter = DELAY_MS + RESET_DELAY_MS,
    .progress = FINAL,
  };

  driver->onTimerFired(&state, values);

  LONGS_EQUAL(0, state.progress);
}

TEST(LedStripDriverProgressTestGroup, doesNotUpdateProgressAfterAllLedsOnButBeforeResetDelay)
{
  const Colour& COLOUR_ON = COLOUR_RED;
  const Colour& COLOUR_OFF = COLOUR_GREEN;
  const uint32_t INITIAL = 1;
  const uint32_t PROGRESS = CONFIG_LEDS_3.numLeds;
  const uint32_t DELAY_MS = 10;
  const uint32_t RESET_DELAY_MS = 3;

  driver->pattern(Pattern::progress)
    ->initialValue(INITIAL)
    ->incDelay(DELAY_MS)
    ->resetDelay(RESET_DELAY_MS)
    ->progressDirection(Direction::forward)
    ->colourOn((Colour*)&COLOUR_ON)
    ->colourOff((Colour*)&COLOUR_OFF);

  led_strip_state_t state = {
    .counter = DELAY_MS,
    .progress = PROGRESS,
  };

  driver->onTimerFired(&state, values);

  LONGS_EQUAL(PROGRESS, state.progress);
}

TEST(LedStripDriverProgressTestGroup, writesCorrectValueForReverseDirection)
{
  const Colour& COLOUR_ON = COLOUR_RED;
  const Colour& COLOUR_OFF = COLOUR_GREEN;
  const uint32_t PROGRESS = 2;

  driver->pattern(Pattern::progress)
    ->initialValue(0)
    ->progressDirection(Direction::reverse)
    ->colourOn((Colour*)&COLOUR_ON)
    ->colourOff((Colour*)&COLOUR_OFF);

  led_strip_state_t state = {
    .counter = 0,
    .progress = PROGRESS,
  };

  driver->onTimerFired(&state, values);

  verify_colour((Colour*)&COLOUR_OFF, lastValuesWritten);
  verify_colours((Colour*)&COLOUR_ON, &lastValuesWritten[COLOURS_PER_LED], 2);
}

/***********************************************************************************************
 * Gradient pattern
 **********************************************************************************************/
TEST_GROUP(LedStripDriverGradientTestGroup)
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

TEST(LedStripDriverGradientTestGroup, writesCorrectValues)
{
  const Colour& COLOUR_START = COLOUR_RED;
  const Colour& COLOUR_END = COLOUR_GREEN;
  const Colour COLOUR_MID = Colour(128, 127, 0);

  driver->pattern(Pattern::gradient)
    ->colourOn((Colour*)&COLOUR_START)
    ->colourOff((Colour*)&COLOUR_END);

  led_strip_state_t state = {
    .counter = 0,
  };

  driver->onTimerFired(&state, values);

  verify_colour((Colour*)&COLOUR_START, lastValuesWritten);
  verify_colour((Colour*)&COLOUR_MID, &lastValuesWritten[COLOURS_PER_LED]);
  verify_colour((Colour*)&COLOUR_END, &lastValuesWritten[2 * COLOURS_PER_LED]);
}

/***********************************************************************************************
 * Snake pattern
 **********************************************************************************************/
TEST_GROUP(LedStripDriverSnakeTestGroup)
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

TEST(LedStripDriverSnakeTestGroup, writesCorrectValuesForInitialState)
{
  const Colour& COLOUR_ON = COLOUR_RED;
  const Colour& COLOUR_OFF = COLOUR_GREEN;

  driver->pattern(Pattern::snake)
    ->length(2)
    ->snakeDirection(Direction::forward)
    ->colourOn((Colour*)&COLOUR_ON)
    ->colourOff((Colour*)&COLOUR_OFF);

  led_strip_state_t state = {
    .counter = 0,
    .progress = 0,
  };

  driver->onTimerFired(&state, values);

  verify_colours((Colour*)&COLOUR_OFF, lastValuesWritten, 3);
}

TEST(LedStripDriverSnakeTestGroup, incrementsProgressAtCorrectCounterValue)
{
  const Colour& COLOUR_ON = COLOUR_RED;
  const Colour& COLOUR_OFF = COLOUR_GREEN;

  driver->pattern(Pattern::snake)
    ->period(1000)
    ->length(2)
    ->snakeDirection(Direction::forward)
    ->colourOn((Colour*)&COLOUR_ON)
    ->colourOff((Colour*)&COLOUR_OFF);

  led_strip_state_t state = {
    .counter = 200,
    .progress = 0,
  };

  driver->onTimerFired(&state, values);

  LONGS_EQUAL(1, state.progress);
}

TEST(LedStripDriverSnakeTestGroup, doesNotIncrementProgressForEveryCounterValue)
{
  const Colour& COLOUR_ON = COLOUR_RED;
  const Colour& COLOUR_OFF = COLOUR_GREEN;

  driver->pattern(Pattern::snake)
    ->period(1000)
    ->length(2)
    ->snakeDirection(Direction::forward)
    ->colourOn((Colour*)&COLOUR_ON)
    ->colourOff((Colour*)&COLOUR_OFF);

  led_strip_state_t state = {
    .counter = 0,
    .progress = 0,
  };

  driver->onTimerFired(&state, values);

  LONGS_EQUAL(0, state.progress);
}

TEST(LedStripDriverSnakeTestGroup, resetsCounterAtCorrectValue)
{
  const Colour& COLOUR_ON = COLOUR_RED;
  const Colour& COLOUR_OFF = COLOUR_GREEN;

  driver->pattern(Pattern::snake)
    ->period(1000)
    ->length(2)
    ->snakeDirection(Direction::forward)
    ->colourOn((Colour*)&COLOUR_ON)
    ->colourOff((Colour*)&COLOUR_OFF);

  led_strip_state_t state = {
    .counter = 200,
    .progress = 0,
  };

  driver->onTimerFired(&state, values);

  LONGS_EQUAL(1, state.counter);
}

TEST(LedStripDriverSnakeTestGroup, writesCorrectValuesForForward)
{
  const Colour& COLOUR_ON = COLOUR_RED;
  const Colour& COLOUR_OFF = COLOUR_GREEN;

  driver->pattern(Pattern::snake)
    ->period(1000)
    ->length(2)
    ->snakeDirection(Direction::forward)
    ->colourOn((Colour*)&COLOUR_ON)
    ->colourOff((Colour*)&COLOUR_OFF);

  led_strip_state_t state = {
    .counter = 0,
    .progress = 2,
  };

  driver->onTimerFired(&state, values);

  verify_colours((Colour*)&COLOUR_ON, lastValuesWritten, 2);
  verify_colours((Colour*)&COLOUR_OFF, lastValuesWritten, 1, 2);
}

TEST(LedStripDriverSnakeTestGroup, writesCorrectValuesForReverse)
{
  const Colour& COLOUR_ON = COLOUR_RED;
  const Colour& COLOUR_OFF = COLOUR_GREEN;

  driver->pattern(Pattern::snake)
    ->period(1000)
    ->length(2)
    ->snakeDirection(Direction::reverse)
    ->colourOn((Colour*)&COLOUR_ON)
    ->colourOff((Colour*)&COLOUR_OFF);

  led_strip_state_t state = {
    .counter = 0,
    .progress = 2,
  };

  driver->onTimerFired(&state, values);

  verify_colours((Colour*)&COLOUR_OFF, lastValuesWritten, 1);
  verify_colours((Colour*)&COLOUR_ON, lastValuesWritten, 2, 1);
}

TEST(LedStripDriverSnakeTestGroup, writesCorrectValuesWhenProgressLessThanLengthForward)
{
  const Colour& COLOUR_ON = COLOUR_RED;
  const Colour& COLOUR_OFF = COLOUR_GREEN;

  driver->pattern(Pattern::snake)
    ->period(1000)
    ->length(4)
    ->snakeDirection(Direction::forward)
    ->colourOn((Colour*)&COLOUR_ON)
    ->colourOff((Colour*)&COLOUR_OFF);

  led_strip_state_t state = {
    .counter = 0,
    .progress = 2,
  };

  driver->onTimerFired(&state, values);

  verify_colours((Colour*)&COLOUR_ON, lastValuesWritten, 2);
  verify_colours((Colour*)&COLOUR_OFF, lastValuesWritten, 1, 2);
}

TEST(LedStripDriverSnakeTestGroup, writesCorrectValuesWhenProgressMoreThanNumLedsReverse)
{
  const Colour& COLOUR_ON = COLOUR_RED;
  const Colour& COLOUR_OFF = COLOUR_GREEN;

  driver->pattern(Pattern::snake)
    ->period(1000)
    ->length(2)
    ->snakeDirection(Direction::reverse)
    ->colourOn((Colour*)&COLOUR_ON)
    ->colourOff((Colour*)&COLOUR_OFF);

  led_strip_state_t state = {
    .counter = 0,
    .progress = 4,
  };

  driver->onTimerFired(&state, values);

  verify_colours((Colour*)&COLOUR_ON, lastValuesWritten, 1);
  verify_colours((Colour*)&COLOUR_OFF, lastValuesWritten, 2, 1);
}

TEST(LedStripDriverSnakeTestGroup, resetsProgressAfterSnakeNoLongerVisible)
{
  const Colour& COLOUR_ON = COLOUR_RED;
  const Colour& COLOUR_OFF = COLOUR_GREEN;
  const uint32_t LENGTH = 2;

  driver->pattern(Pattern::snake)
    ->period(1000)
    ->length(LENGTH)
    ->snakeDirection(Direction::forward)
    ->colourOn((Colour*)&COLOUR_ON)
    ->colourOff((Colour*)&COLOUR_OFF);

  led_strip_state_t state = {
    .counter = 200,
    .progress = CONFIG_LEDS_3.numLeds + LENGTH,
  };

  driver->onTimerFired(&state, values);

  LONGS_EQUAL(0, state.progress);
}
