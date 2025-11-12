#include "touch_actions.h"
#include "core/state_machine.h"
#include "modes/mode_music.h"
#include "modes/mode_chat.h"
#include "modes/mode_theme_preview.h"

void actions_init() {}

void actions_handle(GestureType gesture, DisplayMode_t mode) {
  if (gesture == GESTURE_DOUBLE_TAP) {
    state_cycle_mode();
    return;
  }
  
  switch (mode) {
    case MODE_MUSIC:
      if (gesture == GESTURE_SINGLE_TAP) {
        mode_music_toggle();
      } else if (gesture == GESTURE_SWIPE_LEFT) {
        mode_music_prev();
      } else if (gesture == GESTURE_SWIPE_RIGHT) {
        mode_music_next();
      }
      break;
      
    case MODE_THEME_PREVIEW:
      if (gesture == GESTURE_SINGLE_TAP) {
        mode_theme_apply();
      } else if (gesture == GESTURE_SWIPE_LEFT) {
        mode_theme_prev();
      } else if (gesture == GESTURE_SWIPE_RIGHT) {
        mode_theme_next();
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
