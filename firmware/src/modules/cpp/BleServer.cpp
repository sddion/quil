#include "../h/BleServer.h"
#include "../h/ConfigStore.h"
#include "../h/BatteryManager.h"
#include "../h/WifiManager.h"
#include "../../modes/h/Time.h"
#include "hal/h/Display.h"
#include "config.h"
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CONFIG_CHAR_UUID    "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define STATUS_CHAR_UUID    "beb5483e-36e1-4688-b7f5-ea07361b26a9"

static BLEServer* pServer = nullptr;
static BLECharacteristic* pConfigChar = nullptr;
static BLECharacteristic* pStatusChar = nullptr;
static bool deviceConnected = false;
static bool oldDeviceConnected = false;

class ServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
    Serial.println("[BLE] Device connected");
  }

  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
    Serial.println("[BLE] Device disconnected");
  }
};

class ConfigCallbacks: public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic* pCharacteristic) {
    // Use static buffer to avoid heap allocation in BLE callback context
    static char valueBuf[512];
    std::string stdValue = pCharacteristic->getValue();
    size_t len = min(stdValue.length(), sizeof(valueBuf) - 1);
    memcpy(valueBuf, stdValue.c_str(), len);
    valueBuf[len] = '\0';
    
    if (len > 0) {
      Serial.println("[BLE] Received config:");
      Serial.println(valueBuf);
      
      // Parse JSON and apply config
      // Format: {"ssid":"...", "password":"...", "tz":19800, "wk":"...", "wl":"...", "brightness":128}
      // or command: {"cmd":"restart"} or {"cmd":"reset"}
      
      if (strstr(valueBuf, "\"cmd\"") != nullptr) {
        // Handle command
        if (strstr(valueBuf, "restart") != nullptr) {
          Serial.println("[BLE] Restart command received");
          delay(500);
          ESP.restart();
        } else if (strstr(valueBuf, "reset") != nullptr) {
          Serial.println("[BLE] Factory reset command received");
          ConfigClear();
          delay(500);
          ESP.restart();
        } else if (strstr(valueBuf, "sync_time") != nullptr) {
          Serial.println("[BLE] Time sync requested");
          // NTP will handle this on next update cycle
        }
      } else {
        // Parse config JSON using C-style string operations
        static char ssid[64], pass[64], wk[64], wl[64];
        int tz = 0;
        uint8_t brightness = 128, theme = 0;
        
        // Parse ssid
        char* p = strstr(valueBuf, "\"ssid\":\"");
        if (p) {
          p += 8;
          char* end = strchr(p, '"');
          if (end) {
            size_t slen = min((size_t)(end - p), sizeof(ssid) - 1);
            memcpy(ssid, p, slen);
            ssid[slen] = '\0';
          }
        }
        
        // Parse password
        p = strstr(valueBuf, "\"password\":\"");
        if (p) {
          p += 12;
          char* end = strchr(p, '"');
          if (end) {
            size_t slen = min((size_t)(end - p), sizeof(pass) - 1);
            memcpy(pass, p, slen);
            pass[slen] = '\0';
          }
        }
        
        // Parse timezone
        p = strstr(valueBuf, "\"tz\":");
        if (p) {
          p += 5;
          tz = atoi(p);
        }
        
        // Parse weather key
        p = strstr(valueBuf, "\"wk\":\"");
        if (p) {
          p += 6;
          char* end = strchr(p, '"');
          if (end) {
            size_t slen = min((size_t)(end - p), sizeof(wk) - 1);
            memcpy(wk, p, slen);
            wk[slen] = '\0';
          }
        }
        
        // Parse weather location
        p = strstr(valueBuf, "\"wl\":\"");
        if (p) {
          p += 6;
          char* end = strchr(p, '"');
          if (end) {
            size_t slen = min((size_t)(end - p), sizeof(wl) - 1);
            memcpy(wl, p, slen);
            wl[slen] = '\0';
          }
        }
        
        // Parse brightness
        p = strstr(valueBuf, "\"brightness\":");
        if (p) {
          p += 13;
          brightness = atoi(p);
        }
        
        // Parse theme
        p = strstr(valueBuf, "\"theme\":");
        if (p) {
          p += 8;
          theme = atoi(p);
        }
        
        // Apply parsed values
        if (strstr(valueBuf, "\"ssid\"") && strstr(valueBuf, "\"password\"")) {
          ConfigSaveWifi(ssid, pass);
          Serial.println("[BLE] WiFi config saved");
        }
        
        if (strstr(valueBuf, "\"tz\":")) {
          ConfigSaveTimezone(tz);
          Serial.printf("[BLE] Timezone saved: %d\n", tz);
        }
        
        if (strstr(valueBuf, "\"wk\":") && strstr(valueBuf, "\"wl\":")) {
          ConfigSaveWeather(wk, wl);
          Serial.println("[BLE] Weather config saved");
        }
        
        if (strstr(valueBuf, "\"brightness\":")) {
          ConfigSaveContrast(brightness);
          DisplaySetContrast(brightness);
          Serial.printf("[BLE] Brightness saved: %d\n", brightness);
        }
        
        if (strstr(valueBuf, "\"theme\":")) {
          TimeSetTheme((DisplayTheme_t)theme);
          Serial.printf("[BLE] Theme saved: %d\n", theme);
        }
        
        BleSendStatus(); // Send updated status
      }
    }
  }
};

