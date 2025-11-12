# Quil Firmware - Quick Start

## 🚀 Build & Upload

```bash
# Install PlatformIO (if needed)
pip install platformio

# Build for ESP32
pio run -e esp32

# Upload to connected ESP32
pio run -e esp32 -t upload

# Monitor serial output
pio device monitor -b 115200
```

## 📡 WiFi Setup

### First Boot
1. Device starts AP: **QUIL_SETUP** (password: quil1234)
2. Connect phone/laptop to QUIL_SETUP
3. Open browser to http://192.168.4.1
4. Enter your WiFi credentials
5. Device saves and reboots, connects to your network

### Reset WiFi
Reflash or clear NVS via serial commands (future feature)

## 🎮 Mode Switching

**Double-tap** the MPR121 touch sensor to cycle modes:
1. TIME_DATE - Shows current time (NTP synced)
2. MUSIC - Music playback control
3. CHAT - Voice interaction
4. THEME_PREVIEW - Theme selector (Aurora/NeonPulse/MonoMist)
5. WIFI_INFO - Network status (IP/RSSI)

## 🔧 OTA Updates

Once connected to WiFi:
```bash
# Via PlatformIO
pio run -e esp32 -t upload --upload-port quil.local

# Via Arduino IDE
Tools → Port → quil (Network Port)
```

## 📝 Code Structure

```
firmware/
├── include/       # Global config (pins, constants, types)
├── src/
│   ├── hal/       # Hardware drivers
│   ├── core/      # State machine, diagnostics
│   ├── modules/   # WiFi, config, touch, OTA, HTTP
│   ├── modes/     # 5 display modes
│   └── main.cpp   # Entry point
```

## 🎯 Adding a New Mode

1. Create `modes/mode_yourname.{h,cpp}`
2. Add enum to `types.h`: `MODE_YOURNAME`
3. Add init/update/render functions
4. Include in `main.cpp` and add to switch statement
5. Update mode count in `state_cycle_mode()`

## 🔌 Pin Reference (ESP32)

| Peripheral | Pins | Notes |
|------------|------|-------|
| Display/Touch | 21 (SDA), 22 (SCL) | I2C shared bus |
| Microphone | 25 (WS), 26 (BCLK), 27 (DOUT) | I2S input |
| Speaker | 32 (WS), 33 (BCLK), 19 (DIN) | I2S output |
| LED | 4 | Optional RGB |

## 📊 Diagnostics

Heap and uptime logged every 30 seconds:
```
Heap: 245312 | Uptime: 125
```

## 🐛 Debugging

```bash
# Full serial monitor
pio device monitor -b 115200

# Filter for specific module
pio device monitor -b 115200 --filter log2file

# Check I2C devices
# Add I2C scanner sketch from tests/
```

## 📦 Dependencies (Auto-installed)

- Adafruit SSD1306
- Adafruit GFX
- Adafruit BusIO
- ArduinoJson (for future API)

## 🔒 Security Notes

- WiFi passwords stored in NVS (encrypted by ESP32)
- AP mode password: **quil1234** (change in `config.h`)
- OTA has no auth (add in production)

## 📚 Key Files to Edit

- `config.h` - Change WiFi AP name, timeouts, intervals
- `pins.h` - Remap hardware pins
- `main.cpp` - Modify boot sequence or loop logic
- `modes/*.cpp` - Customize mode behavior

## ⚡ Performance Tips

- Functions ≤ 20 lines enforced
- No blocking delays (use millis() timers)
- Static buffers > heap allocation
- Display updates only when changed

## 🆘 Common Issues

**Display not working?**
- Check I2C address (default 0x3C)
- Verify SDA/SCL pins in `pins.h`

**WiFi won't connect?**
- Check SSID/password via web portal
- Verify 2.4GHz network (5GHz not supported)

**Touch not responding?**
- Verify MPR121 address (0x5A)
- Check I2C connections
- Test with `touch_read_raw()` debug print

**OTA not appearing?**
- Ensure device on same network
- Check mDNS: `ping quil.local`
- Firewall may block Arduino port 3232

## 📞 Next Steps

1. Upload firmware
2. Complete WiFi setup
3. Test mode switching
4. Add OpenAI API keys (future)
5. Customize themes and expressions
