#pragma once
#include "Theme.h"

class MinimalTheme : public Theme {
public:
  void render(String time, String date, String day) override;
};
