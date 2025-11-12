# Gesture System Integration

## New Modules

### gesture_manager.{h,cpp}
Non-blocking gesture decoder with timing-based detection:
- Single tap: <300ms press
- Double tap: 2 taps within 600ms
- Swipe: Sequential electrode detection within 400ms
- Debouncing and state tracking

### touch_actions.{h,cpp}
Mode-specific gesture action router implementing:

| Gesture | Mode | Action |
|---------|------|--------|
| Single Tap | MUSIC | Toggle Play/Pause |
| Single Tap | THEME_PREVIEW | Apply Theme |
| Single Tap | CHAT | Toggle Mute |
| Swipe Left | MUSIC | Previous Track |
| Swipe Right | MUSIC | Next Track |
| Swipe Left | THEME_PREVIEW | Previous Theme |
| Swipe Right | THEME_PREVIEW | Next Theme |
| Double Tap | All Modes | Cycle Mode |

## Integration Pipeline

```
MPR121 (12 electrodes)
    ↓
gesture_detect() - Decode raw touch to GestureType
    ↓
actions_handle() - Route gesture + mode to action
    ↓
Mode functions (toggle/next/prev)
```

## Updated Functions

**mode_music:**
- `mode_music_toggle()` - Play/pause
- `mode_music_next()` - Skip forward
- `mode_music_prev()` - Skip back

**mode_chat:**
- `mode_chat_toggle_mute()` - Mute toggle

**mode_theme_preview:**
- `mode_theme_apply()` - Commit theme

## Main Loop Flow

```cpp
uint16_t touch = hal_mpr121_read_touched();
GestureType gest = gesture_detect(touch, millis());
if (gest != GESTURE_NONE) {
  actions_handle(gest, state_get_mode());
}
```

## Performance

- Non-blocking: Uses millis() timestamps
- Polling rate: Every 10ms via main loop
- Static buffers: No heap allocation
- Debounced: 50ms touch flicker ignored

## Testing

1. Single tap electrode 0 → action depends on mode
2. Double tap (2x within 600ms) → mode cycles
3. Swipe electrode 0→3 → right swipe action
4. Swipe electrode 11→8 → left swipe action
