#ifndef OBELISK_COLOUR_H
#define OBELISK_COLOUR_H

#include "Particle.h"

class Colour {
  uint8_t mRed, mGreen, mBlue;
  bool mIsValid;

public:
  Colour(uint8_t, uint8_t, uint8_t);
  Colour(String);
  uint8_t getRed() const;
  uint8_t getGreen() const;
  uint8_t getBlue() const;
  bool isValid();

  String toString();
};

bool operator==(const Colour& lhs, const Colour& rhs);
bool operator!=(const Colour& lhs, const Colour& rhs);

#endif
