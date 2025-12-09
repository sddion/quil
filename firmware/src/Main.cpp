#include <Arduino.h>
#include "pins.h"
#include "config.h"
#include "types.h"
#include "hal/h/I2C.h"
#include "hal/h/Display.h"
#include "hal/h/NativeTouch.h"
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
#include "modules/h/ConversationManager.h"
#include "modes/h/Time.h"
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
    // === FIRST BOOT: Start BLE (primary) and Web Portal (fallback) ===
    Serial.println("[Boot] First boot - entering setup mode");
    
    BootLoaderShowStage(BOOT_STAGE_WIFI, true);
    // Start web portal (which also starts AP mode)
    WifiStartPortal();
    
    BootLoaderShowStage(BOOT_STAGE_SERVICES, true);
    BleInit();
    
    // Initialize SetupScreen and start waiting
    SetupScreenInit();
    StateSetMode(MODE_SETUP);
    
    Serial.println("[Boot] Setup mode active - BLE (primary) + Web Portal (fallback)");
    Serial.printf("[Boot] Connect to WiFi: %s (Password: %s)\n", WIFI_AP_SSID, WIFI_AP_PASS);
    Serial.printf("[Boot] Or use Quil app for Bluetooth setup\n");
    // Don't proceed further - loop() will handle waiting
    return;
  }
  
  // === NORMAL BOOT: Full initialization ===
  
  // Stage 2: WiFi
  BootLoaderShowStage(BOOT_STAGE_WIFI, false);
  char ssid[33] = {0}, pass[65] = {0};
  if (ConfigLoadWifi(ssid, pass)) {
    WifiConnect(ssid, pass);
  } else {
    Serial.println("[Boot] Failed to load WiFi credentials");
    // Consider fallback behavior: enter setup mode or show error
  }
  
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
  ConversationInit();
  
  // Stage 5: Display
  BootLoaderShowStage(BOOT_STAGE_DISPLAY, false);
  // Time mode is the only display mode now
  
  // Stage 6: Update Check
  BootLoaderShowStage(BOOT_STAGE_UPDATE, false);
  if (WifiHasInternet()) {
    // Check for updates in background
  }
  
  // Stage 7: Complete
  BootLoaderShowStage(BOOT_STAGE_COMPLETE, false);
  delay(500);
  BootLoaderComplete();
  
  // Start in clock mode
  StateSetMode(MODE_CLOCK);
  TimeForceRender();
  
  Serial.println("Quil ready");
}

void loop() {
  DisplayMode_t mode = StateGetMode();
  
  // === SETUP MODE: First boot - waiting for WiFi configuration ===
  if (mode == MODE_SETUP) {
    BleLoop();
    WifiPortalLoop();
    
    if (WifiHasSavedCredentials() && WifiIsConnected()) {
      Serial.println("[Setup] WiFi configured! Restarting...");
      WifiStopPortal();
      BleNotifySetupComplete();
      delay(1000);
      ESP.restart();
    }
    
    SetupScreenUpdate();
    delay(50);
    return;
  }
  
  // === NORMAL MODE ===
  
  // Background tasks
  WifiReconnectTask();
  StateUpdate();
  DiagUpdate();
  OtaHandle();
  BleLoop();
  
  // Touch handling (mute toggle)
  NativeTouchUpdate();
  if (NativeTouchHasTap()) {
    GestureType gest = GestureDetect(0, millis());
    if (gest != GESTURE_NONE) {
      ActionsHandle(gest, mode);
    }
  }
  
  // Animation playback
  if (AnimIsPlaying()) {
    AnimUpdate();
    delay(10);
    return;
  }
  
  // === CONVERSATION MODE ===
  if (mode == MODE_CONVERSATION) {
    ConversationLoop();
    
    // Stream audio to server
    if (VoiceIsListening()) {
      uint8_t audio[256];
      size_t len = VoiceReadBuffer(audio, sizeof(audio));
      if (len > 0) {
        BridgeSendAudio(audio, len);
      }
    }
    
    BridgeHandleResponse();
    ConversationRender();
    
    // Check timeout - return to clock
    if (ConversationTimedOut()) {
      ConversationEnd();
      StateSetMode(MODE_CLOCK);
      VoiceStartListening();  // Resume listening for wake
    }
  }
  
  // === CLOCK MODE (default) ===
  else if (mode == MODE_CLOCK) {
    // Ensure mic is always listening for wake detection
    if (!VoiceIsListening()) {
      VoiceStartListening();
    }
    
    // Read mic to update RMS for wake detection
    uint8_t audio[256];
    size_t len = VoiceReadBuffer(audio, sizeof(audio));
    (void)len; // We just need to update the internal RMS
    
    // Check for wake (voice activity)
    if (WakeDetect()) {
      Serial.println("[Main] Wake detected - starting conversation");
      StateSetMode(MODE_CONVERSATION);
      ConversationStart();
      BridgeSendCommand("wake_quil");
    }
    
    TimeUpdate();
    TimeRender();
  }
  
  delay(10);
}