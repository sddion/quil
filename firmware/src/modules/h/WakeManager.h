#pragma once
#include <Arduino.h>

void WakeInit();
bool wake_detect();
void wake_set_threshold(float thresh);
float wake_get_confidence();