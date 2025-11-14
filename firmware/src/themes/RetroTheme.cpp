#include "RetroTheme.h"
#include "config.h"
#include "oled_theme.h"
#include "../fonts/Cousine_Bold_10.h"
#include "../fonts/Roboto_Mono_Medium_6.h"

void RetroTheme::renderTime(Adafruit_SSD1306& disp, String time, String date, String day) {
  disp.drawBitmap(0, 0, oled_theme_retro, DISPLAY_WIDTH, DISPLAY_HEIGHT, SSD1306_WHITE); // Draw the base bitmap
  
  // Black background - use WHITE text with custom fonts
  disp.setFont(&Cousine_Bold_10);
  disp.setTextColor(SSD1306_WHITE);
  disp.setCursor(6, 14);
  disp.print(time.c_str());
  disp.setFont(); // Reset to default
  
  disp.setFont(&Roboto_Mono_Medium_6);
  disp.setTextColor(SSD1306_WHITE);
  disp.setCursor(6, 32);
  disp.print(day.c_str());
  
  disp.setCursor(6, 42);
  disp.print(date.c_str());
  disp.setFont(); // Reset to default
}

void RetroTheme::renderWeather(Adafruit_SSD1306& disp, const WeatherData& data) {
  if (data.success) {
    disp.setFont(&Roboto_Mono_Medium_6);
    disp.setTextColor(SSD1306_WHITE); // White text on black background
    String temp_c = String(data.temperature_c, 0) + "C";
    disp.setCursor(104, 32);
    disp.print(temp_c.c_str());
    disp.setFont(); // Reset to default
  }
}

void RetroTheme::renderMusic(Adafruit_SSD1306& disp, String song, String artist, bool isPlaying) {
  // Not implemented for this theme
}

void RetroTheme::renderBattery(Adafruit_SSD1306& disp, uint8_t percentage, bool isLow) {
  // Simple battery icon for retro theme (no text)
  // Black background - use WHITE lines
  int x = 100;
  int y = 2;
  int width = 20;
  int height = 10;
  
  // Draw battery outline (white on black)
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
    return;
  }
}
