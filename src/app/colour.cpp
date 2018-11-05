#include "colour.h"
#include "utils.h"

uint8_t hexColourStrValToInt(String hexStr) {
  return (uint8_t) hexStrToInt(hexStr);
}

Colour::Colour(uint8_t red, uint8_t green, uint8_t blue) {
  mRed = red;
  mGreen = green;
  mBlue = blue;
  mIsValid = true;
};

//value = '#rrggbb'
Colour::Colour(String value) {
  if (value.charAt(0) != '#' || value.length() != 7) {
    mIsValid = false;
  } else {
    const String &redStr = value.substring(1,3);
    const String &greenStr = value.substring(3,5);
    const String &blueStr = value.substring(5,7);

    mRed = hexColourStrValToInt(redStr);
    mGreen = hexColourStrValToInt(greenStr);
    mBlue = hexColourStrValToInt(blueStr);
    mIsValid = true;
  }
};

uint8_t Colour::getRed() const {
  return mRed;
}

uint8_t Colour::getGreen() const {
  return mGreen;
}

uint8_t Colour::getBlue() const {
  return mBlue;
}

bool Colour::isValid() {
  return mIsValid;
}

bool operator==(const Colour& lhs, const Colour& rhs) {
  return (
    lhs.getRed() == rhs.getRed() &&
    lhs.getGreen() == rhs.getGreen() &&
    lhs.getBlue() == rhs.getBlue()
  );
}

bool operator!=(const Colour& lhs, const Colour& rhs) {
  return !(lhs == rhs);
}
