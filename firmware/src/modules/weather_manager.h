#pragma once

#include <Arduino.h>

struct WeatherData {
  float temperature_c;
  float temperature_f;
  String condition;
  bool success;
};

class WeatherManager {
public:
  WeatherManager();
  WeatherData getWeatherData(const String& apiKey, const String& location);
};
