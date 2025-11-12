#include "hal_display.h"
#include "config.h"
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include "config.h"

static Adafruit_SSD1306 disp(DISPLAY_WIDTH, DISPLAY_HEIGHT, &Wire, -1);

bool hal_display_init() {
  if (!disp.begin(SSD1306_SWITCHCAPVCC, DISPLAY_ADDR)) return false;
  disp.clearDisplay();
  disp.display();
  return true;
}

void hal_display_clear() {
  disp.clearDisplay();
}

void hal_display_text(const char* str, uint8_t x, uint8_t y) {
  disp.setCursor(x, y);
  disp.setTextSize(1);
  disp.setTextColor(SSD1306_WHITE);
  disp.print(str);
}

void hal_display_bitmap(const uint8_t* bitmap) {
  disp.drawBitmap(0, 0, bitmap, DISPLAY_WIDTH, DISPLAY_HEIGHT, SSD1306_WHITE);
}

void hal_display_update() {
  disp.display();
}
