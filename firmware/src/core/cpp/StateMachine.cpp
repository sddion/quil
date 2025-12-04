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
  current_mode = (DisplayMode_t)((current_mode + 1) % 5);
}

RobotState_t StateGetRobot() {
  return current_state;
}

void StateSetRobot(RobotState_t state) {
  current_state = state;
}