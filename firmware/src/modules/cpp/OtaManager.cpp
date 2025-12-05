#include "../h/OtaManager.h"
#include "config.h"
#include "../h/WifiManager.h"

#include <ArduinoOTA.h>
#include <HTTPClient.h>
#include <Update.h>
#include <ArduinoJson.h>

void OtaInit() {
  ArduinoOTA.setHostname("quil");
  ArduinoOTA.begin();
  Serial.print("[OTA] Current firmware version: ");
  Serial.println(FIRMWARE_VERSION);
}

void OtaHandle() {
  ArduinoOTA.handle();
}

static bool downloadAndFlash(HTTPClient& http, int contentLength) {
  if (!Update.begin(contentLength)) {
    Serial.println("[OTA] Not enough space");
    return false;
  }
  
  WiFiClient* stream = http.getStreamPtr();
  size_t written = Update.writeStream(*stream);
  
  if (written != contentLength) {
    Serial.printf("[OTA] Write failed: %d/%d\n", written, contentLength);
    Update.abort();
    return false;
  }
  
  if (!Update.end() || !Update.isFinished()) {
    Serial.println("[OTA] Flash failed");
    return false;
  }
  
  return true;
}

bool OtaStart(const char* url) {
  if (!WifiIsConnected()) {
    Serial.println("[OTA] No WiFi");
    return false;
  }
  
  Serial.print("[OTA] Downloading: ");
  Serial.println(url);
  
  HTTPClient http;
  http.begin(url);
  http.setTimeout(30000);
  
  int httpCode = http.GET();
  if (httpCode != HTTP_CODE_OK) {
    Serial.printf("[OTA] GET failed: %d\n", httpCode);
    http.end();
    return false;
  }
  
  int size = http.getSize();
  if (size <= 0) {
    Serial.println("[OTA] Invalid size");
    http.end();
    return false;
  }
  
  Serial.printf("[OTA] Size: %d bytes\n", size);
  bool success = downloadAndFlash(http, size);
  http.end();
  
  if (success) {
    Serial.println("[OTA] Success! Restarting...");
    delay(1000);
    ESP.restart();
  }
  
  return success;
}

static int compareVersions(const char* v1, const char* v2) {
  int maj1 = 0, min1 = 0, pat1 = 0;
  int maj2 = 0, min2 = 0, pat2 = 0;
  
  sscanf(v1, "%d.%d.%d", &maj1, &min1, &pat1);
  sscanf(v2, "%d.%d.%d", &maj2, &min2, &pat2);
  
  if (maj1 != maj2) return maj1 - maj2;
  if (min1 != min2) return min1 - min2;
  return pat1 - pat2;
}

static String fetchGithubRelease() {
  HTTPClient http;
  String url = "https://api.github.com/repos/" + String(GITHUB_USER) + "/" + String(GITHUB_REPO) + "/releases/latest";
  
  http.begin(url);
  http.setTimeout(10000);
  http.addHeader("Accept", "application/vnd.github+json");
  
  int code = http.GET();
  if (code != HTTP_CODE_OK) {
    Serial.printf("[OTA] API failed: %d\n", code);
    http.end();
    return "";
  }
  
  String payload = http.getString();
  http.end();
  return payload;
}

static String findFirmwareUrl(JsonDocument& doc) {
  JsonArray assets = doc["assets"];
  if (!assets) return "";
  
  for (JsonObject asset : assets) {
    const char* name = asset["name"];
    if (name && strstr(name, ".bin")) {
      return asset["browser_download_url"].as<String>();
    }
  }
  
  return "";
}

bool OtaCheckGithubUpdate() {
  if (!WifiHasInternet()) {
    Serial.println("[OTA] No internet");
    return false;
  }
  
  Serial.println("[OTA] Checking GitHub...");
  String payload = fetchGithubRelease();
  if (payload.isEmpty()) return false;
  
  JsonDocument doc;
  if (deserializeJson(doc, payload)) {
    Serial.println("[OTA] JSON parse error");
    return false;
  }
  
  const char* tagName = doc["tag_name"];
  if (!tagName) {
    Serial.println("[OTA] No tag_name");
    return false;
  }
  
  // Remove 'v' prefix if present (e.g., "v1.0.1" -> "1.0.1")
  String version = tagName;
  if (version.startsWith("v")) version = version.substring(1);
  
  Serial.printf("[OTA] Remote: %s, Local: %s\n", version.c_str(), FIRMWARE_VERSION);
  
  if (compareVersions(version.c_str(), FIRMWARE_VERSION) <= 0) {
    Serial.println("[OTA] Up to date");
    return false;
  }
  
  String firmwareUrl = findFirmwareUrl(doc);
  if (firmwareUrl.isEmpty()) {
    Serial.println("[OTA] No .bin found");
    return false;
  }
  
  Serial.printf("[OTA] Update: %s -> %s\n", FIRMWARE_VERSION, version.c_str());
  return OtaStart(firmwareUrl.c_str());
}
