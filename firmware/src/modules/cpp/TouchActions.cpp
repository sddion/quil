#include "../h/TouchActions.h"
#include "core/h/StateMachine.h"
#include "modules/h/ConversationManager.h"

void ActionsInit() {}

void ActionsHandle(GestureType gesture, DisplayMode_t mode) {
  // Single tap toggles mute during conversation
  if (gesture == GESTURE_SINGLE_TAP && mode == MODE_CONVERSATION) {
    ConversationToggleMute();
  }
}