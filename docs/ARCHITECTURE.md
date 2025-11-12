# Quil Firmware Architecture

## Structure

```
firmware/
├── include/              # Global headers
│   ├── pins.h           # Pin assignments (ESP32/ESP8266)
│   ├── config.h         # System constants
│   └── types.h          # Core enums
├── src/
│   ├── hal/             # Hardware Abstraction Layer
│   │   ├── hal_i2c.*    # I2C bus wrapper
│   │   ├── hal_display.* # SSD1306 OLED driver
│   │   ├── hal_mpr121.* # Touch sensor driver
│   │   └── hal_i2s.*    # I2S audio (ESP32 only)
│   ├── core/            # Core system
│   │   ├── state_machine.* # State/mode management
│   │   └── diagnostics.* # Heap/uptime monitoring
│   ├── modules/         # Application modules
│   │   ├── config_store.* # NVS preferences
│   │   ├── wifi_manager.* # WiFi STA/AP
│   │   └── ntp_client.*   # Time sync
│   ├── modes/           # Display modes
│   │   ├── mode_time.*     # TIME_DATE mode
│   │   └── mode_wifi_info.* # WIFI_INFO mode
│   └── main.cpp         # Entry point
└── lib/                 # Third-party libs (empty)
```

## HAL Layer

All hardware access goes through HAL functions:
- `hal_i2c_*`: Low-level I2C read/write
- `hal_display_*`: OLED clear/text/bitmap/update
- `hal_mpr121_*`: Touch sensor init/read
- `hal_i2s_*`: Audio I/O (ESP32 only)

## Core Layer

- `state_machine`: Manages DisplayMode_t and RobotState_t
- `diagnostics`: Non-blocking heap/uptime logging

## Modules

- `config_store`: Persistent WiFi credentials via NVS
- `wifi_manager`: Connect STA or start AP
- `ntp_client`: Non-blocking time sync

## Modes

Each mode has init/update/render:
- `mode_time`: Display current time from NTP
- `mode_wifi_info`: Show IP and RSSI

## Coding Rules

- Functions < 20 lines
- Files < 200 lines
- No hardcoded pins (use pins.h)
- No blocking delays (use millis() timers)
- Platform-specific code in `#ifdef ESP32` blocks
