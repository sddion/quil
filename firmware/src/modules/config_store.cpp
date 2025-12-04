#include "config_store.h"
#include "config.h"

#ifdef ESP32
#include <Preferences.h>
static Preferences prefs;
#define CONFIG_KEY_THEME "theme"
#define CONFIG_KEY_WEATHER_API_KEY "weather_api_key"
#define CONFIG_KEY_WEATHER_LOCATION "weather_location"
#define CONFIG_KEY_TIMEZONE "timezone"
#define CONFIG_KEY_CONTRAST "contrast"
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
  for (size_t i = 0; i < strlen(ssid); i++) {
    EEPROM.write(1 + i, ssid[i]);
  }
  EEPROM.write(33, strlen(pass));
  for (size_t i = 0; i < strlen(pass); i++) {
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

bool config_save_weather(const char* api_key, const char* location) {
#ifdef ESP32
  prefs.putString(CONFIG_KEY_WEATHER_API_KEY, api_key);
  prefs.putString(CONFIG_KEY_WEATHER_LOCATION, location);
  return true;
#elif defined(ESP8266)
  // Stub for ESP8266
  return false;
#endif
}

bool config_load_weather(char* api_key, char* location) {
#ifdef ESP32
  String k = prefs.getString(CONFIG_KEY_WEATHER_API_KEY, "");
  String l = prefs.getString(CONFIG_KEY_WEATHER_LOCATION, "");
  if (k.length() == 0) return false;
  strcpy(api_key, k.c_str());
  strcpy(location, l.c_str());
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

bool config_save_timezone(int offset_sec) {
#ifdef ESP32
  prefs.putInt(CONFIG_KEY_TIMEZONE, offset_sec);
  return true;
#elif defined(ESP8266)
  // Store timezone offset at EEPROM address 101-104 (4 bytes for int)
  EEPROM.write(101, (offset_sec >> 24) & 0xFF);
  EEPROM.write(102, (offset_sec >> 16) & 0xFF);
  EEPROM.write(103, (offset_sec >> 8) & 0xFF);
  EEPROM.write(104, offset_sec & 0xFF);
  return EEPROM.commit();
#endif
}

bool config_load_timezone(int* offset_sec) {
#ifdef ESP32
  *offset_sec = prefs.getInt(CONFIG_KEY_TIMEZONE, NTP_OFFSET_SEC);
  return true;
#elif defined(ESP8266)
  // Read timezone offset from EEPROM address 101-104
  int stored = (EEPROM.read(101) << 24) | 
               (EEPROM.read(102) << 16) | 
               (EEPROM.read(103) << 8) | 
               EEPROM.read(104);
  // Check if it's a valid timezone offset (-43200 to +50400 seconds, -12h to +14h)
  if (stored >= -43200 && stored <= 50400) {
    *offset_sec = stored;
    return true;
  }
  *offset_sec = NTP_OFFSET_SEC;
  return false;
#endif
}

bool config_save_contrast(uint8_t level) {
#ifdef ESP32
  prefs.putUChar(CONFIG_KEY_CONTRAST, level);
  return true;
#elif defined(ESP8266)
  // Store contrast at EEPROM address 105
  EEPROM.write(105, level);
  return EEPROM.commit();
#endif
}

bool config_load_contrast(uint8_t* level) {
#ifdef ESP32
  *level = prefs.getUChar(CONFIG_KEY_CONTRAST, 128);
  return true;
#elif defined(ESP8266)
  // Read contrast from EEPROM address 105
  uint8_t stored = EEPROM.read(105);
  // Check if it's a valid value (0 was never set, use default)
  if (stored == 0 || stored == 255) {
    *level = 128;
    return false;
  }
  *level = stored;
  return true;
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
