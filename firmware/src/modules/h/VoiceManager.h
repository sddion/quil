#pragma once
#include <Arduino.h>

void VoiceInit();
void VoiceStartListening();
void VoiceStopListening();
bool voice_is_listening();
size_t voice_read_buffer(uint8_t* buf, size_t len);
void voice_play_response(const uint8_t* data, size_t len);
float voice_get_rms();