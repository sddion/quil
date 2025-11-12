#pragma once
#include <Arduino.h>

bool hal_mpr121_init();
uint16_t hal_mpr121_read_touched();
