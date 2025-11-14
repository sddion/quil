#include "BoldTheme.h"
#include "config.h"
#include "oled_theme.h"

void BoldTheme::renderTime(Adafruit_SSD1306& disp, String time, String date, String day) {
  disp.drawBitmap(0, 0, oled_theme_bold, DISPLAY_WIDTH, DISPLAY_HEIGHT, SSD1306_WHITE); // Draw the base bitmap
  
  // Time in header (black background) - use WHITE text
  disp.setTextSize(2);
  disp.setCursor(4, 4);
  disp.setTextColor(SSD1306_WHITE);
  disp.print(time.c_str());
  
  // Day and Date in white area - use BLACK text (clear pixels)
  disp.setTextSize(1);
  disp.setTextColor(SSD1306_BLACK);
  disp.setCursor(30, 30);
  disp.print(day.c_str());
  disp.setCursor(30, 44);
  disp.print(date.c_str());
}

void BoldTheme::renderWeather(Adafruit_SSD1306& disp, const WeatherData& data) {
  if (data.success) {
    disp.setTextSize(1);
    disp.setTextColor(SSD1306_BLACK); // Black text on white background
    String temp_c = String(data.temperature_c, 0) + "C";
    disp.setCursor(4, 30);
    disp.print(temp_c.c_str());
  }
}

void BoldTheme::renderMusic(Adafruit_SSD1306& disp, String song, String artist, bool isPlaying) {
  // Not implemented for this theme
}

void BoldTheme::renderBattery(Adafruit_SSD1306& disp, uint8_t percentage, bool isLow) {
  // Draw simple battery outline with fill level (no percentage text)
  // Header area is black, so use WHITE for battery icon
  int x = 100;
  int y = 2;
  int width = 20;
  int height = 10;
  
  // Draw battery outline (white on black header)
  disp.drawRect(x, y, width, height, SSD1306_WHITE);
  
  // Draw battery tip
  disp.fillRect(x + width, y + 2, 2, height - 4, SSD1306_WHITE);
  
  // Calculate fill width based on percentage
  int fillWidth = ((width - 2) * percentage) / 100;
  
  // Draw fill level
  if (fillWidth > 0) {
    disp.fillRect(x + 1, y + 1, fillWidth, height - 2, SSD1306_WHITE);
  }
  
  // Blink if low battery
  if (isLow && (millis() / 500) % 2 == 0) {
    // Make it blink by not drawing on alternate cycles
    return;
  }
}
