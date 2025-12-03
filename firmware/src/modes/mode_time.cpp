#include "mode_time.h"
#include "config.h"
#include "hal/hal_display.h"
#include "modules/ntp_client.h"
#include "modules/config_store.h"
#include "modules/weather_manager.h"
#include "modules/battery_manager.h"

static WeatherManager weatherManager;
static WeatherData weatherData;
static char weatherApiKey[65];
static char weatherLocation[65];
static unsigned long lastWeatherUpdate = 0;

void mode_time_init() {
  ntp_init();
  
  // Load and apply timezone offset
  int tz_offset = NTP_OFFSET_SEC;
  if (config_load_timezone(&tz_offset)) {
    ntp_set_timezone(tz_offset);
  }
  
  battery_init();
  
  if (!config_load_weather(weatherApiKey, weatherLocation)) {
    // Use defaults from secrets.env
    strcpy(weatherApiKey, DEFAULT_WEATHER_API_KEY);
    strcpy(weatherLocation, DEFAULT_WEATHER_LOCATION);
  }
}

void mode_time_update() {
  ntp_update();
  battery_update();
  if (millis() - lastWeatherUpdate > 900000) { // 15 minutes
    lastWeatherUpdate = millis();
    weatherData = weatherManager.getWeatherData(weatherApiKey, weatherLocation);
  }
}

void mode_time_render() {
  hal_display_clear();
  
  String time_str = ntp_get_time();
  String date_str = ntp_get_date();
  String day_str = ntp_get_day();
  
  Adafruit_SSD1306& disp = hal_display_get_display(); 
  
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
    bool battery_low = battery_is_low();
    // Battery rendering removed as it was part of theme
    disp.setCursor(100, 0);
    disp.print(battery_pct);
    disp.print("%");
  }
  
  hal_display_update();
}

void mode_time_force_render() {
  mode_time_render();
}

void mode_time_set_theme(DisplayTheme_t theme) {
  // Theme switching disabled
  config_save_theme((uint8_t)theme);
}
