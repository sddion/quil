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
  Serial.printf("Attempting to connect to SSID: %s\n", ssid);
  WiFi.begin(ssid, pass);
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED) {
    Serial.printf("WiFi Status: %d\n", WiFi.status());
    if (millis() - start > WIFI_TIMEOUT_MS) {
      Serial.println("WiFi connection timed out.");
      return false;
    }
    delay(100);
  }
  Serial.println("WiFi connected successfully.");
  return true;
}

bool wifi_start_ap() {
  Serial.println("Starting AP mode...");
  WiFi.mode(WIFI_AP);
  bool result = WiFi.softAP(WIFI_AP_SSID, WIFI_AP_PASS);
  Serial.printf("AP mode started: %s\n", result ? "true" : "false");
  Serial.printf("AP SSID: %s, Password: %s\n", WIFI_AP_SSID, WIFI_AP_PASS);
  return result;
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
