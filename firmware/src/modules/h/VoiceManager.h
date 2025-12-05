#pragma once
#include <Arduino.h>

void VoiceInit();
void VoiceStartListening();
void VoiceStopListening();
bool VoiceIsListening();
size_t VoiceReadBuffer(uint8_t* buf, size_t len);
void VoicePlayResponse(const uint8_t* data, size_t len);
float VoiceGetRms();