#pragma once
#include <Arduino.h>

void CloudInit();
void CloudSyncPrefs();
void CloudSyncLogs();
bool cloud_is_connected();