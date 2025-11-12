#include "mode_time.h"
#include "hal/hal_display.h"
#include "modules/ntp_client.h"
#include "modules/config_store.h"
#include "config.h"

static DisplayTheme_t current_theme = THEME_CLASSIC;

void mode_time_init() {
  ntp_init();
  // Load theme from config
  uint8_t theme_val = 0;
  if (config_load_theme(&theme_val)) {
    current_theme = (DisplayTheme_t)theme_val;
  }
}

void mode_time_update() {
  ntp_update();
}

void mode_time_render() {
  hal_display_clear();
  
  String time_str = ntp_get_time();
  String date_str = ntp_get_date();
  String day_str = ntp_get_day();
  
  switch(current_theme) {
    case THEME_CLASSIC:
      // Classic: Large time, small date below
      hal_display_text_size(2);
      hal_display_text(time_str.c_str(), 10, 20);
      hal_display_text_size(1);
      hal_display_text(date_str.c_str(), 20, 45);
      hal_display_text(day_str.c_str(), 50, 55);
      break;
      
    case THEME_MINIMAL:
      // Minimal: Time only, centered, medium size
      hal_display_text_size(2);
      hal_display_text(time_str.c_str(), 10, 25);
      break;
      
    case THEME_CYBER:
      // Cyber: Time + date side by side, boxed
      hal_display_rect(2, 2, 124, 60, true);
      hal_display_text_size(2);
      hal_display_text(time_str.c_str(), 10, 15);
      hal_display_text_size(1);
      hal_display_text(date_str.c_str(), 25, 40);
      hal_display_text(day_str.c_str(), 50, 50);
      break;
  }
  
  hal_display_update();
}

void mode_time_set_theme(DisplayTheme_t theme) {
  current_theme = theme;
  config_save_theme((uint8_t)theme);
}
