#pragma once

#include <Arduino.h>

// Native ESP32 capacitive touch on GPIO 14 (T6)
#define TOUCH_PIN T6
#define TOUCH_GPIO 14

// Touch thresholds
#define TOUCH_THRESHOLD 40      // Below this = touched (calibrate as needed)
#define TOUCH_DEBOUNCE_MS 100   // Debounce time in milliseconds

// Initialize native touch
void NativeTouchInit();

// Update touch state (call in loop)
void NativeTouchUpdate();

// Check if currently touched
bool NativeTouchIsTouched();

// Check for touch event (tap detection)
bool NativeTouchHasTap();

// Calibrate threshold based on ambient readings
void NativeTouchCalibrate();

// Get raw touch value (for debugging)
int NativeTouchGetRaw();
