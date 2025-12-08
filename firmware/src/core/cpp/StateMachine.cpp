#include "../h/StateMachine.h"
#include <Arduino.h>
#include "config.h"

static DisplayMode_t current_mode = MODE_TIME_DATE;
static RobotState_t current_state = STATE_BOOT;
static unsigned long last_update = 0;

void StateInit() {
  current_state = STATE_IDLE;
  current_mode = MODE_TIME_DATE;
}

void StateUpdate() {
  unsigned long now = millis();
  if (now - last_update < STATE_UPDATE_MS) return;
  last_update = now;
}

void StateSetMode(DisplayMode_t mode) {
  if (mode == current_mode) return;
  current_mode = mode;
}

DisplayMode_t StateGetMode() {
  return current_mode;
}

void StateCycleMode() {
  // Skip MODE_SETUP (0) - only cycle through TIME_DATE, CHAT, WIFI_INFO
  switch (current_mode) {
    case MODE_TIME_DATE:
      current_mode = MODE_CHAT;
      break;
    case MODE_CHAT:
      current_mode = MODE_WIFI_INFO;
      break;
    case MODE_WIFI_INFO:
    default:
      current_mode = MODE_TIME_DATE;
      break;
  }
}

RobotState_t StateGetRobot() {
  return current_state;
}

void StateSetRobot(RobotState_t state) {
  current_state = state;
}