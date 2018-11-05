#include "utils.h"

#define RET_VAL_SUC 0
#define RET_VAL_ERR -1

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

uint32_t hexStrToInt(String hexStr) {
    uint32_t val = 0;
    uint32_t len = hexStr.length();

    for (uint32_t i=0; i < len; i++) {
      uint32_t pos = len - i - 1; // position in number
      uint32_t converted = convertChar(hexStr.charAt(i));

      val += pos > 0 ? BASE_16 * pos * converted : converted;
    }

    return val;
}

/**
 * Convert a string to an unsigned int (no negative numbers)
 * @param value parsed value
 * @param str string to parse
 * @return 0 for success, -1 for error
 */
int32_t strToInt(uint32_t *value, String str) {
  uint32_t len = str.length();
  uint32_t exp = 0;

  *value = 0;

  if (len == 0) {
    return RET_VAL_ERR;
  }

  for (int32_t i=(len - 1); i >= 0; i--) {
    char c = str.charAt(i);

    if (c < OFFSET_NUMBERS || c > (OFFSET_NUMBERS + 9)) {
      return RET_VAL_ERR;
    }

    *value += ((uint32_t)c - OFFSET_NUMBERS) * (exp > 0 ? exp : 1);
    exp = exp > 0 ? exp*10 : 10;
  }

  return RET_VAL_SUC;
}
