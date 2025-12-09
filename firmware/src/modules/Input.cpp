#include "Input.h"
#include "hal/h/NativeTouch.h"
#include "core/h/StateMachine.h"
#include "ConversationManager.h" // Note: This path might need update if ConversationManager moves, but for now we keep as is or update if we know new path. 
// Wait, ConversationManager is in modules/h/ConversationManager.h. I'm not touching it in this plan? 
// checking plan... "Merge related modules... Input, Connectivity, Audio". ConversationManager wasn't explicitly mentioned to be merged, but it was in the modules folder.
// The plan said "Merge small, tightly coupled modules". 
// ConversationManager is large (3.9KB). I will leave it for now or move it to src/modules/ConversationManager.cpp/h in the "Flatten" phase.
// For now, I'll refer to it as "ConversationManager.h" assuming I'll move it to src/modules/ConversationManager.h.
// Actually, I should probably handle the flattening of *all* modules as part of this.
// I'll update the include to just "ConversationManager.h" and ensure I move the file later.

void InputInit() {
  NativeTouchInit();
}

GestureType InputDetect(uint16_t unused, unsigned long unused_ts) {
  NativeTouchUpdate();
  
  if (NativeTouchHasTap()) {
    return GESTURE_SINGLE_TAP;
  }
  
  return GESTURE_NONE;
}

void InputHandleActions(GestureType gesture, DisplayMode_t mode) {
  // Single tap toggles mute during conversation
  if (gesture == GESTURE_SINGLE_TAP && mode == MODE_CONVERSATION) {
    // We need to verify if ConversationToggleMute is available.
    // It was used in TouchActions.cpp.
    ConversationToggleMute();
  }
}
