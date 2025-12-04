#pragma once
#include <Arduino.h>

void cloud_init();
void cloud_sync_prefs();
void cloud_sync_logs();
bool cloud_is_connected();
