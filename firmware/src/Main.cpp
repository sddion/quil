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
#include "modules/h/BleServer.h"
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
#include "modes/h/WifiInfo.h"
#include "modes/h/SetupScreen.h"
#include "core/h/BootLoader.h"

// Global flag for first boot mode
static bool g_isFirstBoot = false;

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
  
  // === PHASE 2: Loading Sequence with Progress Bar ===
  BootLoaderInit();
  
  // Stage 1: Hardware/Settings
  BootLoaderShowStage(BOOT_STAGE_HARDWARE, false);
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
  g_isFirstBoot = !WifiHasSavedCredentials();
  
  if (g_isFirstBoot) {
    // === FIRST BOOT: Start AP and BLE, then wait for configuration ===
    Serial.println("[Boot] First boot - entering setup mode");
    
    BootLoaderShowStage(BOOT_STAGE_WIFI, true);
    WifiStartAp();
    
    BootLoaderShowStage(BOOT_STAGE_SERVICES, true);
    BleInit();
    
    // Initialize SetupScreen and start waiting
    SetupScreenInit();
    StateSetMode(MODE_SETUP);
    
    Serial.println("[Boot] Setup mode active - waiting for WiFi config via app");
    // Don't proceed further - loop() will handle waiting
    return;
  }
  
  // === NORMAL BOOT: Full initialization ===
  
  // Stage 2: WiFi
  BootLoaderShowStage(BOOT_STAGE_WIFI, false);
  char ssid[33], pass[65];
  ConfigLoadWifi(ssid, pass);
  WifiConnect(ssid, pass);
  
  // Stage 3: Time
  BootLoaderShowStage(BOOT_STAGE_TIME, false);
  TimeInit();
  if (WifiIsConnected()) {
    NtpUpdate();
  }
  
  // Stage 4: Services
  BootLoaderShowStage(BOOT_STAGE_SERVICES, false);
  OtaInit();
  BleInit();
  VoiceInit();
  WakeInit();
  BridgeInit();
  
  // Stage 5: Display
  BootLoaderShowStage(BOOT_STAGE_DISPLAY, false);
  ChatInit();
  WifiInfoInit();
  
  // Stage 6: Update Check
  BootLoaderShowStage(BOOT_STAGE_UPDATE, false);
  if (WifiHasInternet()) {
    // Check for updates in background (don't block)
    // OtaCheckGithubUpdate();  // Uncomment when ready to auto-update
  }
  
  // Stage 7: Complete
  BootLoaderShowStage(BOOT_STAGE_COMPLETE, false);
  delay(500);
  BootLoaderComplete();
  
  // Force initial render to Time mode
  StateSetMode(MODE_TIME_DATE);
  TimeForceRender();
  
  Serial.println("Quil ready");
}

void loop() {
  DisplayMode_t mode = StateGetMode();
  
  // === SETUP MODE: First boot - waiting for WiFi configuration ===
  if (mode == MODE_SETUP) {
    BleLoop();  // Handle BLE commands for WiFi config
    
    // Check if WiFi credentials have been configured via BLE
    if (WifiHasSavedCredentials()) {
      Serial.println("[Setup] WiFi configured! Restarting...");
      delay(1000);
      ESP.restart();
    }
    
    // Update and render setup screen
    SetupScreenUpdate();
    delay(50);
    return;
  }
  
  // === NORMAL MODE: Full operation ===
  
  // WiFi reconnection task - handles automatic reconnection and internet checks
  WifiReconnectTask();
  
  StateUpdate();
  DiagUpdate();
  OtaHandle();
  BleLoop();
  
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
  
  switch(mode) {
    case MODE_TIME_DATE:
      TimeUpdate();
      TimeRender();
      break;

    case MODE_CHAT:
      ChatUpdate();
      ChatRender();
      break;
      
    case MODE_WIFI_INFO:
      WifiInfoUpdate();
      WifiInfoRender();
      break;
      
    default:
      break;
  }
  
  delay(10);
}