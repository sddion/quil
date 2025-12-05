# Quil Mobile App

React Native (Expo) app for configuring the Quil robot via Bluetooth Low Energy
(BLE).

## Purpose

This app connects to Quil over BLE to:

- Configure WiFi credentials
- Set timezone and location
- Adjust display brightness and themes
- Trigger OTA updates
- View device status

**Note:** Voice interaction happens directly between ESP32 and server over WiFi.
This app is for configuration only.

## Tech Stack

- **Framework:** Expo SDK 54, React Native 0.81
- **Routing:** expo-router (file-based)
- **State:** @tanstack/react-query, React Context
- **Storage:** @react-native-async-storage
- **BLE:** Web Bluetooth API

## Structure

```
app/
├── app/               # Screens (file-based routing)
│   ├── _layout.tsx    # Root layout, providers
│   ├── index.tsx      # Main home screen
│   ├── devices.tsx    # Device management
│   └── settings.tsx   # App settings
├── components/        # Reusable UI components
├── contexts/          # React contexts (BLE, Settings)
├── hooks/             # Custom hooks (useBLE)
└── lib/               # Utilities (ble-manager)
```

## Setup

```bash
# Install dependencies
bun install

# Start Expo
bun start
```

## BLE Protocol

| UUID              | Purpose              |
| ----------------- | -------------------- |
| `4fafc201-...`    | Quil Service         |
| `beb5483e-...-a8` | Config (Write)       |
| `beb5483e-...-a9` | Status (Read/Notify) |

### Config JSON (App → Device)

```json
{
    "ssid": "WiFiName",
    "password": "secret",
    "tz": 19800,
    "brightness": 128,
    "theme": 0
}
```

### Commands

```json
{"cmd": "restart"}
{"cmd": "sync_time"}
{"cmd": "reset"}
```
