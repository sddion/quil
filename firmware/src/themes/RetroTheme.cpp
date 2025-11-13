#include "RetroTheme.h"
#include "hal/hal_display.h"

void RetroTheme::render(String time, String date, String day) {
  hal_display_rect(0, 0, 128, 64, true); // Outer box
  hal_display_rect(2, 2, 124, 60, true); // Inner box
  hal_display_text_size(2);
  hal_display_text(time.c_str(), 10, 10);
  hal_display_text_size(1);
  hal_display_text(date.c_str(), 15, 35);
  hal_display_text(day.c_str(), 15, 45);
}

void CyberpunkTheme::render(String time, String date, String day)
{
}
