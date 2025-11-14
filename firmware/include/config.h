#pragma once

// Display configuration
#define DISPLAY_WIDTH 128
#define DISPLAY_HEIGHT 64
#define DISPLAY_ADDR 0x3C

// I2C pins (ESP32 default)
#ifdef ESP32
#define I2C_SDA 21
#define I2C_SCL 22
#elif defined(ESP8266)
#define I2C_SDA 4  // D2
#define I2C_SCL 5  // D1
#endif

// MPR121 Touch sensor
#define MPR121_ADDR 0x5A

// Diagnostics
#define HEAP_CHECK_MS 5000

// Config storage
#define CONFIG_NAMESPACE "quil"
#define CONFIG_KEY_SSID "wifi_ssid"
#define CONFIG_KEY_PASS "wifi_pass"
#define CONFIG_KEY_THEME "theme"
#define CONFIG_KEY_WEATHER_API_KEY "weather_api_key"
#define CONFIG_KEY_WEATHER_LOCATION "weather_location"

// NTP configuration
#define NTP_SERVER "pool.ntp.org"
#define NTP_OFFSET_SEC 0
#define NTP_UPDATE_MS 3600000  // Update every hour

// Audio I2S pins (ESP32 only)
#ifdef ESP32
#define I2S_MIC_BCK 26
#define I2S_MIC_WS 25
#define I2S_MIC_DIN 33

#define I2S_SPK_BCK 14
#define I2S_SPK_WS 15
#define I2S_SPK_DOUT 32
#endif
