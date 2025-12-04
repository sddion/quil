#include "../h/TouchActions.h"
#include "core/h/StateMachine.h"

#include "modes/h/Chat.h"
#include "modes/h/ThemePreview.h"

void actions_init() {}

void actions_handle(GestureType gesture, DisplayMode_t mode) {
  if (gesture == GESTURE_DOUBLE_TAP) {
    state_cycle_mode();
    return;
  }
  
  switch (mode) {

      
    case MODE_THEME_PREVIEW:
      if (gesture == GESTURE_SINGLE_TAP) {
        mode_theme_apply();
      }
      break;
      
    case MODE_CHAT:
      if (gesture == GESTURE_SINGLE_TAP) {
        mode_chat_toggle_mute();
      }
      break;
      
    default:
      break;
  }
}
