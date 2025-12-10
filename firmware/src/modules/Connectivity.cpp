#include "config.h"
#include "Connectivity.h"
#include "ConfigStore.h"

#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiUdp.h>
#include <NTPClient.h>

// =======================
// WiFi Manager Implementation
// =======================

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

bool WifiInit() {
  // Load saved credentials from EEPROM
  if (ConfigLoadWifi(saved_ssid, saved_pass)) {
    Serial.println("[WiFi] Loaded credentials from EEPROM");
    Serial.print("[WiFi] SSID: ");
    Serial.println(saved_ssid);
    return true;
  }
  Serial.println("[WiFi] No saved credentials found");
  return false;
}

bool WifiConnect(const char* ssid, const char* pass) {
  if (ssid == NULL || pass == NULL || strlen(ssid) == 0) {
    Serial.println("[WiFi] Invalid credentials");
    return false;
  }
  
  strncpy(saved_ssid, ssid, sizeof(saved_ssid) - 1);
  strncpy(saved_pass, pass, sizeof(saved_pass) - 1);
  saved_ssid[sizeof(saved_ssid) - 1] = '\0';
  saved_pass[sizeof(saved_pass) - 1] = '\0';
  
  ConfigSaveWifi(saved_ssid, saved_pass);
  
  Serial.print("[WiFi] Connecting to: ");
  Serial.println(ssid);
  
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  
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
    
    internet_connected = WifiCheckInternet();
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

bool WifiStartAp() {
  Serial.println("[WiFi] Starting Access Point mode");
  WiFi.mode(WIFI_AP);
  WiFi.setTxPower(WIFI_POWER_8_5dBm); // Prevent brownouts
  
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

bool WifiCheckInternet() {
  if (WiFi.status() != WL_CONNECTED) {
    return false;
  }
  
  WiFiClient client;
  HTTPClient http;
  
  http.begin(client, "http://clients3.google.com/generate_204");
  http.setTimeout(5000);
  
  int httpCode = http.GET();
  http.end();
  
  return (httpCode == 204 || httpCode == 200);
}

void WifiReconnectTask() {
  if (ap_mode) {
    return;
  }
  
  unsigned long now = millis();
  
  if (now - last_wifi_check > WIFI_CHECK_INTERVAL) {
    last_wifi_check = now;
    
    bool wifi_ok = (WiFi.status() == WL_CONNECTED);
    bool inet_ok = wifi_ok && WifiCheckInternet();
    
    if (!wifi_ok || !inet_ok) {
      if (wifi_connected || internet_connected) {
        Serial.println("[WiFi] Connection/Internet lost!");
        wifi_connected = wifi_ok;
        internet_connected = inet_ok;
      }
      
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
    else if (!wifi_connected || !internet_connected) {
      Serial.println("[WiFi] Connection/Internet restored!");
      Serial.print("[WiFi] IP: ");
      Serial.println(WiFi.localIP());
      wifi_connected = true;
      internet_connected = true;
    }
  }
}

bool WifiIsConnected() {
  return wifi_connected && (WiFi.status() == WL_CONNECTED);
}

bool WifiHasInternet() {
  return internet_connected;
}

bool WifiIsApMode() {
  return ap_mode;
}

String WifiGetIp() {
  if (ap_mode) {
    return WiFi.softAPIP().toString();
  }
  return WiFi.localIP().toString();
}

String WifiGetSsid() {
  if (ap_mode) {
    return String(WIFI_AP_SSID);
  }
  return WiFi.SSID();
}

int WifiGetRssi() {
  if (ap_mode) {
    return 0;
  }
  return WiFi.RSSI();
}

void WifiDisconnect() {
  Serial.println("[WiFi] Disconnecting...");
  WiFi.disconnect();
  wifi_connected = false;
  internet_connected = false;
}

bool WifiHasSavedCredentials() {
  char ssid[33], pass[65];
  return ConfigLoadWifi(ssid, pass) && strlen(ssid) > 0;
}

// ============ Portal Mode Functions ============

#include "WebPortal.h" // Expecting this to be moved to src/modules/WebPortal.h

static bool portal_mode = false;

bool WifiStartPortal() {
  Serial.println("[WiFi] Starting configuration portal...");
  
  if (!WifiStartAp()) {
    Serial.println("[WiFi] Failed to start AP for portal");
    return false;
  }
  
  if (!WebPortalInit()) {
    Serial.println("[WiFi] Failed to init web portal");
    WiFi.softAPdisconnect(true);
    ap_mode = false;
    return false;
  }
  
  WebPortalStart();
  portal_mode = true;
  
  Serial.println("[WiFi] Portal mode active");
  return true;
}

bool WifiStopPortal() {
  if (portal_mode) {
    WebPortalStop();
    WiFi.softAPdisconnect(true);
    ap_mode = false;
    portal_mode = false;
    Serial.println("[WiFi] Portal stopped");
  }
  return true;
}

bool WifiIsPortalMode() {
  return portal_mode;
}

static unsigned long portal_connected_time = 0;
static const unsigned long PORTAL_STOP_DELAY = 3000; 

void WifiPortalLoop() {
  if (!portal_mode) return;
  
  WebPortalLoop();
  
  if (WiFi.status() == WL_CONNECTED && wifi_connected == false) {
    Serial.println("[WiFi] Connected via portal!");
    Serial.print("[WiFi] IP: ");
    Serial.println(WiFi.localIP());
    
    // Save the current running config
    // Since we are connected, the current STA credentials are valid.
    // However, we don't have easy access to them here unless we use WiFi.SSID() and password? 
    // WiFi.psk() gives the password.
    ConfigSaveWifi(WiFi.SSID().c_str(), WiFi.psk().c_str());
    Serial.println("[WiFi] Credentials saved to persistent storage");

    wifi_connected = true;
    ap_mode = false; 
    internet_connected = WifiCheckInternet();
    portal_connected_time = millis();
  }
}

// =======================
// NTP Client Implementation
// =======================

static WiFiUDP ntpUDP;
static NTPClient timeClient(ntpUDP, NTP_SERVER, NTP_OFFSET_SEC, 60000);
static bool synced = false;
static int current_offset = NTP_OFFSET_SEC;

void NtpInit() {
  timeClient.begin();
  timeClient.update();
  synced = timeClient.isTimeSet();
}

void NtpUpdate() {
  if (timeClient.update()) {
    synced = true;
  }
}

void NtpSetTimezone(int offset_sec) {
  current_offset = offset_sec;
  timeClient.setTimeOffset(offset_sec);
  timeClient.forceUpdate();
}

String NtpGetTime() {
  if (!synced || !timeClient.isTimeSet()) return "--:--";
  
  int hr = timeClient.getHours();
  int mn = timeClient.getMinutes();
  
  char buf[16];
  snprintf(buf, sizeof(buf), "%02d:%02d", hr, mn);
  return String(buf);
}

String NtpGetDate() {
  if (!synced || !timeClient.isTimeSet()) return "----/--/--";
  
  time_t rawTime = timeClient.getEpochTime();
  struct tm* tmInfo = localtime(&rawTime);
  
  char buf[32];
  snprintf(buf, sizeof(buf), "%04d/%02d/%02d", 
           tmInfo->tm_year + 1900, 
           tmInfo->tm_mon + 1, 
           tmInfo->tm_mday);
  return String(buf);
}

String NtpGetDay() {
  if (!synced || !timeClient.isTimeSet()) return "---";
  
  time_t rawTime = timeClient.getEpochTime();
  struct tm* tmInfo = localtime(&rawTime);
  
  static const char* daynames[] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
  return String(daynames[tmInfo->tm_wday]);
}

int NtpGetHour() {
  if (!synced || !timeClient.isTimeSet()) return 0;
  return timeClient.getHours();
}

int NtpGetMinute() {
  if (!synced || !timeClient.isTimeSet()) return 0;
  return timeClient.getMinutes();
}

int NtpGetSecond() {
  if (!synced || !timeClient.isTimeSet()) return 0;
  return timeClient.getSeconds();
}

bool NtpIsSynced() {
  return synced && timeClient.isTimeSet();
}

// =======================
// Cloud Sync Implementation
// =======================

static bool cloud_connected = false;

void CloudInit() {
  cloud_connected = false;
}

void CloudSyncPrefs() {}

void CloudSyncLogs() {}

bool CloudIsConnected() {
  return cloud_connected;
}
