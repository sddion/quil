#include "../h/TouchActions.h"
#include "core/h/StateMachine.h"
#include "modes/h/Chat.h"

void ActionsInit() {}

void ActionsHandle(GestureType gesture, DisplayMode_t mode) {
  // Double tap cycles between display modes
  if (gesture == GESTURE_DOUBLE_TAP) {
    StateCycleMode();
    return;
  }
  
  // Handle mode-specific gestures
  switch (mode) {
    case MODE_CHAT:
      if (gesture == GESTURE_SINGLE_TAP) {
        ChatToggleMute();
      }
      break;
      
    default:
      break;
  }
}