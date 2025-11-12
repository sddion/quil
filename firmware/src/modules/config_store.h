#pragma once
#include <Arduino.h>

bool config_init();
bool config_save_wifi(const char* ssid, const char* pass);
bool config_load_wifi(char* ssid, char* pass);
bool config_save_string(const char* key, const char* val);
String config_load_string(const char* key);
void config_clear();
