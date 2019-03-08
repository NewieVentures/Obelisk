#include "cloudFunctions.h"
#include "argParser.h"
#include "colours.h"
#include "utils.h"

#define NUM_LEDS_MAX 170

#define ARG_COUNT_BLINK 4
#define ARG_COUNT_COLOUR 1
#define ARG_COUNT_STROBE 2
#define ARG_COUNT_GRADIENT 2
#define ARG_COUNT_PROGRESS 8
#define ARG_COUNT_PULSE 3
#define ARG_COUNT_SNAKE 5
#define ARG_COUNT_WEATHER 9

const argParser::ArgInfo ARG_INFO_PERIOD_MS = {
  .type = ARG_TYPE_NUMBER,
  .min = 10,
  .max = 2147483647
};

const argParser::ArgInfo ARG_INFO_DUTY_CYCLE = {
  .type = ARG_TYPE_NUMBER,
  .min = 10,
  .max = 90
};

const argParser::ArgInfo ARG_INFO_SNAKE_DIRECTION = {
  .type = ARG_TYPE_NUMBER,
  .min = 0,
  .max = 1
};

const argParser::ArgInfo ARG_INFO_SNAKE_LENGTH = {
  .type = ARG_TYPE_NUMBER,
  .min = 1,
  .max = NUM_LEDS_MAX-1
};

const argParser::ArgInfo ARG_INFO_PROGRESS_INITIAL = {
  .type = ARG_TYPE_NUMBER,
  .min = 0,
  .max = NUM_LEDS_MAX-1
};

const argParser::ArgInfo ARG_INFO_PROGRESS_FINAL = {
  .type = ARG_TYPE_NUMBER,
  .min = 0,
  .max = NUM_LEDS_MAX-1
};

const argParser::ArgInfo ARG_INFO_PROGRESS_INCREMENT = {
  .type = ARG_TYPE_NUMBER,
  .min = 1,
  .max = NUM_LEDS_MAX-1
};

const argParser::ArgInfo ARG_INFO_PROGRESS_DIRECTION = {
  .type = ARG_TYPE_NUMBER,
  .min = 0,
  .max = 1
};

const argParser::ArgInfo ARG_INFO_PROGRESS_INC_DELAY_MS = {
  .type = ARG_TYPE_NUMBER,
  .min = 0,
  .max = 2147483647
};

const argParser::ArgInfo ARG_INFO_PROGRESS_RESET_DELAY_MS = {
  .type = ARG_TYPE_NUMBER,
  .min = 0,
  .max = 2147483647
};

const argParser::ArgInfo ARG_INFO_WEATHER_TEMP_FADE_IN = {
  .type = ARG_TYPE_NUMBER,
  .min = 0,
  .max = 999
};

const argParser::ArgInfo ARG_INFO_WEATHER_RAIN_BAND_DEPTH = {
  .type = ARG_TYPE_NUMBER,
  .min = 0,
  .max = NUM_LEDS_MAX-1
};

const argParser::ArgInfo ARG_INFO_WEATHER_RAIN_BAND_SPACING = {
  .type = ARG_TYPE_NUMBER,
  .min = 0,
  .max = NUM_LEDS_MAX-1
};

const argParser::ArgInfo ARG_INFO_WEATHER_RAIN_BAND_SPEED = {
  .type = ARG_TYPE_NUMBER,
  .min = 0,
  .max = 9999
};

const argParser::ArgInfo ARG_INFO_WEATHER_WARNING_FADE_IN = {
  .type = ARG_TYPE_NUMBER,
  .min = 0,
  .max = 9999
};

const argParser::ArgInfo ARG_INFO_WEATHER_WARNING_FADE_OUT = {
  .type = ARG_TYPE_NUMBER,
  .min = 0,
  .max = 9999
};

const argParser::ArgInfo ARG_INFO_WEATHER_WARNING_OFF_DWELL = {
  .type = ARG_TYPE_NUMBER,
  .min = 0,
  .max = 9999
};

const argParser::ArgInfo ARG_INFO_COLOUR = {
  .type = ARG_TYPE_COLOUR
};

