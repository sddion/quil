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
// Increased debounce to filter proximity/false triggers
#define DEBOUNCE_TIME 100
#define TAP_TIMEOUT 300
#define DOUBLE_TAP_WINDOW 400
#define LONG_PRESS_TIME 800
#define MIN_TOUCH_DURATION 120  // Must be > DEBOUNCE_TIME to be meaningful

// HAL functions
void TtpInit();
void TtpUpdate();


// Single sensor event queries
bool TtpHasEvent(TouchSensor_t sensor);
TouchEventData TtpGetEvent(TouchSensor_t sensor);

// State queries
bool TtpIsPressed(TouchSensor_t sensor);
unsigned long TtpGetPressDuration(TouchSensor_t sensor);

// Debug functions
void TtpPrintEvent(const TouchEventData& event);