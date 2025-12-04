#pragma once
#include <Arduino.h>

void HttpInit();
void HttpHandle();
void HttpStop();
bool http_is_running();