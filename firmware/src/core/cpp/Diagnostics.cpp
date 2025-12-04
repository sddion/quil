#include "../h/Diagnostics.h"
#include "config.h"
#include <Arduino.h>

static unsigned long last_check = 0;

void DiagInit() {
  last_check = millis();
}

void DiagUpdate() {
  unsigned long now = millis();
  if (now - last_check < HEAP_CHECK_MS) return;
  last_check = now;
  Serial.printf("Heap: %u | Uptime: %lu\n", DiagFreeHeap(), DiagUptime());
}

uint32_t DiagFreeHeap() {
  return ESP.getFreeHeap();
}

unsigned long DiagUptime() {
  return millis() / 1000;
}