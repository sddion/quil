#include "../h/ConfigStore.h"
#include "config.h"

#include <Preferences.h>
static Preferences prefs;
#define CONFIG_KEY_THEME "theme"
#define CONFIG_KEY_WEATHER_API_KEY "weather_api_key"
#define CONFIG_KEY_WEATHER_LOCATION "weather_location"
#define CONFIG_KEY_TIMEZONE "timezone"
#define CONFIG_KEY_CONTRAST "contrast"
#define CONFIG_KEY_FIRST_BOOT "first_boot"

bool ConfigInit() {
  return prefs.begin(CONFIG_NAMESPACE, false);
}

bool ConfigSaveWifi(const char* ssid, const char* pass) {
  prefs.putString(CONFIG_KEY_SSID, ssid);
  prefs.putString(CONFIG_KEY_PASS, pass);
  return true;
}

bool ConfigLoadWifi(char* ssid, char* pass) {
  String s = prefs.getString(CONFIG_KEY_SSID, "");
  String p = prefs.getString(CONFIG_KEY_PASS, "");
  if (s.length() == 0) return false;
  strcpy(ssid, s.c_str());
  strcpy(pass, p.c_str());
  return true;
}

bool ConfigSaveTheme(uint8_t theme) {
  return prefs.putUChar(CONFIG_KEY_THEME, theme) > 0;
}

bool ConfigLoadTheme(uint8_t* theme) {
  *theme = prefs.getUChar(CONFIG_KEY_THEME, 0);
  return true;
}

bool ConfigSaveWeather(const char* api_key, const char* location) {
  prefs.putString(CONFIG_KEY_WEATHER_API_KEY, api_key);
  prefs.putString(CONFIG_KEY_WEATHER_LOCATION, location);
  return true;
}

bool ConfigLoadWeather(char* api_key, char* location) {
  String k = prefs.getString(CONFIG_KEY_WEATHER_API_KEY, "");
  String l = prefs.getString(CONFIG_KEY_WEATHER_LOCATION, "");
  if (k.length() == 0) return false;
  strcpy(api_key, k.c_str());
  strcpy(location, l.c_str());
  return true;
}

bool ConfigSaveString(const char* key, const char* val) {
  prefs.putString(key, val);
  return true;
}

String ConfigLoadString(const char* key) {
  return prefs.getString(key, "");
}

bool ConfigSaveTimezone(int offset_sec) {
  prefs.putInt(CONFIG_KEY_TIMEZONE, offset_sec);
  return true;
}

bool ConfigLoadTimezone(int* offset_sec) {
  *offset_sec = prefs.getInt(CONFIG_KEY_TIMEZONE, NTP_OFFSET_SEC);
  return true;
}

bool ConfigSaveContrast(uint8_t level) {
  prefs.putUChar(CONFIG_KEY_CONTRAST, level);
  return true;
}

bool ConfigLoadContrast(uint8_t* level) {
  *level = prefs.getUChar(CONFIG_KEY_CONTRAST, 128);
  return true;
}

void ConfigClear() {
  prefs.clear();
}

