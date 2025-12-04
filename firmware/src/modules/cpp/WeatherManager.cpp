#include "weather_manager.h"
#include <WiFiClient.h>
#include <ArduinoJson.h>

#ifdef ESP32
#include <HTTPClient.h> // ESP32 HTTP client library
#elif defined(ESP8266)
#include <ESP8266HTTPClient.h> // ESP8266 HTTP client library
#endif

WeatherManager::WeatherManager() {}

WeatherData WeatherManager::getWeatherData(const String& apiKey, const String& location) {
  WeatherData data = {0, 0, "", false};
  WiFiClient client;
  HTTPClient http;
  String url = "http://api.weatherapi.com/v1/current.json?key=" + apiKey + "&q=" + location;

  http.begin(client, url);
  int httpCode = http.GET();

  if (httpCode > 0) {
    if (httpCode == HTTP_CODE_OK) {
      String payload = http.getString();
      JsonDocument doc;
      DeserializationError error = deserializeJson(doc, payload);

      if (!error) {
        data.temperature_c = doc["current"]["temp_c"];
        data.temperature_f = doc["current"]["temp_f"];
        data.condition = doc["current"]["condition"]["text"].as<String>();
        data.success = true;
      }
    }
  }

  http.end();
  return data;
}
