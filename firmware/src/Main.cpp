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
#include "core/h/BootLoader.h"

void setup() {
  Serial.begin(115200);
  delay(100);
  
  // === PHASE 1: Boot Animation (FIRST!) ===
  I2CInit();
  if (!DisplayInit()) {
    Serial.println("Display fail");
    while(1);
  }
  
  AnimInit();
  AnimPlay(ANIM_BOOT);
  while(AnimIsPlaying()) {
    AnimUpdate();
    delay(10);
  }
  
  DisplayClear();
  DisplayUpdate();
  
  // Determine if this is first boot (AP mode needed)
  bool isFirstBoot = false;
  
  // === PHASE 2: Loading Sequence with Progress Bar ===
  BootLoaderInit();
  
  // Stage 1: Hardware/Settings
  BootLoaderShowStage(BOOT_STAGE_HARDWARE, isFirstBoot);
  ConfigInit();
  DiagInit();
  StateInit();
  GestureInit();
  ActionsInit();
  uint8_t contrast = 128;
  if (ConfigLoadContrast(&contrast)) {
    DisplaySetContrast(contrast);
  }
  
  // Check credentials to determine boot type
  WifiInit();
  isFirstBoot = !WifiHasSavedCredentials();
  
  // Stage 2: WiFi
  BootLoaderShowStage(BOOT_STAGE_WIFI, isFirstBoot);
  if (WifiHasSavedCredentials()) {
    char ssid[33], pass[65];
    ConfigLoadWifi(ssid, pass);
    WifiConnect(ssid, pass);
  } else {
    WifiStartAp();
  }
  
  // Stage 3: Time
  BootLoaderShowStage(BOOT_STAGE_TIME, isFirstBoot);
  TimeInit();
  if (WifiIsConnected()) {
    NtpUpdate();
  }
  
  // Stage 4: Services
  BootLoaderShowStage(BOOT_STAGE_SERVICES, isFirstBoot);
  OtaInit();
  VoiceInit();
  WakeInit();
  BridgeInit();
  
  // Stage 5: Display
  BootLoaderShowStage(BOOT_STAGE_DISPLAY, isFirstBoot);
  ChatInit();
  ThemeInit();
  WifiInfoInit();
  
  // Stage 6: Update Check
  BootLoaderShowStage(BOOT_STAGE_UPDATE, isFirstBoot);
  if (WifiHasInternet()) {
    // Check for updates in background (don't block)
    // OtaCheckGithubUpdate();  // Uncomment when ready to auto-update
  }
  
  // Stage 7: Complete
  BootLoaderShowStage(BOOT_STAGE_COMPLETE, isFirstBoot);
  delay(500);
  BootLoaderComplete();
  
  // Force initial render to Time mode
  TimeForceRender();
  
  Serial.println("Quil ready");
}

void loop() {
  // WiFi reconnection task - handles automatic reconnection and internet checks
  WifiReconnectTask();
  
  StateUpdate();
  DiagUpdate();
  OtaHandle();
  
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