#include "../h/GestureManager.h"
#include "hal/h/Ttp223.h"

void gesture_init() {
  hal_ttp223_init();
}

GestureType gesture_detect(uint16_t unused, unsigned long unused_ts) {
  hal_ttp223_update();
  
  if (hal_ttp223_has_event(TOUCH_SENSOR_A)) {
    TouchEventData event = hal_ttp223_get_event(TOUCH_SENSOR_A);
    
    switch (event.event) {
      case TOUCH_EVENT_SINGLE_TAP:
        return GESTURE_SINGLE_TAP;
      case TOUCH_EVENT_DOUBLE_TAP:
        return GESTURE_DOUBLE_TAP;
      default:
        break;
    }
  }
  
  return GESTURE_NONE;
}