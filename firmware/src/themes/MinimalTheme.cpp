#include "MinimalTheme.h"
#include "config.h"
#include "oled_theme.h"

void MinimalTheme::renderTime(Adafruit_SSD1306& disp, String time, String date, String day) {
  disp.drawBitmap(0, 0, oled_theme_minimal, DISPLAY_WIDTH, DISPLAY_HEIGHT, SSD1306_WHITE); // Draw the base bitmap
  
  // White background - use BLACK text (0 = clear pixels)
  disp.setTextSize(2);
  disp.setCursor(8, 8);
  disp.setTextColor(0);
  disp.print(time.c_str());
  
  disp.setTextSize(1);
  disp.setCursor(8, 32);
  disp.setTextColor(0);
  disp.print(day.c_str());
  
  disp.setCursor(8, 44);
  disp.print(date.c_str());
}

void MinimalTheme::renderWeather(Adafruit_SSD1306& disp, const WeatherData& data) {
  if (data.success) {
    disp.setTextSize(1);
    disp.setTextColor(0); // Black text on white background (0 = clear pixels)
    String temp_c = String(data.temperature_c, 0) + "C";
    disp.setCursor(96, 32);
    disp.print(temp_c.c_str());
    disp.setCursor(96, 44);
    disp.print(data.condition.c_str());
  }
}

void MinimalTheme::renderMusic(Adafruit_SSD1306& disp, String song, String artist, bool isPlaying) {
  // Not implemented for this theme
}

void MinimalTheme::renderBattery(Adafruit_SSD1306& disp, uint8_t percentage, bool isLow) {
  // Simple battery icon for minimal theme (no text)
  // White background - use BLACK lines (0 = clear pixels)
  int x = 100;
  int y = 2;
  int width = 20;
  int height = 10;
  
  // Draw battery outline (black on white)
  disp.drawRect(x, y, width, height, 0);
  
  // Draw battery tip
  disp.fillRect(x + width, y + 2, 2, height - 4, 0);
  
  // Calculate fill width based on percentage
  int fillWidth = ((width - 2) * percentage) / 100;
  
  // Draw fill level
  if (fillWidth > 0) {
    disp.fillRect(x + 1, y + 1, fillWidth, height - 2, 0);
  }
  
  // Blink if low battery
  if (isLow && (millis() / 500) % 2 == 0) {
    return;
  }
}
