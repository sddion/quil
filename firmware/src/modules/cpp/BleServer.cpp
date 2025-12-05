#include "../h/BleServer.h"
#include "../h/ConfigStore.h"
#include "../h/BatteryManager.h"
#include "../h/WifiManager.h"
#include "hal/h/Display.h"
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
    String value = pCharacteristic->getValue();
    if (value.length() > 0) {
      Serial.println("[BLE] Received config:");
      Serial.println(value);
      
      // Parse JSON and apply config
      // Format: {"ssid":"...", "password":"...", "tz":19800, "wk":"...", "wl":"...", "brightness":128}
      // or command: {"cmd":"restart"} or {"cmd":"reset"}
      
      if (value.indexOf("\"cmd\"") >= 0) {
        // Handle command
        if (value.indexOf("restart") >= 0) {
          Serial.println("[BLE] Restart command received");
          delay(500);
          ESP.restart();
        } else if (value.indexOf("reset") >= 0) {
          Serial.println("[BLE] Factory reset command received");
          ConfigClear();
          delay(500);
          ESP.restart();
        } else if (value.indexOf("sync_time") >= 0) {
          Serial.println("[BLE] Time sync requested");
          // NTP will handle this on next update cycle
        }
      } else {
        // Parse config JSON (simplified parsing)
        int ssidStart = value.indexOf("\"ssid\":\"") + 8;
        int ssidEnd = value.indexOf("\"", ssidStart);
        int passStart = value.indexOf("\"password\":\"") + 12;
        int passEnd = value.indexOf("\"", passStart);
        int tzStart = value.indexOf("\"tz\":") + 5;
        int tzEnd = value.indexOf(",", tzStart);
        if (tzEnd < 0) tzEnd = value.indexOf("}", tzStart);
        int wkStart = value.indexOf("\"wk\":\"") + 6;
        int wkEnd = value.indexOf("\"", wkStart);
        int wlStart = value.indexOf("\"wl\":\"") + 6;
        int wlEnd = value.indexOf("\"", wlStart);
        int brStart = value.indexOf("\"brightness\":") + 13;
        int brEnd = value.indexOf("}", brStart);
        if (brEnd < 0) brEnd = value.indexOf(",", brStart);
        
        if (ssidStart > 7 && passStart > 11) {
          String ssid = value.substring(ssidStart, ssidEnd);
          String pass = value.substring(passStart, passEnd);
          ConfigSaveWifi(ssid.c_str(), pass.c_str());
          Serial.println("[BLE] WiFi config saved");
        }
        
        if (tzStart > 4 && tzEnd > tzStart) {
          int tz = value.substring(tzStart, tzEnd).toInt();
          ConfigSaveTimezone(tz);
          Serial.println("[BLE] Timezone saved: " + String(tz));
        }
        
        if (wkStart > 5 && wlStart > 5) {
          String wk = value.substring(wkStart, wkEnd);
          String wl = value.substring(wlStart, wlEnd);
          ConfigSaveWeather(wk.c_str(), wl.c_str());
          Serial.println("[BLE] Weather config saved");
        }
        
        if (brStart > 12 && brEnd > brStart) {
          uint8_t brightness = value.substring(brStart, brEnd).toInt();
          ConfigSaveContrast(brightness);
          DisplaySetContrast(brightness);
          Serial.println("[BLE] Brightness saved: " + String(brightness));
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
  
  uint8_t battery = BatteryGetPercent();
  bool wifiConn = WifiIsConnected();
  char ssid[33] = "";
  char pass[65] = "";
  ConfigLoadWifi(ssid, pass);
  
  int tz = 0;
  ConfigLoadTimezone(&tz);
  
  uint8_t brightness = 128;
  ConfigLoadContrast(&brightness);
  
  // Build JSON status  
  String status = "{\"battery\":" + String(battery) + 
                  ",\"wifiConnected\":" + (wifiConn ? "true" : "false") +
                  ",\"wifiSsid\":\"" + String(ssid) + "\"" +
                  ",\"timezone\":" + String(tz) +
                  ",\"brightness\":" + String(brightness) +
                  ",\"firmwareVersion\":\"1.0.0\"}";
  
  pStatusChar->setValue(status.c_str());
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
