#ifndef OBELISK_CLOUD_FUNCTIONS_H
#define OBELISK_CLOUD_FUNCTIONS_H

#include "Particle.h"
#include "colour.h"
#include "ledStripDriver.h"

class CloudFunctions {
  private:
  LedStripDriver *mLedDriver;
  Colour *mColourOn;
  Colour *mColourOff;

  void deleteColours();

  public:
  CloudFunctions(LedStripDriver *ledDriver, int (*regFn)(String, int (CloudFunctions::*cloudFn)(String), CloudFunctions*));
  ~CloudFunctions();

  int blink(String args);
  int colour(String args);
  int strobe(String args);
  int gradient(String args);
  int progress(String args);
  int snake(String args);
};


#endif
