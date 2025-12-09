# Quil Mobile App

React Native (Expo) app for configuring the Quil robot via WiFi.

## Purpose

This app connects to Quil over WiFi to:

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

## Structure

```
app/
├── app/               # Screens (file-based routing)
│   ├── _layout.tsx    # Root layout, providers
│   ├── index.tsx      # Main home screen
│   ├── devices.tsx    # Device management
│   └── settings.tsx   # App settings
├── components/        # Reusable UI components
├── contexts/          # React contexts (Settings)
├── hooks/             # Custom hooks (useDevice)
└── lib/               # Utilities (device-manager)
```

## Setup

```bash
# Install dependencies
bun install

# Start Expo
bun start
```

## WiFi Protocol

The app communicates with Quil over HTTP:

- `GET /api/status` - Get device status
- `POST /api/config` - Send configuration

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
