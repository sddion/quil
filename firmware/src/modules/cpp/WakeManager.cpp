#include "../h/WakeManager.h"
#include "../h/VoiceManager.h"

// VAD Configuration
static const float MIN_THRESHOLD = 300.0f;        // Minimum threshold floor
static const float THRESHOLD_MULTIPLIER = 2.5f;   // Threshold = ambient * multiplier
static const int REQUIRED_FRAMES = 3;             // Debounce: require N consecutive frames
static const float AMBIENT_DECAY = 0.98f;         // Slow decay for ambient noise estimate
static const float AMBIENT_RISE = 0.05f;          // How fast ambient adapts to louder noise

// VAD State
static float ambientNoise = 200.0f;              // Estimated ambient noise level
static float adaptiveThreshold = 500.0f;          // Current threshold
static int consecutiveFrames = 0;                 // Frames above threshold counter
static float lastConfidence = 0.0f;               // Last RMS value for debugging
static bool calibrated = false;                   // Has initial calibration completed
static int calibrationFrames = 0;                 // Frames used for calibration
static const int CALIBRATION_FRAMES = 20;         // Number of frames to calibrate

void WakeInit() {
  ambientNoise = 200.0f;
  adaptiveThreshold = 500.0f;
  consecutiveFrames = 0;
  lastConfidence = 0.0f;
  calibrated = false;
  calibrationFrames = 0;
}

bool WakeDetect() {
  if (!VoiceIsListening()) return false;
  
  float rms = VoiceGetRms();
  lastConfidence = rms;
  
  // Initial calibration phase - establish ambient noise baseline
  if (!calibrated) {
    ambientNoise = (ambientNoise * calibrationFrames + rms) / (calibrationFrames + 1);
    calibrationFrames++;
    if (calibrationFrames >= CALIBRATION_FRAMES) {
      calibrated = true;
      adaptiveThreshold = max(MIN_THRESHOLD, ambientNoise * THRESHOLD_MULTIPLIER);
      Serial.printf("[Wake] Calibrated: ambient=%.0f, threshold=%.0f\n", ambientNoise, adaptiveThreshold);
    }
    return false;
  }
  
  // Update adaptive threshold based on ambient noise
  adaptiveThreshold = max(MIN_THRESHOLD, ambientNoise * THRESHOLD_MULTIPLIER);
  
  if (rms > adaptiveThreshold) {
    // Possible voice activity
    consecutiveFrames++;
    if (consecutiveFrames >= REQUIRED_FRAMES) {
      consecutiveFrames = 0;
      return true;  // Wake detected!
    }
  } else {
    // No voice activity - reset debounce counter
    consecutiveFrames = 0;
    
    // Slowly update ambient noise estimate (only when quiet)
    // Use asymmetric update: slow decay, faster rise
    if (rms < ambientNoise) {
      ambientNoise = ambientNoise * AMBIENT_DECAY + rms * (1.0f - AMBIENT_DECAY);
    } else if (rms < adaptiveThreshold * 0.7f) {
      // Allow ambient to rise slowly for louder environments
      ambientNoise = ambientNoise * (1.0f - AMBIENT_RISE) + rms * AMBIENT_RISE;
    }
  }
  
  return false;
}

void WakeSetThreshold(float thresh) {
  // This now sets the minimum threshold floor
  adaptiveThreshold = max(thresh, MIN_THRESHOLD);
}

float WakeGetConfidence() {
  return lastConfidence;
}

float WakeGetAmbientNoise() {
  return ambientNoise;
}

float WakeGetThreshold() {
  return adaptiveThreshold;
}