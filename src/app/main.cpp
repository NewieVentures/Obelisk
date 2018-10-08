#include "Particle.h"
#include "dmx.h"
#include "led.h"
#include "serialDebug.h"
#include "colour.h"

static const String LOG_MODULE = "MAIN";

static const uint8_t TEST_DMX_DATA[] = {
  0x00, // Null start code
  0x00, 0xAA, 0x00,
  0xAA, 0x00, 0x00,
  0x00, 0x00, 0xAA,
  0x00, 0xAA, 0x00,
  0xAA, 0x00, 0x00,
  0x00, 0x00, 0xAA,
  0x00, 0xAA, 0x00,
  0xAA, 0x00, 0x00,
  0x00, 0x00, 0xAA,
  // 0x44, 0x55, 0x66,
  // 0x00, 0x00, 0xFF,
  // 0xFF, 0xFF, 0xFF,
  // 0xFF, 0x00, 0x00,
};

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

  dmx::setup();
}

/* Note: Code that blocks for too long (like more than 5 seconds), can make
 * weird things happen (like dropping the network connection).  The built-in
 * delay function shown below safely interleaves required background activity,
 * so arbitrarily long delays can safely be done if you need them.
 */

void loop() {
  dmx::send(TEST_DMX_DATA, sizeof(TEST_DMX_DATA));
}
