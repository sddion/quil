#pragma once

#include <Arduino.h>

// Touch sensor identifiers
typedef enum {
  TOUCH_SENSOR_A = 0
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

// Combined gesture events (removed - single sensor only)
typedef enum {
  COMBINED_GESTURE_NONE = 0
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

// HAL functions
void hal_ttp223_init();
void hal_ttp223_update();


// Single sensor event queries
bool hal_ttp223_has_event(TouchSensor_t sensor);
TouchEventData hal_ttp223_get_event(TouchSensor_t sensor);

// State queries
bool hal_ttp223_is_pressed(TouchSensor_t sensor);
unsigned long hal_ttp223_get_press_duration(TouchSensor_t sensor);

// Debug functions
void hal_ttp223_print_event(const TouchEventData& event);