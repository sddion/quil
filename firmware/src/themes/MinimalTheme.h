#pragma once
#include "Theme.h"

class MinimalTheme : public Theme {
public:
  void renderTime(Adafruit_SSD1306& disp, String time, String date, String day) override;
  void renderWeather(Adafruit_SSD1306& disp, const WeatherData& data) override;
  void renderMusic(Adafruit_SSD1306& disp, String song, String artist, bool isPlaying) override;
  void renderBattery(Adafruit_SSD1306& disp, uint8_t percentage, bool isLow) override;
};
