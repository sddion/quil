#pragma once

#include <Arduino.h>

// Realtime Voice WebSocket Handler
// Connects to Quil server for voice-to-voice AI

// Initialize the realtime voice system
void RealtimeVoiceInit();

// Connect to the Quil server
bool RealtimeVoiceConnect(const char* ServerUrl);

// Disconnect from server
void RealtimeVoiceDisconnect();

// Check if connected
bool RealtimeVoiceIsConnected();

// Main loop handler - call this in loop()
void RealtimeVoiceLoop();

// Start listening (begin streaming mic to server)
void RealtimeVoiceStartListening();

// Stop listening
void RealtimeVoiceStopListening();

// Check if currently listening
bool RealtimeVoiceIsListening();

// Signal end of speech (for manual VAD)
void RealtimeVoiceEndOfSpeech();

// Signal interruption
void RealtimeVoiceInterrupt();

// Set audio volume (0-100)
void RealtimeVoiceSetVolume(uint8_t Volume);

// Get current volume
uint8_t RealtimeVoiceGetVolume();
