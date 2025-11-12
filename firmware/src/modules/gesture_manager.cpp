#include "gesture_manager.h"

static uint16_t last_touch = 0;
static unsigned long last_tap_time = 0;
static uint8_t tap_count = 0;
static uint8_t last_electrode = 0xFF;
static unsigned long swipe_start = 0;

void gesture_init() {
  last_touch = 0;
  tap_count = 0;
}

static uint8_t get_first_electrode(uint16_t touch) {
  for (uint8_t i = 0; i < 12; i++) {
    if (touch & (1 << i)) return i;
  }
  return 0xFF;
}

GestureType gesture_detect(uint16_t touch, unsigned long ts) {
  if (touch && !last_touch) {
    uint8_t elec = get_first_electrode(touch);
    
    if (ts - last_tap_time < 600) {
      tap_count++;
    } else {
      tap_count = 1;
    }
    last_tap_time = ts;
    
    if (last_electrode != 0xFF && (ts - swipe_start < 400)) {
      if (elec > last_electrode + 2) {
        last_electrode = 0xFF;
        last_touch = touch;
        return GESTURE_SWIPE_RIGHT;
      } else if (elec + 2 < last_electrode) {
        last_electrode = 0xFF;
        last_touch = touch;
        return GESTURE_SWIPE_LEFT;
      }
    }
    
    last_electrode = elec;
    swipe_start = ts;
  }
  
  if (!touch && last_touch) {
    unsigned long release_time = ts - last_tap_time;
    if (release_time < 300) {
      if (tap_count >= 2) {
        tap_count = 0;
        last_touch = touch;
        return GESTURE_DOUBLE_TAP;
      }
    }
  }
  
  if (tap_count == 1 && (ts - last_tap_time > 600)) {
    tap_count = 0;
    last_touch = touch;
    return GESTURE_SINGLE_TAP;
  }
  
  last_touch = touch;
  return GESTURE_NONE;
}
