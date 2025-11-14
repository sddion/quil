# Secrets Configuration Guide

This document explains how to configure your Quil device using the `secrets.env` file.

## Overview

The `secrets.env` file stores sensitive configuration data like WiFi credentials and API keys. These values are injected at compile-time as build flags, so your credentials are baked into the firmware but never committed to version control.

## File Location

```
quil/firmware/secrets.env
```

## Configuration Format

The file uses simple `KEY="value"` format:

```env
WIFI_SSID="YourWiFiNetwork"
WIFI_PASSWORD="YourWiFiPassword"
WEATHER_API_KEY="your_api_key_here"
WEATHER_LOCATION="YourCity"
```

## Available Configuration Options

### 1. WiFi Credentials

**WIFI_SSID** - Your WiFi network name
```env
WIFI_SSID="MyHomeWiFi"
```

**WIFI_PASSWORD** - Your WiFi password
```env
WIFI_PASSWORD="MySecurePassword123"
```

**How it works:**
- If the device has no saved WiFi credentials in EEPROM/Preferences, it will use these defaults
- On first boot, it attempts to connect using these credentials
- If successful, credentials are saved to EEPROM for future use
- If empty or connection fails, device starts in AP mode (`QUIL_SETUP` / `quil1234`)

### 2. Weather Configuration

**WEATHER_API_KEY** - Your Weather API key from weatherapi.com
```env
WEATHER_API_KEY="90ee9fa05b7644deb7a234704251311"
```

**WEATHER_LOCATION** - City name for weather data
```env
WEATHER_LOCATION="Guwahati"
```

**How to get a Weather API key:**
1. Visit https://www.weatherapi.com/
2. Sign up for a free account
3. Copy your API key from the dashboard
4. Paste it into `secrets.env`

**Supported location formats:**
- City name: `"London"`
- City, Country: `"New York, USA"`
- Coordinates: `"48.8567,2.3508"` (latitude,longitude)
- US Zipcode: `"10001"`
- UK Postcode: `"SW1"`

## Example Configuration

### Minimal Setup (WiFi Only)
```env
WIFI_SSID="MyNetwork"
WIFI_PASSWORD="MyPassword"
WEATHER_API_KEY=""
WEATHER_LOCATION="Delhi"
```

### Full Setup (WiFi + Weather)
```env
WIFI_SSID="HomeWiFi"
WIFI_PASSWORD="SecurePass123"
WEATHER_API_KEY="90ee9fa05b7644deb7a234704251311"
WEATHER_LOCATION="Guwahati"
```

### Empty Setup (Manual Configuration)
```env
WIFI_SSID=""
WIFI_PASSWORD=""
WEATHER_API_KEY=""
WEATHER_LOCATION="Delhi"
```
_Device will start in AP mode for web-based configuration_

## Build Process

When you run `pio run -e esp8266` or `pio run -e esp32`, the build system:

1. Reads `firmware/secrets.env`
2. Parses the key-value pairs
3. Injects them as C preprocessor defines:
   - `DEFAULT_WIFI_SSID`
   - `DEFAULT_WIFI_PASSWORD`
   - `DEFAULT_WEATHER_API_KEY`
   - `DEFAULT_WEATHER_LOCATION`
4. These are available in your C++ code at compile-time

### Build Output Example
```
Loading secrets from: /home/user/quil/firmware/secrets.env
Loaded secrets: WIFI_SSID=Hom***, WEATHER_LOCATION=Guwahati
Injected 4 build flags from secrets.env
```

## Security Notes

### ✅ Good Practices

1. **Never commit secrets.env to version control**
   - Already in `.gitignore`
   - Contains sensitive passwords and API keys

2. **Use different credentials per device**
   - Each device can have its own `secrets.env`
   - Rebuild firmware for each configuration

3. **Keep API keys private**
   - Weather API keys have usage limits
   - Don't share keys publicly

### ⚠️ Important Warnings

- Secrets are **compiled into the firmware binary**
- Anyone with access to the `.bin` file could potentially extract them
- For production devices, consider using runtime configuration instead
- This system is designed for personal/hobby projects, not commercial products

## Runtime vs Compile-Time Configuration

