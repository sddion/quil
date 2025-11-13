#pragma once
#include "Theme.h"

class BoldTheme : public Theme {
public:
  void render(String time, String date, String day) override;
};
