#include "hal_display.h"
#include "config.h"
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include "config.h"

static Adafruit_SSD1306 disp(DISPLAY_WIDTH, DISPLAY_HEIGHT, &Wire, -1);
static uint8_t current_contrast = 128; // Default mid-level contrast (0-255)

bool hal_display_init() {
  if (!disp.begin(SSD1306_SWITCHCAPVCC, DISPLAY_ADDR)) return false;
  disp.clearDisplay();
  disp.ssd1306_command(0x81); // SSD1306_SETCONTRAST
  disp.ssd1306_command(current_contrast);
  disp.display();
  return true;
}

void hal_display_clear() {
  disp.clearDisplay();
}

void hal_display_text(const char* str, uint8_t x, uint8_t y) {
  disp.setCursor(x, y);
  disp.setTextColor(SSD1306_WHITE);
  disp.print(str);
}

void hal_display_text_size(uint8_t size) {
  disp.setTextSize(size);
}

void hal_display_rect(int16_t x, int16_t y, int16_t w, int16_t h, bool outline) {
  if (outline) {
    disp.drawRect(x, y, w, h, SSD1306_WHITE);
  } else {
    disp.fillRect(x, y, w, h, SSD1306_WHITE);
  }
}

void hal_display_bitmap(const uint8_t* bitmap) {
  disp.drawBitmap(0, 0, bitmap, DISPLAY_WIDTH, DISPLAY_HEIGHT, SSD1306_WHITE);
}

void hal_display_update() {
  disp.display();
}

void hal_display_set_contrast(uint8_t level) {
  current_contrast = level;
  disp.ssd1306_command(0x81); // SSD1306_SETCONTRAST
  disp.ssd1306_command(level);
}

uint8_t hal_display_get_contrast() {
  return current_contrast;
}

Adafruit_SSD1306& hal_display_get_display() {
  return disp;
}
