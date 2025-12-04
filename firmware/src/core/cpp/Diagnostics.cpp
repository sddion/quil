#include "diagnostics.h"
#include "config.h"
#include <Arduino.h>

static unsigned long last_check = 0;

void diag_init() {
  last_check = millis();
}

void diag_update() {
  unsigned long now = millis();
  if (now - last_check < HEAP_CHECK_MS) return;
  last_check = now;
  Serial.printf("Heap: %u | Uptime: %lu\n", diag_free_heap(), diag_uptime());
}

uint32_t diag_free_heap() {
  return ESP.getFreeHeap();
}

unsigned long diag_uptime() {
  return millis() / 1000;
}
