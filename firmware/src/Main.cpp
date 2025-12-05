#include <Arduino.h>
#include "pins.h"
#include "config.h"
#include "types.h"
#include "hal/h/I2C.h"
#include "hal/h/Display.h"
#include "core/h/StateMachine.h"
#include "core/h/Diagnostics.h"
#include "modules/h/ConfigStore.h"
#include "modules/h/WifiManager.h"
#include "modules/h/NtpClient.h"
#include "modules/h/OtaManager.h"
#include "modules/h/HttpServer.h"
#include "modules/h/GestureManager.h"
#include "modules/h/TouchActions.h"
#include "modules/h/VoiceManager.h"
#include "modules/h/WakeManager.h"
#include "modules/h/LlmBridge.h"
#include "modules/h/AnimationManager.h"
#include "modules/h/BatteryManager.h"
#include "hal/h/Ttp223.h"
#include "modes/h/Time.h"

#include "modes/h/Chat.h"
#include "modes/h/ThemePreview.h"
#include "modes/h/WifiInfo.h"
#include "modes/h/SetupScreen.h"

void setup() {
  Serial.begin(115200);
  delay(100);
  
  I2CInit();
  if (!DisplayInit()) {
    Serial.println("Display fail");
    while(1);
  }
  
  ConfigInit();
  DiagInit();
  StateInit();
  GestureInit();
  ActionsInit();
  
  // Load and apply display contrast
  uint8_t contrast = 128;
  if (ConfigLoadContrast(&contrast)) {
    DisplaySetContrast(contrast);
  }
  
  // Initialize WiFi and load saved credentials
  WifiInit();
  
  // Check if we have saved WiFi credentials
  if (WifiHasSavedCredentials()) {
    Serial.println("[Setup] Found saved WiFi credentials");
    char ssid[33], pass[65];
    ConfigLoadWifi(ssid, pass);
    
    // Attempt connection - even if it fails, stay in STA mode and keep retrying
    if (!WifiConnect(ssid, pass)) {
      Serial.println("[Setup] Initial connection failed, will retry in background");
      // Don't start AP mode - just keep trying to connect in background
    }
  } else {
    // No saved credentials - start AP mode for initial setup
    Serial.println("[Setup] No saved credentials - starting AP mode");
    WifiStartAp();
  }
  
  // Always start HTTP server regardless of WiFi connection status
  // This allows web interface access once WiFi connects (even if connection was delayed)
  HttpInit();
  
  OtaInit();
  VoiceInit();
  WakeInit();
  BridgeInit();
  AnimInit();
  AnimPlay(ANIM_BOOT);
  while(AnimIsPlaying()) {
    AnimUpdate();
    delay(10);
  }
  
  // Clear display after boot animation
  DisplayClear();
  DisplayUpdate();
  
  // Show setup screen if in AP mode (first boot)
  if (WifiIsApMode()) {
    SetupScreenInit();
    SetupScreenShow();
    
    // Wait for user to configure WiFi and complete setup
    while (!SetupScreenIsComplete()) {
      SetupScreenUpdate();
      HttpHandle();  // Keep HTTP server running for config
      delay(100);
    }
    // Device will restart after setup completes
  }
  
  TimeInit();

  ChatInit();
  ThemeInit();
  WifiInfoInit();
  
  // Force initial render to prevent black screen
  TimeForceRender();
  
  Serial.println("Quil ready");
  Serial.println("[HTTP] Web interface available at device IP when connected");
}

void loop() {
  // WiFi reconnection task - handles automatic reconnection and internet checks
  WifiReconnectTask();
  
  StateUpdate();
  DiagUpdate();
  OtaHandle();
  HttpHandle();
  
  TtpUpdate();
  
  if (TtpHasEvent(TOUCH_SENSOR_A)) {
    GestureType gest = GestureDetect(0, millis());
    if (gest != GESTURE_NONE) {
      ActionsHandle(gest, StateGetMode());
    }
  }
  
  if (WakeDetect()) {
    BridgeSendCommand("wake_quil");
    VoiceStartListening();
  }
  
  if (VoiceIsListening()) {
    uint8_t audio[256];
    size_t len = VoiceReadBuffer(audio, sizeof(audio));
    if (len > 0) {
      BridgeSendAudio(audio, len);
    }
  }
  
  BridgeHandleResponse();
  
  if (AnimIsPlaying()) {
    AnimUpdate();
    delay(10);
    return;
  }
  
  DisplayMode_t mode = StateGetMode();
  switch(mode) {
    case MODE_TIME_DATE:
      TimeUpdate();
      TimeRender();
      break;

    case MODE_CHAT:
      ChatUpdate();
      ChatRender();
      break;
    case MODE_THEME_PREVIEW:
      ThemeUpdate();
      ThemeRender();
      break;
    case MODE_WIFI_INFO:
      WifiInfoUpdate();
      WifiInfoRender();
      break;
  }
  
  delay(10);
}