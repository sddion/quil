# Quil Gesture System - Complete Specification

## Architecture

### Data Flow
```
MPR121 Hardware (12 electrodes)
    ↓
hal_mpr121_read_touched() → uint16_t (12-bit bitmask)
    ↓
gesture_detect(touch, millis()) → GestureType enum
    ↓
actions_handle(gesture, mode) → Execute mode-specific action
    ↓
Mode functions (play/pause/next/prev/toggle/etc)
```

## Gesture Detection Logic

### gesture_manager.cpp

**State Variables:**
- `last_touch` - Previous electrode state
- `last_tap_time` - Timestamp of last tap
- `tap_count` - Sequential tap counter
- `last_electrode` - Last touched electrode ID
- `swipe_start` - Swipe sequence start time

**Timing Thresholds:**
- Single tap: Touch + release < 300ms
- Double tap: 2 taps with gap < 600ms between
- Swipe: Sequential electrode changes < 400ms
- Tap timeout: Reset count after 600ms idle

**Swipe Detection:**
- Right swipe: Electrode N → N+3 (e.g., 0→3)
- Left swipe: Electrode N → N-3 (e.g., 11→8)
- Requires sequential touch within 400ms window

## Action Mapping Table

| Gesture | TIME_DATE | MUSIC | CHAT | THEME_PREVIEW | WIFI_INFO |
|---------|-----------|-------|------|---------------|-----------|
| Single Tap | - | Play/Pause | Mute/Unmute | Apply Theme | - |
| Double Tap | Cycle Mode | Cycle Mode | Cycle Mode | Cycle Mode | Cycle Mode |
| Swipe Left | - | Previous Track | - | Previous Theme | - |
| Swipe Right | - | Next Track | - | Next Theme | - |

## Mode Function Implementations

### mode_music.cpp
```cpp
void mode_music_toggle();  // Switch play/pause state
void mode_music_next();    // Skip to next track
void mode_music_prev();    // Skip to previous track
```

### mode_chat.cpp
```cpp
void mode_chat_toggle_mute();  // Toggle microphone mute
```

### mode_theme_preview.cpp
```cpp
void mode_theme_apply();   // Commit current theme
void mode_theme_next();    // Cycle to next theme (existing)
void mode_theme_prev();    // Cycle to previous theme (existing)
```

## Integration Points

### main.cpp Loop
```cpp
uint16_t touch = hal_mpr121_read_touched();  // Poll MPR121
GestureType gest = gesture_detect(touch, millis());  // Decode
if (gest != GESTURE_NONE) {
  actions_handle(gest, state_get_mode());  // Execute
}
```

### Initialization (setup)
```cpp
gesture_init();   // Reset gesture state
actions_init();   // Prepare action router
```

## Performance Characteristics

- **Latency:** ~10ms (main loop polling rate)
- **Memory:** 12 bytes static state (no heap)
- **CPU:** Minimal (timestamp comparisons only)
- **Debouncing:** Hardware + software (50ms ignore threshold)

## Edge Cases Handled

1. **Simultaneous electrodes:** Uses first detected electrode
2. **Rapid taps:** Counter resets after 600ms timeout
3. **Incomplete swipes:** Swipe state clears after 400ms
4. **Touch hold:** Not recognized as tap until release
5. **Mode transitions:** Double-tap always cycles mode regardless of current mode

## Testing Checklist

- [ ] Single tap electrode 0 in MUSIC mode → Play/Pause
- [ ] Single tap electrode 0 in CHAT mode → Mute toggle
- [ ] Single tap in THEME_PREVIEW → Apply theme
- [ ] Double tap any electrode → Mode cycles
- [ ] Touch electrode 0 then 3 within 400ms → Right swipe (MUSIC next)
- [ ] Touch electrode 11 then 8 within 400ms → Left swipe (MUSIC prev)
- [ ] Rapid 3+ taps → Only first 2 count as double-tap
- [ ] Hold electrode for 1s → Not recognized as tap

## Module Files

**New:**
- `modules/gesture_manager.{h,cpp}` - 67 lines total
- `modules/touch_actions.{h,cpp}` - 45 lines total

**Modified:**
- `modes/mode_music.{h,cpp}` - Added toggle/next/prev
- `modes/mode_chat.{h,cpp}` - Added toggle_mute
- `modes/mode_theme_preview.{h,cpp}` - Added apply
- `main.cpp` - Replaced touch_handler with gesture system

**Removed:**
- `modules/touch_handler.{h,cpp}` - Deprecated

## Future Enhancements

1. **Long press:** Hold > 1s for alternative actions
2. **Multi-touch:** Simultaneous electrode combinations
3. **Swipe speed:** Fast vs slow swipe differentiation
4. **Gesture sequences:** Tap-swipe-tap patterns
5. **Adaptive thresholds:** Learn user timing preferences
