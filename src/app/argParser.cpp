#include "Particle.h"
#include "colour.h"
#include "argParser.h"
#include "utils.h"

namespace argParser {
  /**
   * Split string into tokens using ',' as the delimiter
   * @param output Array of Strings.  Size must be at least 'argLimit'.
   * @param args Comma delimeted string to tokenise
   * @param argLimit Maximum number of arguments expected
   * @return number of arguments on success, error code on failure
   * @see cloudFunctions.h
   */
  int32_t tokeniseArgs(String* output, const String args, const uint32_t argLimit) {
    String s(args);
    const char delimiter = ',';

    int32_t pos = 0;
    uint8_t argCount = 0;

    while ((pos = s.indexOf(delimiter)) >= 0) {
      output[argCount] = s.substring(0, pos);
      ++argCount;
      s = s.substring(pos + sizeof(delimiter));

      if (argCount >= argLimit && s.length() > 0) {
        return RET_VAL_TOO_MANY_ARGS;
      }
    }

    if (argCount < argLimit && s.length() == 0) {
      return RET_VAL_TOO_FEW_ARGS;
    }

    output[argCount] = s;
    ++argCount;

    return argCount;
  }

  int32_t parseAndValidateArgs(String* output, const ArgConfig* config, String args) {
    int32_t argCount = tokeniseArgs(output, args, config->length);

    if (argCount < 0) {
      return argCount;
    }

    if ((uint32_t)argCount != config->length) {
      return RET_VAL_TOO_FEW_ARGS;
    }

    for (uint32_t i=0; i<config->length; i++) {
      String *arg = &output[i];
      const ArgInfo *info = &config->info[i];

      if (info->type == ARG_TYPE_NUMBER) {
        int32_t value;
        int32_t result = strToInt((uint32_t*)&value, *arg);

        if (result != 0 || value < info->min || value > info->max) {
          return RET_VAL_INVALID_ARG;
        }
      } else {
        Colour col = Colour(*arg);
        if (!col.isValid()) {
          return RET_VAL_INVALID_ARG;
        }
      }
    }

    return 0;
  }
}
