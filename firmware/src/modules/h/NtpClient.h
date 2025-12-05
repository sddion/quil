#pragma once

#include <Arduino.h>

// NTP Client using WiFiUDP - more reliable than configTime()
// Based on working parola_matrix.ino implementation

void NtpInit();
void NtpUpdate();
void NtpSetTimezone(int offset_sec);
String NtpGetTime();
String NtpGetDate();
String NtpGetDay();
int NtpGetHour();
int NtpGetMinute();
int NtpGetSecond();
bool NtpIsSynced();