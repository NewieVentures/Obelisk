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
  strobe
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
} led_strip_state_t;

class LedStripDriver {
private:
  led_strip_config_t* mConfig;

  void handleBlinkPattern(led_strip_state_t *state, uint8_t *values);
  void handlePulsePattern(led_strip_state_t *state, uint8_t *values);

protected:
  uint32_t mPeriodMs;
  Colour *mColourOn;
  Colour *mColourOff;
  Pattern mPattern;

  uint8_t mDutyCycle;

  Direction mSnakeDirection;
  uint32_t mSnakeLength;

  uint32_t mProgressInitial;
  uint32_t mProgressIncrement;
  uint32_t mProgressDelayMs;

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
  LedStripDriver* length(uint32_t numLeds);

  /*
  * Used by snake pattern to set direction of snake
  * eg forward = first LED to last
  *    backward = last LED to first
  */
  LedStripDriver* direction(Direction direction);

  /* Used by progress pattern to set inital progress value */
  LedStripDriver* initial(uint32_t progress);

  /* Used by progress pattern to set number of LEDs per increment */
  LedStripDriver* increment(uint32_t leds);

  /* Used by progress pattern to set number of ms between increments */
  LedStripDriver* delay(uint32_t delayMs);
};

#endif
