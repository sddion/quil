# WARP.md

This file provides guidance to WARP (warp.dev) when working with code in this repository.

---

## Project Overview

**Quil** is an interactive, voice‑activated desktop companion robot built on the ESP platform (ESP8266 for prototyping, ESP32 for production). It uses OpenAI APIs for voice interactions, drives expressive animations on a 128×64 OLED, and accepts touch input via an MPR121 capacitive sensor.

This document focuses on a Warp-friendly developer workflow: fast terminal actions (build, upload, monitor), reusable command blocks, and concise environment guidance so contributors can be productive using Warp as their primary terminal.

---

## Key Paths

```
firmware/                # ESP firmware (Arduino framework)
assets/                  # bitmaps, audio, and asset sources
kicad/                   # schematics & PCB
tests/                   # small Arduino test sketches
tools/                   # development utilities
```

---

## Environment variables (local dev)

Keep secrets out of git. Use Warp sessions or `.env.local` when developing locally.

* `OPENAI_API_KEY` — OpenAI API key for dev voice/TTS tests
* `SUPABASE_URL` / `SUPABASE_ANON_KEY` — optional cloud sync for preferences/themes
* `QUIL_DEVICE_ID` — device identifier when testing multi-device flows

Add these to your local environment (Warp session variables or `.env`) and add `.env*` to `.gitignore`.

---

## Quick Commands (copy into Warp Blocks)

### Arduino-CLI (if using Arduino CLI)

```bash
# compile for ESP32 (replace FQBN as needed)
arduino-cli compile --fqbn esp32:esp32:esp32 firmware

# upload (mac/linux example)
arduino-cli upload -p /dev/ttyUSB0 --fqbn esp32:esp32:esp32 firmware

# monitor serial
arduino-cli monitor -p /dev/ttyUSB0 -b 115200
```

### PlatformIO (recommended if you prefer unified builds)

```bash
# build
pio run

# upload for ESP32 environment
pio run -e esp32 -t upload

# serial monitor
pio device monitor -b 115200
```

### esptool.py (low-level flashing / erase)

```bash
esptool.py --port /dev/ttyUSB0 erase_flash
esptool.py --port /dev/ttyUSB0 write_flash -fm dio 0x1000 firmware.bin
```

### Serve Web Config Portal (local testing)

```bash
# simple static server (from project root)
cd assets/web-portal || ./public
npx serve . --port 8000
# or
python3 -m http.server 8000
```

---

## Recommended Warp Workflows / Blocks

* **Build ESP32** — `pio run -e esp32` or `arduino-cli compile` block
* **Upload ESP32** — `pio run -e esp32 -t upload` / `arduino-cli upload` block
* **Serial Monitor** — `pio device monitor` block
* **Provisioning Portal** — start/stop local static server block
* **Run Tests** — small blocks that flash `tests/` sketches for component checks
* **Format & Lint** — run `clang-format` / `prettier` blocks

Create Blocks for each frequently used action and compose them into a Workflow that runs build → upload → monitor.

---

## Build & Development Notes

* Project targets both ESP8266 and ESP32. Use `#ifdef ESP8266` / `#elif defined(ESP32)` where platform-specific behavior is required (I2S audio, pin mappings).
* Keep functions short and modules isolated (project rule: functions < 20 lines where practical).
* For development, PlatformIO offers a simpler multi-environment workflow. Arduino CLI is useful for minimal installs or CI containers.
* Serial logs should be enabled behind a `LOG_LEVEL` toggle so Warp sessions can capture concise logs during demos.

---

## REST Endpoints (Firmware) — expected for the web portal / app

The firmware should expose a small set of HTTP endpoints used by the Web Config Portal and the mobile app:

* `GET /status` — returns `{ wifi, signal, mode, theme, battery }`
* `GET /scan` — returns list of nearby SSIDs
* `POST /provision` — body `{ ssid, password, lang, timezone, theme }`
* `POST /applyTheme` — body `{ id }`
* `POST /restart` — friendly restart endpoint
* `POST /factoryReset` — clears saved settings
* `POST /ota` — optional: initiate OTA update

