#pragma once
#include <Arduino.h>

// Boot stages for progress tracking
enum BootStage {
  BOOT_STAGE_HARDWARE = 0,
  BOOT_STAGE_WIFI,
  BOOT_STAGE_TIME,
  BOOT_STAGE_SERVICES,
  BOOT_STAGE_DISPLAY,
  BOOT_STAGE_UPDATE,
  BOOT_STAGE_COMPLETE,
  BOOT_STAGE_COUNT
};

void BootLoaderInit();
void BootLoaderShowStage(BootStage stage, bool isFirstBoot);
void BootLoaderComplete();
