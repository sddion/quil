#pragma once
#include <Arduino.h>

typedef enum {
  GESTURE_NONE,
  GESTURE_SINGLE_TAP,
  GESTURE_DOUBLE_TAP
} GestureType;

void gesture_init();
GestureType gesture_detect(uint16_t touch, unsigned long ts);
