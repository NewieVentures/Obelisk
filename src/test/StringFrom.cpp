#include "StringFrom.h"

SimpleString StringFrom(const Colour& c) {
    uint8_t red = c.getRed();
    uint8_t green = c.getGreen();
    uint8_t blue = c.getBlue();
    
    return SimpleString(StringFromFormat("#%02X%02X%02X (%d, %d, %d)",
                                         red, green, blue,
                                         red, green, blue));
};
