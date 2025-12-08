#pragma once
#include <Arduino.h>

void BleInit();
void BleLoop();
void BleSendStatus();
bool BleIsConnected();
void BleStop();
void BleNotifySetupComplete();
