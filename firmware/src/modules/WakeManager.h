#pragma once
#include <Arduino.h>

void wake_init();
bool wake_detect();
void wake_set_threshold(float thresh);
float wake_get_confidence();
