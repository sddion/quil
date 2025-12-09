#include "../h/ConversationManager.h"
#include "hal/h/Display.h"
#include "modules/h/VoiceManager.h"

static ConversationState_t convState = CONV_STATE_IDLE;
static bool isMuted = false;
static unsigned long lastActivityTime = 0;
static unsigned long conversationStartTime = 0;

void ConversationInit() {
  convState = CONV_STATE_IDLE;
  isMuted = false;
  lastActivityTime = 0;
  Serial.println("[Conversation] Initialized");
}

void ConversationStart() {
  convState = CONV_STATE_LISTENING;
  isMuted = false;
  lastActivityTime = millis();
  conversationStartTime = millis();
  
  // Start listening for voice input
  VoiceStartListening();
  
  Serial.println("[Conversation] Started - listening for input");
}

void ConversationEnd() {
  convState = CONV_STATE_IDLE;
  
  // Stop voice processing
  VoiceStopListening();
  
  Serial.println("[Conversation] Ended - returning to clock");
}

void ConversationLoop() {
  if (convState == CONV_STATE_IDLE) return;
  
  // Update activity timer when there's voice activity
  if (VoiceIsListening() || convState == CONV_STATE_SPEAKING) {
    lastActivityTime = millis();
  }
  
  // Handle mute state
  if (isMuted && VoiceIsListening()) {
    VoiceStopListening();
  }
}

void ConversationRender() {
  if (convState == CONV_STATE_IDLE) return;
  
  Adafruit_SSD1306& display = DisplayGetDisplay();
  DisplayClear();
  
  // Show conversation state
  display.setTextSize(1);
  display.setCursor(0, 0);
  
  switch (convState) {
    case CONV_STATE_LISTENING:
      display.print("Listening...");
      break;
      
    case CONV_STATE_THINKING:
      display.print("Thinking...");
      break;
      
    case CONV_STATE_SPEAKING:
      display.print("Speaking...");
      break;
      
    case CONV_STATE_WAITING:
      display.print("...");
      break;
      
    default:
      break;
  }
  
  // Show mute indicator
  if (isMuted) {
    display.setCursor(100, 0);
    display.print("MUTED");
  }
  
  // Show timeout countdown in last 5 seconds
  unsigned long elapsed = millis() - lastActivityTime;
  if (elapsed > CONVERSATION_TIMEOUT_MS - 5000) {
    int secondsLeft = (CONVERSATION_TIMEOUT_MS - elapsed) / 1000;
    display.setCursor(0, 56);
    display.print("Closing in ");
    display.print(secondsLeft);
    display.print("s");
  }
  
  DisplayUpdate();
}

bool ConversationTimedOut() {
  if (convState == CONV_STATE_IDLE) return false;
  if (convState == CONV_STATE_SPEAKING) return false; // Don't timeout while speaking
  
  return (millis() - lastActivityTime) >= CONVERSATION_TIMEOUT_MS;
}

bool ConversationIsActive() {
  return convState != CONV_STATE_IDLE;
}

void ConversationToggleMute() {
  isMuted = !isMuted;
  Serial.printf("[Conversation] Mute: %s\n", isMuted ? "ON" : "OFF");
  
  if (isMuted) {
    VoiceStopListening();
  } else if (convState == CONV_STATE_LISTENING || convState == CONV_STATE_WAITING) {
    VoiceStartListening();
  }
}

bool ConversationIsMuted() {
  return isMuted;
}

ConversationState_t ConversationGetState() {
  return convState;
}

void ConversationOnSpeechStart() {
  if (convState == CONV_STATE_WAITING) {
    convState = CONV_STATE_LISTENING;
  }
  lastActivityTime = millis();
  Serial.println("[Conversation] Speech started");
}

void ConversationOnSpeechEnd() {
  if (convState == CONV_STATE_LISTENING) {
    convState = CONV_STATE_THINKING;
  }
  lastActivityTime = millis();
  Serial.println("[Conversation] Speech ended");
}

void ConversationOnResponseStart() {
  convState = CONV_STATE_SPEAKING;
  lastActivityTime = millis();
  Serial.println("[Conversation] Response playing");
}

void ConversationOnResponseEnd() {
  convState = CONV_STATE_WAITING;
  lastActivityTime = millis();
  Serial.println("[Conversation] Waiting for follow-up");
}
