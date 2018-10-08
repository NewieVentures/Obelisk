#ifndef OBELISK_COLOUR_H
#define OBELISK_COLOUR_H

#include <string>

using namespace std;

class Colour {
  uint8_t mRed, mGreen, mBlue;

public:
  Colour(uint8_t, uint8_t, uint8_t);
  Colour(string);
  uint8_t getRed() const;
  uint8_t getGreen() const;
  uint8_t getBlue() const;
  bool operator==(const Colour& c);
  bool operator!=(const Colour *c);
  bool operator!=(const Colour& c);
};

#endif
