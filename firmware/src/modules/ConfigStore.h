#pragma once
#include <Arduino.h>

bool ConfigInit();
bool ConfigSaveWifi(const char* ssid, const char* pass);
bool ConfigLoadWifi(char* ssid, char* pass);
bool ConfigSaveString(const char* key, const char* val);
String ConfigLoadString(const char* key);
bool ConfigSaveTheme(uint8_t theme);
bool ConfigLoadTheme(uint8_t* theme);
bool ConfigSaveWeather(const char* api_key, const char* location);
bool ConfigLoadWeather(char* api_key, char* location);
bool ConfigSaveTimezone(int offset_sec);
bool ConfigLoadTimezone(int* offset_sec);
bool ConfigSaveContrast(uint8_t level);
bool ConfigLoadContrast(uint8_t* level);
bool ConfigLoadServerUrl(char* url);
void ConfigClear();
