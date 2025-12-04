#include "../h/ThemePreview.h"
#include "hal/h/Display.h"

static const char* themes[] = {"Aurora", "NeonPulse", "MonoMist"};
static uint8_t current_theme = 0;

void mode_theme_init() {}

void mode_theme_update() {}

void mode_theme_render() {
  hal_display_clear();
  hal_display_text("THEME", 40, 5);
  hal_display_text(themes[current_theme], 25, 25);
  hal_display_update();
}

void mode_theme_next() {
  current_theme = (current_theme + 1) % 3;
}

void mode_theme_prev() {
  current_theme = (current_theme == 0) ? 2 : current_theme - 1;
}

void mode_theme_apply() {}
