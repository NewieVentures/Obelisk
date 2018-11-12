#ifndef OBELISK_LED_STRIP_H
#define OBELISK_LED_STRIP_H

#include "ledStripDriver.h"

namespace ledStrip {
  void setup();
  void onTimerFired();

  LedStripDriver* getDriver();
}

#endif
