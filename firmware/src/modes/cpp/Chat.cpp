#include "../h/Chat.h"
#include "hal/h/Display.h"
#include "../../modules/Audio.h"

void ChatInit() {
  AudioInit();
}

void ChatUpdate() {}

void ChatRender() {
  DisplayClear();
  DisplayText("CHAT", 45, 10);
  bool active = AudioIsListening();
  DisplayText(active ? "Listening..." : "Idle", 30, 30);
  DisplayUpdate();
}

void ChatStartListen() {
  AudioStartListening();
}

void ChatStopListen() {
  AudioStopListening();
}

void ChatToggleMute() {
  if (AudioIsListening()) {
    AudioStopListening();
  } else {
    AudioStartListening();
  }
}