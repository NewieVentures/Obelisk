#include "Particle.h"
#include "config.h"
#include "colour.h"
#include "colours.h"
#include "dmx.h"
#include "ledStripDriver.h"
#include "ledStrip.h"

static LedStripDriver *ledDriver;
static led_strip_state_t ledState;
static uint8_t ledValues[NUM_LEDS * COLOURS_PER_LED];
static const Colour COLOUR_START = COLOUR_BLUE;
static const Colour COLOUR_END = COLOUR_BLACK;

static void updateLedsDmx(uint8_t *values, uint32_t length) {
  dmx::send(values, length);
}

static const led_strip_config_t CONFIG_LED_STRIP = {
  .numLeds = NUM_LEDS,
  .writeValueFn = updateLedsDmx,
  .resolutionMs = TIMER_RESOLUTION_MS,
};

void ledStrip::onTimerFired() {
  ledDriver->onTimerFired(&ledState, ledValues);
}

void ledStrip::setup() {
  dmx::setup();

  ledDriver = new LedStripDriver((led_strip_config_t*)&CONFIG_LED_STRIP);
  ledDriver->initState(&ledState);

  //default pattern on power-up
  ledDriver->pattern(Pattern::pulse)
    ->period(2000)
    ->colourOn((Colour*)&COLOUR_START)
    ->colourOff((Colour*)&COLOUR_END);
}

LedStripDriver* ledStrip::getDriver() {
  return ledDriver;
}
