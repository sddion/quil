#pragma once
#include <Arduino.h>

bool hal_display_init();
void hal_display_clear();
void hal_display_text(const char* str, uint8_t x, uint8_t y);
void hal_display_text_size(uint8_t size);
void hal_display_rect(int16_t x, int16_t y, int16_t w, int16_t h, bool outline);
void hal_display_bitmap(const uint8_t* bitmap);
void hal_display_update();
