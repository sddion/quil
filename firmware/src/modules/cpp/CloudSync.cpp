#include "../h/CloudSync.h"

static bool connected = false;

void CloudInit() {
  connected = false;
}

void CloudSyncPrefs() {}

void CloudSyncLogs() {}

bool CloudIsConnected() {
  return connected;
}