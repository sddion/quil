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
  EEPROM.write(0, strlen(ssid));
  for (int i = 0; i < strlen(ssid); i++) {
    EEPROM.write(1 + i, ssid[i]);
  }
  EEPROM.write(33, strlen(pass));
  for (int i = 0; i < strlen(pass); i++) {
    EEPROM.write(34 + i, pass[i]);
  }
  return EEPROM.commit();
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
  int ssid_len = EEPROM.read(0);
  if (ssid_len <= 0 || ssid_len > 32) return false;
  for (int i = 0; i < ssid_len; i++) {
    ssid[i] = EEPROM.read(1 + i);
  }
  ssid[ssid_len] = '\0';
  int pass_len = EEPROM.read(33);
  if (pass_len <= 0 || pass_len > 64) return false;
  for (int i = 0; i < pass_len; i++) {
    pass[i] = EEPROM.read(34 + i);
  }
  pass[pass_len] = '\0';
  return true;
#endif
}

bool config_save_theme(uint8_t theme) {
#ifdef ESP32
  return prefs.putUChar(CONFIG_KEY_THEME, theme) > 0;
#elif defined(ESP8266)
  EEPROM.write(100, theme);
  return EEPROM.commit();
#endif
}

bool config_load_theme(uint8_t* theme) {
#ifdef ESP32
  *theme = prefs.getUChar(CONFIG_KEY_THEME, 0);
  return true;
#elif defined(ESP8266)
  *theme = EEPROM.read(100);
  return true;
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
  for (int i = 0; i < EEPROM_SIZE; i++) {
    EEPROM.write(i, 0);
  }
  EEPROM.commit();
#endif
}
