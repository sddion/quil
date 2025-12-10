#pragma once

// Display configuration
#define DISPLAY_WIDTH 128
#define DISPLAY_HEIGHT 64
#define DISPLAY_ADDR 0x3C

// I2C configuration
#define I2C_SDA 21
#define I2C_SCL 22
#define I2C_FREQ 400000

// WiFi AP configuration
#define WIFI_AP_SSID "QUIL SETUP"
#define WIFI_AP_PASS "quil1234"
#define WIFI_TIMEOUT_MS 10000
#define PORTAL_PORT 80

// Diagnostics
#define HEAP_CHECK_MS 5000
#define STATE_UPDATE_MS 50

// Config storage
#define CONFIG_NAMESPACE "quil"
#define CONFIG_KEY_SSID "wifi_ssid"
#define CONFIG_KEY_PASS "wifi_pass"
#define CONFIG_KEY_THEME "theme"
#define CONFIG_KEY_WEATHER_API_KEY "weather_api_key"
#define CONFIG_KEY_WEATHER_LOCATION "weather_location"

// NTP configuration
#define NTP_SERVER "pool.ntp.org"
#define NTP_OFFSET_SEC 19800  // IST (India Standard Time) UTC+5:30 = 19800 seconds
#define NTP_DAYLIGHT_OFFSET_SEC 0  // No daylight saving in India
#define NTP_UPDATE_MS 3600000  // Update every hour

// Audio I2S pins
// Audio I2S pins (Verified)
#define I2S_MIC_BCK 32
#define I2S_MIC_WS 25
#define I2S_MIC_DIN 33

#define I2S_SPK_BCK 27
#define I2S_SPK_WS 26
#define I2S_SPK_DOUT 23

// Firmware version
#define FIRMWARE_VERSION "1.1.0"

// Quil Server
#define QUIL_SERVER_URL "wss://mymyquilbot.deno.dev/ws"

