#include "../h/GestureManager.h"
#include "hal/h/NativeTouch.h"

void GestureInit() {
  NativeTouchInit();
}

GestureType GestureDetect(uint16_t unused, unsigned long unused_ts) {
  NativeTouchUpdate();
  
  if (NativeTouchHasTap()) {
    return GESTURE_SINGLE_TAP;
  }
  
  return GESTURE_NONE;
}