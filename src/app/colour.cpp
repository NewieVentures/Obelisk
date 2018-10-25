#include <stdexcept>
#include "String.h"
#include "colour.h"
#include "utils.h"

uint8_t hexColourStrValToInt(String hexStr) {
  return (uint8_t) hexStrToInt(hexStr);
}

Colour::Colour(uint8_t red, uint8_t green, uint8_t blue) {
  mRed = red;
  mGreen = green;
  mBlue = blue;
};

//value = '#rrggbb'
Colour::Colour(String value) {
  if (value.at(0) != '#' || value.length() != 7) {
    throw std::invalid_argument("Invalid colour, must be #RRGGBB");
  } else {
    const String &redStr = value.substr(1,2);
    const String &greenStr = value.substr(3,2);
    const String &blueStr = value.substr(5,2);

    mRed = hexColourStrValToInt(redStr);
    mGreen = hexColourStrValToInt(greenStr);
    mBlue = hexColourStrValToInt(blueStr);
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
