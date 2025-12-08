#pragma once

#include <Arduino.h>

// Initialize the web portal (LittleFS + async server)
bool WebPortalInit();

// Start the captive portal (call after WifiStartAp)
void WebPortalStart();

// Stop the web portal
void WebPortalStop();

// Process DNS and web server (call in loop when portal is active)
void WebPortalLoop();

// Check if portal is active
bool WebPortalIsActive();

// Set callback for when WiFi credentials are received
typedef void (*WifiCredentialsCallback)(const char* ssid, const char* password);
void WebPortalSetCredentialsCallback(WifiCredentialsCallback callback);
