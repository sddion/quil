#include "ClassicTheme.h"
#include "hal/hal_display.h"

void BoldTheme::render(String time, String date, String day) {
  hal_display_text_size(2);
  hal_display_text(time.c_str(), 10, 20);
  hal_display_text_size(1);
  hal_display_text(date.c_str(), 20, 45);
  hal_display_text(day.c_str(), 50, 55);
}
