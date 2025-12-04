#pragma once
#include <Arduino.h>

void voice_init();
void voice_start_listening();
void voice_stop_listening();
bool voice_is_listening();
size_t voice_read_buffer(uint8_t* buf, size_t len);
void voice_play_response(const uint8_t* data, size_t len);
float voice_get_rms();
