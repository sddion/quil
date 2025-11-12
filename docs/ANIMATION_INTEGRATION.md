# Animation Integration - Phase 9

## Overview

Integrated pre-rendered bitmap animations stored in PROGMEM for memory-efficient playback on ESP32/ESP8266.

## Animations Available

| Animation | Frames | Use Case |
|-----------|--------|----------|
| ANIM_BOOT | 28 | Boot sequence on startup |
| ANIM_THINKING | 56 | AI processing, chat mode |
| ANIM_ANGRY | 34 | Error states, negative emotion |
| ANIM_THUGLIFE | 36 | Theme preview, easter egg |

## Module: animation_manager

### Functions

```cpp
void anim_init();                    // Initialize animation system
void anim_play(AnimationType type);  // Start animation
void anim_stop();                    // Stop current animation
void anim_update();                  // Update frame (call in loop)
bool anim_is_playing();              // Check playback status
```

### Animation Types

```cpp
typedef enum {
  ANIM_NONE,
  ANIM_THINKING,
  ANIM_ANGRY,
  ANIM_THUGLIFE,
  ANIM_BOOT
} AnimationType;
```

## Implementation

### Storage Format

Each animation stored as PROGMEM arrays:
- Individual frames: `_thinking_000[]`, `_thinking_001[]`, etc.
- Frame pointer array: `thinking_frames[]`
- Frame count: `THINKING_FRAMES` macro

### Frame Playback

- **Frame rate:** 20 FPS (50ms delay)
- **Non-blocking:** Uses `millis()` timing
- **Loop behavior:** Auto-loops continuously
- **Display:** Direct `hal_display` integration

### Memory Usage

Total PROGMEM allocation:
- Thinking: ~56KB (56 frames × 1KB)
- Angry: ~34KB (34 frames × 1KB)
- Thuglife: ~36KB (36 frames × 1KB)
- Boot: ~28KB (28 frames × 1KB)

**Total:** ~154KB PROGMEM

## Integration Points

### Boot Sequence (main.cpp)

```cpp
anim_init();
anim_play(ANIM_BOOT);
while(anim_is_playing()) {
  anim_update();
  delay(10);
}
```

### Main Loop (main.cpp)

```cpp
if (anim_is_playing()) {
  anim_update();
  delay(10);
  return;  // Skip mode rendering
}
```

### Mode Triggers

**Chat Mode (thinking animation):**
```cpp
if (wake_detect()) {
  anim_play(ANIM_THINKING);
}
```

**Error States (angry animation):**
```cpp
if (error_occurred) {
  anim_play(ANIM_ANGRY);
}
```

**Theme Preview (thuglife):**
```cpp
if (theme == THEME_THUGLIFE) {
  anim_play(ANIM_THUGLIFE);
}
```

## File Structure

```
firmware/
├── assets/
│   └── bitmaps_arrays/
│       ├── thinking/
│       │   ├── thinking.h       (header with externs)
│       │   └── thinking.cpp     (frame data)
│       ├── angry/
│       │   ├── angry.h
│       │   └── angry.cpp
│       ├── thuglife/
│       │   ├── thuglife.h
│       │   └── thuglife.cpp
│       └── bootanimation/
│           ├── bootanimation.h
│           └── bootanimation.cpp
└── src/
    └── modules/
        ├── animation_manager.h
        └── animation_manager.cpp
```

## Build Configuration

### platformio.ini Updates

```ini
build_flags = 
    -I firmware/assets/bitmaps_arrays
build_src_filter = +<*> +<../../assets/bitmaps_arrays/*>
```

## Performance

- **Frame load time:** <1ms (PROGMEM read)
- **Display update:** ~5ms (I2C transfer)
- **Total frame time:** ~6ms actual, 50ms target
- **CPU usage:** Minimal (non-blocking)
- **Heap usage:** 0 bytes (all static/PROGMEM)

## Stopping Animations

Animations auto-loop by default. To stop:

```cpp
anim_stop();  // Manual stop
```

Or animations auto-stop on mode change when loop checks `anim_is_playing()`.

## Adding New Animations

1. Create bitmap array in `assets/bitmaps_arrays/newAnim/`
2. Generate `newAnim.h` with externs
3. Add `ANIM_NEWANIM` to enum in `animation_manager.h`
4. Add case to switch in `anim_play()`
5. Include header in `animation_manager.cpp`

## Troubleshooting

**Animation not displaying:**
- Check PROGMEM includes: `#include <avr/pgmspace.h>`
- Verify frame count matches array size
- Check `build_src_filter` in platformio.ini

**Flickering:**
- Frame delay too short
- Increase `frame_delay` in animation_manager.cpp

**Memory errors:**
- Too many animations loaded
- Use `pgm_read_ptr()` for PROGMEM access
- Check available flash: `pio run -t size`

## Build Verification

```bash
# Build for ESP32
pio run -e esp32

# Check binary size
pio run -e esp32 -t size

# Build for ESP8266
pio run -e esp8266

# Verify binaries exist
ls -lh .pio/build/esp32/firmware.bin
ls -lh .pio/build/esp8266/firmware.bin
```

## Future Enhancements

1. **Variable frame rates** - Per-animation speed control
2. **One-shot playback** - Non-looping option
3. **Blend transitions** - Fade between animations
4. **Compression** - RLE encoding for smaller PROGMEM
5. **Runtime loading** - Load from SD card or SPIFFS
