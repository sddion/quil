#include "hal_ttp223.h"
#include "pins.h"

typedef enum {
  STATE_IDLE,
  STATE_PRESSED,
  STATE_RELEASED,
  STATE_WAIT_DOUBLE_TAP,
  STATE_LONG_PRESS,
  STATE_HOLDING
} SensorState_t;

struct SensorData {
  uint8_t pin;
  SensorState_t state;
  
  bool current_reading;
  bool last_reading;
  bool debounced_state;
  
  unsigned long last_change_time;
  unsigned long press_start_time;
  unsigned long release_time;
  unsigned long debounce_start;
  
  uint8_t tap_count;
  bool long_press_triggered;
  bool hold_start_triggered;
  
  bool has_pending_event;
  TouchEventData pending_event;
};

struct GestureTracker {
  bool sensor_a_pressed;
  bool sensor_b_pressed;
  unsigned long sensor_a_press_time;
  unsigned long sensor_b_press_time;
  unsigned long sensor_a_release_time;
  unsigned long sensor_b_release_time;
  
  bool has_pending_gesture;
  CombinedGestureData pending_gesture;
};

static SensorData sensors[2];
static GestureTracker gesture_tracker;

static void update_sensor(SensorData* sensor, TouchSensor_t id);
static void update_gestures();
static bool read_pin_debounced(SensorData* sensor);
static void trigger_event(SensorData* sensor, TouchSensor_t id, TouchEvent_t event, unsigned long duration = 0);
static void trigger_gesture(CombinedGesture_t gesture, unsigned long duration = 0);

void hal_ttp223_init() {
  sensors[TOUCH_SENSOR_A].pin = PIN_TOUCH_A;
  sensors[TOUCH_SENSOR_A].state = STATE_IDLE;
  sensors[TOUCH_SENSOR_A].current_reading = false;
  sensors[TOUCH_SENSOR_A].last_reading = false;
  sensors[TOUCH_SENSOR_A].debounced_state = false;
  sensors[TOUCH_SENSOR_A].last_change_time = 0;
  sensors[TOUCH_SENSOR_A].press_start_time = 0;
  sensors[TOUCH_SENSOR_A].release_time = 0;
  sensors[TOUCH_SENSOR_A].debounce_start = 0;
  sensors[TOUCH_SENSOR_A].tap_count = 0;
  sensors[TOUCH_SENSOR_A].long_press_triggered = false;
  sensors[TOUCH_SENSOR_A].hold_start_triggered = false;
  sensors[TOUCH_SENSOR_A].has_pending_event = false;
  
  sensors[TOUCH_SENSOR_B].pin = PIN_TOUCH_B;
  sensors[TOUCH_SENSOR_B].state = STATE_IDLE;
  sensors[TOUCH_SENSOR_B].current_reading = false;
  sensors[TOUCH_SENSOR_B].last_reading = false;
  sensors[TOUCH_SENSOR_B].debounced_state = false;
  sensors[TOUCH_SENSOR_B].last_change_time = 0;
  sensors[TOUCH_SENSOR_B].press_start_time = 0;
  sensors[TOUCH_SENSOR_B].release_time = 0;
  sensors[TOUCH_SENSOR_B].debounce_start = 0;
  sensors[TOUCH_SENSOR_B].tap_count = 0;
  sensors[TOUCH_SENSOR_B].long_press_triggered = false;
  sensors[TOUCH_SENSOR_B].hold_start_triggered = false;
  sensors[TOUCH_SENSOR_B].has_pending_event = false;
  
  gesture_tracker.sensor_a_pressed = false;
  gesture_tracker.sensor_b_pressed = false;
  gesture_tracker.sensor_a_press_time = 0;
  gesture_tracker.sensor_b_press_time = 0;
  gesture_tracker.sensor_a_release_time = 0;
  gesture_tracker.sensor_b_release_time = 0;
  gesture_tracker.has_pending_gesture = false;
  
  pinMode(PIN_TOUCH_A, INPUT);
  pinMode(PIN_TOUCH_B, INPUT);
  
  Serial.println("[TTP223] Touch sensors initialized");
  Serial.print("[TTP223] Sensor A pin: ");
  Serial.println(PIN_TOUCH_A);
  Serial.print("[TTP223] Sensor B pin: ");
  Serial.println(PIN_TOUCH_B);
}

