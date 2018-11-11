#include "Particle.h"

#include "dmx.h"
#include "led.h"
#include "serialDebug.h"
#include "colour.h"
#include "colours.h"
#include "ledStripDriver.h"
#include "cloudFunctions.h"
#include "config.h"

static const String LOG_MODULE = "MAIN";

static uint8_t ledValues[NUM_LEDS * COLOURS_PER_LED];

static void updateLedsDmx(uint8_t *values, uint32_t length) {
  dmx::send(values, length);
}

static const led_strip_config_t CONFIG_LED_STRIP = {
    .numLeds = NUM_LEDS,
    .writeValueFn = updateLedsDmx,
    .resolutionMs = TIMER_RESOLUTION_MS,
};

static LedStripDriver *ledDriver;
static led_strip_state_t ledState;
static CloudFunctions *cloudFunctions;

static void onLedTimerFired() {
  ledDriver->onTimerFired(&ledState, ledValues);
}

const Colour& COLOUR_ON = COLOUR_RED;
const Colour& COLOUR_OFF = COLOUR_BLUE;

Timer ledTimer(TIMER_RESOLUTION_MS, onLedTimerFired);

void registerFunc(String funcName, int32_t (*func)(String arg)) {
  bool result = Particle.function(funcName, func);

  if (!result) {
    serialDebugPrint(LOG_MODULE, "Failed to register function " + funcName);
  } else {
    serialDebugPrint(LOG_MODULE, "Registered function " + funcName);
  }
}

int regFn(String name, int (CloudFunctions::*cloudFn)(String arg), CloudFunctions *cls) {
  return Particle.function(name, cloudFn, cls);
}

void setup() {
  dmx::setup();

  ledDriver = new LedStripDriver((led_strip_config_t*)&CONFIG_LED_STRIP);
  ledDriver->initState(&ledState);

  //default pattern on power-up
  ledDriver->pattern(Pattern::gradient)
    ->colourOn((Colour*)&COLOUR_ON)
    ->colourOff((Colour*)&COLOUR_OFF);

  cloudFunctions = new CloudFunctions(ledDriver, &regFn);

  ledTimer.start();
}

/* Note: Code that blocks for too long (like more than 5 seconds), can make
 * weird things happen (like dropping the network connection).  The built-in
 * delay function shown below safely interleaves required background activity,
 * so arbitrarily long delays can safely be done if you need them.
 */
void loop() {
}
