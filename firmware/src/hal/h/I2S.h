#pragma once
#include <Arduino.h>

#ifdef ESP32
bool hal_i2s_init_mic();
bool hal_i2s_init_speaker();
size_t hal_i2s_read_mic(uint8_t* buffer, size_t len);
size_t hal_i2s_write_speaker(const uint8_t* data, size_t len);
#endif
