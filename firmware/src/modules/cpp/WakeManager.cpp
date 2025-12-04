#include "../h/WakeManager.h"
#include "../h/VoiceManager.h"

static float threshold = 500.0f;
static float last_confidence = 0.0f;

void WakeInit() {
  threshold = 500.0f;
}

bool wake_detect() {
  if (!voice_is_listening()) return false;
  
  float rms = voice_get_rms();
  last_confidence = rms;
  
  if (rms > threshold) {
    return true;
  }
  
  return false;
}

void wake_set_threshold(float thresh) {
  threshold = thresh;
}

float wake_get_confidence() {
  return last_confidence;
}