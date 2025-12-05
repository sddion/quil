#include "../h/AnimationManager.h"
#include "hal/h/Display.h"

#include <pgmspace.h>
#include "../../animations/h/AnimationFrames.h"

static AnimationType current_anim = ANIM_NONE;
static uint16_t current_frame = 0;
static uint16_t total_frames = 0;
static unsigned long last_frame_time = 0;
static const unsigned char* const* frame_array = nullptr;
static bool playing = false;
static const uint16_t frame_delay = 50;

void AnimInit() {
  current_anim = ANIM_NONE;
  playing = false;
}

void AnimPlay(AnimationType type) {
  current_anim = type;
  current_frame = 0;
  playing = true;
  last_frame_time = millis();
  
  switch(type) {
    case ANIM_THINKING:
      frame_array = thinking_frames;
      total_frames = THINKING_FRAMES;
      break;
    case ANIM_ANGRY:
      frame_array = angry_frames;
      total_frames = ANGRY_FRAMES;
      break;

    case ANIM_BOOT:
      frame_array = boot_frames;
      total_frames = BOOT_FRAMES;
      break;
    default:
      playing = false;
      return;
  }
}

void AnimStop() {
  playing = false;
  current_anim = ANIM_NONE;
}

void AnimUpdate() {
  if (!playing || frame_array == nullptr) return;
  
  unsigned long now = millis();
  if (now - last_frame_time < frame_delay) return;
  
  last_frame_time = now;
  
  const unsigned char* frame = (const unsigned char*)pgm_read_ptr(&frame_array[current_frame]);
  DisplayClear();
  DisplayBitmap(frame);
  DisplayUpdate();
  
  current_frame++;
  if (current_frame >= total_frames) {
    // Stop boot animation after completion (don't loop)
    if (current_anim == ANIM_BOOT) {
      playing = false;
      current_anim = ANIM_NONE;
    } else {
      // Other animations can loop
      current_frame = 0;
    }
  }
}

bool AnimIsPlaying() {
  return playing;
}