void hal_ttp223_update() {
  update_sensor(&sensors[TOUCH_SENSOR_A], TOUCH_SENSOR_A);
  update_sensor(&sensors[TOUCH_SENSOR_B], TOUCH_SENSOR_B);
  update_gestures();
}

static bool read_pin_debounced(SensorData* sensor) {
  unsigned long now = millis();
  bool current = digitalRead(sensor->pin);
  
  if (current != sensor->last_reading) {
    sensor->debounce_start = now;
    sensor->last_reading = current;
  }
  
  if ((now - sensor->debounce_start) >= DEBOUNCE_TIME) {
    if (current != sensor->debounced_state) {
      sensor->debounced_state = current;
      sensor->last_change_time = now;
      return true;
    }
  }
  
  return false;
}

static void update_sensor(SensorData* sensor, TouchSensor_t id) {
  unsigned long now = millis();
  bool state_changed = read_pin_debounced(sensor);
  bool is_pressed = sensor->debounced_state;
  
  switch (sensor->state) {
    case STATE_IDLE:
      if (state_changed && is_pressed) {
        sensor->state = STATE_PRESSED;
        sensor->press_start_time = now;
        sensor->long_press_triggered = false;
        sensor->hold_start_triggered = false;
      }
      break;
      
    case STATE_PRESSED:
      if (state_changed && !is_pressed) {
        unsigned long press_duration = now - sensor->press_start_time;
        sensor->release_time = now;
        
        if (press_duration < TAP_TIMEOUT) {
          sensor->tap_count++;
          sensor->state = STATE_WAIT_DOUBLE_TAP;
        } else {
          sensor->state = STATE_IDLE;
          sensor->tap_count = 0;
        }
      } else if (!state_changed && is_pressed) {
        unsigned long hold_duration = now - sensor->press_start_time;
        
        if (hold_duration >= LONG_PRESS_TIME && !sensor->long_press_triggered) {
          sensor->long_press_triggered = true;
          trigger_event(sensor, id, TOUCH_EVENT_LONG_PRESS, hold_duration);
          sensor->state = STATE_HOLDING;
        }
      }
      break;
      
    case STATE_WAIT_DOUBLE_TAP:
      if (state_changed && is_pressed) {
        sensor->tap_count++;
        sensor->press_start_time = now;
        sensor->state = STATE_PRESSED;
        
        if (sensor->tap_count >= 2) {
          trigger_event(sensor, id, TOUCH_EVENT_DOUBLE_TAP);
          sensor->tap_count = 0;
          sensor->state = STATE_RELEASED;
        }
      } else if (!state_changed && !is_pressed) {
        if ((now - sensor->release_time) >= DOUBLE_TAP_WINDOW) {
          if (sensor->tap_count == 1) {
            trigger_event(sensor, id, TOUCH_EVENT_SINGLE_TAP);
          }
          sensor->tap_count = 0;
          sensor->state = STATE_IDLE;
        }
      }
      break;
      
    case STATE_HOLDING:
      if (!sensor->hold_start_triggered) {
        trigger_event(sensor, id, TOUCH_EVENT_HOLD_START);
        sensor->hold_start_triggered = true;
      }
      
      if (state_changed && !is_pressed) {
        unsigned long hold_duration = now - sensor->press_start_time;
        trigger_event(sensor, id, TOUCH_EVENT_HOLD_RELEASE, hold_duration);
        sensor->state = STATE_IDLE;
        sensor->tap_count = 0;
      }
      break;
      
    case STATE_RELEASED:
      if (state_changed && !is_pressed) {
        sensor->state = STATE_IDLE;
        sensor->tap_count = 0;
      }
      break;
      
    case STATE_LONG_PRESS:
      // Transition state - should not stay here
      sensor->state = STATE_HOLDING;
      break;
  }
}

