#ifndef OBELISK_STATUS_LED_H
#define OBELISK_STATUS_LED_H

namespace statusLed {
  void rapidFlash();
  void flash();
  void slowFlash();
  void blink();

  void onTimerFired();
  void setup();

  void update(uint8_t *value, uint32_t length);
};

#endif
