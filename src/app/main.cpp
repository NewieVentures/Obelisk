#include "Particle.h"

#include "serialDebug.h"
#include "ledStrip.h"
#include "cloudFunctions.h"
#include "statusLed.h"
#include "config.h"
#include "events.h"
#include "timers.h"

//run user code on boot to drive status LED
SYSTEM_MODE(SEMI_AUTOMATIC);

static const String LOG_MODULE = "MAIN";

static CloudFunctions *cloudFunctions;

int regFn(String name, int (CloudFunctions::*cloudFn)(String arg), CloudFunctions *cls) {
  return Particle.function(name, cloudFn, cls);
}

void setup() {
  statusLed::setup();
  ledStrip::setup();
  events::setup();
  timers::setup();

  cloudFunctions = new CloudFunctions(ledStrip::getDriver(), &regFn);

  Particle.connect();
}

/* Note: Code that blocks for too long (like more than 5 seconds), can make
 * weird things happen (like dropping the network connection).  The built-in
 * delay function shown below safely interleaves required background activity,
 * so arbitrarily long delays can safely be done if you need them.
 */
void loop() {
}
