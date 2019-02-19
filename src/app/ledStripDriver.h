#ifndef OBELISK_LED_STRIP_DRIVER_H
#define OBELISK_LED_STRIP_DRIVER_H

#include "colour.h"

enum Pattern {
  blink,
  colour,
  gradient,
  progress,
  pulse,
  snake,
  strobe,
  weather,
};

enum Direction {
  forward,
  reverse
};

typedef struct {
  uint32_t numLeds;
  void (*writeValueFn)(uint8_t *values, uint32_t length);
  uint32_t resolutionMs;
} led_strip_config_t;

typedef struct {
  uint32_t counter;
  int32_t dutyDirection;
  double dutyCycle;
  uint32_t progress;
  int8_t weatherTempFadeDirection;
} led_strip_state_t;

class LedStripDriver {
private:
  led_strip_config_t* mConfig;

  void handleBlinkPattern(led_strip_state_t *state, uint8_t *values);
  void handlePulsePattern(led_strip_state_t *state, uint8_t *values);
  void handleColourPattern(led_strip_state_t *state, uint8_t *values);
  void handleStrobePattern(led_strip_state_t *state, uint8_t *values);
  void handleProgressPattern(led_strip_state_t *state, uint8_t *values);
  void handleGradientPattern(led_strip_state_t *state, uint8_t *values);
  void handleSnakePattern(led_strip_state_t *state, uint8_t *values);
  void handleWeatherPattern(led_strip_state_t *state, uint8_t *values);

protected:
  uint32_t mPeriodMs;
  Colour *mColourOn;
  Colour *mColourOff;
  Pattern mPattern;

  uint8_t mDutyCycle;

  Direction mSnakeDirection;
  uint32_t mSnakeLength;

  uint8_t mProgressInitial;
  uint8_t mProgressFinal;
  uint8_t mProgressIncrement;
  uint32_t mProgressIncrementDelayMs;
  uint32_t mProgressResetDelayMs;
  Direction mProgressDirection;

  uint32_t mWeatherTempFadeIntervalSecs;

public:
  void initState(led_strip_state_t *state);
  LedStripDriver(led_strip_config_t *config);
  void onTimerFired(led_strip_state_t *state, uint8_t *values);

  LedStripDriver* period(uint32_t valueMs);
  LedStripDriver* colourOn(Colour *colour);
  LedStripDriver* colourOff(Colour *colour);
  LedStripDriver* pattern(Pattern pattern);

  /*
   * Used by pulse pattern to set % of time spent turning on vs off
   * eg 50 -> same time fading on as off
   *    20 -> 20% time used to fade on, 80% to fade off
   *    80 -> 80% time used to fade on, 20% to fade off
   */
  LedStripDriver* dutyCycle(uint8_t value);

  /* Used by snake pattern to set length of snake in LEDs */
  LedStripDriver* length(uint8_t numLeds);

  /*
  * Used by snake pattern to set direction of snake
  * eg forward = first LED to last
  *    reverse = last LED to first
  */
  LedStripDriver* snakeDirection(Direction direction);

  /* Used by progress pattern to set inital progress value */
  LedStripDriver* initialValue(uint8_t progress);

  /* Used by progress pattern to set number of LEDs per increment */
  LedStripDriver* increment(uint8_t leds);

  /* Used by progress pattern to set number of ms between increments */
  LedStripDriver* incDelay(uint32_t delayMs);

  /* Used by progress pattern to set number of ms between patterns */
  LedStripDriver* resetDelay(uint32_t delayMs);

  /* Used by progress pattern to set maximum progress value */
  LedStripDriver* finalValue(uint8_t progress);

  /* Used by weather pattern to set temperature fade interval (secs) */
  LedStripDriver* tempFadeInterval(uint32_t progress);

  /*
  * Used by progress pattern to set direction of increment
  * eg forward = first LED to last
  *    reverse = last LED to first
  */
  LedStripDriver* progressDirection(Direction direction);

  uint32_t getPeriod() { return mPeriodMs; };
  Colour* getColourOn() { return mColourOn; };
  Colour* getColourOff() { return mColourOff; };
  Pattern getPattern() { return mPattern; };

  uint8_t getDutyCycle() { return mDutyCycle; };

  uint8_t getProgressInitial() { return mProgressInitial; };
  uint8_t getProgressFinal() { return mProgressFinal; };
  uint8_t getProgressIncrement() { return mProgressIncrement; };
  uint32_t getProgressIncrementDelay() { return mProgressIncrementDelayMs; };
  uint32_t getProgressResetDelay() { return mProgressResetDelayMs; };
  Direction getProgressDirection() { return mProgressDirection; };

  uint32_t getSnakeLength() { return mSnakeLength; };
  Direction getSnakeDirection() { return mSnakeDirection; };
};

#endif
