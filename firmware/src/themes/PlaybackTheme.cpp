#include "PlaybackTheme.h"
#include "hal/hal_display.h"

void PlaybackTheme::render(String time, String date, String day) {
  hal_display_rect(0, 0, 128, 64, true); // Outline the entire screen
  hal_display_text_size(2);
  hal_display_text(time.c_str(), 10, 20);
  hal_display_text_size(1);
  hal_display_text(date.c_str(), 15, 40);
  hal_display_text(day.c_str(), 15, 50);
}
