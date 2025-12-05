#include "../h/TouchActions.h"
#include "core/h/StateMachine.h"

#include "modes/h/Chat.h"
#include "modes/h/ThemePreview.h"

void ActionsInit() {}

void ActionsHandle(GestureType gesture, DisplayMode_t mode) {
  if (gesture == GESTURE_DOUBLE_TAP) {
    StateCycleMode();
    return;
  }
  
  switch (mode) {

      
    case MODE_THEME_PREVIEW:
      if (gesture == GESTURE_SINGLE_TAP) {
        ThemeApply();
      }
      break;
      
    case MODE_CHAT:
      if (gesture == GESTURE_SINGLE_TAP) {
        ChatToggleMute();
      }
      break;
      
    default:
      break;
  }
}