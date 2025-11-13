#include "mode_time.h"
#include "hal/hal_display.h"
#include "modules/ntp_client.h"
#include "modules/config_store.h"
#include "themes/Theme.h"
#include "themes/MinimalTheme.h"
#include "themes/BoldTheme.h"
#include "themes/RetroTheme.h"
#include "themes/PlaybackTheme.h"


static BoldTheme boldTheme;
static MinimalTheme minimalTheme;
static RetroTheme retroTheme;
static PlaybackTheme playbackTheme;

static Theme* currentTheme = &boldTheme;
static DisplayTheme_t currentThemeType = THEME_BOLD;

void mode_time_init() {
  ntp_init();
  uint8_t theme_val = 0;
  if (config_load_theme(&theme_val)) {
    mode_time_set_theme((DisplayTheme_t)theme_val);
  }
}

void mode_time_update() {
  ntp_update();
}

void mode_time_render() {
  hal_display_clear();
  
  if (!ntp_is_synced()) {
    hal_display_text_size(2);
    hal_display_update();
    return;
  }
  
  String time_str = ntp_get_time();
  String date_str = ntp_get_date();
  String day_str = ntp_get_day();
  
  currentTheme->render(time_str, date_str, day_str);
  
  hal_display_update();
}

void mode_time_set_theme(DisplayTheme_t theme) {
  currentThemeType = theme;
  switch (theme) {
    case THEME_CLASSIC:
      currentTheme = &classicTheme;
      break;
    case THEME_MINIMAL:
      currentTheme = &minimalTheme;
      break;
    case THEME_AURORA:
      currentTheme = &auroraTheme;
      break;
    case THEME_CYBERPUNK:
      currentTheme = &cyberpunkTheme;
      break;
    case THEME_AESTHETIC:
      currentTheme = &aestheticTheme;
      break;
  }
  config_save_theme((uint8_t)theme);
}
