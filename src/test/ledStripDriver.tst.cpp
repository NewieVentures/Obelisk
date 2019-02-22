#include <CppUTest/TestHarness.h>
#include <CppUTestExt/MockSupport.h>
#include <cstring>

#include "StringFrom.h"
#include "colour.h"
#include "colours.h"
#include "ledStripDriver.h"
#include "config.h"

#define MAX_LEDS 10

const Colour COLOUR_ON = COLOUR_RED;
const Colour COLOUR_OFF = COLOUR_GREEN;

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

static const led_strip_config_t CONFIG_LEDS_6 = {
  .numLeds = 6,
  .writeValueFn = writeValueStub,
  .resolutionMs = 1,
};

#define WEATHER_TEST_LED_CONFIG CONFIG_LEDS_6

static void verify_colours(Colour *expected,
                           uint8_t *values,
                           uint32_t len,
                           uint32_t offset = 0) {
  for (uint32_t i=offset; i<len; i++) {
    uint32_t index = i * 3;
    Colour actual = Colour(values[index+INDEX_RED],
                           values[index+INDEX_GREEN],
                           values[index+INDEX_BLUE]);
    String message;

    message = message.format("Incorrect colour at index %d, expected: %s, actual: %s ",
                             index,
                             expected->toString().c_str(),
                             actual.toString().c_str());

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

TEST(LedStripDriverPulseTestGroup, writesCorrectInitialValues)
{
  const Colour& COLOUR_START = COLOUR_RED;
  const Colour& COLOUR_END = COLOUR_GREEN;

  driver->pattern(Pattern::pulse)
    ->period(3)
    ->colourOn((Colour*)&COLOUR_START)
    ->colourOff((Colour*)&COLOUR_END);

  led_strip_state_t state = {
    .counter = 0,
    .dutyDirection = 1,
  };

  driver->onTimerFired(&state, values);

  verify_colours((Colour*)&COLOUR_START, lastValuesWritten, CONFIG_LEDS_3.numLeds);
}

TEST(LedStripDriverPulseTestGroup, writesCorrectStepValues)
{
  const Colour& COLOUR_START = COLOUR_RED;
  const Colour& COLOUR_END = COLOUR_GREEN;
  const Colour COLOUR_MID = Colour(127, 127, 0);

  driver->pattern(Pattern::pulse)
    ->period(3000)
    ->colourOn((Colour*)&COLOUR_START)
    ->colourOff((Colour*)&COLOUR_END);

  led_strip_state_t state = {
    .counter = 1500,
    .dutyDirection = 1,
  };

  driver->onTimerFired(&state, values);

  verify_colours((Colour*)&COLOUR_MID, lastValuesWritten, CONFIG_LEDS_3.numLeds);
}

TEST(LedStripDriverPulseTestGroup, writesCorrectFinalValues)
{
  const Colour& COLOUR_START = COLOUR_RED;
  const Colour& COLOUR_END = COLOUR_GREEN;

  driver->pattern(Pattern::pulse)
    ->period(3)
    ->colourOn((Colour*)&COLOUR_START)
    ->colourOff((Colour*)&COLOUR_END);

  led_strip_state_t state = {
    .counter = 2,
    .dutyDirection = 1,
  };

  driver->onTimerFired(&state, values);

  verify_colours((Colour*)&COLOUR_END, lastValuesWritten, CONFIG_LEDS_3.numLeds);
}

TEST(LedStripDriverPulseTestGroup, resetsCounterAndReversesDirection)
{
  const Colour& COLOUR_START = COLOUR_RED;
  const Colour& COLOUR_END = COLOUR_GREEN;

  driver->pattern(Pattern::pulse)
    ->period(3)
    ->colourOn((Colour*)&COLOUR_START)
    ->colourOff((Colour*)&COLOUR_END);

  led_strip_state_t state = {
    .counter = 3,
    .dutyDirection = 1,
  };

  driver->onTimerFired(&state, values);

  LONGS_EQUAL(1, state.counter);
  LONGS_EQUAL(-1, state.dutyDirection);
}

TEST(LedStripDriverPulseTestGroup, writesCorrectValuesForReverseDirection)
{
  const Colour& COLOUR_START = COLOUR_RED;
  const Colour& COLOUR_END = COLOUR_GREEN;

  driver->pattern(Pattern::pulse)
    ->period(3000)
    ->colourOn((Colour*)&COLOUR_START)
    ->colourOff((Colour*)&COLOUR_END);

  led_strip_state_t state = {
    .counter = 0,
    .dutyDirection = -1,
  };

  driver->onTimerFired(&state, values);

  verify_colours((Colour*)&COLOUR_END, lastValuesWritten, CONFIG_LEDS_3.numLeds);
}

TEST(LedStripDriverPulseTestGroup, writesCorrectValuesWhenDirectionReversed)
{
  const Colour& COLOUR_START = COLOUR_RED;
  const Colour& COLOUR_END = COLOUR_GREEN;

  driver->pattern(Pattern::pulse)
    ->period(3000)
    ->colourOn((Colour*)&COLOUR_START)
    ->colourOff((Colour*)&COLOUR_END);

  led_strip_state_t state = {
    .counter = 3000,
    .dutyDirection = 1,
  };

  driver->onTimerFired(&state, values);

  verify_colours((Colour*)&COLOUR_END, lastValuesWritten, CONFIG_LEDS_3.numLeds);
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
  driver->pattern(Pattern::blink)
    ->period(10)
    ->dutyCycle(50)
    ->colourOn((Colour*)&COLOUR_ON)
    ->colourOff((Colour*)&COLOUR_OFF);

  led_strip_state_t state = {
    .counter = 0,
  };

  driver->onTimerFired(&state, values);

  verify_colours((Colour*)&COLOUR_ON, lastValuesWritten, 3);
}

TEST(LedStripDriverBlinkTestGroup, writesOffValueForAllLedsWithCorrectColourAfterPeriod)
{
  driver->pattern(Pattern::blink)
    ->period(10)
    ->dutyCycle(50)
    ->colourOn((Colour*)&COLOUR_ON)
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
    ->colourOn((Colour*)&COLOUR_ON)
    ->colourOff((Colour*)&COLOUR_OFF);

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

TEST(LedStripDriverProgressTestGroup, allLedsOffIfFinalValueLessThanMaxLeds)
{
  const uint32_t FINAL = 1;
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
    .progress = 0,
  };

  driver->onTimerFired(&state, values);

  verify_colours((Colour*)&COLOUR_OFF, &lastValuesWritten[0], 3);
}

TEST(LedStripDriverProgressTestGroup, incrementsByGivenAmount)
{
  const uint32_t INCREMENT = 2;
  const uint32_t INCREMENT_DELAY = 100;

  driver->pattern(Pattern::progress)
    ->initialValue(0)
    ->increment(INCREMENT)
    ->incDelay(INCREMENT_DELAY)
    ->progressDirection(Direction::forward)
    ->colourOn((Colour*)&COLOUR_ON)
    ->colourOff((Colour*)&COLOUR_OFF);

  led_strip_state_t state = {
    .counter = INCREMENT_DELAY,
    .progress = 0,
  };

  driver->onTimerFired(&state, values);

  LONGS_EQUAL(INCREMENT, state.progress);
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

/***********************************************************************************************
 * Weather pattern
 **********************************************************************************************/
TEST_GROUP(LedStripDriverWeatherTestGroup)
{
  void setup() {
    const uint32_t valuesLength = WEATHER_TEST_LED_CONFIG.numLeds * COLOURS_PER_LED;

    driver = new LedStripDriver((led_strip_config_t*)&WEATHER_TEST_LED_CONFIG);
    lastValuesWritten = new uint8_t[valuesLength];
    memset(lastValuesWritten, 0, valuesLength);
    memset(values, 0, MAX_LEDS * COLOURS_PER_LED);
  }

  void teardown() {
    delete driver;
    delete[] lastValuesWritten;
  }
};

TEST(LedStripDriverWeatherTestGroup, writesCorrectInitialValuesForTemperatureLayerForwardDirection)
{
  const Colour& COLOUR_START = COLOUR_RED;
  const Colour& COLOUR_END = COLOUR_GREEN;
  const uint32_t TEMPERATURE_FADE_INTERVAL_SECS = 4;

  driver->pattern(Pattern::weather)
    ->colourOn((Colour*)&COLOUR_START)
    ->colourOff((Colour*)&COLOUR_END)
    ->tempFadeInterval(TEMPERATURE_FADE_INTERVAL_SECS);

  led_strip_state_t state = {
    .counter = 0,
    .weatherTempFadeDirection = 1,
  };

  driver->onTimerFired(&state, values);

  verify_colours((Colour*)&COLOUR_ON, lastValuesWritten, WEATHER_TEST_LED_CONFIG.numLeds);
}

TEST(LedStripDriverWeatherTestGroup, writesCorrectMidValuesForTemperatureLayerForwardDirection)
{
  const Colour& COLOUR_START = COLOUR_RED;
  const Colour& COLOUR_END = COLOUR_GREEN;
  const Colour& COLOUR_MID = Colour(127, 127, 0);
  const uint32_t TEMPERATURE_FADE_INTERVAL_SECS = 4;

  driver->pattern(Pattern::weather)
    ->colourOn((Colour*)&COLOUR_START)
    ->colourOff((Colour*)&COLOUR_END)
    ->tempFadeInterval(TEMPERATURE_FADE_INTERVAL_SECS);

  led_strip_state_t state = {
    .counter = (TEMPERATURE_FADE_INTERVAL_SECS * 1000) / 2,
    .weatherTempFadeDirection = 1,
  };

  driver->onTimerFired(&state, values);

  verify_colours((Colour*)&COLOUR_MID, lastValuesWritten, WEATHER_TEST_LED_CONFIG.numLeds);
}

TEST(LedStripDriverWeatherTestGroup, writesCorrectEndValuesForTemperatureLayerForwardDirection)
{
  const Colour& COLOUR_START = COLOUR_RED;
  const Colour& COLOUR_END = COLOUR_GREEN;
  const uint32_t TEMPERATURE_FADE_INTERVAL_SECS = 4;

  driver->pattern(Pattern::weather)
    ->colourOn((Colour*)&COLOUR_START)
    ->colourOff((Colour*)&COLOUR_END)
    ->tempFadeInterval(TEMPERATURE_FADE_INTERVAL_SECS);

  led_strip_state_t state = {
    .counter = (TEMPERATURE_FADE_INTERVAL_SECS * 1000) - WEATHER_TEST_LED_CONFIG.resolutionMs,
    .weatherTempFadeDirection = 1,
  };

  driver->onTimerFired(&state, values);

  verify_colours((Colour*)&COLOUR_END, lastValuesWritten, WEATHER_TEST_LED_CONFIG.numLeds);
}

TEST(LedStripDriverWeatherTestGroup, resetsCounter)
{
  const Colour& COLOUR_START = COLOUR_RED;
  const Colour& COLOUR_END = COLOUR_GREEN;
  const uint32_t TEMPERATURE_FADE_INTERVAL_SECS = 4;

  driver->pattern(Pattern::weather)
    ->colourOn((Colour*)&COLOUR_START)
    ->colourOff((Colour*)&COLOUR_END)
    ->tempFadeInterval(TEMPERATURE_FADE_INTERVAL_SECS);

  led_strip_state_t state = {
    .counter = (TEMPERATURE_FADE_INTERVAL_SECS * 1000),
    .weatherTempFadeDirection = 1,
  };

  driver->onTimerFired(&state, values);

  LONGS_EQUAL(1, state.counter);
}

TEST(LedStripDriverWeatherTestGroup, changesFadeDirectionWhenCounterReset)
{
  const Colour& COLOUR_START = COLOUR_RED;
  const Colour& COLOUR_END = COLOUR_GREEN;
  const uint32_t TEMPERATURE_FADE_INTERVAL_SECS = 4;

  driver->pattern(Pattern::weather)
    ->colourOn((Colour*)&COLOUR_START)
    ->colourOff((Colour*)&COLOUR_END)
    ->tempFadeInterval(TEMPERATURE_FADE_INTERVAL_SECS);

  led_strip_state_t state = {
    .counter = (TEMPERATURE_FADE_INTERVAL_SECS * 1000),
    .weatherTempFadeDirection = -1,
  };

  driver->onTimerFired(&state, values);

  LONGS_EQUAL(1, state.weatherTempFadeDirection);
}

TEST(LedStripDriverWeatherTestGroup, writesCorrectValuesForEndTemperatureLayerBackwardDirection)
{
  const Colour& COLOUR_START = COLOUR_RED;
  const Colour& COLOUR_END = COLOUR_GREEN;
  const uint32_t TEMPERATURE_FADE_INTERVAL_SECS = 4;

  driver->pattern(Pattern::weather)
    ->colourOn((Colour*)&COLOUR_START)
    ->colourOff((Colour*)&COLOUR_END)
    ->tempFadeInterval(TEMPERATURE_FADE_INTERVAL_SECS);

  led_strip_state_t state = {
    .counter = (TEMPERATURE_FADE_INTERVAL_SECS * 1000),
    .weatherTempFadeDirection = -1,
    .weatherRainCounter = 0,
    .weatherRainPosition = 0,

    .weatherWarningCounter = 0,
    .weatherWarningFadeState = fadeIn,
  };

  driver->onTimerFired(&state, values);

  verify_colours((Colour*)&COLOUR_START, lastValuesWritten, WEATHER_TEST_LED_CONFIG.numLeds);
}

TEST(LedStripDriverWeatherTestGroup, writesCorrectValuesForInitialRainCounter)
{
  const Colour& COLOUR_START = COLOUR_RED;
  const Colour& COLOUR_END = COLOUR_GREEN;
  const Colour& COLOUR_RAIN = COLOUR_WHITE;
  const uint32_t TEMPERATURE_FADE_INTERVAL_SECS = 4;

  driver->pattern(Pattern::weather)
    ->colourOn((Colour*)&COLOUR_START)
    ->colourOff((Colour*)&COLOUR_END)
    ->tempFadeInterval((uint32_t)TEMPERATURE_FADE_INTERVAL_SECS)
    ->rainBandHeight(1)
    ->rainBandSpacing(1)
    ->rainBandIncrementDelay(500)
    ->rainBandColour((Colour*)&COLOUR_RAIN);

  led_strip_state_t state = {
    .counter = 0,
    .weatherTempFadeDirection = 1,
    .weatherRainCounter = 0,
    .weatherRainPosition = 0,
  };

  driver->onTimerFired(&state, values);

  verify_colour((Colour*)&COLOUR_RAIN, &lastValuesWritten[0]);
  verify_colour((Colour*)&COLOUR_START, &lastValuesWritten[3]);
  verify_colour((Colour*)&COLOUR_RAIN, &lastValuesWritten[6]);
  verify_colour((Colour*)&COLOUR_START, &lastValuesWritten[9]);
  verify_colour((Colour*)&COLOUR_RAIN, &lastValuesWritten[12]);
  verify_colour((Colour*)&COLOUR_START, &lastValuesWritten[15]);
}

TEST(LedStripDriverWeatherTestGroup, incrementsRainCounterCorrectly)
{
  const Colour& COLOUR_START = COLOUR_RED;
  const Colour& COLOUR_END = COLOUR_GREEN;
  const Colour& COLOUR_RAIN = COLOUR_WHITE;
  const uint32_t TEMPERATURE_FADE_INTERVAL_SECS = 4;
  const uint32_t RAIN_INC_DELAY_MS = 500;

  driver->pattern(Pattern::weather)
    ->colourOn((Colour*)&COLOUR_START)
    ->colourOff((Colour*)&COLOUR_END)
    ->tempFadeInterval(TEMPERATURE_FADE_INTERVAL_SECS)
    ->rainBandHeight(1)
    ->rainBandSpacing(1)
    ->rainBandIncrementDelay(RAIN_INC_DELAY_MS)
    ->rainBandColour((Colour*)&COLOUR_RAIN);

  led_strip_state_t state = {
    .counter = 0,
    .weatherTempFadeDirection = 1,
    .weatherRainCounter = 0,
    .weatherRainPosition = 0,
  };

  driver->onTimerFired(&state, values);

  LONGS_EQUAL(WEATHER_TEST_LED_CONFIG.resolutionMs, state.weatherRainCounter);
}

TEST(LedStripDriverWeatherTestGroup, writesCorrectValuesForIncrementedRainPosition)
{
  const Colour& COLOUR_START = COLOUR_RED;
  const Colour& COLOUR_END = COLOUR_GREEN;
  const Colour& COLOUR_RAIN = COLOUR_WHITE;
  const uint32_t TEMPERATURE_FADE_INTERVAL_SECS = 4;
  const uint32_t RAIN_INC_DELAY_MS = 500;

  driver->pattern(Pattern::weather)
    ->colourOn((Colour*)&COLOUR_START)
    ->colourOff((Colour*)&COLOUR_END)
    ->tempFadeInterval(TEMPERATURE_FADE_INTERVAL_SECS)
    ->rainBandHeight(1)
    ->rainBandSpacing(1)
    ->rainBandIncrementDelay(RAIN_INC_DELAY_MS)
    ->rainBandColour((Colour*)&COLOUR_RAIN);

  led_strip_state_t state = {
    .counter = 0,
    .weatherTempFadeDirection = 1,
    .weatherRainCounter = 0,
    .weatherRainPosition = 1,
  };

  driver->onTimerFired(&state, values);

  verify_colour((Colour*)&COLOUR_START, &lastValuesWritten[0]);
  verify_colour((Colour*)&COLOUR_RAIN, &lastValuesWritten[3]);
  verify_colour((Colour*)&COLOUR_START, &lastValuesWritten[6]);
  verify_colour((Colour*)&COLOUR_RAIN, &lastValuesWritten[9]);
  verify_colour((Colour*)&COLOUR_START, &lastValuesWritten[12]);
  verify_colour((Colour*)&COLOUR_RAIN, &lastValuesWritten[15]);
}

TEST(LedStripDriverWeatherTestGroup, incrementsRainPositionCorrectly)
{
  const Colour& COLOUR_START = COLOUR_RED;
  const Colour& COLOUR_END = COLOUR_GREEN;
  const Colour& COLOUR_RAIN = COLOUR_WHITE;
  const uint32_t TEMPERATURE_FADE_INTERVAL_SECS = 4;
  const uint32_t RAIN_INC_DELAY_MS = 500;

  driver->pattern(Pattern::weather)
    ->colourOn((Colour*)&COLOUR_START)
    ->colourOff((Colour*)&COLOUR_END)
    ->tempFadeInterval(TEMPERATURE_FADE_INTERVAL_SECS)
    ->rainBandHeight(1)
    ->rainBandSpacing(1)
    ->rainBandIncrementDelay(RAIN_INC_DELAY_MS)
    ->rainBandColour((Colour*)&COLOUR_RAIN);

  led_strip_state_t state = {
    .counter = 0,
    .weatherTempFadeDirection = 1,
    .weatherRainCounter = RAIN_INC_DELAY_MS - 1,
    .weatherRainPosition = 0,
  };

  driver->onTimerFired(&state, values);

  LONGS_EQUAL(1, state.weatherRainPosition);
}

TEST(LedStripDriverWeatherTestGroup, resetsRainCounterCorrectly)
{
  const Colour& COLOUR_START = COLOUR_RED;
  const Colour& COLOUR_END = COLOUR_GREEN;
  const Colour& COLOUR_RAIN = COLOUR_WHITE;
  const uint32_t TEMPERATURE_FADE_INTERVAL_SECS = 4;
  const uint32_t RAIN_INC_DELAY_MS = 500;

  driver->pattern(Pattern::weather)
    ->colourOn((Colour*)&COLOUR_START)
    ->colourOff((Colour*)&COLOUR_END)
    ->tempFadeInterval(TEMPERATURE_FADE_INTERVAL_SECS)
    ->rainBandHeight(1)
    ->rainBandSpacing(1)
    ->rainBandIncrementDelay(RAIN_INC_DELAY_MS)
    ->rainBandColour((Colour*)&COLOUR_RAIN);

  led_strip_state_t state = {
    .counter = 0,
    .weatherTempFadeDirection = 1,
    .weatherRainCounter = RAIN_INC_DELAY_MS - 1,
    .weatherRainPosition = 0,
  };

  driver->onTimerFired(&state, values);

  LONGS_EQUAL(0, state.weatherRainCounter);
}

TEST(LedStripDriverWeatherTestGroup, resetsRainPositionCorrectly)
{
  const Colour& COLOUR_START = COLOUR_RED;
  const Colour& COLOUR_END = COLOUR_GREEN;
  const Colour& COLOUR_RAIN = COLOUR_WHITE;
  const uint32_t TEMPERATURE_FADE_INTERVAL_SECS = 4;
  const uint32_t RAIN_INC_DELAY_MS = 500;

  driver->pattern(Pattern::weather)
    ->colourOn((Colour*)&COLOUR_START)
    ->colourOff((Colour*)&COLOUR_END)
    ->tempFadeInterval(TEMPERATURE_FADE_INTERVAL_SECS)
    ->rainBandHeight(1)
    ->rainBandSpacing(1)
    ->rainBandIncrementDelay(RAIN_INC_DELAY_MS)
    ->rainBandColour((Colour*)&COLOUR_RAIN);

  led_strip_state_t state = {
    .counter = 0,
    .weatherTempFadeDirection = 1,
    .weatherRainCounter = RAIN_INC_DELAY_MS - 1,
    .weatherRainPosition = (uint8_t)WEATHER_TEST_LED_CONFIG.numLeds - 1,
  };

  driver->onTimerFired(&state, values);

  LONGS_EQUAL(0, state.weatherRainPosition);
}

TEST(LedStripDriverWeatherTestGroup, writesCorrectValuesWhenRainBandsWrapAround)
{
  const Colour& COLOUR_START = COLOUR_RED;
  const Colour& COLOUR_END = COLOUR_GREEN;
  const Colour& COLOUR_RAIN = COLOUR_WHITE;
  const uint32_t TEMPERATURE_FADE_INTERVAL_SECS = 4;
  const uint32_t RAIN_INC_DELAY_MS = 500;

  driver->pattern(Pattern::weather)
    ->colourOn((Colour*)&COLOUR_START)
    ->colourOff((Colour*)&COLOUR_END)
    ->tempFadeInterval(TEMPERATURE_FADE_INTERVAL_SECS)
    ->rainBandHeight(1)
    ->rainBandSpacing(1)
    ->rainBandIncrementDelay(RAIN_INC_DELAY_MS)
    ->rainBandColour((Colour*)&COLOUR_RAIN);

  led_strip_state_t state = {
    .counter = 0,
    .weatherTempFadeDirection = 1,
    .weatherRainCounter = 0,
    .weatherRainPosition = 2,
  };

  driver->onTimerFired(&state, values);

  verify_colour((Colour*)&COLOUR_RAIN, &lastValuesWritten[0]);
  verify_colour((Colour*)&COLOUR_START, &lastValuesWritten[3]);
  verify_colour((Colour*)&COLOUR_RAIN, &lastValuesWritten[6]);
  verify_colour((Colour*)&COLOUR_START, &lastValuesWritten[9]);
  verify_colour((Colour*)&COLOUR_RAIN, &lastValuesWritten[12]);
  verify_colour((Colour*)&COLOUR_START, &lastValuesWritten[15]);
}

TEST(LedStripDriverWeatherTestGroup, writesCorrectValuesForRainBandsReverseDirection)
{
  const Colour& COLOUR_START = COLOUR_RED;
  const Colour& COLOUR_END = COLOUR_GREEN;
  const Colour& COLOUR_RAIN = COLOUR_WHITE;
  const uint32_t TEMPERATURE_FADE_INTERVAL_SECS = 4;
  const uint32_t RAIN_INC_DELAY_MS = 500;

  driver->pattern(Pattern::weather)
    ->colourOn((Colour*)&COLOUR_START)
    ->colourOff((Colour*)&COLOUR_END)
    ->tempFadeInterval(TEMPERATURE_FADE_INTERVAL_SECS)
    ->rainBandHeight(1)
    ->rainBandSpacing(2)
    ->rainDirection(Direction::reverse)
    ->rainBandIncrementDelay(RAIN_INC_DELAY_MS)
    ->rainBandColour((Colour*)&COLOUR_RAIN);

  led_strip_state_t state = {
    .counter = 0,
    .weatherTempFadeDirection = 1,
    .weatherRainCounter = 0,
    .weatherRainPosition = 0,
  };

  driver->onTimerFired(&state, values);

  verify_colour((Colour*)&COLOUR_START, &lastValuesWritten[0]);
  verify_colour((Colour*)&COLOUR_START, &lastValuesWritten[3]);
  verify_colour((Colour*)&COLOUR_RAIN, &lastValuesWritten[6]);
  verify_colour((Colour*)&COLOUR_START, &lastValuesWritten[9]);
  verify_colour((Colour*)&COLOUR_START, &lastValuesWritten[12]);
  verify_colour((Colour*)&COLOUR_RAIN, &lastValuesWritten[15]);
}

TEST(LedStripDriverWeatherTestGroup, writesCorrectValuesForRainBandsIncrementedReverseDirection)
{
  const Colour& COLOUR_START = COLOUR_RED;
  const Colour& COLOUR_END = COLOUR_GREEN;
  const Colour& COLOUR_RAIN = COLOUR_WHITE;
  const uint32_t TEMPERATURE_FADE_INTERVAL_SECS = 4;
  const uint32_t RAIN_INC_DELAY_MS = 500;

  driver->pattern(Pattern::weather)
    ->colourOn((Colour*)&COLOUR_START)
    ->colourOff((Colour*)&COLOUR_END)
    ->tempFadeInterval(TEMPERATURE_FADE_INTERVAL_SECS)
    ->rainBandHeight(1)
    ->rainBandSpacing(2)
    ->rainDirection(Direction::reverse)
    ->rainBandIncrementDelay(RAIN_INC_DELAY_MS)
    ->rainBandColour((Colour*)&COLOUR_RAIN);

  led_strip_state_t state = {
    .counter = 0,
    .weatherTempFadeDirection = 1,
    .weatherRainCounter = 0,
    .weatherRainPosition = 1,
  };

  driver->onTimerFired(&state, values);

  verify_colour((Colour*)&COLOUR_START, &lastValuesWritten[0]);
  verify_colour((Colour*)&COLOUR_RAIN, &lastValuesWritten[3]);
  verify_colour((Colour*)&COLOUR_START, &lastValuesWritten[6]);
  verify_colour((Colour*)&COLOUR_START, &lastValuesWritten[9]);
  verify_colour((Colour*)&COLOUR_RAIN, &lastValuesWritten[12]);
  verify_colour((Colour*)&COLOUR_START, &lastValuesWritten[15]);
}

TEST(LedStripDriverWeatherTestGroup, usesCorrectColourForRainBands)
{
  const Colour& COLOUR_START = COLOUR_RED;
  const Colour& COLOUR_END = COLOUR_GREEN;
  const Colour& COLOUR_RAIN = COLOUR_BLUE;
  const uint32_t TEMPERATURE_FADE_INTERVAL_SECS = 4;
  const uint32_t RAIN_INC_DELAY_MS = 500;

  driver->pattern(Pattern::weather)
    ->colourOn((Colour*)&COLOUR_START)
    ->colourOff((Colour*)&COLOUR_END)
    ->tempFadeInterval(TEMPERATURE_FADE_INTERVAL_SECS)
    ->rainBandHeight(1)
    ->rainBandSpacing(1)
    ->rainBandIncrementDelay(RAIN_INC_DELAY_MS)
    ->rainBandColour((Colour*)&COLOUR_RAIN);

  led_strip_state_t state = {
    .counter = 0,
    .weatherTempFadeDirection = 1,
    .weatherRainCounter = 0,
    .weatherRainPosition = 0,
  };

  driver->onTimerFired(&state, values);

  verify_colour((Colour*)&COLOUR_RAIN, &lastValuesWritten[0]);
}

TEST(LedStripDriverWeatherTestGroup, writesCorrectValuesForIncreasedRainBandHeight)
{
  const Colour& COLOUR_START = COLOUR_RED;
  const Colour& COLOUR_END = COLOUR_GREEN;
  const Colour& COLOUR_RAIN = COLOUR_WHITE;
  const uint32_t TEMPERATURE_FADE_INTERVAL_SECS = 4;
  const uint32_t RAIN_INC_DELAY_MS = 500;
  const uint8_t RAIN_BAND_HEIGHT_LEDS = 3;

  driver->pattern(Pattern::weather)
    ->colourOn((Colour*)&COLOUR_START)
    ->colourOff((Colour*)&COLOUR_END)
    ->tempFadeInterval(TEMPERATURE_FADE_INTERVAL_SECS)
    ->rainBandHeight(RAIN_BAND_HEIGHT_LEDS)
    ->rainBandSpacing(1)
    ->rainBandIncrementDelay(RAIN_INC_DELAY_MS)
    ->rainBandColour((Colour*)&COLOUR_RAIN);

  led_strip_state_t state = {
    .counter = 0,
    .weatherTempFadeDirection = 1,
    .weatherRainCounter = 0,
    .weatherRainPosition = 0,
  };

  driver->onTimerFired(&state, values);

  verify_colour((Colour*)&COLOUR_RAIN, &lastValuesWritten[0]);
  verify_colour((Colour*)&COLOUR_RAIN, &lastValuesWritten[3]);
  verify_colour((Colour*)&COLOUR_RAIN, &lastValuesWritten[6]);
  verify_colour((Colour*)&COLOUR_START, &lastValuesWritten[9]);
  verify_colour((Colour*)&COLOUR_RAIN, &lastValuesWritten[12]);
  verify_colour((Colour*)&COLOUR_RAIN, &lastValuesWritten[15]);
}

TEST(LedStripDriverWeatherTestGroup, writesCorrectValuesForIncreasedRainBandSpacing)
{
  const Colour& COLOUR_START = COLOUR_RED;
  const Colour& COLOUR_END = COLOUR_GREEN;
  const Colour& COLOUR_RAIN = COLOUR_WHITE;
  const uint32_t TEMPERATURE_FADE_INTERVAL_SECS = 4;
  const uint32_t RAIN_INC_DELAY_MS = 500;
  const uint8_t RAIN_BAND_HEIGHT_LEDS = 1;
  const uint8_t RAIN_BAND_SPACING_LEDS = 3;

  driver->pattern(Pattern::weather)
    ->colourOn((Colour*)&COLOUR_START)
    ->colourOff((Colour*)&COLOUR_END)
    ->tempFadeInterval(TEMPERATURE_FADE_INTERVAL_SECS)
    ->rainBandHeight(RAIN_BAND_HEIGHT_LEDS)
    ->rainBandSpacing(RAIN_BAND_SPACING_LEDS)
    ->rainBandIncrementDelay(RAIN_INC_DELAY_MS)
    ->rainBandColour((Colour*)&COLOUR_RAIN);

  led_strip_state_t state = {
    .counter = 0,
    .weatherTempFadeDirection = 1,
    .weatherRainCounter = 0,
    .weatherRainPosition = 0,
  };

  driver->onTimerFired(&state, values);

  verify_colour((Colour*)&COLOUR_RAIN, &lastValuesWritten[0]);
  verify_colour((Colour*)&COLOUR_START, &lastValuesWritten[3]);
  verify_colour((Colour*)&COLOUR_START, &lastValuesWritten[6]);
  verify_colour((Colour*)&COLOUR_START, &lastValuesWritten[9]);
  verify_colour((Colour*)&COLOUR_RAIN, &lastValuesWritten[12]);
  verify_colour((Colour*)&COLOUR_START, &lastValuesWritten[15]);
}

TEST(LedStripDriverWeatherTestGroup, doesNotIncrementsRainPositionIfCounterLessThanIncDelay)
{
  const Colour& COLOUR_START = COLOUR_RED;
  const Colour& COLOUR_END = COLOUR_GREEN;
  const Colour& COLOUR_RAIN = COLOUR_WHITE;
  const uint32_t TEMPERATURE_FADE_INTERVAL_SECS = 4;
  const uint32_t RAIN_INC_DELAY_MS = 500;

  driver->pattern(Pattern::weather)
    ->colourOn((Colour*)&COLOUR_START)
    ->colourOff((Colour*)&COLOUR_END)
    ->tempFadeInterval(TEMPERATURE_FADE_INTERVAL_SECS)
    ->rainBandHeight(1)
    ->rainBandSpacing(1)
    ->rainBandIncrementDelay(RAIN_INC_DELAY_MS)
    ->rainBandColour((Colour*)&COLOUR_RAIN);

  led_strip_state_t state = {
    .counter = 0,
    .weatherTempFadeDirection = 1,
    .weatherRainCounter = RAIN_INC_DELAY_MS - 2,
    .weatherRainPosition = 0,
  };

  driver->onTimerFired(&state, values);

  LONGS_EQUAL(0, state.weatherRainPosition);
}

TEST(LedStripDriverWeatherTestGroup, writesCorrectValuesForWeatherWarningInitialState)
{
  const Colour& COLOUR_START = COLOUR_RED;
  const Colour& COLOUR_END = COLOUR_GREEN;
  const Colour& COLOUR_WARNING = COLOUR_BLUE;
  const uint32_t TEMPERATURE_FADE_INTERVAL_SECS = 4;
  const uint32_t WARNING_FADE_IN_MS = 500;
  const uint32_t WARNING_FADE_OUT_MS = 2000;
  const uint32_t WARNING_OFF_DWELL_MS = 1000;

  driver->pattern(Pattern::weather)
    ->colourOn((Colour*)&COLOUR_START)
    ->colourOff((Colour*)&COLOUR_END)
    ->tempFadeInterval(TEMPERATURE_FADE_INTERVAL_SECS)
    ->warningColour((Colour*)&COLOUR_WARNING)
    ->warningFadeIn(WARNING_FADE_IN_MS)
    ->warningFadeOut(WARNING_FADE_OUT_MS)
    ->warningOffDwell(WARNING_OFF_DWELL_MS);

  led_strip_state_t state = {
    .counter = 0,
    .weatherTempFadeDirection = 1,
    .weatherRainCounter = 0,
    .weatherRainPosition = 0,
    .weatherWarningCounter = 0,
    .weatherWarningFadeState = fadeIn,
  };

  driver->onTimerFired(&state, values);

  //'transparent' at first
  verify_colours((Colour*)&COLOUR_START, lastValuesWritten, 6);
}

TEST(LedStripDriverWeatherTestGroup, writesCorrectValuesForWeatherWarningMidFadeInState)
{
  const Colour& COLOUR_START = COLOUR_RED;
  const Colour& COLOUR_END = COLOUR_GREEN;
  const Colour& COLOUR_WARNING = COLOUR_WHITE;
  const Colour COLOUR_MID = Colour(127, 127, 127);
  const uint32_t TEMPERATURE_FADE_INTERVAL_SECS = 4;
  const uint32_t WARNING_FADE_IN_MS = 500;
  const uint32_t WARNING_FADE_OUT_MS = 2000;
  const uint32_t WARNING_OFF_DWELL_MS = 1000;

  driver->pattern(Pattern::weather)
    ->colourOn((Colour*)&COLOUR_START)
    ->colourOff((Colour*)&COLOUR_END)
    ->tempFadeInterval(TEMPERATURE_FADE_INTERVAL_SECS)
    ->warningColour((Colour*)&COLOUR_WARNING)
    ->warningFadeIn(WARNING_FADE_IN_MS)
    ->warningFadeOut(WARNING_FADE_OUT_MS)
    ->warningOffDwell(WARNING_OFF_DWELL_MS);

  led_strip_state_t state = {
    .counter = 0,
    .weatherTempFadeDirection = 1,
    .weatherRainCounter = 0,
    .weatherRainPosition = 0,
    .weatherWarningCounter = WARNING_FADE_IN_MS/2 - 1,
    .weatherWarningFadeState = fadeIn,
  };

  driver->onTimerFired(&state, values);

  verify_colours((Colour*)&COLOUR_MID, lastValuesWritten, 6);
}

TEST(LedStripDriverWeatherTestGroup, writesCorrectValuesForWeatherWarningEndOfFadeInState)
{
  const Colour& COLOUR_START = COLOUR_RED;
  const Colour& COLOUR_END = COLOUR_GREEN;
  const Colour& COLOUR_WARNING = COLOUR_WHITE;
  const uint32_t TEMPERATURE_FADE_INTERVAL_SECS = 4;
  const uint32_t WARNING_FADE_IN_MS = 500;
  const uint32_t WARNING_FADE_OUT_MS = 2000;
  const uint32_t WARNING_OFF_DWELL_MS = 1000;

  driver->pattern(Pattern::weather)
    ->colourOn((Colour*)&COLOUR_START)
    ->colourOff((Colour*)&COLOUR_END)
    ->tempFadeInterval(TEMPERATURE_FADE_INTERVAL_SECS)
    ->warningColour((Colour*)&COLOUR_WARNING)
    ->warningFadeIn(WARNING_FADE_IN_MS)
    ->warningFadeOut(WARNING_FADE_OUT_MS)
    ->warningOffDwell(WARNING_OFF_DWELL_MS);

  led_strip_state_t state = {
    .counter = 0,
    .weatherTempFadeDirection = 1,
    .weatherRainCounter = 0,
    .weatherRainPosition = 0,
    .weatherWarningCounter = WARNING_FADE_IN_MS - 2,
    .weatherWarningFadeState = fadeIn,
  };

  driver->onTimerFired(&state, values);

  verify_colours((Colour*)&COLOUR_WARNING, lastValuesWritten, 6);
}

TEST(LedStripDriverWeatherTestGroup, incrementsWeatherWarningCounter)
{
  const Colour& COLOUR_START = COLOUR_RED;
  const Colour& COLOUR_END = COLOUR_GREEN;
  const Colour& COLOUR_WARNING = COLOUR_WHITE;
  const uint32_t TEMPERATURE_FADE_INTERVAL_SECS = 4;
  const uint32_t WARNING_FADE_IN_MS = 500;
  const uint32_t WARNING_FADE_OUT_MS = 2000;
  const uint32_t WARNING_OFF_DWELL_MS = 1000;

  driver->pattern(Pattern::weather)
    ->colourOn((Colour*)&COLOUR_START)
    ->colourOff((Colour*)&COLOUR_END)
    ->tempFadeInterval(TEMPERATURE_FADE_INTERVAL_SECS)
    ->warningColour((Colour*)&COLOUR_WARNING)
    ->warningFadeIn(WARNING_FADE_IN_MS)
    ->warningFadeOut(WARNING_FADE_OUT_MS)
    ->warningOffDwell(WARNING_OFF_DWELL_MS);

  led_strip_state_t state = {
    .counter = 0,
    .weatherTempFadeDirection = 1,
    .weatherRainCounter = 0,
    .weatherRainPosition = 0,
    .weatherWarningCounter = 0,
    .weatherWarningFadeState = fadeIn,
  };

  driver->onTimerFired(&state, values);

  LONGS_EQUAL(WEATHER_TEST_LED_CONFIG.resolutionMs, state.weatherWarningCounter);
}

TEST(LedStripDriverWeatherTestGroup, resetsWeatherWarningCounterAfterFadeIn)
{
  const Colour& COLOUR_START = COLOUR_RED;
  const Colour& COLOUR_END = COLOUR_GREEN;
  const Colour& COLOUR_WARNING = COLOUR_WHITE;
  const uint32_t TEMPERATURE_FADE_INTERVAL_SECS = 4;
  const uint32_t WARNING_FADE_IN_MS = 500;
  const uint32_t WARNING_FADE_OUT_MS = 2000;
  const uint32_t WARNING_OFF_DWELL_MS = 1000;

  driver->pattern(Pattern::weather)
    ->colourOn((Colour*)&COLOUR_START)
    ->colourOff((Colour*)&COLOUR_END)
    ->tempFadeInterval(TEMPERATURE_FADE_INTERVAL_SECS)
    ->warningColour((Colour*)&COLOUR_WARNING)
    ->warningFadeIn(WARNING_FADE_IN_MS)
    ->warningFadeOut(WARNING_FADE_OUT_MS)
    ->warningOffDwell(WARNING_OFF_DWELL_MS);

  led_strip_state_t state = {
    .counter = 0,
    .weatherTempFadeDirection = 1,
    .weatherRainCounter = 0,
    .weatherRainPosition = 0,
    .weatherWarningCounter = WARNING_FADE_IN_MS-1,
    .weatherWarningFadeState = fadeIn,
  };

  driver->onTimerFired(&state, values);

  LONGS_EQUAL(0, state.weatherWarningCounter);
}

TEST(LedStripDriverWeatherTestGroup, reversesWeatherWarningFadeDirectionAfterFadeIn)
{
  const Colour& COLOUR_START = COLOUR_RED;
  const Colour& COLOUR_END = COLOUR_GREEN;
  const Colour& COLOUR_WARNING = COLOUR_WHITE;
  const uint32_t TEMPERATURE_FADE_INTERVAL_SECS = 4;
  const uint32_t WARNING_FADE_IN_MS = 500;
  const uint32_t WARNING_FADE_OUT_MS = 2000;
  const uint32_t WARNING_OFF_DWELL_MS = 1000;

  driver->pattern(Pattern::weather)
    ->colourOn((Colour*)&COLOUR_START)
    ->colourOff((Colour*)&COLOUR_END)
    ->tempFadeInterval(TEMPERATURE_FADE_INTERVAL_SECS)
    ->warningColour((Colour*)&COLOUR_WARNING)
    ->warningFadeIn(WARNING_FADE_IN_MS)
    ->warningFadeOut(WARNING_FADE_OUT_MS)
    ->warningOffDwell(WARNING_OFF_DWELL_MS);

  led_strip_state_t state = {
    .counter = 0,
    .weatherTempFadeDirection = 1,
    .weatherRainCounter = 0,
    .weatherRainPosition = 0,
    .weatherWarningCounter = WARNING_FADE_IN_MS - WEATHER_TEST_LED_CONFIG.resolutionMs,
    .weatherWarningFadeState = fadeIn,
  };

  driver->onTimerFired(&state, values);

  LONGS_EQUAL(fadeOut, state.weatherWarningFadeState);
}

TEST(LedStripDriverWeatherTestGroup, writesCorrectValuesForWeatherWarningMidFadeOutState)
{
  const Colour& COLOUR_START = COLOUR_RED;
  const Colour& COLOUR_END = COLOUR_GREEN;
  const Colour& COLOUR_WARNING = COLOUR_WHITE;
  const Colour COLOUR_MID = Colour(127, 127, 127);
  const uint32_t TEMPERATURE_FADE_INTERVAL_SECS = 4;
  const uint32_t WARNING_FADE_IN_MS = 500;
  const uint32_t WARNING_FADE_OUT_MS = 2000;
  const uint32_t WARNING_OFF_DWELL_MS = 1000;

  driver->pattern(Pattern::weather)
    ->colourOn((Colour*)&COLOUR_START)
    ->colourOff((Colour*)&COLOUR_END)
    ->tempFadeInterval(TEMPERATURE_FADE_INTERVAL_SECS)
    ->warningColour((Colour*)&COLOUR_WARNING)
    ->warningFadeIn(WARNING_FADE_IN_MS)
    ->warningFadeOut(WARNING_FADE_OUT_MS)
    ->warningOffDwell(WARNING_OFF_DWELL_MS);

  led_strip_state_t state = {
    .counter = 0,
    .weatherTempFadeDirection = 1,
    .weatherRainCounter = 0,
    .weatherRainPosition = 0,
    .weatherWarningCounter = WARNING_FADE_OUT_MS/2 - WEATHER_TEST_LED_CONFIG.resolutionMs,
    .weatherWarningFadeState = fadeOut,
  };

  driver->onTimerFired(&state, values);

  verify_colours((Colour*)&COLOUR_MID, lastValuesWritten, 6);
}

TEST(LedStripDriverWeatherTestGroup, writesCorrectValuesForWeatherWarningDwellTime)
{
  const Colour& COLOUR_START = COLOUR_RED;
  const Colour& COLOUR_END = COLOUR_GREEN;
  const Colour& COLOUR_WARNING = COLOUR_WHITE;
  const uint32_t TEMPERATURE_FADE_INTERVAL_SECS = 4;
  const uint32_t WARNING_FADE_IN_MS = 500;
  const uint32_t WARNING_FADE_OUT_MS = 2000;
  const uint32_t WARNING_OFF_DWELL_MS = 1000;

  driver->pattern(Pattern::weather)
    ->colourOn((Colour*)&COLOUR_START)
    ->colourOff((Colour*)&COLOUR_END)
    ->tempFadeInterval(TEMPERATURE_FADE_INTERVAL_SECS)
    ->warningColour((Colour*)&COLOUR_WARNING)
    ->warningFadeIn(WARNING_FADE_IN_MS)
    ->warningFadeOut(WARNING_FADE_OUT_MS)
    ->warningOffDwell(WARNING_OFF_DWELL_MS);

  led_strip_state_t state = {
    .counter = 0,
    .weatherTempFadeDirection = 1,
    .weatherRainCounter = 0,
    .weatherRainPosition = 0,
    .weatherWarningCounter = WARNING_OFF_DWELL_MS - (2 * WEATHER_TEST_LED_CONFIG.resolutionMs),
    .weatherWarningFadeState = offDwell,
  };

  driver->onTimerFired(&state, values);

  verify_colours((Colour*)&COLOUR_START, lastValuesWritten, 6);
}

TEST(LedStripDriverWeatherTestGroup, setsWarningFadeStateToFadeInAfterOffDwell)
{
  const Colour& COLOUR_START = COLOUR_RED;
  const Colour& COLOUR_END = COLOUR_GREEN;
  const Colour& COLOUR_WARNING = COLOUR_WHITE;
  const uint32_t TEMPERATURE_FADE_INTERVAL_SECS = 4;
  const uint32_t WARNING_FADE_IN_MS = 500;
  const uint32_t WARNING_FADE_OUT_MS = 2000;
  const uint32_t WARNING_OFF_DWELL_MS = 1000;

  driver->pattern(Pattern::weather)
    ->colourOn((Colour*)&COLOUR_START)
    ->colourOff((Colour*)&COLOUR_END)
    ->tempFadeInterval(TEMPERATURE_FADE_INTERVAL_SECS)
    ->warningColour((Colour*)&COLOUR_WARNING)
    ->warningFadeIn(WARNING_FADE_IN_MS)
    ->warningFadeOut(WARNING_FADE_OUT_MS)
    ->warningOffDwell(WARNING_OFF_DWELL_MS);

  led_strip_state_t state = {
    .counter = 0,
    .weatherTempFadeDirection = 1,
    .weatherRainCounter = 0,
    .weatherRainPosition = 0,
    .weatherWarningCounter = WARNING_OFF_DWELL_MS -  WEATHER_TEST_LED_CONFIG.resolutionMs,
    .weatherWarningFadeState = offDwell,
  };

  driver->onTimerFired(&state, values);

  CHECK_EQUAL(fadeIn, state.weatherWarningFadeState);
}

TEST(LedStripDriverWeatherTestGroup, doesNotDrawRainBandsPastEndOfLedStrip)
{
  const Colour& COLOUR_START = COLOUR_RED;
  const Colour& COLOUR_END = COLOUR_GREEN;
  const Colour& COLOUR_RAIN = COLOUR_WHITE;
  const Colour& COLOUR_INIT = COLOUR_BLACK;
  const uint32_t TEMPERATURE_FADE_INTERVAL_SECS = 4;
  const uint32_t RAIN_INC_DELAY_MS = 500;
  const uint8_t RAIN_BAND_HEIGHT_LEDS = 3;

  driver->pattern(Pattern::weather)
    ->colourOn((Colour*)&COLOUR_START)
    ->colourOff((Colour*)&COLOUR_END)
    ->tempFadeInterval(TEMPERATURE_FADE_INTERVAL_SECS)
    ->rainBandHeight(RAIN_BAND_HEIGHT_LEDS)
    ->rainBandSpacing(2)
    ->rainBandIncrementDelay(RAIN_INC_DELAY_MS)
    ->rainBandColour((Colour*)&COLOUR_RAIN);

  led_strip_state_t state = {
    .counter = 0,
    .weatherTempFadeDirection = 1,
    .weatherRainCounter = 0,
    .weatherRainPosition = 0,
  };

  driver->onTimerFired(&state, values);

  verify_colour((Colour*)&COLOUR_INIT, &values[18]);//last must be unchanged from init
}

TEST(LedStripDriverWeatherTestGroup, doesNotDrawRainBandsPastEndOfLedStripReverseDirection)
{
  const Colour& COLOUR_START = COLOUR_RED;
  const Colour& COLOUR_END = COLOUR_GREEN;
  const Colour& COLOUR_RAIN = COLOUR_WHITE;
  const Colour& COLOUR_INIT = COLOUR_BLACK;
  const uint32_t TEMPERATURE_FADE_INTERVAL_SECS = 4;
  const uint32_t RAIN_INC_DELAY_MS = 500;
  const uint8_t RAIN_BAND_HEIGHT_LEDS = 3;

  driver->pattern(Pattern::weather)
    ->colourOn((Colour*)&COLOUR_START)
    ->colourOff((Colour*)&COLOUR_END)
    ->tempFadeInterval(TEMPERATURE_FADE_INTERVAL_SECS)
    ->rainBandHeight(RAIN_BAND_HEIGHT_LEDS)
    ->rainBandSpacing(2)
    ->rainDirection(Direction::reverse)
    ->rainBandIncrementDelay(RAIN_INC_DELAY_MS)
    ->rainBandColour((Colour*)&COLOUR_RAIN);

  led_strip_state_t state = {
    .counter = 0,
    .weatherTempFadeDirection = 1,
    .weatherRainCounter = 0,
    .weatherRainPosition = 0,
  };

  driver->onTimerFired(&state, values);

  verify_colour((Colour*)&COLOUR_INIT, &values[18]);//last must be unchanged from init
}

TEST(LedStripDriverWeatherTestGroup, warningUsesProvidedColour)
{
  const Colour& COLOUR_START = COLOUR_RED;
  const Colour& COLOUR_END = COLOUR_GREEN;
  const Colour& COLOUR_WARNING = COLOUR_BLUE;
  const uint32_t TEMPERATURE_FADE_INTERVAL_SECS = 4;
  const uint32_t WARNING_FADE_IN_MS = 500;
  const uint32_t WARNING_FADE_OUT_MS = 2000;
  const uint32_t WARNING_OFF_DWELL_MS = 1000;

  driver->pattern(Pattern::weather)
    ->colourOn((Colour*)&COLOUR_START)
    ->colourOff((Colour*)&COLOUR_END)
    ->tempFadeInterval(TEMPERATURE_FADE_INTERVAL_SECS)
    ->warningColour((Colour*)&COLOUR_WARNING)
    ->warningFadeIn(WARNING_FADE_IN_MS)
    ->warningFadeOut(WARNING_FADE_OUT_MS)
    ->warningOffDwell(WARNING_OFF_DWELL_MS);

  led_strip_state_t state = {
    .counter = 0,
    .weatherTempFadeDirection = 1,
    .weatherRainCounter = 0,
    .weatherRainPosition = 0,
    .weatherWarningCounter = WARNING_FADE_IN_MS,
    .weatherWarningFadeState = fadeIn,
  };

  driver->onTimerFired(&state, values);

  verify_colours((Colour*)&COLOUR_WARNING, lastValuesWritten, WEATHER_TEST_LED_CONFIG.numLeds);
}

/***********************************************************************************************
 * Initial states
 **********************************************************************************************/
TEST_GROUP(LedStripDriverInitStateTestGroup)
{
  void setup() {
  }

  void teardown() {
  }
};

TEST(LedStripDriverInitStateTestGroup, initialisesCounter)
{
  led_strip_state_t state;
  LedStripDriver driver((led_strip_config_t*)&CONFIG_LEDS_1);
  driver.initState(&state);

  LONGS_EQUAL(0, state.counter);
}

TEST(LedStripDriverInitStateTestGroup, initialisesDutyCycle)
{
  led_strip_state_t state;
  LedStripDriver driver((led_strip_config_t*)&CONFIG_LEDS_1);
  driver.initState(&state);

  DOUBLES_EQUAL(1, state.dutyCycle, 0.00001);
}

TEST(LedStripDriverInitStateTestGroup, initialisesDutyDirection)
{
  led_strip_state_t state;
  LedStripDriver driver((led_strip_config_t*)&CONFIG_LEDS_1);
  driver.initState(&state);

  LONGS_EQUAL(1, state.dutyDirection);
}

TEST(LedStripDriverInitStateTestGroup, initialisesWeatherTempFadeDirection)
{
  led_strip_state_t state;
  LedStripDriver driver((led_strip_config_t*)&CONFIG_LEDS_1);
  driver.initState(&state);

  CHECK_EQUAL(1, state.weatherTempFadeDirection);
}

TEST(LedStripDriverInitStateTestGroup, initialisesWeatherRainCounter)
{
  led_strip_state_t state;
  LedStripDriver driver((led_strip_config_t*)&CONFIG_LEDS_1);
  driver.initState(&state);

  LONGS_EQUAL(0, state.weatherRainCounter);
}

TEST(LedStripDriverInitStateTestGroup, initialisesWeatherRainPosition)
{
  led_strip_state_t state;
  LedStripDriver driver((led_strip_config_t*)&CONFIG_LEDS_1);
  driver.initState(&state);

  CHECK_EQUAL(0, state.weatherRainPosition);
}

TEST(LedStripDriverInitStateTestGroup, initialisesWeatherWarningCounter)
{
  led_strip_state_t state;
  LedStripDriver driver((led_strip_config_t*)&CONFIG_LEDS_1);
  driver.initState(&state);

  LONGS_EQUAL(0, state.weatherWarningCounter);
}

TEST(LedStripDriverInitStateTestGroup, initialisesWeatherWarningFadeState)
{
  led_strip_state_t state;
  LedStripDriver driver((led_strip_config_t*)&CONFIG_LEDS_1);
  driver.initState(&state);

  LONGS_EQUAL(fadeIn, state.weatherWarningFadeState);
}


//<colour 1>,<colour 2>,<fade time (secs 0-999)>,<band depth (# leds)>,<band speed (ms 0-9999)>,<band spacing (#leds)>,<weather warning fade in time (0-9999 ms, 0=no warning)>,<weather warning fade out time (0-9999 ms)>,<weather warning off dwell time 0-9999ms>
