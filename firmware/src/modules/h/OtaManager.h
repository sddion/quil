#pragma once
#include <Arduino.h>

void OtaInit();
void OtaHandle();
bool OtaStart(const char* url);