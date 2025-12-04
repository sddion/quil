#include "../h/Display.h"
#include "config.h"
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include "config.h"

static Adafruit_SSD1306 disp(DISPLAY_WIDTH, DISPLAY_HEIGHT, &Wire, -1);
static uint8_t current_contrast = 128; // Default mid-level contrast (0-255)

bool DisplayInit() {
  if (!disp.begin(SSD1306_SWITCHCAPVCC, DISPLAY_ADDR)) return false;
  disp.clearDisplay();
  disp.ssd1306_command(0x81); // SSD1306_SETCONTRAST
  disp.ssd1306_command(current_contrast);
  disp.display();
  return true;
}

void DisplayClear() {
  disp.clearDisplay();
}

void DisplayText(const char* str, uint8_t x, uint8_t y) {
  disp.setCursor(x, y);
  disp.setTextColor(SSD1306_WHITE);
  disp.print(str);
}

void DisplayTextSize(uint8_t size) {
  disp.setTextSize(size);
}

void DisplayRect(int16_t x, int16_t y, int16_t w, int16_t h, bool outline) {
  if (outline) {
    disp.drawRect(x, y, w, h, SSD1306_WHITE);
  } else {
    disp.fillRect(x, y, w, h, SSD1306_WHITE);
  }
}

void DisplayBitmap(const uint8_t* bitmap) {
  disp.drawBitmap(0, 0, bitmap, DISPLAY_WIDTH, DISPLAY_HEIGHT, SSD1306_WHITE);
}

void DisplayUpdate() {
  disp.display();
}

void DisplaySetContrast(uint8_t level) {
  current_contrast = level;
  disp.ssd1306_command(0x81); // SSD1306_SETCONTRAST
  disp.ssd1306_command(level);
}

uint8_t DisplayGetContrast() {
  return current_contrast;
}

Adafruit_SSD1306& DisplayGetDisplay() {
  return disp;
}