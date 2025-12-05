#include "../h/VoiceManager.h"
#include "hal/h/I2S.h"

static bool listening = false;
static uint8_t audio_buffer[512];
static size_t buffer_pos = 0;

void VoiceInit() {
  I2SInitMic();
  I2SInitSpeaker();
  listening = false;
}

void VoiceStartListening() {
  listening = true;
  buffer_pos = 0;
}

void VoiceStopListening() {
  listening = false;
}

bool VoiceIsListening() {
  return listening;
}

size_t VoiceReadBuffer(uint8_t* buf, size_t len) {
  if (!listening) return 0;
  return I2SReadMic(buf, len);
}

void VoicePlayResponse(const uint8_t* data, size_t len) {
  I2SWriteSpeaker(data, len);
}

float VoiceGetRms() {
  if (buffer_pos < 100) return 0.0f;
  float sum = 0;
  for (size_t i = 0; i < buffer_pos; i += 2) {
    int16_t sample = (audio_buffer[i+1] << 8) | audio_buffer[i];
    sum += sample * sample;
  }
  return sqrt(sum / (buffer_pos / 2));
}