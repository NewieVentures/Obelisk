#include <stdexcept>
#include <cstring>

#include "String.h"
#include "colour.h"
#include "argParser.h"
#include "utils.h"

using namespace std;

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
    const String delimiter = ",";

    size_t pos = 0;
    uint8_t argCount = 0;

    while ((pos = s.find(delimiter)) != string::npos) {
      output[argCount] = s.substr(0, pos);
      ++argCount;
      s.erase(0, pos + delimiter.length());

      if (argCount >= argLimit && !s.empty()) {
        return RET_VAL_TOO_MANY_ARGS;
      }
    }

    if (argCount < argLimit && s.empty()) {
      return RET_VAL_TOO_FEW_ARGS;
    }

    output[argCount] = s;
    ++argCount;

    return argCount;
  }

  int32_t parseAndValidateArgs(String* output, ArgConfig* config, String args) {
    String tokenisedArgs[config->length];
    int32_t argCount = tokeniseArgs(tokenisedArgs, args, config->length);

    if (argCount < 0) {
      return argCount;
    }

    if ((uint32_t)argCount != config->length) {
      return RET_VAL_TOO_FEW_ARGS;
    }

    for (uint32_t i=0; i<config->length; i++) {
      String *arg = &tokenisedArgs[i];
      const ArgInfo *info = &config->info[i];

      try {
        if (info->type == ARG_TYPE_NUMBER) {
          int32_t value = strToInt(*arg);
          if (value < info->min || value > info->max) {
            return RET_VAL_INVALID_ARG;
          }
        } else {
          Colour col = Colour(*arg); // parse colour value and throw if invalid
          col.getRed(); // stop unused variable warning...
        }
      } catch (const std::invalid_argument& ia) {
        return RET_VAL_INVALID_ARG;
      }
    }

    return 0;
  }
}
