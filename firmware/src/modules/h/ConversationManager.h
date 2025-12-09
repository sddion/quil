#pragma once

#include <Arduino.h>

// Conversation timeout in milliseconds (return to clock after silence)
#define CONVERSATION_TIMEOUT_MS 15000  // 15 seconds

// Conversation states
typedef enum {
  CONV_STATE_IDLE,      // Not in conversation
  CONV_STATE_LISTENING, // Listening for user speech
  CONV_STATE_THINKING,  // Waiting for AI response
  CONV_STATE_SPEAKING,  // Playing AI response
  CONV_STATE_WAITING    // Waiting for follow-up
} ConversationState_t;

// Initialize conversation manager
void ConversationInit();

// Start a new conversation (called when wake detected)
void ConversationStart();

// End conversation (return to clock mode)
void ConversationEnd();

// Main conversation loop (call when in conversation mode)
void ConversationLoop();

// Render conversation UI
void ConversationRender();

// Check if conversation has timed out
bool ConversationTimedOut();

// Check if conversation is currently active
bool ConversationIsActive();

// Toggle mute state
void ConversationToggleMute();

// Check if muted
bool ConversationIsMuted();

// Get current conversation state
ConversationState_t ConversationGetState();

// Signal that user started/stopped speaking
void ConversationOnSpeechStart();
void ConversationOnSpeechEnd();

// Signal that AI response started/ended
void ConversationOnResponseStart();
void ConversationOnResponseEnd();