const argParser::ArgInfo ARGS_INFO_STROBE[] = {
  ARG_INFO_PERIOD_MS,
  ARG_INFO_COLOUR
};

const argParser::ArgInfo ARGS_INFO_PULSE[] = {
  ARG_INFO_PERIOD_MS,
  ARG_INFO_COLOUR,
  ARG_INFO_COLOUR
};

const argParser::ArgInfo ARGS_INFO_BLINK[] = {
  ARG_INFO_PERIOD_MS,
  ARG_INFO_DUTY_CYCLE,
  ARG_INFO_COLOUR,
  ARG_INFO_COLOUR
};

const argParser::ArgInfo ARGS_INFO_SNAKE[] = {
  ARG_INFO_PERIOD_MS,
  ARG_INFO_SNAKE_DIRECTION,
  ARG_INFO_SNAKE_LENGTH,
  ARG_INFO_COLOUR,
  ARG_INFO_COLOUR
};

const argParser::ArgInfo ARGS_INFO_PROGRESS[] = {
  ARG_INFO_PROGRESS_INITIAL,
  ARG_INFO_PROGRESS_FINAL,
  ARG_INFO_PROGRESS_INCREMENT,
  ARG_INFO_PROGRESS_INC_DELAY_MS,
  ARG_INFO_PROGRESS_RESET_DELAY_MS,
  ARG_INFO_PROGRESS_DIRECTION,
  ARG_INFO_COLOUR,
  ARG_INFO_COLOUR
};

const argParser::ArgInfo ARGS_INFO_WEATHER[] = {
  ARG_INFO_COLOUR,
  ARG_INFO_COLOUR,
  ARG_INFO_WEATHER_TEMP_FADE_IN,
  ARG_INFO_WEATHER_RAIN_BAND_DEPTH,
  ARG_INFO_WEATHER_RAIN_BAND_SPEED,
  ARG_INFO_WEATHER_RAIN_BAND_SPACING,
  ARG_INFO_WEATHER_WARNING_FADE_IN,
  ARG_INFO_WEATHER_WARNING_FADE_OUT,
  ARG_INFO_WEATHER_WARNING_OFF_DWELL
};

const argParser::ArgInfo ARGS_INFO_GRADIENT[] = {
  ARG_INFO_COLOUR,
  ARG_INFO_COLOUR
};

const argParser::ArgInfo ARGS_INFO_COLOUR[] = {
  ARG_INFO_COLOUR
};

const argParser::ArgConfig ARG_CONFIG_STROBE = {
  .info = ARGS_INFO_STROBE,
  .length = ARG_COUNT_STROBE,
};

const argParser::ArgConfig ARG_CONFIG_BLINK = {
  .info = ARGS_INFO_BLINK,
  .length = ARG_COUNT_BLINK,
};

const argParser::ArgConfig ARG_CONFIG_COLOUR = {
  .info = ARGS_INFO_COLOUR,
  .length = ARG_COUNT_COLOUR,
};

const argParser::ArgConfig ARG_CONFIG_GRADIENT = {
  .info = ARGS_INFO_GRADIENT,
  .length = ARG_COUNT_GRADIENT,
};

const argParser::ArgConfig ARG_CONFIG_PROGRESS = {
  .info = ARGS_INFO_PROGRESS,
  .length = ARG_COUNT_PROGRESS,
};

const argParser::ArgConfig ARG_CONFIG_PULSE = {
  .info = ARGS_INFO_PULSE,
  .length = ARG_COUNT_PULSE,
};

const argParser::ArgConfig ARG_CONFIG_SNAKE = {
  .info = ARGS_INFO_SNAKE,
  .length = ARG_COUNT_SNAKE,
};

const argParser::ArgConfig ARG_CONFIG_WEATHER = {
  .info = ARGS_INFO_WEATHER,
  .length = ARG_COUNT_WEATHER,
};

static Direction intToDirection(uint8_t value) {
  return value == 0 ? Direction::forward : Direction::reverse;
}

