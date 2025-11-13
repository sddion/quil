#pragma once

typedef enum {
  THEME_MINIMAL = 0,
  THEME_BOLD = 1,
  THEME_RETRO = 2,
  THEME_PLAYBACK = 3
} DisplayTheme_t;

void mode_time_init();
void mode_time_update();
void mode_time_render();
void mode_time_set_theme(DisplayTheme_t theme);
