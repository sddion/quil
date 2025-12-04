# TTP223 Single Touch Sensor System

## Overview

The Quil firmware uses one TTP223 capacitive touch sensor for user interaction. This system provides a reliable single-sensor approach that supports tap-based gesture detection.

## Hardware Configuration

### Pin Assignments

#### ESP32
- **Sensor A**: GPIO34 (ADC1_CH6) - Input only pin

#### ESP8266
- **Sensor A**: D5 (GPIO14)

### Sensor Characteristics
- **Type**: TTP223 Capacitive Touch Sensor
- **Signal**: Active-HIGH (outputs HIGH when touched)
- **Voltage**: 3.3V compatible
- **Response Time**: <100ms
- **Debounce**: 50ms (implemented in software)

## Supported Gestures

### Individual Sensor Events

The sensor can detect:

1. **Single Tap**
   - Quick touch and release
   - Duration: < 300ms
   - Use case: Select, confirm, cycle options

2. **Double Tap**
   - Two quick taps within 400ms
   - Automatically detected after second tap
   - Use case: Activate, enter mode, apply changes

3. **Long Press**
   - Hold for 800ms or more
   - Triggers once when threshold is reached
   - Use case: Enter settings, power options

4. **Hold Start**
   - Triggered when long press threshold is reached
   - Sensor remains pressed after trigger
   - Use case: Begin continuous action

5. **Hold Release**
   - Triggered when sensor is released after hold
   - Includes total hold duration
   - Use case: End continuous action

## Timing Configuration

All timing values are in milliseconds and defined in `hal_ttp223.h`:

```cpp
#define DEBOUNCE_TIME 50              // Debounce delay
#define TAP_TIMEOUT 300               // Max duration for tap
#define DOUBLE_TAP_WINDOW 400         // Max time between taps
#define LONG_PRESS_TIME 800           // Long press threshold
```

## Software Architecture

### Module Structure

```
hal/
  hal_ttp223.h          # HAL interface and data structures
  hal_ttp223.cpp        # State machine implementation

modules/
  gesture_manager.h     # High-level gesture interface
  gesture_manager.cpp   # Maps TTP223 events to GestureType
  touch_actions.h       # Action mapping interface
  touch_actions.cpp     # Maps gestures to mode-specific actions
```

### Data Flow

```
TTP223 Sensor (Hardware)
         ↓
hal_ttp223_update() - Read & debounce pin
         ↓
State Machine - Process timing & patterns
         ↓
Event Detection
         ↓
gesture_detect() - Convert to GestureType
         ↓
actions_handle() - Execute mode-specific action
```

### State Machine

The sensor runs a state machine:

- **IDLE**: No touch detected
- **PRESSED**: Touch detected, timing started
- **WAIT_DOUBLE_TAP**: Released after first tap, waiting for second
- **LONG_PRESS**: Long press threshold reached
- **HOLDING**: Currently in hold state
- **RELEASED**: Clean release state before returning to idle

## API Reference

### Initialization

```cpp
void hal_ttp223_init();
```
Initializes the touch sensor and configures GPIO pin. Call once in `setup()`.

### Update Loop

```cpp
void hal_ttp223_update();
```
**MUST** be called every loop iteration. Updates sensor state machine and detects gestures.

### Event Queries

```cpp
bool hal_ttp223_has_event(TouchSensor_t sensor);
TouchEventData hal_ttp223_get_event(TouchSensor_t sensor);
```
Check for and retrieve sensor events. Events are consumed when retrieved.

**TouchSensor_t values:**
- `TOUCH_SENSOR_A`

### State Queries

```cpp
bool hal_ttp223_is_pressed(TouchSensor_t sensor);
unsigned long hal_ttp223_get_press_duration(TouchSensor_t sensor);
```
Query current sensor state and press duration in real-time.

### Debug Functions

```cpp
void hal_ttp223_print_event(const TouchEventData& event);
```
Print human-readable event information to Serial. Automatically called when events are triggered.

## Data Structures

### TouchEventData

```cpp
struct TouchEventData {
  TouchSensor_t sensor;      // Which sensor triggered
  TouchEvent_t event;        // Event type
  unsigned long timestamp;   // When event occurred
  unsigned long duration;    // Duration (for hold/press events)
};
```

## Integration Example

### Basic Usage

```cpp
void setup() {
  Serial.begin(115200);
  hal_ttp223_init();  // Initialize touch sensor
}

void loop() {
  hal_ttp223_update();  // MUST call every loop
  
  // Check sensor A events
  if (hal_ttp223_has_event(TOUCH_SENSOR_A)) {
    TouchEventData event = hal_ttp223_get_event(TOUCH_SENSOR_A);
    
    if (event.event == TOUCH_EVENT_SINGLE_TAP) {
      Serial.println("A tapped - cycle mode");
    }
  }
}
```

### Mode-Based Actions

The firmware uses `touch_actions.cpp` to map gestures to mode-specific actions:

| Gesture | TIME_DATE | CHAT | THEME_PREVIEW | WIFI_INFO |
|---------|-----------|------|---------------|-----------|
| Single Tap | - | Mute/Unmute | Apply Theme | - |
| Double Tap | Cycle Mode | Cycle Mode | Cycle Mode | Cycle Mode |

## Serial Debug Output

When events occur, automatic debug output is generated:

```
[TTP223] Touch sensor initialized
[TTP223] Sensor A pin: 34

[TTP223] Event: Sensor A - SINGLE_TAP
[TTP223] Event: Sensor A - DOUBLE_TAP
[TTP223] Event: Sensor A - LONG_PRESS (850ms)
[TTP223] Event: Sensor A - HOLD_START
[TTP223] Event: Sensor A - HOLD_RELEASE (2340ms)
```

## Performance Characteristics

- **Non-blocking**: All operations use `millis()` for timing
- **Memory**: ~100 bytes static allocation (no heap usage)
- **CPU**: Minimal overhead, suitable for 80MHz ESP8266
- **Latency**: 
  - Tap detection: 50-350ms
  - Long press: 850ms

## Troubleshooting

### False Triggers
- **Cause**: Electrical noise, poor grounding
- **Solution**: Add 100nF capacitor between sensor output and ground

### Missed Taps
- **Cause**: Debounce time too high, user tapping too fast
- **Solution**: Reduce `DEBOUNCE_TIME` to 30ms (minimum recommended: 20ms)

### No Response
- **Verify**: Sensor power (3.3V), correct pin assignment
- **Check**: Serial output for initialization messages
- **Test**: Use `hal_ttp223_is_pressed()` to verify raw sensor readings

## Future Enhancements

Potential additions to the system:

1. **Triple Tap**: Detect three quick taps
2. **Adjustable Timing**: Runtime configuration of timing thresholds
3. **Touch Sensitivity**: GPIO analog read for pressure detection
4. **Haptic Feedback**: Vibration motor response to touch

## References

- TTP223 Datasheet: [Tontek TTP223-BA6 Datasheet](https://www.tontek.com.tw/uploads/product/106/TTP223-BA6.pdf)
- ESP32 GPIO Reference: [ESP32 Pin Reference](https://randomnerdtutorials.com/esp32-pinout-reference-gpios/)
- ESP8266 GPIO Reference: [ESP8266 Pin Reference](https://randomnerdtutorials.com/esp8266-pinout-reference-gpios/)