Use these endpoints in the web portal served from `assets/` during first‑boot provisioning.

---

## Code Style & Development Guidelines (enforced)

* **Function length**: aim for < 20 lines. Keep implementation small and readable.
* **File length**: prefer ≤ 300 lines per file. Break modules into `.h` + `.cpp` pairs.
* **State manager**: all mode transitions must flow through a central `StateManager` module.
* **Hardware abstraction**: inject hardware interfaces for testability; avoid direct hardware calls from high-level services.

---

## Display & Touch

* 128×64 SSD1306 OLED via I2C with pre-rendered bitmaps in PROGMEM.
* MPR121 capacitive touch: gestures implemented on the firmware side and mapped to simple enums consumed by the `StateManager`.

Active gesture mapping (firmware):

| Gesture               | Mode          | Action                                                       |
| --------------------- | ------------- | ------------------------------------------------------------ |
| Single Tap            | MUSIC         | Play / Pause                                                 |
| Single Tap            | THEME_PREVIEW | Apply Selected Theme                                         |
| Single Tap            | CHAT          | Mute / Unmute Mic                                            |
| Swipe Left / Right    | MUSIC         | Previous / Next Song                                         |
| Swipe Left / Right    | THEME_PREVIEW | Previous / Next Theme                                        |
| Double Tap (any mode) | —             | Switch Mode (cycle TIME_DATE → MUSIC → CHAT → THEME_PREVIEW) |

---

## Important Types

`firmware/src/core/QuilTypes.h` defines central enums:
* `RobotState_t` - Robot states (BOOTING, IDLE, LISTENING, THINKING, SPEAKING, PLAYING_MUSIC, ERROR)
* `Expression_t` - Display expressions (NORMAL, HAPPY, SAD, ANGRY, SLEEPY, WINK, SURPRISED, BLINK, THINKING, LOGO)
* `DisplayMode_t` - Display modes (TIME_DATE, MUSIC, CHAT, THEME_PREVIEW)

---

## Testing & Debugging Tips

* Use small `tests/` sketches to validate display, touch, and audio subsystems. Create Warp Blocks that flash the appropriate test sketch and open the serial monitor automatically.
* Use `pio device monitor` with `--raw` when binary streams (audio debug) are present.
* During provisioning tests, run the Web Portal locally and connect a phone/computer to the device AP (QUIL_SETUP_xx).

---

## Security

* Never store plaintext Wi‑Fi credentials in source control. Encrypt before writing to NVS or only save when user confirms.
* Use short‑lived session tokens for local HTTP provisioning flows. Consider a simple acceptance token generated at softAP start and validated by the portal.

---

## Known Issues / TODOs

1. ✅ `DisplayMode_t` enum added to `QuilTypes.h`
2. ✅ `DisplayModule` implemented (basic functionality)
3. ✅ `StateManager` implemented (basic state machine)
4. ✅ Main firmware entry point created at `firmware/src/main.cpp`
5. ✅ PlatformIO configuration added for easy builds
6. `VoiceModule` needs full OpenAI integration (Whisper, Chat, TTS) and streaming logic.
7. `TouchModule` requires robust MPR121 gesture recognition and debouncing.
8. `DisplayModule` needs Irisoled bitmap integration for expressions.
9. Network/WiFi module not yet implemented.
10. Audio module (I2S) not yet implemented.
11. No automated test framework; tests are manual via Arduino/PlatformIO uploads.

---

## Recommended Warp session tips

* Create a session per device (e.g., `quil-esp32`, `quil-provision`) with appropriate ENV vars set per session.
* Add Blocks for the most common sequences (build → upload → monitor). That speeds development and demos.
* Keep a `WarpBookmarks` list for important files: `firmware/src/modules/`, `firmware/src/core/QuilTypes.h`, `assets/`.

---

## Contacts & Links

* Project owner / maintainer: *(add your contact here)*
* Repo: local repository root

---

*End of WARP.md — designed for use inside Warp as a concise, actionable developer
