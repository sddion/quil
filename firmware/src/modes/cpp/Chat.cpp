#include "mode_chat.h"
#include "hal/hal_display.h"
#include "../modules/voice_manager.h"

void mode_chat_init() {
  voice_init();
}

void mode_chat_update() {}

void mode_chat_render() {
  hal_display_clear();
  hal_display_text("CHAT", 45, 10);
  bool active = voice_is_listening();
  hal_display_text(active ? "Listening..." : "Idle", 30, 30);
  hal_display_update();
}

void mode_chat_start_listen() {
  voice_start_listening();
}

void mode_chat_stop_listen() {
  voice_stop_listening();
}

void mode_chat_toggle_mute() {
  if (voice_is_listening()) {
    voice_stop_listening();
  } else {
    voice_start_listening();
  }
}
