#pragma once
#include <Arduino.h>

typedef enum {
  ANIM_NONE,
  ANIM_THINKING,
  ANIM_ANGRY,
  ANIM_THUGLIFE,
  ANIM_BOOT
} AnimationType;

void anim_init();
void anim_play(AnimationType type);
void anim_stop();
void anim_update();
bool anim_is_playing();