CloudFunctions::CloudFunctions(LedStripDriver *ledDriver, int (*regFn)(String, int (CloudFunctions::*cloudFn)(String), CloudFunctions*)) {
  mLedDriver = ledDriver;
  mColourOn = new COLOUR_BLACK;
  mColourOff = new COLOUR_BLACK;
  mWeatherRainColour = new COLOUR_WHITE;
  mWeatherWarningColour = new COLOUR_WHITE;

  regFn(String("blink"), (&CloudFunctions::blink), this);
  regFn(String("colour"), (&CloudFunctions::colour), this);
  regFn(String("strobe"), (&CloudFunctions::strobe), this);
  regFn(String("gradient"), (&CloudFunctions::gradient), this);
  regFn(String("progress"), (&CloudFunctions::progress), this);
  regFn(String("pulse"), (&CloudFunctions::pulse), this);
  regFn(String("snake"), (&CloudFunctions::snake), this);
  regFn(String("weather"), (&CloudFunctions::weather), this);
}

CloudFunctions::~CloudFunctions() {
  deleteColours();
}

void CloudFunctions::deleteColours() {
  if (mColourOn != nullptr) {
    delete mColourOn;
    mColourOn = nullptr;
  }

  if (mColourOff != nullptr) {
    delete mColourOff;
    mColourOff = nullptr;
  }

  if (mWeatherRainColour != nullptr) {
    delete mWeatherRainColour;
    mWeatherRainColour = nullptr;
  }

  if (mWeatherWarningColour != nullptr) {
    delete mWeatherWarningColour;
    mWeatherWarningColour = nullptr;
  }
}

int CloudFunctions::blink(String args) {
  String parsedArgs[ARG_COUNT_BLINK];
  int32_t result = parseAndValidateArgs(parsedArgs, &ARG_CONFIG_BLINK, args);
  uint32_t period;
  uint32_t dutyCycle;

  if (result == 0) {
    deleteColours();

    mColourOn = new Colour(parsedArgs[2]);
    mColourOff = new Colour(parsedArgs[3]);

    strToInt(&period, parsedArgs[0]);
    strToInt(&dutyCycle, parsedArgs[1]);

    mLedDriver->pattern(Pattern::blink)
      ->period(period)
      ->dutyCycle((uint8_t)dutyCycle)
      ->colourOn(mColourOn)
      ->colourOff(mColourOff);
  }

  return result;
}

int CloudFunctions::colour(String args) {
  String parsedArgs[ARG_COUNT_COLOUR];
  int32_t result = parseAndValidateArgs(parsedArgs, &ARG_CONFIG_COLOUR, args);

  if (result == 0) {
    deleteColours();

    mColourOn = new Colour(parsedArgs[0]);
    mColourOff = nullptr;

    mLedDriver->pattern(Pattern::colour)
      ->colourOn(mColourOn);
  }

  return result;
}

int CloudFunctions::strobe(String args) {
  String parsedArgs[ARG_COUNT_STROBE];
  int32_t result = parseAndValidateArgs(parsedArgs, &ARG_CONFIG_STROBE, args);
  uint32_t period;

  if (result == 0) {
    deleteColours();

    mColourOn = new Colour(parsedArgs[1]);
    mColourOff = new COLOUR_BLACK;

    strToInt(&period, parsedArgs[0]);

    mLedDriver->pattern(Pattern::strobe)
      ->period(period)
      ->colourOn(mColourOn)
      ->colourOff(mColourOff);
  }

  return result;
}

int CloudFunctions::gradient(String args) {
  String parsedArgs[ARG_COUNT_GRADIENT];
  int32_t result = parseAndValidateArgs(parsedArgs, &ARG_CONFIG_GRADIENT, args);

  if (result == 0) {
    deleteColours();

    mColourOn = new Colour(parsedArgs[0]);
    mColourOff = new Colour(parsedArgs[1]);

    mLedDriver->pattern(Pattern::gradient)
      ->colourOff(mColourOff)
      ->colourOn(mColourOn);
  }

  return result;
}

