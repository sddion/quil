#include "animation_manager.h"
#include "hal/hal_display.h"

#ifdef ESP32
#include <pgmspace.h>
#include "../animations/animation_frames.h"

static AnimationType current_anim = ANIM_NONE;
static uint16_t current_frame = 0;
static uint16_t total_frames = 0;
static unsigned long last_frame_time = 0;
static const unsigned char* const* frame_array = nullptr;
static bool playing = false;
static const uint16_t frame_delay = 50;

void anim_init() {
  current_anim = ANIM_NONE;
  playing = false;
}

void anim_play(AnimationType type) {
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
    case ANIM_THUGLIFE:
      frame_array = thuglife_frames;
      total_frames = THUGLIFE_FRAMES;
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

void anim_stop() {
  playing = false;
  current_anim = ANIM_NONE;
}

void anim_update() {
  if (!playing || frame_array == nullptr) return;
  
  unsigned long now = millis();
  if (now - last_frame_time < frame_delay) return;
  
  last_frame_time = now;
  
  const unsigned char* frame = (const unsigned char*)pgm_read_ptr(&frame_array[current_frame]);
  hal_display_clear();
  hal_display_bitmap(frame);
  hal_display_update();
  
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

bool anim_is_playing() {
  return playing;
}

#elif defined(ESP8266)
// ESP8266 limited animation support - boot animation only
#include "../assets/bitmaps_arrays/bootanimation/bootanimation.h"

static AnimationType current_anim = ANIM_NONE;
static uint16_t current_frame = 0;
static uint16_t total_frames = 0;
static unsigned long last_frame_time = 0;
static const unsigned char* const* frame_array = nullptr;
static bool playing = false;
static const uint16_t frame_delay = 50;

void anim_init() {
  current_anim = ANIM_NONE;
  playing = false;
}

void anim_play(AnimationType type) {
  if (type != ANIM_BOOT) {
    // Only boot animation supported on ESP8266
    return;
  }
  
  current_anim = type;
  current_frame = 0;
  playing = true;
  last_frame_time = millis();
  frame_array = boot_frames;
  total_frames = BOOT_FRAMES;
}

void anim_stop() {
  playing = false;
  current_anim = ANIM_NONE;
}

void anim_update() {
  if (!playing || frame_array == nullptr) return;
  
  unsigned long now = millis();
  if (now - last_frame_time < frame_delay) return;
  
  last_frame_time = now;
  
  const unsigned char* frame = (const unsigned char*)pgm_read_ptr(&frame_array[current_frame]);
  hal_display_clear();
  hal_display_bitmap(frame);
  hal_display_update();
  
  current_frame++;
  if (current_frame >= total_frames) {
    playing = false;
    current_anim = ANIM_NONE;
  }
}

bool anim_is_playing() {
  return playing;
}
#endif
