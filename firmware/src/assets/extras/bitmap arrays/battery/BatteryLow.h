
#pragma once
#include <Arduino.h>

#define BATTERY_LOW_FRAMES 2

extern const unsigned char battery_low1[] PROGMEM;  
extern const unsigned char battery_low2[] PROGMEM;

extern const unsigned char* const battery_low_frames[] PROGMEM = {battery_low1, battery_low2};

