#include "hexStrToInt.h"

static const uint8_t BASE_16 = 16;

static const uint8_t VAL_NUM_MIN = 48;
static const uint8_t VAL_NUM_MAX = 48+9;
static const uint8_t VAL_HEX_LC_MIN = 97;
static const uint8_t VAL_HEX_LC_MAX = 102;
static const uint8_t VAL_HEX_UC_MIN = 65;
static const uint8_t VAL_HEX_UC_MAX = 70;

static const uint8_t OFFSET_NUMBERS = 48;
static const uint8_t OFFSET_HEX_UPPERCASE = 55;
static const uint8_t OFFSET_HEX_LOWERCASE = 87;

static uint8_t convertChar(char c) {
  uint8_t val = -1;

  if (c >= VAL_NUM_MIN && c <= VAL_NUM_MAX) {
    val = c - OFFSET_NUMBERS;
  } else if (c >= VAL_HEX_LC_MIN && c <= VAL_HEX_LC_MAX) {
    val = c - OFFSET_HEX_LOWERCASE;
  } else if (c >= VAL_HEX_UC_MIN && c <= VAL_HEX_UC_MAX) {
    val = c - OFFSET_HEX_UPPERCASE;
  }

  return val;
}

uint32_t hexStrToInt(std::string hexStr) {
    uint32_t val = 0;
    uint32_t len = hexStr.length();

    for (uint32_t i=0; i < len; i++) {
      uint32_t pos = len - i - 1; // position in number
      uint32_t converted = convertChar(hexStr.at(i));

      val += pos > 0 ? BASE_16 * pos * converted : converted;
    }

    return val;
}
