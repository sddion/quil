#include "../h/NativeTouch.h"

static int touchThreshold = TOUCH_THRESHOLD;
static bool lastTouchState = false;
static bool currentTouchState = false;
static bool hasPendingTap = false;
static unsigned long lastTouchChange = 0;
static unsigned long touchStartTime = 0;

void NativeTouchInit() {
  // GPIO 14 = T6 on ESP32
  // No special setup needed - touchRead() handles it
  
  // Auto-calibrate on startup
  NativeTouchCalibrate();
  
  Serial.println("[NativeTouch] Initialized on GPIO 14 (T6)");
  Serial.printf("[NativeTouch] Threshold: %d\n", touchThreshold);
}

void NativeTouchCalibrate() {
  // Take multiple readings and set threshold
  int sum = 0;
  const int samples = 10;
  
  for (int i = 0; i < samples; i++) {
    sum += touchRead(TOUCH_PIN);
    delay(10);
  }
  
  int ambient = sum / samples;
  // Set threshold at 60% of ambient (touched value drops significantly)
  touchThreshold = ambient * 0.6;
  
  // Ensure minimum threshold
  if (touchThreshold < 20) touchThreshold = 20;
  if (touchThreshold > 60) touchThreshold = 60;
  
  Serial.printf("[NativeTouch] Calibrated: ambient=%d, threshold=%d\n", ambient, touchThreshold);
}

void NativeTouchUpdate() {
  unsigned long now = millis();
  
  // Read touch value
  int touchValue = touchRead(TOUCH_PIN);
  bool isTouched = (touchValue < touchThreshold);
  
  // Debounce
  if (isTouched != lastTouchState) {
    lastTouchChange = now;
    lastTouchState = isTouched;
  }
  
  // Apply debounce
  if ((now - lastTouchChange) >= TOUCH_DEBOUNCE_MS) {
    bool previousState = currentTouchState;
    currentTouchState = lastTouchState;
    
    // Detect tap (touch then release)
    if (previousState && !currentTouchState) {
      unsigned long touchDuration = now - touchStartTime;
      // Valid tap: 50ms to 500ms
      if (touchDuration >= 50 && touchDuration <= 500) {
        hasPendingTap = true;
        Serial.printf("[NativeTouch] Tap detected (%lu ms)\n", touchDuration);
      }
    }
    
    // Track touch start time
    if (!previousState && currentTouchState) {
      touchStartTime = now;
    }
  }
}

bool NativeTouchIsTouched() {
  return currentTouchState;
}

bool NativeTouchHasTap() {
  if (hasPendingTap) {
    hasPendingTap = false;
    return true;
  }
  return false;
}

int NativeTouchGetRaw() {
  return touchRead(TOUCH_PIN);
}
