#ifndef THEMES_H
#define THEMES_H

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "fonts/FreeSansBold12pt7b.h"
#include "fonts/FreeSans9pt7b.h"
#include "fonts/FreeSans7pt7b.h"

struct Theme {
    const GFXfont* fontTime;
    const GFXfont* fontDay;
    const GFXfont* fontDate;

    int timeX, timeY;
    int weatherX, weatherY;
    int dayX, dayY;
    int dateX, dateY;
    int cornerX, cornerY;

    void (*render)(Adafruit_SSD1306&, const tm&, const Theme&, const char*);
};

// Built-in themes
extern Theme MinimalHeader;
extern Theme BoldHeader;
extern Theme RetroHeader;

// Active theme
extern Theme* currentTheme;

// Set active theme
void setTheme(Theme& t);

// Main renderer (calls theme.render)
void renderCurrentTheme(
    Adafruit_SSD1306 &d,
    const tm &t,
    const char* dayStr
);

#endif
