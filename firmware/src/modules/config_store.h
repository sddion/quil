#pragma once
#include <Arduino.h>

bool config_init();
bool config_save_wifi(const char* ssid, const char* pass);
bool config_load_wifi(char* ssid, char* pass);
bool config_save_string(const char* key, const char* val);
String config_load_string(const char* key);
bool config_save_theme(uint8_t theme);
bool config_load_theme(uint8_t* theme);
bool config_save_weather(const char* api_key, const char* location);
bool config_load_weather(char* api_key, char* location);
void config_clear();
