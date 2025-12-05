# Quil Project Memory

> This file serves as the persistent context for working on the Quil project.

## Project Overview

**Quil** is an interactive, voice-activated desktop companion robot built on the ESP32 platform. It uses OpenAI Realtime API for **untethered, real-time voice-to-voice AI** interactions over WiFi, drives expressive animations on a 128x64 OLED display, and accepts touch input via a TTP223 capacitive sensor.

The project consists of **two main components**:

1.  **Firmware** (`/firmware/src`): The embedded C++ code running on the ESP32.
2.  **Server** (`/server`): A Deno WebSocket backend that bridges ESP32 to OpenAI Realtime API.
3.  **App** (`/app`): A React Native (Expo) mobile app for configuration via Bluetooth.

---

## Architecture (Updated)

```
┌─────────────────────────────────────────────────────────────────────────────────┐
│                              USER                                               │
│                                │                                                 │
│         ┌──────────────────────┼──────────────────────────────┐                 │
│         │                      │                              │                 │
│         ▼                      ▼                              ▼                 │
│   ┌──────────┐          ┌───────────┐                  ┌─────────────┐          │
│   │   APP    │   BLE    │ FIRMWARE  │     WebSocket    │   SERVER    │          │
│   │  (Expo)  │ ◀─────▶ │  (ESP32)  │ ◀────────────▶ │   (Deno)    │          │
│   └──────────┘          └───────────┘                  └──────┬──────┘          │
│         │                      │                              │                 │
│  Config Only            WiFi + Voice                   OpenAI Realtime          │
│                               │                               │                 │
│                               ▼                               ▼                 │
│                      ┌───────────────┐                ┌───────────────┐         │
│                      │ WeatherAPI    │                │    OpenAI     │         │
│                      │ NTP Server    │                │   Realtime    │         │
│                      │ GitHub Releases│               │    API        │         │
│                      └───────────────┘                └───────────────┘         │
└─────────────────────────────────────────────────────────────────────────────────┘
```

### Voice Flow (NEW - Untethered)

1. **ESP32** captures mic audio via I2S (24kHz PCM16)
2. **ESP32** streams raw audio over **WebSocket** to Deno Server
3. **Server** forwards audio to OpenAI Realtime API
4. **OpenAI** returns response audio
5. **Server** sends audio chunks back to ESP32
6. **ESP32** plays audio via I2S speaker

**No USB/Serial bridge required!** Device works standalone with WiFi.

---

## 1. Firmware (ESP32)

**Location:** `/firmware/src`
**Build System:** PlatformIO (`/platformio.ini`)
**Board:** `esp32dev` (ESP32 DevKit)
**Framework:** Arduino

### Key Hardware

*   **Display:** SSD1306 OLED (128x64) via I2C
*   **Touch Sensor:** TTP223 (x1)
*   **Microphone:** INMP441 (I2S) - 24kHz
*   **Speaker:** MAX98357A (I2S) - 24kHz

### New Realtime Voice Module (`modules/cpp/RealtimeVoice.cpp`)

| Function | Description |
| :--- | :--- |
| `RealtimeVoiceInit()` | Initialize the voice system |
| `RealtimeVoiceConnect(url)` | Connect to Quil server WebSocket |
| `RealtimeVoiceLoop()` | Main loop handler (call in loop()) |
| `RealtimeVoiceStartListening()` | Begin streaming mic to server |
| `RealtimeVoiceStopListening()` | Stop streaming |
| `RealtimeVoiceEndOfSpeech()` | Signal end of speech (manual VAD) |
| `RealtimeVoiceInterrupt()` | Cancel AI response |
| `RealtimeVoiceSetVolume(v)` | Set playback volume (0-100) |

### Updated I2S (`hal/cpp/I2S.cpp`)

*   **Sample Rate:** 24kHz (OpenAI Realtime requirement)
*   **Format:** PCM16 mono
*   Mic: I2S_NUM_0, Speaker: I2S_NUM_1

### Dependencies (`platformio.ini`)

```ini
lib_deps = 
    adafruit/Adafruit SSD1306@^2.5.15
    bblanchon/ArduinoJson@^7.4.2
    esphome/ESP32-audioI2S@^2.3.0
    arduino-libraries/NTPClient@^3.2.1
    links2004/WebSockets@^2.4.1
```

---

## 2. Server (Deno)

