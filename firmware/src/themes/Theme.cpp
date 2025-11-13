#include "themes.h"

Theme* currentTheme = nullptr;

void setTheme(Theme& t) {
    currentTheme = &t;
}

void renderCurrentTheme(Adafruit_SSD1306 &d, const tm &t, const char *dayStr) {
    if (currentTheme && currentTheme->render) {
        currentTheme->render(d, t, *currentTheme, dayStr);
    }
}