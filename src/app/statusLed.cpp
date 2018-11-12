#include "Particle.h"
#include "config.h"
#include "colour.h"
#include "ledStripDriver.h"
#include "statusLed.h"

//need COLOURS_PER_LED values to use ledStripDriver, even only one is required
static uint8_t statusLedValues[1 * COLOURS_PER_LED];

static LedStripDriver *statusLedDriver;
static led_strip_state_t statusLedState;

//ensure the first value is the correct LED state
#ifdef COLOUR_ORDER_GGRRBB
static const Colour STATUS_COLOUR_ON(0, 1, 0);
#else
static const Colour STATUS_COLOUR_ON(1, 0, 0);
#endif

static const Colour STATUS_COLOUR_OFF(0, 0, 0);

static const led_strip_config_t CONFIG_STATUS_LED = {
  .numLeds = 1,
  .writeValueFn = statusLed::update,
  .resolutionMs = TIMER_RESOLUTION_STATUS_LED_MS,
};

static void flashLED(uint32_t periodMs) {
  statusLedDriver->pattern(Pattern::strobe)
    ->period(periodMs)
    ->colourOn((Colour*)&STATUS_COLOUR_ON);
}

// namespace statusLed {
void statusLed::setup() {
  pinMode(PIN_STATUS_LED, OUTPUT);

  statusLedDriver = new LedStripDriver((led_strip_config_t*)&CONFIG_STATUS_LED);
  statusLedDriver->initState(&statusLedState);

  statusLedDriver->pattern(Pattern::strobe)
    ->period(STATUS_RAPID_FLASH_PERIOD_MS)
    ->colourOn((Colour*)&STATUS_COLOUR_ON)
    ->colourOff((Colour*)&STATUS_COLOUR_OFF);
}

void statusLed::update(uint8_t *value, uint32_t length) {
  uint8_t ledVal = HIGH; //LED off

  if (*value == 1) {
    ledVal = LOW; //turn LED on
  }

  digitalWrite(PIN_STATUS_LED, ledVal);
}

void statusLed::onTimerFired() {
  statusLedDriver->onTimerFired(&statusLedState, statusLedValues);
}

void statusLed::rapidFlash() {
  flashLED(STATUS_RAPID_FLASH_PERIOD_MS);
}

void statusLed::flash() {
  flashLED(STATUS_FLASH_PERIOD_MS);
}

void statusLed::slowFlash() {
  flashLED(STATUS_SLOW_FLASH_PERIOD_MS);
}

void statusLed::blink() {
  statusLedDriver->pattern(Pattern::blink)
    ->period(STATUS_BLINK_PERIOD_MS)
    ->dutyCycle(10)
    ->colourOn((Colour*)&STATUS_COLOUR_ON)
    ->colourOff((Colour*)&STATUS_COLOUR_OFF);
}
// };
