#include "../h/Chat.h"
#include "hal/h/Display.h"
#include "../../modules/h/VoiceManager.h"

void ChatInit() {
  VoiceInit();
}

void ChatUpdate() {}

void ChatRender() {
  DisplayClear();
  DisplayText("CHAT", 45, 10);
  bool active = voice_is_listening();
  DisplayText(active ? "Listening..." : "Idle", 30, 30);
  DisplayUpdate();
}

void ChatStartListen() {
  VoiceStartListening();
}

void ChatStopListen() {
  VoiceStopListening();
}

void ChatToggleMute() {
  if (voice_is_listening()) {
    VoiceStopListening();
  } else {
    VoiceStartListening();
  }
}