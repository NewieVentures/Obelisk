#include "ledStripDriver.h"
#include "colour.h"

const Colour COLOUR_DEFAULT = Colour(50, 0, 0);
const Colour COLOUR_BLACK = Colour(0, 0, 0);

#define COLOURS_PER_LED 3
#define PWM_DUTY_STEPS 10
#define DUTY_DIR_INC 1
#define DUTY_DIR_DEC -1
#define DUTY_MAX 99
#define DUTY_MIN 1

void LedStripDriver::initState(led_strip_state_t *state) {
  state->counter = 0;
  state->dutyCycle = 1;
  state->dutyDirection = DUTY_DIR_INC;
}

LedStripDriver::LedStripDriver(led_strip_config_t *config) {
  mConfig = config;
  mPeriodMs = 1000;
  mColourOn = (Colour*)&COLOUR_DEFAULT;
  mColourOff = (Colour*)&COLOUR_BLACK;
  mPattern = colour;

  mDutyCycle = 50;

  mSnakeDirection = Direction::forward;
  mSnakeLength = 3;

  mProgressInitial = 0;
  mProgressIncrement = 1;
  mProgressDelayMs = 1000;
};

inline void reverseDutyCycleDirection(led_strip_state_t *state) {
  state->dutyDirection = -state->dutyDirection;
}

void writeColourValues(uint8_t *values, uint32_t numLeds, Colour *colour) {
  for (uint32_t i=0; i<numLeds; i++) {
      uint32_t index = i*3;
      values[index+0] = colour->getGreen();
      values[index+1] = colour->getRed();
      values[index+2] = colour->getBlue();
    }
}

double calcDutyCycleIncrement(uint8_t dutyCycle, uint32_t pulsePeriod, uint32_t pwmPeriod) {
  return (double)(100 * 100 / dutyCycle)/((double)pulsePeriod / pwmPeriod);
}

void updateDutyCycle(led_strip_state_t *state, double increment, double decrement) {
  if (state->dutyDirection == DUTY_DIR_INC) {
    state->dutyCycle += increment;
  } else {
    state->dutyCycle -= decrement;
  }
}

void LedStripDriver::handlePulsePattern(led_strip_state_t *state, uint8_t *values) {
  uint32_t pwmPeriod = mConfig->resolutionMs * PWM_DUTY_STEPS;
  uint32_t onTimeMs = (uint32_t)((pwmPeriod * state->dutyCycle) / 100);

  double dutyIncrement = calcDutyCycleIncrement(mDutyCycle, mPeriodMs, pwmPeriod);
  double dutyDecrement = calcDutyCycleIncrement((100 - mDutyCycle), mPeriodMs, pwmPeriod);

  Colour *colour;

  if (state->counter >= pwmPeriod) {
    state->counter = 0;
    updateDutyCycle(state, dutyIncrement, dutyDecrement);

    if (state->dutyCycle > DUTY_MAX) {
      state->dutyCycle = DUTY_MAX;
      reverseDutyCycleDirection(state);
    } else if (state->dutyCycle < DUTY_MIN) {
      state->dutyCycle = DUTY_MIN;
      reverseDutyCycleDirection(state);
    }
  }

  if (state->counter < onTimeMs) {
    colour = mColourOn;
  } else {
    colour = mColourOff;
  }

  writeColourValues(values, mConfig->numLeds, colour);
}

void LedStripDriver::handleBlinkPattern(led_strip_state_t *state, uint8_t *values) {
  uint32_t onTimeMs = (uint32_t)((mPeriodMs * mDutyCycle) / 100);
  Colour *colour;

  if (state->counter >= mPeriodMs) {
    state->counter = 0;
  }

  if (state->counter < onTimeMs) {
    colour = mColourOn;
  } else {
    colour = mColourOff;
  }

  writeColourValues(values, mConfig->numLeds, colour);
}

void LedStripDriver::handleColourPattern(led_strip_state_t *state, uint8_t *values) {
  writeColourValues(values, mConfig->numLeds, mColourOn);
}

void LedStripDriver::handleStrobePattern(led_strip_state_t *state, uint8_t *values) {
  uint32_t onTimeMs = (uint32_t)(mPeriodMs / 2);
  Colour *colour;

  if (state->counter >= mPeriodMs) {
    state->counter = 0;
  }

  if (state->counter < onTimeMs) {
    colour = mColourOn;
  } else {
    colour = mColourOff;
  }

  writeColourValues(values, mConfig->numLeds, colour);
}

void LedStripDriver::onTimerFired(led_strip_state_t *state, uint8_t *values) {
  const uint32_t numLedValues = COLOURS_PER_LED * mConfig->numLeds;

  switch(mPattern) {
    case blink:
      handleBlinkPattern(state, values);
      break;

    case pulse:
      handlePulsePattern(state, values);
      break;

    case colour:
      handleColourPattern(state, values);
      break;

    case strobe:
      handleStrobePattern(state, values);
      break;

    default:
      break;
  }

  mConfig->writeValueFn(values, numLedValues);

  state->counter += mConfig->resolutionMs;
};

LedStripDriver* LedStripDriver::period(uint32_t valueMs) {
  mPeriodMs = valueMs;
  return this;
}

LedStripDriver* LedStripDriver::dutyCycle(uint8_t value) {
  mDutyCycle = value;
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
LedStripDriver* LedStripDriver::length(uint32_t numLeds) {
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
LedStripDriver* LedStripDriver::initial(uint32_t progress) {
  mProgressInitial = progress;
  return this;
};

/* Used by progress pattern to set number of LEDs per increment */
LedStripDriver* LedStripDriver::increment(uint32_t leds) {
  mProgressIncrement = leds;
  return this;
};

/* Used by progress pattern to set number of ms between increments */
LedStripDriver* LedStripDriver::delay(uint32_t delayMs) {
  mProgressDelayMs = delayMs;
  return this;
};
