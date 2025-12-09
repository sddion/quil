#include "../h/VoiceManager.h"
#include "hal/h/I2S.h"

static bool listening = false;
static float last_rms = 0.0f;

void VoiceInit() {
  I2SInitMic();
  I2SInitSpeaker();
  listening = false;
}

void VoiceStartListening() {
  listening = true;
}

void VoiceStopListening() {
  listening = false;
}

bool VoiceIsListening() {
  return listening;
}

size_t VoiceReadBuffer(uint8_t* buf, size_t len) {
  if (!listening) return 0;
  
  size_t bytesRead = I2SReadMic(buf, len);
  
  // Calculate RMS from the samples for wake detection
  if (bytesRead >= 100) {
    float sum = 0;
    for (size_t i = 0; i < bytesRead - 1; i += 2) {
      int16_t sample = (buf[i+1] << 8) | buf[i];
      sum += (float)sample * sample;
    }
    last_rms = sqrt(sum / (bytesRead / 2));
  }
  
  return bytesRead;
}

void VoicePlayResponse(const uint8_t* data, size_t len) {
  I2SWriteSpeaker(data, len);
}

float VoiceGetRms() {
  return last_rms;
}