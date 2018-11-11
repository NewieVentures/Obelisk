#include "ledStripDriver.h"
#include "colour.h"
#include "colours.h"
#include "config.h"

const Colour COLOUR_DEFAULT = Colour(50, 0, 0);
const Colour COL_BLACK = COLOUR_BLACK;

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
  mColourOff = (Colour*)&COL_BLACK;
  mPattern = colour;

  mDutyCycle = 50;

  mSnakeDirection = Direction::forward;
  mSnakeLength = 3;

  mProgressInitial = 0;
  mProgressFinal = config->numLeds;
  mProgressIncrement = 1;
  mProgressIncrementDelayMs = 1000;
  mProgressResetDelayMs = 0;
  mProgressDirection = Direction::forward;
};

inline void reverseDutyCycleDirection(led_strip_state_t *state) {
  state->dutyDirection = -state->dutyDirection;
}

void writeColourValues(uint8_t *values, uint32_t numLeds, Colour *colour) {
  for (uint32_t i=0; i<numLeds; i++) {
      uint32_t index = i*3;
      values[index+INDEX_RED] = colour->getRed();
      values[index+INDEX_GREEN] = colour->getGreen();
      values[index+INDEX_BLUE] = colour->getBlue();
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

void LedStripDriver::handleProgressPattern(led_strip_state_t *state, uint8_t *values) {
  uint32_t progressValue = mProgressInitial + state->progress;
  uint32_t ledsOn = (progressValue > mProgressFinal ? mProgressFinal : progressValue);
  uint32_t ledsOff = mProgressFinal - ledsOn;

  if (ledsOff == 0) {
    if (state->counter >= (mProgressIncrementDelayMs + mProgressResetDelayMs)) {
      state->counter = 0;
      state->progress = 0;
    }
  } else if (state->counter >= mProgressIncrementDelayMs) {
    state->progress += 1;
    state->counter = 0;
  }

  if (mProgressDirection == Direction::forward) {
    writeColourValues(values, ledsOn, mColourOn);
    writeColourValues(&values[ledsOn * COLOURS_PER_LED], ledsOff, mColourOff);
  } else {
    uint32_t onIndex = (mConfig->numLeds - ledsOn) * COLOURS_PER_LED;
    uint32_t offIndex = onIndex - (ledsOff * COLOURS_PER_LED);
    writeColourValues(&values[onIndex], ledsOn, mColourOn);
    writeColourValues(&values[offIndex], ledsOff, mColourOff);
  }
}

uint8_t calcGradientColourValue(int32_t gradient, int32_t offset, uint32_t step) {
  int32_t value = (gradient * (int32_t)step) + offset;

  if (value < 0) {
    value = 0;
  }

  if (value > 255) {
    value = 255;
  }

  return value;
}

void LedStripDriver::handleGradientPattern(led_strip_state_t *state, uint8_t *values) {
  const uint32_t num_leds = mConfig->numLeds;
  const uint32_t steps = num_leds - 1;
  int32_t offsets[COLOURS_PER_LED];
  int32_t gradients[COLOURS_PER_LED];

  offsets[INDEX_RED] = mColourOn->getRed();
  offsets[INDEX_GREEN] = mColourOn->getGreen();
  offsets[INDEX_BLUE] = mColourOn->getBlue();

  gradients[INDEX_RED] = (mColourOff->getRed() - offsets[INDEX_RED]) / (int32_t)steps;
  gradients[INDEX_GREEN] = (mColourOff->getGreen() - offsets[INDEX_GREEN]) / steps;
  gradients[INDEX_BLUE] = (mColourOff->getBlue() - offsets[INDEX_BLUE]) / steps;

  //ensure no rounding errors for end values
  values[steps * COLOURS_PER_LED + INDEX_RED] = mColourOff->getRed();
  values[steps * COLOURS_PER_LED + INDEX_GREEN] = mColourOff->getGreen();
  values[steps * COLOURS_PER_LED + INDEX_BLUE] = mColourOff->getBlue();

  for (uint32_t i=0; i < steps; i++) {
    values[i * COLOURS_PER_LED + INDEX_RED] = calcGradientColourValue(gradients[INDEX_RED],
                                                                      offsets[INDEX_RED],
                                                                      i);
    values[i * COLOURS_PER_LED + INDEX_GREEN] = calcGradientColourValue(gradients[INDEX_GREEN],
                                                                        offsets[INDEX_GREEN],
                                                                        i);
    values[i * COLOURS_PER_LED + INDEX_BLUE] = calcGradientColourValue(gradients[INDEX_BLUE],
                                                                       offsets[INDEX_BLUE],
                                                                       i);
  }
}

void LedStripDriver::handleSnakePattern(led_strip_state_t *state, uint8_t *values) {
  const uint32_t PROGRESS_MAX = mConfig->numLeds + mSnakeLength;
  const uint32_t INCREMENT_MS = mPeriodMs / (mConfig->numLeds + mSnakeLength);
  uint32_t start;
  uint32_t end;

  if (mSnakeDirection == forward) {
    start = mSnakeLength > state->progress ? 0 : state->progress - mSnakeLength;
    end = state->progress;
  } else {
    end = PROGRESS_MAX - state->progress;
    start = state->progress < mConfig->numLeds ? (end - mSnakeLength): 0;
  }


  for (uint8_t i=0; i < mConfig->numLeds; i++) {
    Colour *colour = (i >= start && i < end) ? mColourOn : mColourOff;

    values[(i * COLOURS_PER_LED) + INDEX_RED] = colour->getRed();
    values[(i * COLOURS_PER_LED) + INDEX_GREEN] = colour->getGreen();
    values[(i * COLOURS_PER_LED) + INDEX_BLUE] = colour->getBlue();
  }

  if (state->counter >= INCREMENT_MS) {
    state->progress += 1;
    state->counter = 0;

    if (state->progress >= PROGRESS_MAX) {
      state->progress = 0;
    }
  }
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

    case progress:
      handleProgressPattern(state, values);
      break;

    case gradient:
      handleGradientPattern(state, values);
      break;

    case snake:
      handleSnakePattern(state, values);
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
LedStripDriver* LedStripDriver::length(uint8_t numLeds) {
  mSnakeLength = numLeds;
  return this;
};

/*
* Used by snake pattern to set direction of snake
* eg forward = first LED to last
*    reverse = last LED to first
*/
LedStripDriver* LedStripDriver::snakeDirection(Direction direction) {
  mSnakeDirection = direction;
  return this;
};

/*
* Used by progress pattern to set direction of increment
* eg forward = first LED to last
*    reverse = last LED to first
*/
LedStripDriver* LedStripDriver::progressDirection(Direction direction) {
  mProgressDirection = direction;
  return this;
};

/* Used by progress pattern to set inital progress value */
LedStripDriver* LedStripDriver::initialValue(uint8_t progress) {
  mProgressInitial = progress;
  return this;
};

/* Used by progress pattern to set number of LEDs per increment */
LedStripDriver* LedStripDriver::increment(uint8_t leds) {
  mProgressIncrement = leds;
  return this;
};

/* Used by progress pattern to set number of ms between increments */
LedStripDriver* LedStripDriver::incDelay(uint32_t delayMs) {
  mProgressIncrementDelayMs = delayMs;
  return this;
};

/* Used by progress pattern to set number of ms between patterns */
LedStripDriver* LedStripDriver::resetDelay(uint32_t delayMs) {
  mProgressResetDelayMs = delayMs;
  return this;
};

/* Used by progress pattern to set final progress value */
LedStripDriver* LedStripDriver::finalValue(uint8_t progress) {
  mProgressFinal = progress;
  return this;
};