void BleInit() {
  BLEDevice::init("Quil");
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new ServerCallbacks());

  BLEService* pService = pServer->createService(SERVICE_UUID);
  
  // Config characteristic (write)
  pConfigChar = pService->createCharacteristic(
    CONFIG_CHAR_UUID,
    BLECharacteristic::PROPERTY_WRITE
  );
  pConfigChar->setCallbacks(new ConfigCallbacks());
  
  // Status characteristic (read + notify)
  pStatusChar = pService->createCharacteristic(
    STATUS_CHAR_UUID,
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY
  );
  pStatusChar->addDescriptor(new BLE2902());
  
  pService->start();
  
  BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
  
  Serial.println("[BLE] Server started, advertising...");
}

void BleSendStatus() {
  if (!pStatusChar) return;
  
  uint8_t battery = BatteryGetPercentage();
  bool wifiConn = WifiIsConnected();
  char ssid[33] = "";
  char pass[65] = "";
  ConfigLoadWifi(ssid, pass);
  
  int tz = 0;
  ConfigLoadTimezone(&tz);
  
  uint8_t brightness = 128;
  ConfigLoadContrast(&brightness);
  
  uint8_t theme = 0;
  ConfigLoadTheme(&theme);
  
  // Use static buffer to avoid heap allocation in BLE callback context
  // This prevents crashes during active audio streaming
  static char statusBuf[256];
  snprintf(statusBuf, sizeof(statusBuf),
    "{\"battery\":%u,\"wifiConnected\":%s,\"wifiSsid\":\"%s\","
    "\"timezone\":%d,\"brightness\":%u,\"theme\":%u,"
    "\"firmwareVersion\":\"" FIRMWARE_VERSION "\"}",
    battery,
    wifiConn ? "true" : "false",
    ssid,
    tz,
    brightness,
    theme
  );
  
  pStatusChar->setValue(statusBuf);
  if (deviceConnected) {
    pStatusChar->notify();
  }
}

void BleLoop() {
  // Handle reconnection advertising
  if (!deviceConnected && oldDeviceConnected) {
    delay(500);
    pServer->startAdvertising();
    Serial.println("[BLE] Restart advertising");
    oldDeviceConnected = deviceConnected;
  }
  if (deviceConnected && !oldDeviceConnected) {
    oldDeviceConnected = deviceConnected;
    // Small delay to let BLE stack stabilize after connection
    delay(100);
    BleSendStatus(); // Send initial status on connect
  }
}

bool BleIsConnected() {
  return deviceConnected;
}

void BleStop() {
  if (pServer) {
    BLEDevice::deinit(true);
    pServer = nullptr;
    Serial.println("[BLE] Server stopped");
  }
}

void BleNotifySetupComplete() {
  if (!pStatusChar || !deviceConnected) return;
  
  String notification = "{\"event\":\"setup_complete\",\"status\":\"success\"}";
  pStatusChar->setValue(notification.c_str());
  pStatusChar->notify();
  Serial.println("[BLE] Setup complete notification sent");
}
