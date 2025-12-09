#pragma once

#include <Arduino.h>

// WiFi manager with robust reconnection and internet checking
// Based on parola_matrix.ino implementation

bool WifiInit();
bool WifiConnect(const char* ssid, const char* pass);
bool WifiStartAp();
bool WifiStartPortal();  // Start AP mode with web configuration portal
bool WifiStopPortal();   // Stop portal and switch to STA mode
bool WifiIsConnected();
bool WifiHasInternet();
bool WifiIsApMode();
bool WifiIsPortalMode();
bool WifiCheckInternet();
void WifiReconnectTask();
void WifiPortalLoop();   // Call in loop when portal is active
void WifiDisconnect();
bool WifiHasSavedCredentials();

String WifiGetIp();
String WifiGetSsid();
int WifiGetRssi();