int CloudFunctions::progress(String args) {
  String parsedArgs[ARG_COUNT_PROGRESS];
  int32_t result = parseAndValidateArgs(parsedArgs, &ARG_CONFIG_PROGRESS, args);
  uint32_t initialValue;
  uint32_t finalValue;
  uint32_t increment;
  uint32_t incDelay;
  uint32_t resetDelay;
  uint32_t direction;

  if (result == 0) {
    deleteColours();

    strToInt(&initialValue, parsedArgs[0]);
    strToInt(&finalValue, parsedArgs[1]);
    strToInt(&increment, parsedArgs[2]);
    strToInt(&incDelay, parsedArgs[3]);
    strToInt(&resetDelay, parsedArgs[4]);
    strToInt(&direction, parsedArgs[5]);

    mColourOn = new Colour(parsedArgs[6]);
    mColourOff = new Colour(parsedArgs[7]);

    mLedDriver->pattern(Pattern::progress)
      ->initialValue((uint8_t)initialValue)
      ->finalValue((uint8_t)finalValue)
      ->increment((uint8_t)increment)
      ->incDelay(incDelay)
      ->resetDelay(resetDelay)
      ->progressDirection(intToDirection(direction))
      ->colourOn(mColourOn)
      ->colourOff(mColourOff);
  }

  return result;
}

int CloudFunctions::pulse(String args) {
  String parsedArgs[ARG_COUNT_PULSE];
  int32_t result = parseAndValidateArgs(parsedArgs, &ARG_CONFIG_PULSE, args);
  uint32_t period;

  if (result == 0) {
    deleteColours();

    strToInt(&period, parsedArgs[0]);

    mColourOn = new Colour(parsedArgs[1]);
    mColourOff = new Colour(parsedArgs[2]);

    mLedDriver->pattern(Pattern::pulse)
      ->period(period)
      ->colourOn(mColourOn)
      ->colourOff(mColourOff);
  }

  return result;
}

int CloudFunctions::snake(String args) {
  String parsedArgs[ARG_COUNT_SNAKE];
  int32_t result = parseAndValidateArgs(parsedArgs, &ARG_CONFIG_SNAKE, args);
  uint32_t period;
  uint32_t direction;
  uint32_t length;

  if (result == 0) {
    deleteColours();

    strToInt(&period, parsedArgs[0]);
    strToInt(&direction, parsedArgs[1]);
    strToInt(&length, parsedArgs[2]);

    mColourOn = new Colour(parsedArgs[3]);
    mColourOff = new Colour(parsedArgs[4]);

    mLedDriver->pattern(Pattern::snake)
      ->period(period)
      ->length(length)
      ->snakeDirection(intToDirection(direction))
      ->colourOn(mColourOn)
      ->colourOff(mColourOff);
  }

  return result;
}

int CloudFunctions::weather(String args) {
  String parsedArgs[ARG_COUNT_WEATHER];
  int32_t result = parseAndValidateArgs(parsedArgs, &ARG_CONFIG_WEATHER, args);
  uint32_t tempFadeSecs;
  uint32_t bandDepth;
  uint32_t bandSpeed;
  uint32_t bandSpacing;
  uint32_t warningFadeInMs;
  uint32_t warningFadeOutMs;
  uint32_t warningOffDwellMs;

  if (result == 0) {
    deleteColours();

    mColourOn = new Colour(parsedArgs[0]);
    mColourOff = new Colour(parsedArgs[1]);

    strToInt(&tempFadeSecs, parsedArgs[2]);
    strToInt(&bandDepth, parsedArgs[3]);
    strToInt(&bandSpeed, parsedArgs[4]);
    strToInt(&bandSpacing, parsedArgs[5]);
    strToInt(&warningFadeInMs, parsedArgs[6]);
    strToInt(&warningFadeOutMs, parsedArgs[7]);
    strToInt(&warningOffDwellMs, parsedArgs[8]);

    mLedDriver->pattern(Pattern::weather)
      ->colourOn(mColourOn)
      ->colourOff(mColourOff)
      ->tempFadeInterval(tempFadeSecs)
      ->rainBandHeight(bandDepth)
      ->rainBandIncrementDelay(bandSpeed)
      ->rainBandSpacing(bandSpacing)
      ->rainBandColour(mWeatherRainColour)
      ->rainDirection(Direction::reverse)
      ->warningColour(mWeatherWarningColour)
      ->warningFadeIn(warningFadeInMs)
      ->warningFadeOut(warningFadeOutMs)
      ->warningOffDwell(warningOffDwellMs);
  }

  return result;
}
