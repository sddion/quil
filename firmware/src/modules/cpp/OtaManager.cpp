#include "../h/OtaManager.h"

#include <ArduinoOTA.h>

void OtaInit() {
  ArduinoOTA.setHostname("quil");
  ArduinoOTA.begin();
}

void OtaHandle() {
  ArduinoOTA.handle();
}

bool OtaStart(const char* url) {
  return false;
}

