#include "wifi_manager.h"
#ifdef ESP32
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif
#include "config.h"

bool wifi_init() {
  WiFi.mode(WIFI_STA);
  return true;
}

bool wifi_connect(const char* ssid, const char* pass) {
  WiFi.begin(ssid, pass);
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED) {
    if (millis() - start > WIFI_TIMEOUT_MS) return false;
    delay(100);
  }
  return true;
}

bool wifi_start_ap() {
  WiFi.mode(WIFI_AP);
  return WiFi.softAP(WIFI_AP_SSID, WIFI_AP_PASS);
}

bool wifi_is_connected() {
  return WiFi.status() == WL_CONNECTED;
}

String wifi_get_ip() {
  return WiFi.localIP().toString();
}

int wifi_get_rssi() {
  return WiFi.RSSI();
}