**Location:** `/server`
**Runtime:** Deno
**Deployment:** Vercel (https://myquilbot.vercel.app/)

### Structure (PascalCase naming)

```
server/
├── Main.ts                    # Entry point, HTTP/WS handler
├── deno.json                  # Deno configuration
├── vercel.json                # Vercel deployment config
├── .env.example               # Environment template
└── Lib/
    ├── Config.ts              # All configuration constants
    ├── OpenAI/
    │   └── RealtimeClient.ts  # OpenAI Realtime WebSocket client
    └── Audio/
        └── AudioUtils.ts      # Base64 encoding, chunking
└── Api/
    └── Esp32/
        └── Handler.ts         # ESP32 WebSocket handler
```

### Key Files

| File | Description |
| :--- | :--- |
| `Main.ts` | HTTP server with WebSocket upgrade at `/ws` |
| `Lib/Config.ts` | OpenAI API key, persona, voice settings |
| `Lib/OpenAI/RealtimeClient.ts` | WebSocket client for OpenAI Realtime API |
| `Api/Esp32/Handler.ts` | Bridges ESP32 to OpenAI, handles audio |

### Endpoints

| Path | Type | Description |
| :--- | :--- | :--- |
| `/` or `/health` | HTTP | Health check, returns JSON status |
| `/ws` or `/esp32` | WebSocket | ESP32 realtime voice connection |

### Environment Variables

```bash
OPENAI_API_KEY=sk-...    # Required
HOST=0.0.0.0             # Server host
PORT=8000                # Server port
DEV_MODE=true            # Enable debug logging
```

### Development

```bash
cd server
deno task Dev   # Start with watch mode
deno task Start # Production start
```

---

## 3. App (React Native / Expo)

**Location:** `/app`
**Framework:** Expo SDK 54, React Native 0.81
**Purpose:** BLE configuration only (not voice)

### Key Files

*   `/app/app/_layout.tsx` - Root layout, providers
*   `/app/app/index.tsx` - Main home screen
*   `/app/lib/ble-manager.ts` - Web Bluetooth wrapper
*   `/app/hooks/use-ble.ts` - BLE React hook
*   `/app/contexts/settings.tsx` - Settings persistence

---

## BLE Protocol

| UUID | Type | Purpose |
| :--- | :--- | :--- |
| `4fafc201-1fb5-459e-8fcc-c5c9c331914b` | Service | Quil Service |
| `beb5483e-36e1-4688-b7f5-ea07361b26a8` | Characteristic | Config (Write) |
| `beb5483e-36e1-4688-b7f5-ea07361b26a9` | Characteristic | Status (Read/Notify) |

---

## WebSocket Protocol (ESP32 ↔ Server)

### ESP32 → Server

| Type | Format | Description |
| :--- | :--- | :--- |
| Binary | Raw PCM16 | Microphone audio data |
| Text | `{"Type":"instruction","Msg":"end_of_speech"}` | Manual VAD signal |
| Text | `{"Type":"instruction","Msg":"INTERRUPT"}` | Cancel AI response |
| Text | `{"Type":"instruction","Msg":"ping"}` | Keep-alive |

### Server → ESP32

| Type | Format | Description |
| :--- | :--- | :--- |
| Binary | Raw PCM16 chunks | AI response audio (1024 byte chunks) |
| Text | `{"Type":"auth","Status":"connected"}` | Auth confirmation |
| Text | `{"Type":"server","Msg":"RESPONSE.COMPLETE"}` | AI finished speaking |
| Text | `{"Type":"server","Msg":"AUDIO.COMMITTED"}` | User audio processed |
| Text | `{"Type":"pong"}` | Ping response |
| Text | `{"Type":"error","Message":"..."}` | Error notification |

---

## Development Commands

### Firmware

```bash
pio run -t upload     # Build and upload
pio device monitor    # Monitor serial
```

### Server (Deno)

```bash
cd server
deno task Dev         # Development with watch
deno task Deploy      # Deploy to Vercel
```

### App (Expo)

```bash
cd app
bun install
bun start
```

---

## Configuration

### ESP32 Server URL

Set in `ConfigStore`:
- Default: `wss://myquilbot.vercel.app/ws`
- Local dev: `ws://192.168.x.x:8000/ws`

### Quil Persona (Server)

Defined in `/server/Lib/Config.ts`:
- Warm, friendly voice companion
- Concise responses for voice
- Voice: "alloy"

---

*Last updated: 2025-12-06*
