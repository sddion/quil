#include "MinimalTheme.h"
#include "hal/hal_display.h"

void MinimalTheme::render(String time, String date, String day) {
  hal_display_text_size(2);
  hal_display_text(time.c_str(), 10, 25);
}