### Compile-Time (secrets.env) - Current System
**Pros:**
- Simple to use
- Works immediately after flashing
- No setup required on device

**Cons:**
- Need to recompile to change credentials
- Secrets in binary file
- Not suitable for multiple devices with different configs

### Runtime (Web Portal/EEPROM) - Also Supported
**How to use:**
1. Leave `secrets.env` empty (or omit WiFi credentials)
2. Device boots in AP mode: `QUIL_SETUP` / `quil1234`
3. Connect to AP and visit `http://192.168.4.1`
4. Configure WiFi via web portal
5. Credentials saved to EEPROM/Preferences
6. Device reboots and connects automatically

**Pros:**
- No recompilation needed
- Change credentials anytime via web
- Same firmware for all devices

**Cons:**
- Manual setup required
- Need to access web portal

## Troubleshooting

### Device starts in AP mode even with credentials in secrets.env
**Solution:** Check that:
- WiFi SSID and password are correct (case-sensitive)
- WiFi network is within range
- WiFi uses 2.4GHz (ESP8266/ESP32 don't support 5GHz)
- No special characters causing parsing issues

### Weather not showing
**Solution:** Check that:
- Weather API key is valid
- Location name is recognized by weatherapi.com
- Device has internet connectivity (not just WiFi)
- API key hasn't exceeded free tier limits

### Build fails with "secrets not found"
**Solution:**
- File must be at `firmware/secrets.env` (not `secrets.env` in root)
- File must be named exactly `secrets.env` (lowercase, no spaces)
- If missing, create it with at least empty values

### Secrets not being loaded
**Solution:**
```bash
# Clean build cache
pio run -e esp8266 -t clean

# Rebuild
pio run -e esp8266
```

## Testing Your Configuration

### 1. Verify secrets are loaded at build time:
```bash
pio run -e esp8266 2>&1 | grep "Loading secrets"
```
Expected output:
```
Loading secrets from: /path/to/quil/firmware/secrets.env
Loaded secrets: WIFI_SSID=Hom***, WEATHER_LOCATION=Guwahati
Injected 4 build flags from secrets.env
```

### 2. Check serial monitor after flashing:
```bash
pio device monitor -e esp8266
```
Expected output:
```
[Setup] Using default WiFi credentials from secrets.env
[Setup] SSID: YourWiFiNetwork
[WiFi] Connecting to: YourWiFiNetwork
[WiFi] Connected! IP: 192.168.1.100
```

### 3. Verify weather data:
- Wait 15 minutes for first weather update
- Check display shows temperature (e.g., "25C")
- If not showing, check API key and location

## Default Values

If `secrets.env` is missing or values are empty:

```cpp
DEFAULT_WIFI_SSID=""              // Empty = AP mode
DEFAULT_WIFI_PASSWORD=""          // Empty = AP mode
DEFAULT_WEATHER_API_KEY=""        // Empty = no weather
DEFAULT_WEATHER_LOCATION="Delhi"  // Default city
```

## Advanced: Multiple Configurations

### For multiple devices with different settings:

1. **Create separate env files:**
   ```
   secrets.bedroom.env
   secrets.office.env
   secrets.living-room.env
   ```

2. **Before building, copy the desired config:**
   ```bash
   cp firmware/secrets.bedroom.env firmware/secrets.env
   pio run -e esp8266 -t upload
   ```

3. **Or create a build script:**
   ```bash
   #!/bin/bash
   for device in bedroom office living-room; do
     cp firmware/secrets.$device.env firmware/secrets.env
     pio run -e esp8266
     mv .pio/build/esp8266/firmware.bin ./firmware-$device.bin
   done
   ```

## Summary

✅ **secrets.env** = Compile-time defaults  
✅ **EEPROM/Preferences** = Runtime storage (overrides secrets.env)  
✅ **AP Mode** = Fallback when no credentials available  
✅ **Web Portal** = Configure credentials at runtime  

**Recommended workflow:**
1. Set up `secrets.env` with your credentials
2. Build and flash firmware
3. Device connects automatically
4. Credentials saved to EEPROM
5. Future builds can use empty `secrets.env` (device uses saved EEPROM)

---

**Need help?** Check the main README.md or open an issue on GitHub.