#include "mode_time.h"
#include "config.h"
#include "hal/hal_display.h"
#include "modules/ntp_client.h"
#include "modules/config_store.h"
#include "modules/weather_manager.h"
#include "modules/battery_manager.h"
#include "themes/Theme.h"
#include "themes/MinimalTheme.h"
#include "themes/BoldTheme.h"
#include "themes/RetroTheme.h"
#include "themes/PlaybackTheme.h"
#include "themes/current_theme.h"

static WeatherManager weatherManager;
static WeatherData weatherData;
static char weatherApiKey[65];
static char weatherLocation[65];
static unsigned long lastWeatherUpdate = 0;

static BoldTheme boldTheme;
static MinimalTheme minimalTheme;
static RetroTheme retroTheme;
static PlaybackTheme playbackTheme;

static DisplayTheme_t currentThemeType = THEME_BOLD;

void mode_time_init() {
  currentTheme = &boldTheme;
  ntp_init();
  
  // Load and apply timezone offset
  int tz_offset = NTP_OFFSET_SEC;
  if (config_load_timezone(&tz_offset)) {
    ntp_set_timezone(tz_offset);
  }
  
  battery_init();
  uint8_t theme_val = 0;
  if (config_load_theme(&theme_val)) {
    mode_time_set_theme((DisplayTheme_t)theme_val);
  }
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
  currentTheme->renderTime(disp, time_str, date_str, day_str);
  currentTheme->renderWeather(disp, weatherData);
  
  // Render battery status only if battery is connected
  if (battery_is_connected()) {
    uint8_t battery_pct = battery_get_percentage();
    bool battery_low = battery_is_low();
    currentTheme->renderBattery(disp, battery_pct, battery_low);
  }
  
  hal_display_update();
}

void mode_time_force_render() {
  // Force an immediate render after boot animation
  // This ensures the theme bitmap is displayed right away
  mode_time_render();
}

void mode_time_set_theme(DisplayTheme_t theme) {
  currentThemeType = theme;
  switch (theme) {
    case THEME_MINIMAL:
      currentTheme = &minimalTheme;
      break;
    case THEME_BOLD:
      currentTheme = &boldTheme;
      break;
    case THEME_RETRO:
      currentTheme = &retroTheme;
      break;
    case THEME_PLAYBACK:
      currentTheme = &playbackTheme;
      break;
  }
  config_save_theme((uint8_t)theme);
}
