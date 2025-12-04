#pragma once
#include <Arduino.h>
#include <Adafruit_SSD1306.h>

bool DisplayInit();
void DisplayClear();
void DisplayText(const char* str, uint8_t x, uint8_t y);
void DisplayTextSize(uint8_t size);
void DisplayRect(int16_t x, int16_t y, int16_t w, int16_t h, bool outline);
void DisplayBitmap(const uint8_t* bitmap);
void DisplayUpdate();
void DisplaySetContrast(uint8_t level);
uint8_t DisplayGetContrast();
Adafruit_SSD1306& DisplayGetDisplay();