#pragma once

#include <Arduino.h>

// WiFi manager with robust reconnection and internet checking
// Based on parola_matrix.ino implementation

bool WifiInit();
bool WifiConnect(const char* ssid, const char* pass);
bool WifiStartAp();
bool WifiIsConnected();
bool WifiHasInternet();
bool WifiIsApMode();
bool WifiCheckInternet();
void WifiReconnectTask();
void WifiDisconnect();
bool WifiHasSavedCredentials();

String WifiGetIp();
String WifiGetSsid();
int WifiGetRssi();