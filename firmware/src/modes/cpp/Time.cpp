#include "../h/Time.h"
#include "config.h"
#include "hal/h/Display.h"
#include "modules/h/NtpClient.h"
#include "modules/h/ConfigStore.h"
#include "modules/h/WeatherManager.h"
#include "modules/h/BatteryManager.h"

static WeatherManager weatherManager;
static WeatherData weatherData;
static char weatherApiKey[65];
static char weatherLocation[65];
static unsigned long lastWeatherUpdate = 0;

void TimeInit() {
  NtpInit();
  
  // Load and apply timezone offset
  int tz_offset = NTP_OFFSET_SEC;
  if (config_load_timezone(&tz_offset)) {
    NtpSetTimezone(tz_offset);
  }
  
  BatteryInit();
  
  if (!config_load_weather(weatherApiKey, weatherLocation)) {
    // No saved weather config - initialize to empty
    weatherApiKey[0] = '\0';
    weatherLocation[0] = '\0';
  }
}

void TimeUpdate() {
  NtpUpdate();
  BatteryUpdate();
  if (millis() - lastWeatherUpdate > 900000) { // 15 minutes
    lastWeatherUpdate = millis();
    weatherData = weatherManager.getWeatherData(weatherApiKey, weatherLocation);
  }
}

void TimeRender() {
  DisplayClear();
  
  String time_str = ntp_get_time();
  String date_str = ntp_get_date();
  String day_str = ntp_get_day();
  
  Adafruit_SSD1306& disp = DisplayGetDisplay(); 
  
  // Placeholder rendering until new themes are added
  disp.setTextSize(1);
  disp.setTextColor(SSD1306_WHITE);
  disp.setCursor(0, 0);
  disp.println(time_str);
  disp.println(date_str);
  disp.println(day_str);

  // Render battery status only if battery is connected
  if (battery_is_connected()) {
    uint8_t battery_pct = battery_get_percentage();
    // Battery rendering removed as it was part of theme
    disp.setCursor(100, 0);
    disp.print(battery_pct);
    disp.print("%");
  }
  
  DisplayUpdate();
}

void TimeForceRender() {
  TimeRender();
}

void TimeSetTheme(DisplayTheme_t theme) {
  // Theme switching disabled
  config_save_theme((uint8_t)theme);
}