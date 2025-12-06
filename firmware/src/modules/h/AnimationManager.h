#pragma once
#include <Arduino.h>

typedef enum {
  ANIM_NONE,
  ANIM_THINKING,
  ANIM_BOOT
} AnimationType;

void AnimInit();
void AnimPlay(AnimationType type);
void AnimStop();
void AnimUpdate();
bool AnimIsPlaying();