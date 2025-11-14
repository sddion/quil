#pragma once
#include <pgmspace.h>

#ifdef ESP8266
  #undef PROGMEM
  #define PROGMEM
#endif  

#define THEME_WIDTH 128
#define THEME_HEIGHT 64

extern const unsigned char oled_theme_bold [] PROGMEM;
extern const unsigned char oled_theme_minimal [] PROGMEM;
extern const unsigned char oled_theme_playback [] PROGMEM;
extern const unsigned char oled_theme_retro [] PROGMEM;
