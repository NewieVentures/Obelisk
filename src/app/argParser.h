#ifndef OBELISK_ARG_PARSER_H
#define OBELISK_ARG_PARSER_H

#include "Particle.h"

/**
 * @brief Colour codes to be sent as '#RRGGBB' where each value is a hexadecimal string
 * eg red = '#FF0000'
 */

namespace argParser {
  const int32_t  RET_VAL_SUC = 0;
  const int32_t  RET_VAL_TOO_FEW_ARGS = -1;
  const int32_t  RET_VAL_TOO_MANY_ARGS = -2;
  const int32_t  RET_VAL_INVALID_ARG = -3;

  #define ARG_TYPE_NUMBER 0
  #define ARG_TYPE_COLOUR 1

  typedef struct {
    uint32_t type;
    int32_t min; /* only applicable to number type */
    int32_t max; /* only applicable to number type */
  } ArgInfo;

  typedef struct {
    const ArgInfo* info;
    uint32_t length;
  } ArgConfig;


  int32_t parseAndValidateArgs(String* output, const ArgConfig* config, String args);
}

#endif
