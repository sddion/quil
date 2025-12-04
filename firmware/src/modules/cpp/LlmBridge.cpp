#include "../h/LlmBridge.h"
#include "../h/VoiceManager.h"

static bool response_ready = false;

void BridgeInit() {
  Serial.println("{\"bridge\":\"ready\"}");
}

void BridgeSendAudio(const uint8_t* data, size_t len) {
  Serial.write(data, len);
  Serial.println();
}

void BridgeSendCommand(const char* cmd) {
  Serial.print("{\"event\":\"");
  Serial.print(cmd);
  Serial.println("\"}");
}

void BridgeHandleResponse() {
  if (Serial.available() > 0) {
    uint8_t buf[256];
    size_t len = Serial.readBytes(buf, sizeof(buf));
    if (len > 0) {
      voice_play_response(buf, len);
      response_ready = true;
    }
  }
}

bool bridge_has_response() {
  bool r = response_ready;
  response_ready = false;
  return r;
}