#include "../h/WifiManager.h"
#include "../h/ConfigStore.h"
#include "config.h"
#ifdef ESP32
#include <WiFi.h>
#include <HTTPClient.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#endif

// WiFi state tracking
static bool wifi_connected = false;
static bool internet_connected = false;
static bool ap_mode = false;
static unsigned long last_wifi_check = 0;
static unsigned long last_reconnect_attempt = 0;
static const unsigned long WIFI_CHECK_INTERVAL = 10000;    // Check every 10 seconds
static const unsigned long RECONNECT_INTERVAL = 5000;      // Retry every 5 seconds
static const unsigned long INITIAL_CONNECT_TIMEOUT = 20000; // 20 seconds for initial connection

static char saved_ssid[33] = {0};
static char saved_pass[65] = {0};

bool wifi_init() {
  // Load saved credentials from EEPROM
  if (config_load_wifi(saved_ssid, saved_pass)) {
    Serial.println("[WiFi] Loaded credentials from EEPROM");
    Serial.print("[WiFi] SSID: ");
    Serial.println(saved_ssid);
    return true;
  }
  Serial.println("[WiFi] No saved credentials found");
  return false;
}

bool wifi_connect(const char* ssid, const char* pass) {
  if (ssid == NULL || pass == NULL || strlen(ssid) == 0) {
    Serial.println("[WiFi] Invalid credentials");
    return false;
  }
  
  // Save credentials for reconnection
  strncpy(saved_ssid, ssid, sizeof(saved_ssid) - 1);
  strncpy(saved_pass, pass, sizeof(saved_pass) - 1);
  saved_ssid[sizeof(saved_ssid) - 1] = '\0';
  saved_pass[sizeof(saved_pass) - 1] = '\0';
  
  // Save to EEPROM
  config_save_wifi(saved_ssid, saved_pass);
  
  Serial.print("[WiFi] Connecting to: ");
  Serial.println(ssid);
  
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  
  #ifdef ESP8266
  WiFi.persistent(true);
  WiFi.setSleepMode(WIFI_NONE_SLEEP);
  #endif
  
  WiFi.begin(ssid, pass);
  
  unsigned long start = millis();
  Serial.print("[WiFi] Connecting");
  
  while (WiFi.status() != WL_CONNECTED && millis() - start < INITIAL_CONNECT_TIMEOUT) {
    delay(100);
    Serial.print(".");
    yield();
  }
  Serial.println();
  
  if (WiFi.status() == WL_CONNECTED) {
    wifi_connected = true;
    ap_mode = false;
    Serial.print("[WiFi] Connected! IP: ");
    Serial.println(WiFi.localIP());
    
    // Check internet connectivity
    internet_connected = wifi_check_internet();
    if (internet_connected) {
      Serial.println("[WiFi] Internet access confirmed");
    } else {
      Serial.println("[WiFi] Connected but no internet access");
    }
    
    return true;
  }
  
  Serial.println("[WiFi] Initial connection failed");
  Serial.println("[WiFi] Will retry in background...");
  wifi_connected = false;
  last_reconnect_attempt = millis();
  
  return false;
}

bool wifi_start_ap() {
  Serial.println("[WiFi] Starting Access Point mode");
  WiFi.mode(WIFI_AP);
  
  bool result = WiFi.softAP(WIFI_AP_SSID, WIFI_AP_PASS);
  
  if (result) {
    ap_mode = true;
    wifi_connected = false;
    internet_connected = false;
    Serial.print("[WiFi] AP started - SSID: ");
    Serial.print(WIFI_AP_SSID);
    Serial.print(", Password: ");
    Serial.println(WIFI_AP_PASS);
    Serial.print("[WiFi] AP IP: ");
    Serial.println(WiFi.softAPIP());
  } else {
    Serial.println("[WiFi] Failed to start AP");
  }
  
  return result;
}

bool wifi_check_internet() {
  if (WiFi.status() != WL_CONNECTED) {
    return false;
  }
  
  WiFiClient client;
  HTTPClient http;
  
  // Use Google's captive portal check endpoint
  http.begin(client, "http://clients3.google.com/generate_204");
  http.setTimeout(5000);
  
  int httpCode = http.GET();
  http.end();
  
  return (httpCode == 204 || httpCode == 200);
}

void wifi_reconnect_task() {
  // Don't run reconnection in AP mode
  if (ap_mode) {
    return;
  }
  
  unsigned long now = millis();
  
  // Check WiFi and internet status periodically
  if (now - last_wifi_check > WIFI_CHECK_INTERVAL) {
    last_wifi_check = now;
    
    bool wifi_ok = (WiFi.status() == WL_CONNECTED);
    bool inet_ok = wifi_ok && wifi_check_internet();
    
    // Detect WiFi/Internet loss
    if (!wifi_ok || !inet_ok) {
      if (wifi_connected || internet_connected) {
        Serial.println("[WiFi] Connection/Internet lost!");
        wifi_connected = wifi_ok;
        internet_connected = inet_ok;
      }
      
      // Attempt reconnection if enough time has passed
      if (now - last_reconnect_attempt > RECONNECT_INTERVAL) {
        last_reconnect_attempt = now;
        
        if (!wifi_ok && strlen(saved_ssid) > 0) {
          Serial.println("[WiFi] Attempting reconnection...");
          WiFi.disconnect();
          delay(100);
          WiFi.begin(saved_ssid, saved_pass);
        }
      }
    }
    // Detect WiFi/Internet restoration
    else if (!wifi_connected || !internet_connected) {
      Serial.println("[WiFi] Connection/Internet restored!");
      Serial.print("[WiFi] IP: ");
      Serial.println(WiFi.localIP());
      wifi_connected = true;
      internet_connected = true;
    }
  }
}

bool wifi_is_connected() {
  return wifi_connected && (WiFi.status() == WL_CONNECTED);
}

bool wifi_has_internet() {
  return internet_connected;
}

bool wifi_is_ap_mode() {
  return ap_mode;
}

String wifi_get_ip() {
  if (ap_mode) {
    return WiFi.softAPIP().toString();
  }
  return WiFi.localIP().toString();
}

String wifi_get_ssid() {
  if (ap_mode) {
    return String(WIFI_AP_SSID);
  }
  return WiFi.SSID();
}

int wifi_get_rssi() {
  if (ap_mode) {
    return 0;
  }
  return WiFi.RSSI();
}

void wifi_disconnect() {
  Serial.println("[WiFi] Disconnecting...");
  WiFi.disconnect();
  wifi_connected = false;
  internet_connected = false;
}

bool wifi_has_saved_credentials() {
  char ssid[33], pass[65];
  return config_load_wifi(ssid, pass) && strlen(ssid) > 0;
}