#include "../h/CloudSync.h"

static bool connected = false;

void cloud_init() {
  connected = false;
}

void cloud_sync_prefs() {}

void cloud_sync_logs() {}

bool cloud_is_connected() {
  return connected;
}
