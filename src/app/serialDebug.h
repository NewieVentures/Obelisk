#ifndef OBELISK_SERIAL_DEBUG_H
#define OBELISK_SERIAL_DEBUG_H

#include "Particle.h"

static const uint32_t LOG_SIZE_MAX = 256;

void serialDebugSetup();
void serialDebugPrint(String module, String s);

#endif
