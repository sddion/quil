#include "../h/WebPortal.h"
#include "../h/WifiManager.h"
#include "../h/ConfigStore.h"
#include "../h/BatteryManager.h"
#include "hal/h/Display.h"
#include "modes/h/Time.h"
#include "config.h"
#include <WiFi.h>
#include <DNSServer.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <ESPmDNS.h>

// DNS server for captive portal redirect
static DNSServer dnsServer;
static const byte DNS_PORT = 53;

// Async web server
static AsyncWebServer server(80);

// State
static bool portalActive = false;
static bool littleFsInitialized = false;
static WifiCredentialsCallback credentialsCallback = nullptr;

// Connection state for polling
static bool connecting = false;
static bool connectionFailed = false;
static String pendingSsid = "";
static String pendingPassword = "";

// Forward declarations
static void handleRoot(AsyncWebServerRequest *request);
static void handleScan(AsyncWebServerRequest *request);
static void handleConnect(AsyncWebServerRequest *request, uint8_t *data, size_t len);
static void handleStatus(AsyncWebServerRequest *request);
static void handleNotFound(AsyncWebServerRequest *request);

bool WebPortalInit() {
  // Initialize LittleFS
  if (!LittleFS.begin(true)) {
    Serial.println("[WebPortal] LittleFS mount failed");
    return false;
  }
  
  littleFsInitialized = true;
  Serial.println("[WebPortal] LittleFS mounted");
  
  // List files for debugging
  File root = LittleFS.open("/");
  File file = root.openNextFile();
  Serial.println("[WebPortal] Files:");
  while (file) {
    Serial.printf("  - %s (%d bytes)\n", file.name(), file.size());
    file = root.openNextFile();
  }
  
  return true;
}

void WebPortalStart() {
  if (!littleFsInitialized) {
    if (!WebPortalInit()) {
      Serial.println("[WebPortal] Failed to initialize, cannot start");
      return;
    }
  }
  
  // Start DNS server for captive portal
  dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());
  Serial.println("[WebPortal] DNS server started");
  
  // Serve static files from LittleFS
  server.serveStatic("/", LittleFS, "/").setDefaultFile("index.html");
  
  // API endpoints
  server.on("/api/scan", HTTP_GET, handleScan);
  
  server.on("/api/connect", HTTP_POST, 
    [](AsyncWebServerRequest *request) {},
    NULL,
    [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
      if (index == 0 && len == total) {
        // Simple case: entire body in one chunk
        handleConnect(request, data, len);
      } else {
        // For chunked bodies, would need accumulation buffer
        request->send(400, "application/json", "{\"success\":false,\"message\":\"Request too large\"}");
      }
    }
  );
  
  // Config endpoint for app to send settings
  server.on("/api/config", HTTP_POST, 
    [](AsyncWebServerRequest *request) {},
    NULL,
    [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
      if (index == 0 && len == total) {
        Serial.println("[WebPortal] Received config from app");
        
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, data, len);
        
        if (error) {
          request->send(400, "application/json", "{\"success\":false,\"message\":\"Invalid JSON\"}");
          return;
        }
        
        // Handle command
        if (doc.containsKey("cmd")) {
          const char* cmd = doc["cmd"];
          if (strcmp(cmd, "restart") == 0) {
            request->send(200, "application/json", "{\"success\":true}");
            delay(500);
            ESP.restart();
          } else if (strcmp(cmd, "reset") == 0) {
            ConfigClear();
            request->send(200, "application/json", "{\"success\":true}");
            delay(500);
            ESP.restart();
          }
          return;
        }
        
        // Apply config values
        if (doc.containsKey("tz")) {
          int tz = doc["tz"];
          ConfigSaveTimezone(tz);
          Serial.printf("[WebPortal] Timezone saved: %d\n", tz);
        }
        
        if (doc.containsKey("brightness")) {
          uint8_t brightness = doc["brightness"];
          ConfigSaveContrast(brightness);
          DisplaySetContrast(brightness);
          Serial.printf("[WebPortal] Brightness saved: %d\n", brightness);
        }
        
        if (doc.containsKey("theme")) {
          uint8_t theme = doc["theme"];
          TimeSetTheme((DisplayTheme_t)theme);
          Serial.printf("[WebPortal] Theme saved: %d\n", theme);
        }
        
        if (doc.containsKey("ssid") && doc.containsKey("password")) {
          const char* ssid = doc["ssid"];
          const char* pass = doc["password"];
          ConfigSaveWifi(ssid, pass);
          Serial.println("[WebPortal] WiFi config saved");
        }
        
        if (doc.containsKey("wk") && doc.containsKey("wl")) {
          const char* wk = doc["wk"];
          const char* wl = doc["wl"];
          ConfigSaveWeather(wk, wl);
          Serial.println("[WebPortal] Weather config saved");
        }
        
        request->send(200, "application/json", "{\"success\":true}");
      } else {
        request->send(400, "application/json", "{\"success\":false,\"message\":\"Request too large\"}");
      }
    }
  );
  
  server.on("/api/status", HTTP_GET, handleStatus);
  
  // Captive portal detection endpoints
  server.on("/generate_204", HTTP_GET, handleRoot);        // Android
  server.on("/fwlink", HTTP_GET, handleRoot);              // Windows
  server.on("/hotspot-detect.html", HTTP_GET, handleRoot); // Apple
  server.on("/connecttest.txt", HTTP_GET, handleRoot);     // Windows 10
  
  // Handle all other requests (redirect to portal)
  server.onNotFound(handleNotFound);
  
  server.begin();
  portalActive = true;
  
  Serial.println("[WebPortal] Web server started");
  Serial.printf("[WebPortal] Portal URL: http://%s\n", WiFi.softAPIP().toString().c_str());
}

