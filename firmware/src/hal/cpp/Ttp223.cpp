#include "../h/Ttp223.h"
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



static SensorData sensor;

static void update_sensor(SensorData* s, TouchSensor_t id);
static bool read_pin_debounced(SensorData* s);
static void trigger_event(SensorData* s, TouchSensor_t id, TouchEvent_t event, unsigned long duration = 0);

void TtpInit() {
  sensor.pin = PIN_TOUCH_A;
  sensor.state = STATE_IDLE;
  sensor.current_reading = false;
  sensor.last_reading = false;
  sensor.debounced_state = false;
  sensor.last_change_time = 0;
  sensor.press_start_time = 0;
  sensor.release_time = 0;
  sensor.debounce_start = 0;
  sensor.tap_count = 0;
  sensor.long_press_triggered = false;
  sensor.hold_start_triggered = false;
  sensor.has_pending_event = false;
  
  pinMode(PIN_TOUCH_A, INPUT);
  
  Serial.println("[TTP223] Touch sensor initialized");
  Serial.print("[TTP223] Sensor A pin: ");
  Serial.println(PIN_TOUCH_A);
}

void TtpUpdate() {
  update_sensor(&sensor, TOUCH_SENSOR_A);
}

static bool read_pin_debounced(SensorData* s) {
  unsigned long now = millis();
  bool current = digitalRead(s->pin);
  
  if (current != s->last_reading) {
    s->debounce_start = now;
    s->last_reading = current;
  }
  
  if ((now - s->debounce_start) >= DEBOUNCE_TIME) {
    if (current != s->debounced_state) {
      s->debounced_state = current;
      s->last_change_time = now;
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



static void trigger_event(SensorData* s, TouchSensor_t id, TouchEvent_t event, unsigned long duration) {
  if (!s->has_pending_event) {
    s->pending_event.sensor = id;
    s->pending_event.event = event;
    s->pending_event.timestamp = millis();
    s->pending_event.duration = duration;
    s->has_pending_event = true;
    
    TtpPrintEvent(s->pending_event);
  }
}



bool TtpHasEvent(TouchSensor_t s) {
  if (s == TOUCH_SENSOR_A) {
    return sensor.has_pending_event;
  }
  return false;
}

TouchEventData TtpGetEvent(TouchSensor_t s) {
  TouchEventData event;
  event.sensor = s;
  event.event = TOUCH_EVENT_NONE;
  event.timestamp = 0;
  event.duration = 0;
  
  if (s == TOUCH_SENSOR_A) {
    if (sensor.has_pending_event) {
      event = sensor.pending_event;
      sensor.has_pending_event = false;
    }
  }
  
  return event;
}



bool TtpIsPressed(TouchSensor_t s) {
  if (s == TOUCH_SENSOR_A) {
    return sensor.debounced_state;
  }
  return false;
}

unsigned long TtpGetPressDuration(TouchSensor_t s) {
  if (s == TOUCH_SENSOR_A) {
    if (sensor.debounced_state) {
      return millis() - sensor.press_start_time;
    }
  }
  return 0;
}

void TtpPrintEvent(const TouchEventData& event) {
  Serial.print("[TTP223] Event: Sensor A - ");
  
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