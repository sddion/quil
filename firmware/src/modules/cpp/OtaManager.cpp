#include "../h/OtaManager.h"

#ifdef ESP32
#include <ArduinoOTA.h>

void ota_init() {
  ArduinoOTA.setHostname("quil");
  ArduinoOTA.begin();
}

void ota_handle() {
  ArduinoOTA.handle();
}

bool ota_start(const char* url) {
  return false;
}

#elif defined(ESP8266)
#include <ArduinoOTA.h>

void ota_init() {
  ArduinoOTA.setHostname("quil");
  ArduinoOTA.begin();
}

void ota_handle() {
  ArduinoOTA.handle();
}

bool ota_start(const char* url) {
  return false;
}
#endif
