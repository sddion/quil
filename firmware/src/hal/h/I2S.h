#pragma once

#include <Arduino.h>

// I2S Configuration for OpenAI Realtime API
// Mic: INMP441, Speaker: MAX98357A

#define I2S_SAMPLE_RATE_MIC 24000      // OpenAI Realtime uses 24kHz
#define I2S_SAMPLE_RATE_SPEAKER 24000

// Initialize I2S microphone (INMP441)
bool I2SInitMic();

// Initialize I2S speaker (MAX98357A)
bool I2SInitSpeaker();

// Read audio from microphone
// Returns number of bytes read
size_t I2SReadMic(uint8_t* Buffer, size_t Length);

// Write audio to speaker
// Returns number of bytes written
size_t I2SWriteSpeaker(const uint8_t* Data, size_t Length);

// Reconfigure I2S sample rate
bool I2SSetSampleRate(uint32_t MicRate, uint32_t SpeakerRate);