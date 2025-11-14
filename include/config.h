#pragma once

#define DISPLAY_WIDTH 128
#define DISPLAY_HEIGHT 64
#define DISPLAY_ADDR 0x3C
#define MPR121_ADDR 0x5A
#define I2C_FREQ 400000

#define WIFI_AP_SSID "QUIL_SETUP"
#define WIFI_AP_PASS "quil1234"
#define WIFI_TIMEOUT_MS 10000
#define PORTAL_PORT 80

#define NTP_SERVER "pool.ntp.org"
#define NTP_OFFSET_SEC 19800  // IST (India Standard Time) UTC+5:30 = 5*3600 + 30*60 = 19800 seconds
#define NTP_DAYLIGHT_OFFSET_SEC 0  // No daylight saving in India
#define NTP_UPDATE_MS 3600000

#define HEAP_CHECK_MS 30000
#define STATE_UPDATE_MS 50

#define CONFIG_NAMESPACE "quil"
#define CONFIG_KEY_SSID "ssid"
#define CONFIG_KEY_PASS "pass"
#define CONFIG_KEY_TZ "tz"
#define CONFIG_KEY_LANG "lang"
