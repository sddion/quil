#pragma once
#include <Arduino.h>

void WakeInit();
bool WakeDetect();
void WakeSetThreshold(float thresh);
float WakeGetConfidence();
float WakeGetAmbientNoise();
float WakeGetThreshold();