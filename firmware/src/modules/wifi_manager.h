#pragma once
#include <Arduino.h>

bool wifi_init();
bool wifi_connect(const char* ssid, const char* pass);
bool wifi_start_ap();
bool wifi_is_connected();
String wifi_get_ip();
int wifi_get_rssi();
