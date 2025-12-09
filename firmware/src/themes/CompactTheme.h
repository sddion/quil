#pragma once
#include <Arduino.h>
#include "hal/h/Display.h"
#include "modules/StatusIcons.h"
#include "modules/BatteryManager.h"
#include "modules/Connectivity.h"

void CompactThemeRender(int hour, int minute, const char* dateStr, const char* dayStr, 
                       uint8_t batteryPct, int rssi, bool wifiConnected,
                       uint8_t weatherCode, const char* tempStr, const char* condStr);
