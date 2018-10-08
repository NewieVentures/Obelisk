#include "led.h"

static const uint32_t EXT_LED_VAL_HIGH = 1;
static const uint32_t EXT_LED_VAL_LOW = 0;

static const uint32_t extLed = D6;

void setupLED() {
  pinMode(extLed, OUTPUT);
}

int32_t setLed(String args) {
  char value = args[0];
  uint32_t pinValue;

  if (value == '0') {
    pinValue = EXT_LED_VAL_LOW;
  } else if (value == '1') {
    pinValue = EXT_LED_VAL_HIGH;
  } else {
    return -1;
  }

  digitalWrite(extLed, pinValue);
  return 0;
}
