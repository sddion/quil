#pragma once

#include <Arduino.h>

// NTP Client using WiFiUDP - more reliable than configTime()
// Based on working parola_matrix.ino implementation

void ntp_init();
void ntp_update();
void ntp_set_timezone(int offset_sec);
String ntp_get_time();
String ntp_get_date();
String ntp_get_day();
int ntp_get_hour();
int ntp_get_minute();
int ntp_get_second();
bool ntp_is_synced();