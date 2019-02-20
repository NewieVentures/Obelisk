#include "ledStripDriver.h"
#include "colour.h"
#include "colours.h"
#include "config.h"

const Colour COLOUR_DEFAULT = Colour(50, 0, 0);
const Colour COL_BLACK = COLOUR_BLACK;
const Colour COL_WHITE = COLOUR_WHITE;

#define DUTY_DIR_INC 1
#define DUTY_DIR_DEC -1
#define DUTY_MAX 99
#define DUTY_MIN 1

uint8_t calcGradientColourValue(double gradient, double offset, uint32_t step);

void LedStripDriver::initState(led_strip_state_t *state) {
  state->counter = 0;
  state->dutyCycle = 1;
  state->dutyDirection = DUTY_DIR_INC;
  state->weatherTempFadeDirection = 1;
  state->weatherRainCounter = 0;
  state->weatherRainPosition = 0;
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

  mWeatherTempFadeIntervalSecs = 4;
  mWeatherRainBandHeightLeds = 0; // no rain by default
  mWeatherRainBandIncDelayMs = 0;
  mWeatherRainBandSpacingLeds = 0;
  mWeatherRainBandColour = (Colour*)&COL_WHITE;

  mWeatherWarningColour = (Colour*)&COL_WHITE;
  mWeatherWarningFadeInMs = 0;
  mWeatherWarningFadeOutMs = 0;
  mWeatherWarningOffDwellMs = 0;
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

void calculatePulseOffsets(double *offsets, Colour *startCol) {
  offsets[INDEX_RED] = startCol->getRed();
  offsets[INDEX_GREEN] = startCol->getGreen();
  offsets[INDEX_BLUE] = startCol->getBlue();
}

void calculatePulseGradients(double *gradients,
                             double *offsets,
                             Colour *startCol,
                             Colour *endCol,
                             uint32_t steps) {
  gradients[INDEX_RED] = (double)(endCol->getRed() - offsets[INDEX_RED]) / (double)steps;
  gradients[INDEX_GREEN] = (double)(endCol->getGreen() - offsets[INDEX_GREEN]) / (double)steps;
  gradients[INDEX_BLUE] = (double)(endCol->getBlue() - offsets[INDEX_BLUE]) / (double)steps;
}

void LedStripDriver::handlePulsePattern(led_strip_state_t *state, uint8_t *values) {
  const uint32_t num_leds = mConfig->numLeds;
  const uint32_t steps = (mPeriodMs / mConfig->resolutionMs) - 1;
  uint32_t currentStep;
  double offsets[COLOURS_PER_LED];
  double gradients[COLOURS_PER_LED];
  uint8_t valueRed, valueGreen, valueBlue;
  Colour *startCol, *endCol;

  if (state->counter >= mPeriodMs) {
    state->counter = 0;
    state->dutyDirection *= -1;
  }

  currentStep = state->counter / mConfig->resolutionMs;

  if (state->dutyDirection > 0) {
    startCol = mColourOn;
    endCol = mColourOff;
  } else {
    startCol = mColourOff;
    endCol = mColourOn;
  }

  calculatePulseOffsets(offsets, startCol);
  calculatePulseGradients(gradients, offsets, startCol, endCol, steps);

  if (state->counter >= (mPeriodMs-mConfig->resolutionMs)) {
    valueRed = endCol->getRed();
    valueGreen = endCol->getGreen();
    valueBlue = endCol->getBlue();
  } else {
    valueRed = calcGradientColourValue(gradients[INDEX_RED],
                                       offsets[INDEX_RED],
                                       currentStep);
    valueGreen = calcGradientColourValue(gradients[INDEX_GREEN],
                                         offsets[INDEX_GREEN],
                                         currentStep);
    valueBlue = calcGradientColourValue(gradients[INDEX_BLUE],
                                        offsets[INDEX_BLUE],
                                        currentStep);
  }

  for (uint32_t i=0; i < num_leds; i++) {
    values[i * COLOURS_PER_LED + INDEX_RED] = valueRed;
    values[i * COLOURS_PER_LED + INDEX_GREEN] = valueGreen;
    values[i * COLOURS_PER_LED + INDEX_BLUE] = valueBlue;
  }
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
  uint32_t ledsOff = mConfig->numLeds - ledsOn;

  if ((mProgressFinal - ledsOn) == 0) {
    if (state->counter >= (mProgressIncrementDelayMs + mProgressResetDelayMs)) {
      state->counter = 0;
      state->progress = 0;
    }
  } else if (state->counter >= mProgressIncrementDelayMs) {
    state->progress += mProgressIncrement;
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

uint8_t calcGradientColourValue(double gradient, double offset, uint32_t step) {
  int32_t value = (int32_t)((gradient * (int32_t)step) + offset);

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

void LedStripDriver::handleWeatherPattern(led_strip_state_t *state, uint8_t *values) {
  const uint32_t num_leds = mConfig->numLeds;
  uint32_t steps = ((mWeatherTempFadeIntervalSecs * 1000) / mConfig->resolutionMs) - 1;
  uint32_t currentStep = state->counter / mConfig->resolutionMs;
  uint8_t redVal, greenVal, blueVal;
  Colour *colourStart, *colourEnd;

  int32_t offsets[COLOURS_PER_LED];
  double gradients[COLOURS_PER_LED];

  if (currentStep >= steps) {
    state->counter = 0;
    state->weatherTempFadeDirection *= -1;

    currentStep = 0;
  }

  if (state->weatherTempFadeDirection > 0) {
    colourStart = mColourOn;
    colourEnd = mColourOff;
  } else {
    colourStart = mColourOff; //reverse colours
    colourEnd = mColourOn;
  }

  offsets[INDEX_RED] = colourStart->getRed();
  offsets[INDEX_GREEN] = colourStart->getGreen();
  offsets[INDEX_BLUE] = colourStart->getBlue();

  gradients[INDEX_RED] = (colourEnd->getRed() - offsets[INDEX_RED]) / (double)steps;
  gradients[INDEX_GREEN] = (colourEnd->getGreen() - offsets[INDEX_GREEN]) / (double)steps;
  gradients[INDEX_BLUE] = (colourEnd->getBlue() - offsets[INDEX_BLUE]) / (double)steps;

  if (currentStep >= (steps - 1)) {
    //ensure no rounding errors for end value
    redVal = colourEnd->getRed();
    greenVal = colourEnd->getGreen();
    blueVal = colourEnd->getBlue();
  } else {
    redVal = calcGradientColourValue(gradients[INDEX_RED], offsets[INDEX_RED], currentStep);
    greenVal = calcGradientColourValue(gradients[INDEX_GREEN], offsets[INDEX_GREEN], currentStep);
    blueVal = calcGradientColourValue(gradients[INDEX_BLUE], offsets[INDEX_BLUE], currentStep);
  }

  for (uint32_t i=0; i < num_leds; i++) {
    values[i * COLOURS_PER_LED + INDEX_RED] = redVal;
    values[i * COLOURS_PER_LED + INDEX_GREEN] = greenVal;
    values[i * COLOURS_PER_LED + INDEX_BLUE] = blueVal;
  }

  // add rain bands
  state->weatherRainCounter += mConfig->resolutionMs;
  if (state->weatherRainCounter >= mWeatherRainBandIncDelayMs) {
    state->weatherRainCounter = 0;
    state->weatherRainPosition += 1;

    if (state->weatherRainPosition >= mConfig->numLeds) {
      state->weatherRainPosition = 0;
    }
  }

  if (mWeatherRainBandHeightLeds > 0) {
    //get initial position with bands wrapping around
    uint32_t bandAndSpacingHeight = mWeatherRainBandHeightLeds + mWeatherRainBandSpacingLeds;
    uint32_t rainInitialPosition = state->weatherRainPosition % bandAndSpacingHeight;

    for (uint32_t i = rainInitialPosition; i < num_leds; i += bandAndSpacingHeight) {
      for (uint32_t j = 0; j < mWeatherRainBandHeightLeds; j++) {
        values[(i+j) * COLOURS_PER_LED + INDEX_RED] = mWeatherRainBandColour->getRed();
        values[(i+j) * COLOURS_PER_LED + INDEX_GREEN] = mWeatherRainBandColour->getGreen();
        values[(i+j) * COLOURS_PER_LED + INDEX_BLUE] = mWeatherRainBandColour->getBlue();
      }
    }
  }

  state->weatherWarningCounter += mConfig->resolutionMs;

  switch(state->weatherWarningFadeState) {
    case fadeIn:
      if (state->weatherWarningCounter >= mWeatherWarningFadeInMs) {
        state->weatherWarningCounter = 0;
        state->weatherWarningFadeState = fadeOut;
      }
      break;

    case fadeOut:
      if (state->weatherWarningCounter >= mWeatherWarningFadeOutMs) {
        state->weatherWarningCounter = 0;
        state->weatherWarningFadeState = offDwell;
      }
      break;

    case offDwell:
      if (state->weatherWarningCounter >= mWeatherWarningOffDwellMs) {
        state->weatherWarningCounter = 0;
        state->weatherWarningFadeState = fadeIn;
      }
      break;
  }

  // weather warning
  if (mWeatherWarningFadeInMs > 0) {
    Colour warningColourStart = COLOUR_BLACK;

    if (state->weatherWarningFadeState == fadeIn) {
      steps = mWeatherWarningFadeInMs / mConfig->resolutionMs;
      currentStep = state->weatherWarningCounter / mConfig->resolutionMs;

      offsets[INDEX_RED] = warningColourStart.getRed();
      offsets[INDEX_GREEN] = warningColourStart.getGreen();
      offsets[INDEX_BLUE] = warningColourStart.getBlue();

      gradients[INDEX_RED] = (mWeatherWarningColour->getRed() - offsets[INDEX_RED]) / (double)steps;
      gradients[INDEX_GREEN] = (mWeatherWarningColour->getGreen() - offsets[INDEX_GREEN]) / (double)steps;
      gradients[INDEX_BLUE] = (mWeatherWarningColour->getBlue() - offsets[INDEX_BLUE]) / (double)steps;

      redVal = calcGradientColourValue(gradients[INDEX_RED], offsets[INDEX_RED], currentStep);
      greenVal = calcGradientColourValue(gradients[INDEX_GREEN], offsets[INDEX_GREEN], currentStep);
      blueVal = calcGradientColourValue(gradients[INDEX_BLUE], offsets[INDEX_BLUE], currentStep);
    } else if (state->weatherWarningFadeState == fadeOut) {
      steps = mWeatherWarningFadeOutMs / mConfig->resolutionMs;
      currentStep = state->weatherWarningCounter / mConfig->resolutionMs;

      offsets[INDEX_RED] = mWeatherWarningColour->getRed();
      offsets[INDEX_GREEN] = mWeatherWarningColour->getGreen();
      offsets[INDEX_BLUE] = mWeatherWarningColour->getBlue();

      gradients[INDEX_RED] = (warningColourStart.getRed() - offsets[INDEX_RED]) / (double)steps;
      gradients[INDEX_GREEN] = (warningColourStart.getGreen() - offsets[INDEX_GREEN]) / (double)steps;
      gradients[INDEX_BLUE] = (warningColourStart.getBlue() - offsets[INDEX_BLUE]) / (double)steps;

      redVal = calcGradientColourValue(gradients[INDEX_RED], offsets[INDEX_RED], currentStep);
      greenVal = calcGradientColourValue(gradients[INDEX_GREEN], offsets[INDEX_GREEN], currentStep);
      blueVal = calcGradientColourValue(gradients[INDEX_BLUE], offsets[INDEX_BLUE], currentStep);
    } else {
      redVal = warningColourStart.getRed();
      greenVal = warningColourStart.getGreen();
      blueVal = warningColourStart.getBlue();
    }

    if (currentStep == (steps - 1)) {
      redVal = mWeatherWarningColour->getRed();
      greenVal = mWeatherWarningColour->getGreen();
      blueVal = mWeatherWarningColour->getBlue();
    }

    //black is transparent for the warning layer
    if ((redVal | greenVal | blueVal) > 0) {
      for (uint32_t i=0; i < num_leds; i++) {
        values[i * COLOURS_PER_LED + INDEX_RED] = redVal;
        values[i * COLOURS_PER_LED + INDEX_GREEN] = greenVal;
        values[i * COLOURS_PER_LED + INDEX_BLUE] = blueVal;
      }
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

    case weather:
      handleWeatherPattern(state, values);
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

/* Used by weather pattern to set temperature fade interval (secs) */
LedStripDriver* LedStripDriver::tempFadeInterval(uint32_t intervalSecs) {
  mWeatherTempFadeIntervalSecs = intervalSecs;
  return this;
};

/* Used by weather pattern to set rain band height */
LedStripDriver* LedStripDriver::rainBandHeight(uint8_t leds) {
  mWeatherRainBandHeightLeds = leds;
  return this;
}

/* Used by weather pattern to set the delay between band movements */
LedStripDriver* LedStripDriver::rainBandIncrementDelay(uint32_t delayMs) {
  mWeatherRainBandIncDelayMs = delayMs;
  return this;
}

/* Used by weather pattern to set rain band spacing (# leds apart) */
LedStripDriver* LedStripDriver::rainBandSpacing(uint8_t leds) {
  mWeatherRainBandSpacingLeds = leds;
  return this;
}

/* Used by weather pattern to set rain band colour */
LedStripDriver* LedStripDriver::rainBandColour(Colour *colour) {
  mWeatherRainBandColour = colour;
  return this;
}

/* Used by weather pattern to set weather warning colour */
LedStripDriver* LedStripDriver::warningColour(Colour *colour) {
  mWeatherWarningColour = colour;
  return this;
}
/* Used by weather pattern to set weather warning fade in time (ms) */
LedStripDriver* LedStripDriver::warningFadeIn(uint32_t fadeTimeMs) {
  mWeatherWarningFadeInMs = fadeTimeMs;
  return this;
}
/* Used by weather pattern to set weather warning fade in time (ms) */
LedStripDriver* LedStripDriver::warningFadeOut(uint32_t fadeTimeMs) {
  mWeatherWarningFadeOutMs = fadeTimeMs;
  return this;
}
/* Used by weather pattern to set weather warning off dwell time (ms) */
LedStripDriver* LedStripDriver::warningOffDwell(uint32_t offDwellMs) {
  mWeatherWarningOffDwellMs = offDwellMs;
  return this;
}