static void update_gestures() {
  unsigned long now = millis();
  bool a_pressed = sensors[TOUCH_SENSOR_A].debounced_state;
  bool b_pressed = sensors[TOUCH_SENSOR_B].debounced_state;
  
  bool a_just_pressed = a_pressed && !gesture_tracker.sensor_a_pressed;
  bool b_just_pressed = b_pressed && !gesture_tracker.sensor_b_pressed;
  bool a_just_released = !a_pressed && gesture_tracker.sensor_a_pressed;
  bool b_just_released = !b_pressed && gesture_tracker.sensor_b_pressed;
  
  if (a_just_pressed) {
    gesture_tracker.sensor_a_press_time = now;
  }
  if (b_just_pressed) {
    gesture_tracker.sensor_b_press_time = now;
  }
  if (a_just_released) {
    gesture_tracker.sensor_a_release_time = now;
  }
  if (b_just_released) {
    gesture_tracker.sensor_b_release_time = now;
  }
  
  if (a_just_pressed && b_pressed) {
    unsigned long time_diff = (now > gesture_tracker.sensor_b_press_time) ? 
                              (now - gesture_tracker.sensor_b_press_time) : 0;
    if (time_diff < 100) {
      trigger_gesture(COMBINED_GESTURE_SIMULTANEOUS_PRESS);
    }
  } else if (b_just_pressed && a_pressed) {
    unsigned long time_diff = (now > gesture_tracker.sensor_a_press_time) ? 
                              (now - gesture_tracker.sensor_a_press_time) : 0;
    if (time_diff < 100) {
      trigger_gesture(COMBINED_GESTURE_SIMULTANEOUS_PRESS);
    }
  }
  
  if (a_just_released && b_just_released) {
    trigger_gesture(COMBINED_GESTURE_SIMULTANEOUS_RELEASE);
  }
  
  if (b_just_pressed && gesture_tracker.sensor_a_pressed) {
    unsigned long time_diff = now - gesture_tracker.sensor_a_press_time;
    if (time_diff > 0 && time_diff < SWIPE_MAX_TIME) {
      trigger_gesture(COMBINED_GESTURE_SWIPE_A_TO_B, time_diff);
    }
  }
  
  if (a_just_pressed && gesture_tracker.sensor_b_pressed) {
    unsigned long time_diff = now - gesture_tracker.sensor_b_press_time;
    if (time_diff > 0 && time_diff < SWIPE_MAX_TIME) {
      trigger_gesture(COMBINED_GESTURE_SWIPE_B_TO_A, time_diff);
    }
  }
  
  gesture_tracker.sensor_a_pressed = a_pressed;
  gesture_tracker.sensor_b_pressed = b_pressed;
}

static void trigger_event(SensorData* sensor, TouchSensor_t id, TouchEvent_t event, unsigned long duration) {
  if (!sensor->has_pending_event) {
    sensor->pending_event.sensor = id;
    sensor->pending_event.event = event;
    sensor->pending_event.timestamp = millis();
    sensor->pending_event.duration = duration;
    sensor->has_pending_event = true;
    
    hal_ttp223_print_event(sensor->pending_event);
  }
}

static void trigger_gesture(CombinedGesture_t gesture, unsigned long duration) {
  if (!gesture_tracker.has_pending_gesture) {
    gesture_tracker.pending_gesture.gesture = gesture;
    gesture_tracker.pending_gesture.timestamp = millis();
    gesture_tracker.pending_gesture.duration = duration;
    gesture_tracker.has_pending_gesture = true;
    
    hal_ttp223_print_gesture(gesture_tracker.pending_gesture);
  }
}

