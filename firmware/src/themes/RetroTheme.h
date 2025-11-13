#pragma once
#include "Theme.h"


class RetroTheme : public Theme {
public:
  void render(String time, String date, String day) override;
};
