# Quil Firmware

ESP32 firmware for the Quil AI Companion Robot. Handles voice capture, WebSocket
streaming to server, and audio playback.

## Hardware

| Component  | Model               | Purpose             |
| ---------- | ------------------- | ------------------- |
| MCU        | ESP32 DevKit        | Main controller     |
| Display    | SSD1306 OLED 128x64 | Animations & status |
| Microphone | INMP441             | Voice input (I2S)   |
| Speaker    | MAX98357A           | Audio output (I2S)  |
| Touch      | TTP223              | Touch input         |

## Structure

```
firmware/src/
├── Main.cpp           # Entry point, boot sequence
├── core/              # State machine, diagnostics
├── hal/               # Hardware abstraction (I2S, Display, Touch)
├── modes/             # UI screens (Time, Chat, Setup)
├── modules/           # Functional modules
│   ├── RealtimeVoice  # WebSocket voice streaming (NEW)
│   ├── WifiManager    # WiFi connectivity
│   ├── BleServer      # BLE configuration
│   └── ...
└── themes/            # Display themes
```

## Key Module: RealtimeVoice

Streams audio to the Quil server over WebSocket for AI processing.

```cpp
#include "modules/h/RealtimeVoice.h"

// Initialize
RealtimeVoiceInit();

// Connect to server
RealtimeVoiceConnect("wss://myquilbot-5p9j92amhsxa.deno.dev/ws");

// In loop()
RealtimeVoiceLoop();

// Start/stop listening
RealtimeVoiceStartListening();
RealtimeVoiceStopListening();
```

## Build & Upload

```bash
# Build and upload
pio run -t upload

# Monitor serial output
pio device monitor
```

## Configuration

Audio settings in `hal/h/I2S.h`:

- Sample rate: 24kHz (OpenAI requirement)
- Format: PCM16 mono

Server URL configured via BLE app or hardcoded in firmware.

## Dependencies

See `platformio.ini`:

- Adafruit SSD1306
- ArduinoJson
- WebSockets (links2004)
- ESP32-audioI2S
