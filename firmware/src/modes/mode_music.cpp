#include "mode_music.h"
#include "hal/hal_display.h"
#include "themes/current_theme.h"
#ifdef ESP32
#include "hal/hal_i2s.h"
#endif

static bool playing = false;
static String song = "Neon Dreams";
static String artist = "Synth Wave";

void mode_music_init() {
#ifdef ESP32
  hal_i2s_init_speaker();
#endif
}

void mode_music_update() {}

void mode_music_render() {
  hal_display_clear();
  if (currentTheme) {
    Adafruit_SSD1306& disp = hal_display_get_display();
    currentTheme->renderMusic(disp, song, artist, playing);
  }
  hal_display_update();
}

void mode_music_play() {
  playing = true;
}

void mode_music_pause() {
  playing = false;
}

void mode_music_toggle() {
  playing = !playing;
}

void mode_music_next() {}

void mode_music_prev() {}
