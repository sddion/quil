#pragma once
#include "Theme.h"

class PlaybackTheme : public Theme {
public:
  void render(String time, String date, String day) override;
};
