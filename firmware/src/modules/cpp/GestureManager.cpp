#include "../h/GestureManager.h"
#include "hal/h/Ttp223.h"

void GestureInit() {
  TtpInit();
}

GestureType gesture_detect(uint16_t unused, unsigned long unused_ts) {
  TtpUpdate();
  
  if (TtpHasEvent(TOUCH_SENSOR_A)) {
    TouchEventData event = TtpGetEvent(TOUCH_SENSOR_A);
    
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