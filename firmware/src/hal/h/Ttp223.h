#pragma once

#include <Arduino.h>

// Touch sensor identifiers
typedef enum {
  TOUCH_SENSOR_A = 0,
  TOUCH_SENSOR_B = 1,
  TOUCH_SENSOR_BOTH = 2
} TouchSensor_t;

// Single sensor events
typedef enum {
  TOUCH_EVENT_NONE = 0,
  TOUCH_EVENT_SINGLE_TAP,
  TOUCH_EVENT_DOUBLE_TAP,
  TOUCH_EVENT_LONG_PRESS,
  TOUCH_EVENT_HOLD_START,
  TOUCH_EVENT_HOLD_RELEASE
} TouchEvent_t;

// Combined gesture events
typedef enum {
  COMBINED_GESTURE_NONE = 0,
  COMBINED_GESTURE_SWIPE_A_TO_B,
  COMBINED_GESTURE_SWIPE_B_TO_A,
  COMBINED_GESTURE_SIMULTANEOUS_PRESS,
  COMBINED_GESTURE_SIMULTANEOUS_RELEASE
} CombinedGesture_t;

// Touch event data structure
struct TouchEventData {
  TouchSensor_t sensor;
  TouchEvent_t event;
  unsigned long timestamp;
  unsigned long duration;
};

// Combined gesture data structure
struct CombinedGestureData {
  CombinedGesture_t gesture;
  unsigned long timestamp;
  unsigned long duration;
};

// Timing configuration (all in milliseconds)
#define DEBOUNCE_TIME 50
#define TAP_TIMEOUT 300
#define DOUBLE_TAP_WINDOW 400
#define LONG_PRESS_TIME 800
#define SWIPE_MAX_TIME 500

// HAL functions
void hal_ttp223_init();
void hal_ttp223_update();

// Single sensor event queries
bool hal_ttp223_has_event(TouchSensor_t sensor);
TouchEventData hal_ttp223_get_event(TouchSensor_t sensor);

// Combined gesture queries
bool hal_ttp223_has_gesture();
CombinedGestureData hal_ttp223_get_gesture();

// State queries
bool hal_ttp223_is_pressed(TouchSensor_t sensor);
unsigned long hal_ttp223_get_press_duration(TouchSensor_t sensor);

// Debug functions
void hal_ttp223_print_event(const TouchEventData& event);
void hal_ttp223_print_gesture(const CombinedGestureData& gesture);