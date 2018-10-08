#include "ledStripDriver.h"
#include "colour.h"

const Colour COLOUR_DEFAULT = Colour(255, 255, 255);

LedStripDriver::LedStripDriver(led_strip_config_t *config) {
  mConfig = config;
  mPeriodMs = 1000;
  mColourOn = (Colour*)&COLOUR_DEFAULT;
  mColourOff = (Colour*)&COLOUR_DEFAULT;
  mPattern = colour;

  mPulseDutyCycle = 50;

  mSnakeDirection = Direction::forward;
  mSnakeLength = 3;

  mProgressInitial = 0;
  mProgressIncrement = 1;
  mProgressDelayMs = 1000;
};

void LedStripDriver::onTimerFired(led_strip_state_t *state) {
    uint8_t values[3];
    values[0] = mColourOn->getRed();
    values[1] = mColourOn->getGreen();
    values[2] = mColourOn->getBlue();
    mConfig->write_value_fn(values, 3);
};

LedStripDriver* LedStripDriver::period(uint32_t valueMs) {
  mPeriodMs = valueMs;
  return this;
}

LedStripDriver* LedStripDriver::dutyCycle(uint8_t value) {
  mPulseDutyCycle = value;
  return this;
};

LedStripDriver* LedStripDriver::colourOn(Colour *colour) {
  mColourOn = colour;
  return this;
};

LedStripDriver* LedStripDriver::colourOff(Colour *colour) {
  mColourOff = colour;
  return this;
};

LedStripDriver* LedStripDriver::pattern(Pattern pattern) {
  mPattern = pattern;
  return this;
};

/* Used by snake pattern to set length of snake in LEDs */
LedStripDriver* LedStripDriver::length(uint8_t numLeds) {
  mSnakeLength = numLeds;
  return this;
};

/*
* Used by snake pattern to set direction of snake
* eg Forward = first LED to last
*    Backward = last LED to first
*/
LedStripDriver* LedStripDriver::direction(Direction direction) {
  mSnakeDirection = direction;
  return this;
};

/* Used by progress pattern to set inital progress value */
LedStripDriver* LedStripDriver::initial(uint8_t progress) {
  mProgressInitial = progress;
  return this;
};

/* Used by progress pattern to set number of LEDs per increment */
LedStripDriver* LedStripDriver::increment(uint8_t leds) {
  mProgressIncrement = leds;
  return this;
};

/* Used by progress pattern to set number of ms between increments */
LedStripDriver* LedStripDriver::delay(uint32_t delayMs) {
  mProgressDelayMs = delayMs;
  return this;
};
