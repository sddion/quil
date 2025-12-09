#pragma once
#include <Arduino.h>

// --- Gesture Definitions ---
typedef enum {
  GESTURE_NONE,
  GESTURE_SINGLE_TAP,
  GESTURE_DOUBLE_TAP
} GestureType;

// --- Touch Actions Definitions ---
#include "types.h" 

void InputInit();
GestureType InputDetect(uint16_t touch, unsigned long ts);
void InputHandleActions(GestureType gesture, DisplayMode_t mode);
