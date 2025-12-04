#pragma once

#include <Arduino.h>
#include <Adafruit_SSD1306.h>
#include "modules/weather_manager.h"

class Theme {
public:
  virtual void renderTime(Adafruit_SSD1306& disp, String time, String date, String day) = 0;
  virtual void renderWeather(Adafruit_SSD1306& disp, const WeatherData& data) = 0;

};

