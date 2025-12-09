#include "../h/Diagnostics.h"
#include "config.h"
#include <Arduino.h>

static unsigned long last_check = 0;

void DiagInit() {
  last_check = millis();
}

#include "../../modules/Connectivity.h"

void DiagUpdate() {
  unsigned long now = millis();
  if (now - last_check < HEAP_CHECK_MS) return;
  last_check = now;
  
  String ip = WifiGetIp();
  const char* mode = WifiIsApMode() ? "AP" : (WifiIsConnected() ? "STA" : "DISC");
  
  Serial.printf("Heap: %u | Uptime: %lu | WiFi: %s | IP: %s\n", 
    DiagFreeHeap(), DiagUptime(), mode, ip.c_str());
}

uint32_t DiagFreeHeap() {
  return ESP.getFreeHeap();
}

unsigned long DiagUptime() {
  return millis() / 1000;
}