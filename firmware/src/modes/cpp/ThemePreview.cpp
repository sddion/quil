#include "../h/ThemePreview.h"
#include "hal/h/Display.h"

static const char* themes[] = {"Aurora", "NeonPulse", "MonoMist"};
static uint8_t current_theme = 0;

void ThemeInit() {}

void ThemeUpdate() {}

void ThemeRender() {
  DisplayClear();
  DisplayText("THEME", 40, 5);
  DisplayText(themes[current_theme], 25, 25);
  DisplayUpdate();
}

void ThemeNext() {
  current_theme = (current_theme + 1) % 3;
}

void ThemePrev() {
  current_theme = (current_theme == 0) ? 2 : current_theme - 1;
}

void ThemeApply() {}