#include "serialDebug.h"
#include <stdio.h>

void serialDebugSetup() {
  Serial.begin(115200);
}

void serialDebugPrint(String module, String message) {
  char s[LOG_SIZE_MAX];

  sprintf(s, "[%s]: %s", module.c_str(), message.c_str());

  Serial.println(s);
}
