# Animation Integration Guide

This guide explains how to integrate and manage pre-rendered bitmap animations in the Quil firmware. The animations are stored in PROGMEM to conserve RAM and are played back in a non-blocking manner.

## Animation System Overview

The animation system is managed by the `animation_manager` module. It is responsible for initializing, playing, stopping, and updating animations.

### Playback Flow

The following diagram illustrates the animation playback flow:

```mermaid
graph TD
    A[Mode or System Event] -->|anim_play(ANIM_TYPE)| B{Animation Manager};
    B -->|Is an animation requested?| C{Yes};
    C --> D[Set Active Animation];
    D --> E[Reset Frame Counter];
    E --> F{Main Loop};
    F -->|anim_update()| G{Time to show next frame?};
    G -->|Yes| H[Load frame from PROGMEM];
    H --> I[Draw frame to display buffer];
    I --> F;
```

## Available Animations

| Animation | Frames | Use Case |
|---|---|---|
| `ANIM_BOOT` | 28 | Played once on device startup. |
| `ANIM_THINKING` | 56 | Shown when the device is processing a request. |
| `ANIM_ANGRY` | 34 | Displayed in response to an error or negative event. |
| `ANIM_THUGLIFE` | 36 | An easter-egg theme. |

## `animation_manager` Module

### Public Functions

```cpp
// Initializes the animation system.
void anim_init();

// Starts playing an animation of the given type.
void anim_play(AnimationType type);

// Stops the currently playing animation.
void anim_stop();

// Updates the animation frame. Should be called in the main loop.
void anim_update();

// Returns true if an animation is currently playing.
bool anim_is_playing();
```

### `AnimationType` Enum

```cpp
typedef enum {
  ANIM_NONE,
  ANIM_THINKING,
  ANIM_ANGRY,
  ANIM_THUGLIFE,
  ANIM_BOOT
} AnimationType;
```

## Implementation Details

### Storage Format

Each animation is stored as a series of PROGMEM arrays:

-   **Frame Data**: The raw bitmap data for each frame (e.g., `_thinking_000[]`, `_thinking_001[]`).
-   **Frame Pointer Array**: An array of pointers to the individual frame data arrays (e.g., `thinking_frames[]`).
-   **Frame Count**: A macro that defines the total number of frames in the animation (e.g., `THINKING_FRAMES`).

### Frame Playback

-   **Frame Rate**: 20 FPS (50ms delay between frames).
-   **Non-Blocking**: Uses `millis()` for timing to avoid blocking the main loop.
-   **Looping**: Animations loop automatically until `anim_stop()` is called.

## How to Add a New Animation

1.  **Prepare Your Bitmaps**:
    *   Create a sequence of bitmap images for your animation.
    *   Make sure they are the correct size and format for the display.

2.  **Convert to C Arrays**:
    *   Use a tool to convert your bitmap images into C arrays.
    *   Place the generated `.cpp` and `.h` files in a new directory under `firmware/assets/bitmaps_arrays/`. For example: `firmware/assets/bitmaps_arrays/new_animation/`.

3.  **Update `animation_manager.h`**:
    *   Add a new entry to the `AnimationType` enum for your animation:
        ```cpp
        typedef enum {
          // ... existing animations
          ANIM_NEW_ANIMATION
        } AnimationType;
        ```

4.  **Update `animation_manager.cpp`**:
    *   Include the header file for your new animation:
        ```cpp
        #include "new_animation.h"
        ```
    *   In the `anim_play()` function, add a `case` for your new animation to the `switch` statement. This tells the animation manager how to load your animation's data.

5.  **Update Build Configuration**:
    *   Ensure that the `platformio.ini` file is configured to include your new animation files in the build. The existing `build_src_filter` should already cover this if you've placed your files in the correct directory.

## Troubleshooting

*   **Animation Not Displaying**:
    *   Verify that the `build_src_filter` in `platformio.ini` includes the path to your animation files.
    *   Double-check that the frame count in your animation's header file matches the actual number of frames.
*   **Flickering**:
    *   This can happen if the frame delay is too short. Try increasing the `frame_delay` value in `animation_manager.cpp`.
*   **Memory Errors**:
    *   If you run out of PROGMEM, you may need to reduce the number of frames in your animations or optimize their size. You can check the available flash space by running `pio run -t size`.
