#include <string>
#include "colour.h"
#include "hexStrToInt.h"

uint8_t hexColourStrValToInt(string hexStr) {
    return (uint8_t) hexStrToInt(hexStr);
}

Colour::Colour(uint8_t red, uint8_t green, uint8_t blue) {
    mRed = red;
    mGreen = green;
    mBlue = blue;
};

//value = '#rrggbb'
Colour::Colour(string value) {
    if (value.at(0) != '#' || value.length() != 7) {
        mRed = 0;
        mGreen = 0;
        mBlue = 0;
    } else {
        const string &redStr = value.substr(1,2);
        const string &greenStr = value.substr(3,2);
        const string &blueStr = value.substr(5,2);

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

bool operator==(const Colour& lhs, const Colour& rhs)
{
    return (
       lhs.getRed() == rhs.getRed() &&
       lhs.getGreen() == rhs.getGreen() &&
       lhs.getBlue() == rhs.getBlue()
    );
}

bool operator!=(const Colour& lhs, const Colour& rhs)
{
  return !(lhs == rhs);
}
