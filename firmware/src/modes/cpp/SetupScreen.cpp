#include "../h/SetupScreen.h"
#include "hal/h/Display.h"
#include "modules/Connectivity.h"
#include <Adafruit_GFX.h>

// Mode icon bitmap
static const unsigned char PROGMEM image_Mode_bits[] = {
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x06,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0f,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x0f,0x00,0x00,0x00,0x00,0x00,0x00,0x03,0x1f,0x8c,0x00,0x00,
  0x00,0x00,0x00,0x07,0xdf,0xbe,0x00,0x00,0x00,0x00,0x00,0x07,0xff,0xfe,0x00,0x00,
  0x00,0x00,0x00,0x07,0xff,0xfe,0x00,0x00,0x00,0x00,0x00,0x03,0xe0,0x7c,0x00,0x00,
  0x00,0x00,0x00,0x03,0xc0,0x3c,0x00,0x00,0x00,0x00,0x00,0x03,0x80,0x1c,0x00,0x00,
  0x00,0x00,0x00,0x1f,0x80,0x1f,0x80,0x00,0x00,0x00,0x00,0x3f,0x00,0x0f,0xc0,0x00,
  0x00,0x00,0x63,0x3f,0x00,0x0f,0xc0,0x00,0x00,0x00,0x77,0xbf,0x00,0x0f,0xc0,0x00,
  0x00,0x00,0xff,0x87,0x80,0x1e,0x00,0x00,0x00,0x01,0xff,0xc3,0x80,0x1c,0x00,0x00,
  0x00,0x07,0xc1,0xf3,0xc0,0x3c,0x00,0x00,0x00,0x07,0x80,0xf7,0xf0,0xfe,0x00,0x00,
  0x00,0x03,0x80,0x67,0xff,0xfe,0x00,0x00,0x00,0x01,0x80,0x67,0xff,0xfe,0x00,0x00,
  0x00,0x03,0x80,0x67,0x9f,0x9e,0x00,0x00,0x00,0x07,0x80,0xf0,0x0f,0x00,0x00,0x00,
  0x00,0x07,0xc1,0xf0,0x0f,0x00,0x00,0x00,0x00,0x01,0xff,0xc0,0x0f,0x00,0x00,0x00,
  0x00,0x00,0xff,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x77,0x80,0x00,0x00,0x00,0x00,
  0x00,0x00,0x63,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};

enum SetupStage {
  STAGE_SHOW_IP,
  STAGE_CONNECTING,
  STAGE_CONNECTED,
  STAGE_TEST_INTERNET,
  STAGE_INTERNET_OK,
  STAGE_TEST_NTP,
  STAGE_NTP_OK,
  STAGE_COMPLETE,
  STAGE_DONE
};

static SetupStage current_stage = STAGE_SHOW_IP;
static unsigned long stage_start_time = 0;
static int progress = 0;
static bool setup_complete = false;

void SetupScreenInit() {
  current_stage = STAGE_SHOW_IP;
  stage_start_time = millis();
  progress = 0;
  setup_complete = false;
}

void SetupScreenShow() {
  Adafruit_SSD1306& display = DisplayGetDisplay();
  display.clearDisplay();
  
  // Draw mode icon at top center
  display.drawBitmap(32, 5, image_Mode_bits, 64, 32, 1);
  
  // Draw text based on current stage
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  
  String statusText = "";
  
  switch(current_stage) {
    case STAGE_SHOW_IP:
      statusText = "IP: " + WifiGetIp();
      display.setCursor(10, 42);
      display.print(statusText);
      display.setCursor(15, 52);
      display.print("Connect via WiFi");
      break;
      
    case STAGE_CONNECTING:
      display.setCursor(25, 45);
      display.print("Connecting...");
      break;
      
    case STAGE_CONNECTED:
      display.setCursor(28, 45);
      display.print("Connected!");
      break;
      
    case STAGE_TEST_INTERNET:
      display.setCursor(10, 45);
      display.print("Testing Internet...");
      break;
      
    case STAGE_INTERNET_OK:
      display.setCursor(20, 45);
      display.print("Internet OK");
      break;
      
    case STAGE_TEST_NTP:
      display.setCursor(15, 45);
      display.print("Syncing Time...");
      break;
      
    case STAGE_NTP_OK:
      display.setCursor(28, 45);
      display.print("Time Synced");
      break;
      
    case STAGE_COMPLETE:
      display.setCursor(15, 45);
      display.print("Setup Complete!");
      break;
      
    case STAGE_DONE:
      display.setCursor(10, 45);
      display.print("Restarting...");
      break;
  }
  
  // Draw progress bar (only show during connection stages)
  if (current_stage >= STAGE_CONNECTING && current_stage < STAGE_DONE) {
    display.drawRect(7, 56, 114, 6, 1);
    display.fillRect(8, 57, progress, 4, 1);
  }
  
  display.display();
}

void SetupScreenUpdate() {
  unsigned long now = millis();
  unsigned long elapsed = now - stage_start_time;
  
  switch(current_stage) {
    case STAGE_SHOW_IP:
      // Show IP for 3 seconds, then wait for WiFi connection
      if (elapsed > 3000) {
        if (WifiIsConnected()) {
          current_stage = STAGE_CONNECTING;
          stage_start_time = now;
          progress = 0;
        }
      }
      break;
      
    case STAGE_CONNECTING:
      // Simulate connection progress
      progress = min(112, (int)((elapsed / 1000.0) * 56));
      if (elapsed > 2000) {
        current_stage = STAGE_CONNECTED;
        stage_start_time = now;
        progress = 0;
      }
      break;
      
    case STAGE_CONNECTED:
      progress = min(112, (int)((elapsed / 500.0) * 112));
      if (elapsed > 500) {
        current_stage = STAGE_TEST_INTERNET;
        stage_start_time = now;
        progress = 0;
      }
      break;
      
    case STAGE_TEST_INTERNET:
      progress = min(112, (int)((elapsed / 2000.0) * 56));
      if (elapsed > 2000) {
        bool has_internet = WifiCheckInternet();
        current_stage = has_internet ? STAGE_INTERNET_OK : STAGE_TEST_NTP;
        stage_start_time = now;
        progress = has_internet ? 56 : 0;
      }
      break;
      
    case STAGE_INTERNET_OK:
      progress = min(112, 56 + (int)((elapsed / 500.0) * 56));
      if (elapsed > 500) {
        current_stage = STAGE_TEST_NTP;
        stage_start_time = now;
        progress = 56;
      }
      break;
      
    case STAGE_TEST_NTP:
      progress = min(112, 56 + (int)((elapsed / 2000.0) * 28));
      if (elapsed > 2000) {
        NtpUpdate();
        current_stage = STAGE_NTP_OK;
        stage_start_time = now;
        progress = 84;
      }
      break;
      
    case STAGE_NTP_OK:
      progress = min(112, 84 + (int)((elapsed / 500.0) * 28));
      if (elapsed > 500) {
        current_stage = STAGE_COMPLETE;
        stage_start_time = now;
        progress = 112;
      }
      break;
      
    case STAGE_COMPLETE:
      if (elapsed > 1000) {
        current_stage = STAGE_DONE;
        stage_start_time = now;
      }
      break;
      
    case STAGE_DONE:
      if (elapsed > 1500) {
        setup_complete = true;
        ESP.restart();
      }
      break;
  }
  
  SetupScreenShow();
}

bool SetupScreenIsComplete() {
  return setup_complete;
}
