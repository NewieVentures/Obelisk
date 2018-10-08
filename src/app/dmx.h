#ifndef OBELISK_DMX_H
#define OBELISK_DMX_H

#include "Particle.h"

namespace dmx {
  void setup();

  void send(const uint8_t *data, const uint32_t len);
}

#endif
