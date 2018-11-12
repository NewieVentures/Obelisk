#include "Particle.h"
#include "config.h"
#include "timers.h"
#include "ledStrip.h"
#include "statusLed.h"

Timer ledTimer(TIMER_RESOLUTION_MS, ledStrip::onTimerFired);
Timer statusLedTimer(TIMER_RESOLUTION_STATUS_LED_MS, statusLed::onTimerFired);

void timers::setup() {
  statusLedTimer.start();
  ledTimer.start();
}
