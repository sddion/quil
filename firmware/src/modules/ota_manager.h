#pragma once
#include <Arduino.h>

void ota_init();
void ota_handle();
bool ota_start(const char* url);
