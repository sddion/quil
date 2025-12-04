#include "../h/StateMachine.h"
#include <Arduino.h>
#include "config.h"

static DisplayMode_t current_mode = MODE_TIME_DATE;
static RobotState_t current_state = STATE_BOOT;
static unsigned long last_update = 0;

void state_init() {
  current_state = STATE_IDLE;
  current_mode = MODE_TIME_DATE;
}

void state_update() {
  unsigned long now = millis();
  if (now - last_update < STATE_UPDATE_MS) return;
  last_update = now;
}

void state_set_mode(DisplayMode_t mode) {
  if (mode == current_mode) return;
  current_mode = mode;
}

DisplayMode_t state_get_mode() {
  return current_mode;
}

void state_cycle_mode() {
  current_mode = (DisplayMode_t)((current_mode + 1) % 5);
}

RobotState_t state_get_robot() {
  return current_state;
}

void state_set_robot(RobotState_t state) {
  current_state = state;
}
