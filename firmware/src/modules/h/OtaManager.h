#pragma once
#include <Arduino.h>

void OtaInit();
void OtaHandle();
bool ota_start(const char* url);