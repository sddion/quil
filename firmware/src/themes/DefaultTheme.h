#pragma once
#include <Arduino.h>
#include "hal/h/Display.h"
#include "modules/h/StatusIcons.h"
#include "modules/h/BatteryManager.h"
#include "modules/h/WifiManager.h"
#include "modules/h/NtpClient.h"

void DefaultThemeRender(int hour, int minute, const char* dateStr, const char* dayStr, 
                       uint8_t batteryPct, int rssi, bool wifiConnected, bool btConnected,
                       uint8_t weatherCode, const char* tempStr, const char* condStr);
