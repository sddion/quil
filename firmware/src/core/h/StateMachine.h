#pragma once
#include "types.h"

void state_init();
void state_update();
void state_set_mode(DisplayMode_t mode);
void state_cycle_mode();
DisplayMode_t state_get_mode();
RobotState_t state_get_robot();
void state_set_robot(RobotState_t state);
