#pragma once
#include <Arduino.h>

// --- Connectivity Manager ---
// Combines WifiManager, NtpClient, and CloudSync

// WiFi Functions
bool WifiInit();
bool WifiConnect(const char* ssid, const char* pass);
bool WifiStartAp();
bool WifiStartPortal();
bool WifiStopPortal();
bool WifiIsConnected();
bool WifiHasInternet();
bool WifiIsApMode();
bool WifiIsPortalMode();
bool WifiCheckInternet();
void WifiReconnectTask();
void WifiPortalLoop();
void WifiDisconnect();
bool WifiHasSavedCredentials();

String WifiGetIp();
String WifiGetSsid();
int WifiGetRssi();

// NTP Functions
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

// Cloud Functions
void CloudInit();
void CloudSyncPrefs();
void CloudSyncLogs();
bool CloudIsConnected();