bool hal_ttp223_has_event(TouchSensor_t sensor) {
  if (sensor == TOUCH_SENSOR_A || sensor == TOUCH_SENSOR_B) {
    return sensors[sensor].has_pending_event;
  }
  return false;
}

TouchEventData hal_ttp223_get_event(TouchSensor_t sensor) {
  TouchEventData event;
  event.sensor = sensor;
  event.event = TOUCH_EVENT_NONE;
  event.timestamp = 0;
  event.duration = 0;
  
  if (sensor == TOUCH_SENSOR_A || sensor == TOUCH_SENSOR_B) {
    if (sensors[sensor].has_pending_event) {
      event = sensors[sensor].pending_event;
      sensors[sensor].has_pending_event = false;
    }
  }
  
  return event;
}

bool hal_ttp223_has_gesture() {
  return gesture_tracker.has_pending_gesture;
}

CombinedGestureData hal_ttp223_get_gesture() {
  CombinedGestureData gesture = gesture_tracker.pending_gesture;
  gesture_tracker.has_pending_gesture = false;
  return gesture;
}

bool hal_ttp223_is_pressed(TouchSensor_t sensor) {
  if (sensor == TOUCH_SENSOR_A || sensor == TOUCH_SENSOR_B) {
    return sensors[sensor].debounced_state;
  } else if (sensor == TOUCH_SENSOR_BOTH) {
    return sensors[TOUCH_SENSOR_A].debounced_state && 
           sensors[TOUCH_SENSOR_B].debounced_state;
  }
  return false;
}

unsigned long hal_ttp223_get_press_duration(TouchSensor_t sensor) {
  if (sensor == TOUCH_SENSOR_A || sensor == TOUCH_SENSOR_B) {
    if (sensors[sensor].debounced_state) {
      return millis() - sensors[sensor].press_start_time;
    }
  }
  return 0;
}

void hal_ttp223_print_event(const TouchEventData& event) {
  Serial.print("[TTP223] Event: Sensor ");
  Serial.print(event.sensor == TOUCH_SENSOR_A ? "A" : "B");
  Serial.print(" - ");
  
  switch (event.event) {
    case TOUCH_EVENT_SINGLE_TAP:
      Serial.print("SINGLE_TAP");
      break;
    case TOUCH_EVENT_DOUBLE_TAP:
      Serial.print("DOUBLE_TAP");
      break;
    case TOUCH_EVENT_LONG_PRESS:
      Serial.print("LONG_PRESS");
      break;
    case TOUCH_EVENT_HOLD_START:
      Serial.print("HOLD_START");
      break;
    case TOUCH_EVENT_HOLD_RELEASE:
      Serial.print("HOLD_RELEASE");
      break;
    default:
      Serial.print("NONE");
      break;
  }
  
  if (event.duration > 0) {
    Serial.print(" (");
    Serial.print(event.duration);
    Serial.print("ms)");
  }
  
  Serial.println();
}

void hal_ttp223_print_gesture(const CombinedGestureData& gesture) {
  Serial.print("[TTP223] Gesture: ");
  
  switch (gesture.gesture) {
    case COMBINED_GESTURE_SWIPE_A_TO_B:
      Serial.print("SWIPE_A_TO_B");
      break;
    case COMBINED_GESTURE_SWIPE_B_TO_A:
      Serial.print("SWIPE_B_TO_A");
      break;
    case COMBINED_GESTURE_SIMULTANEOUS_PRESS:
      Serial.print("SIMULTANEOUS_PRESS");
      break;
    case COMBINED_GESTURE_SIMULTANEOUS_RELEASE:
      Serial.print("SIMULTANEOUS_RELEASE");
      break;
    default:
      Serial.print("NONE");
      break;
  }
  
  if (gesture.duration > 0) {
    Serial.print(" (");
    Serial.print(gesture.duration);
    Serial.print("ms)");
  }
  
  Serial.println();
}