void WebPortalStop() {
  if (portalActive) {
    server.end();
    dnsServer.stop();
    portalActive = false;
    Serial.println("[WebPortal] Stopped");
  }
}

void WebPortalLoop() {
  if (!portalActive) return;
  
  dnsServer.processNextRequest();
  
  // Check if connection attempt completed
  if (connecting) {
    if (WifiIsConnected()) {
      connecting = false;
      connectionFailed = false;
      Serial.println("[WebPortal] WiFi connection successful!");
    } else if (millis() > 0) {
      // Still trying, WifiConnect handles timeout internally
    }
  }
}

bool WebPortalIsActive() {
  return portalActive;
}

void WebPortalSetCredentialsCallback(WifiCredentialsCallback callback) {
  credentialsCallback = callback;
}

// ============ Request Handlers ============

static void handleRoot(AsyncWebServerRequest *request) {
  request->redirect("/");
}

static void handleScan(AsyncWebServerRequest *request) {
  Serial.println("[WebPortal] Scanning WiFi networks...");
  
  int n = WiFi.scanNetworks();
  
  JsonDocument doc;
  JsonArray networks = doc["networks"].to<JsonArray>();
  
  for (int i = 0; i < n; i++) {
    // Skip duplicates
    bool duplicate = false;
    for (int j = 0; j < i; j++) {
      if (WiFi.SSID(i) == WiFi.SSID(j)) {
        duplicate = true;
        break;
      }
    }
    if (duplicate || WiFi.SSID(i).length() == 0) continue;
    
    JsonObject network = networks.add<JsonObject>();
    network["ssid"] = WiFi.SSID(i);
    network["rssi"] = WiFi.RSSI(i);
    network["secure"] = (WiFi.encryptionType(i) != WIFI_AUTH_OPEN);
  }
  
  WiFi.scanDelete();
  
  String response;
  serializeJson(doc, response);
  
  request->send(200, "application/json", response);
  Serial.printf("[WebPortal] Found %d networks\n", networks.size());
}

static void handleConnect(AsyncWebServerRequest *request, uint8_t *data, size_t len) {
  Serial.println("[WebPortal] Received connect request");
  
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, data, len);
  
  if (error) {
    request->send(400, "application/json", "{\"success\":false,\"message\":\"Invalid JSON\"}");
    return;
  }
  
  const char* ssid = doc["ssid"];
  const char* password = doc["password"];
  
  if (!ssid || strlen(ssid) == 0) {
    request->send(400, "application/json", "{\"success\":false,\"message\":\"SSID required\"}");
    return;
  }
  
  Serial.printf("[WebPortal] Connecting to: %s\n", ssid);
  
  // Store pending credentials
  pendingSsid = String(ssid);
  pendingPassword = String(password ? password : "");
  connecting = true;
  connectionFailed = false;
  
  // Notify callback if set
  if (credentialsCallback) {
    credentialsCallback(ssid, password ? password : "");
  }
  
  // Start connection attempt in background
  // Switch WiFi mode to allow both AP and STA
  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(ssid, password);
  
  request->send(200, "application/json", "{\"success\":true,\"message\":\"Connecting...\"}");
}

static void handleStatus(AsyncWebServerRequest *request) {
  JsonDocument doc;
  
  // WiFi status
  doc["wifiConnected"] = (WiFi.status() == WL_CONNECTED);
  doc["wifiSSID"] = WiFi.SSID();
  doc["ip"] = WiFi.localIP().toString();
  
  // Device info
  doc["firmwareVersion"] = FIRMWARE_VERSION;
  doc["battery"] = BatteryGetPercentage();
  
  // Load saved config
  int tz = 0;
  ConfigLoadTimezone(&tz);
  doc["timezone"] = tz;
  
  uint8_t brightness = 128;
  ConfigLoadContrast(&brightness);
  doc["brightness"] = brightness;
  
  uint8_t theme = 0;
  ConfigLoadTheme(&theme);
  doc["theme"] = theme;
  
  String response;
  serializeJson(doc, response);
  request->send(200, "application/json", response);
}

static void handleNotFound(AsyncWebServerRequest *request) {
  // Redirect all unknown requests to the portal
  request->redirect("http://" + WiFi.softAPIP().toString() + "/");
}
