#include "Particle.h"
#include "dmx.h"
#include "led.h"
#include "serialDebug.h"
#include "colour.h"
#include "colours.h"
#include "ledStripDriver.h"

static const int DEBUG_PIN_1 = D1;
static const int DEBUG_PIN_2 = D2;
static const String LOG_MODULE = "MAIN";

#define NUM_LEDS 5
#define COLOURS_PER_LED 3
static uint8_t ledValues[NUM_LEDS * COLOURS_PER_LED];

static void updateLedsDmx(uint8_t *values, uint32_t length) {
  pinSetFast(DEBUG_PIN_2);
  dmx::send(values, length);
  pinResetFast(DEBUG_PIN_2);
}

static const uint32_t TIMER_RESOLUTION_MS = 1;
static const led_strip_config_t CONFIG_LED_STRIP = {
    .numLeds = NUM_LEDS,
    .writeValueFn = updateLedsDmx,
    .resolutionMs = TIMER_RESOLUTION_MS,
};

static LedStripDriver* ledDriver;
static led_strip_state_t ledState;

static void onLedTimerFired() {
  pinSetFast(DEBUG_PIN_1);
  ledDriver->onTimerFired(&ledState, ledValues);
  pinResetFast(DEBUG_PIN_1);
}

const Colour& COLOUR_ON = COLOUR_ORANGE;
const Colour& COLOUR_OFF = COLOUR_BLACK;

Timer ledTimer(TIMER_RESOLUTION_MS, onLedTimerFired);

void registerFunc(String funcName, int32_t (*func)(String arg)) {
  bool result = Particle.function(funcName, func);

  if (!result) {
    serialDebugPrint(LOG_MODULE, "Failed to register function " + funcName);
  } else {
    serialDebugPrint(LOG_MODULE, "Registered function " + funcName);
  }
}

void setup() {
  // setupLED();
  // serialDebugSetup();

  // serialDebugPrint(LOG_MODULE, "Setup()");

  // String s = "0A";

  // char buf[128];
  // sprintf(buf, "value = %d;", val);
  // Serial.println(buf);

  // registerFunc("setLed", setLed);

  pinMode(DEBUG_PIN_1, OUTPUT);
  pinMode(DEBUG_PIN_2, OUTPUT);
  pinResetFast(DEBUG_PIN_1);
  // pinResetFast(DEBUG_PIN_2);

  dmx::setup();

  ledDriver = new LedStripDriver((led_strip_config_t*)&CONFIG_LED_STRIP);
  ledDriver->initState(&ledState);

  ledDriver->pattern(Pattern::pulse)
    ->period(1000)
    ->dutyCycle(80)
    ->colourOn((Colour*)&COLOUR_ON)
    ->colourOff((Colour*)&COLOUR_OFF);

  ledTimer.start();
}

/* Note: Code that blocks for too long (like more than 5 seconds), can make
 * weird things happen (like dropping the network connection).  The built-in
 * delay function shown below safely interleaves required background activity,
 * so arbitrarily long delays can safely be done if you need them.
 */
void loop() {
}
