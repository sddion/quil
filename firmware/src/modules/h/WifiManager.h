#pragma once

#include <Arduino.h>

// WiFi manager with robust reconnection and internet checking
// Based on parola_matrix.ino implementation

bool wifi_init();
bool wifi_connect(const char* ssid, const char* pass);
bool wifi_start_ap();
bool wifi_is_connected();
bool wifi_has_internet();
bool wifi_is_ap_mode();
bool wifi_check_internet();
void wifi_reconnect_task();
void wifi_disconnect();
bool wifi_has_saved_credentials();

String wifi_get_ip();
String wifi_get_ssid();
int wifi_get_rssi();