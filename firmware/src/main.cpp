#include <Arduino.h>
#include "pins.h"
#include "config.h"
#include "types.h"
#include "hal/hal_i2c.h"
#include "hal/hal_display.h"
#include "core/state_machine.h"
#include "core/diagnostics.h"
#include "modules/config_store.h"
#include "modules/wifi_manager.h"
#include "modules/ntp_client.h"
#include "modules/ota_manager.h"
#include "modules/http_server.h"
#include "modules/gesture_manager.h"
#include "modules/touch_actions.h"
#include "modules/voice_manager.h"
#include "modules/wake_manager.h"
#include "modules/llm_bridge.h"
#include "modules/animation_manager.h"
#include "modules/battery_manager.h"
#include "hal/hal_mpr121.h"
#include "modes/mode_time.h"
#include "modes/mode_music.h"
#include "modes/mode_chat.h"
#include "modes/mode_theme_preview.h"
#include "modes/mode_wifi_info.h"

void setup() {
  Serial.begin(115200);
  delay(100);
  
  hal_i2c_init();
  if (!hal_display_init()) {
    Serial.println("Display fail");
    while(1);
  }
  
  config_init();
  diag_init();
  state_init();
  gesture_init();
  actions_init();
  
  // Initialize WiFi and load saved credentials
  wifi_init();
  
  // Check if we have saved WiFi credentials
  if (wifi_has_saved_credentials()) {
    Serial.println("[Setup] Found saved WiFi credentials");
    char ssid[33], pass[65];
    config_load_wifi(ssid, pass);
    
    // Attempt connection - even if it fails, stay in STA mode and keep retrying
    if (!wifi_connect(ssid, pass)) {
      Serial.println("[Setup] Initial connection failed, will retry in background");
      // Don't start AP mode - just keep trying to connect in background
    }
  } else if (strlen(DEFAULT_WIFI_SSID) > 0) {
    // No saved credentials but we have defaults from secrets.env
    Serial.println("[Setup] Using default WiFi credentials from secrets.env");
    Serial.print("[Setup] SSID: ");
    Serial.println(DEFAULT_WIFI_SSID);
    
    // Attempt connection with defaults
    if (!wifi_connect(DEFAULT_WIFI_SSID, DEFAULT_WIFI_PASSWORD)) {
      Serial.println("[Setup] Initial connection failed, will retry in background");
    }
  } else {
    // No saved credentials and no defaults - start AP mode for initial setup
    Serial.println("[Setup] No saved credentials - starting AP mode");
    wifi_start_ap();
    http_init();
  }
  
  ota_init();
  voice_init();
  wake_init();
  bridge_init();
  anim_init();
  anim_play(ANIM_BOOT);
  while(anim_is_playing()) {
    anim_update();
    delay(10);
  }
  
  // Clear display after boot animation
  hal_display_clear();
  hal_display_update();
  
  mode_time_init();
  mode_music_init();
  mode_chat_init();
  mode_theme_init();
  mode_wifi_init();
  
  // Force initial render to prevent black screen
  mode_time_force_render();
  
  Serial.println("Quil ready");
}

void loop() {
  // WiFi reconnection task - handles automatic reconnection and internet checks
  wifi_reconnect_task();
  
  state_update();
  diag_update();
  ota_handle();
  http_handle();
  
  uint16_t touch = hal_mpr121_read_touched();
  GestureType gest = gesture_detect(touch, millis());
  if (gest != GESTURE_NONE) {
    actions_handle(gest, state_get_mode());
  }
  
  if (wake_detect()) {
    bridge_send_command("wake_quil");
    voice_start_listening();
  }
  
  if (voice_is_listening()) {
    uint8_t audio[256];
    size_t len = voice_read_buffer(audio, sizeof(audio));
    if (len > 0) {
      bridge_send_audio(audio, len);
    }
  }
  
  bridge_handle_response();
  
  if (anim_is_playing()) {
    anim_update();
    delay(10);
    return;
  }
  
  DisplayMode_t mode = state_get_mode();
  switch(mode) {
    case MODE_TIME_DATE:
      mode_time_update();
      mode_time_render();
      break;
    case MODE_MUSIC:
      mode_music_update();
      mode_music_render();
      break;
    case MODE_CHAT:
      mode_chat_update();
      mode_chat_render();
      break;
    case MODE_THEME_PREVIEW:
      mode_theme_update();
      mode_theme_render();
      break;
    case MODE_WIFI_INFO:
      mode_wifi_update();
      mode_wifi_render();
      break;
  }
  
  delay(10);
}
