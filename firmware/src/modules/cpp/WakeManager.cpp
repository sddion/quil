#include "../h/WakeManager.h"
#include "../h/VoiceManager.h"

static float threshold = 500.0f;
static float last_confidence = 0.0f;

void WakeInit() {
  threshold = 500.0f;
}

bool WakeDetect() {
  if (!VoiceIsListening()) return false;
  
  float rms = VoiceGetRms();
  last_confidence = rms;
  
  if (rms > threshold) {
    return true;
  }
  
  return false;
}

void WakeSetThreshold(float thresh) {
  threshold = thresh;
}

float WakeGetConfidence() {
  return last_confidence;
}