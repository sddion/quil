#include "config_store.h"
#include "config.h"

#ifdef ESP32
#include <Preferences.h>
static Preferences prefs;
#elif defined(ESP8266)
#include <EEPROM.h>
#define EEPROM_SIZE 512
#endif

bool config_init() {
#ifdef ESP32
  return prefs.begin(CONFIG_NAMESPACE, false);
#elif defined(ESP8266)
  EEPROM.begin(EEPROM_SIZE);
  return true;
#endif
}

bool config_save_wifi(const char* ssid, const char* pass) {
#ifdef ESP32
  prefs.putString(CONFIG_KEY_SSID, ssid);
  prefs.putString(CONFIG_KEY_PASS, pass);
  return true;
#elif defined(ESP8266)
  // Stub for ESP8266
  return false;
#endif
}

bool config_load_wifi(char* ssid, char* pass) {
#ifdef ESP32
  String s = prefs.getString(CONFIG_KEY_SSID, "");
  String p = prefs.getString(CONFIG_KEY_PASS, "");
  if (s.length() == 0) return false;
  strcpy(ssid, s.c_str());
  strcpy(pass, p.c_str());
  return true;
#elif defined(ESP8266)
  // Stub for ESP8266
  return false;
#endif
}

bool config_save_string(const char* key, const char* val) {
#ifdef ESP32
  prefs.putString(key, val);
  return true;
#elif defined(ESP8266)
  // Stub for ESP8266
  return false;
#endif
}

String config_load_string(const char* key) {
#ifdef ESP32
  return prefs.getString(key, "");
#elif defined(ESP8266)
  // Stub for ESP8266
  return "";
#endif
}

void config_clear() {
#ifdef ESP32
  prefs.clear();
#elif defined(ESP8266)
  // Stub for ESP8266
#endif
}
