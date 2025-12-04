#pragma once
#include "types.h"

void StateInit();
void StateUpdate();
void StateSetMode(DisplayMode_t mode);
void StateCycleMode();
DisplayMode_t StateGetMode();
RobotState_t StateGetRobot();
void StateSetRobot(RobotState_t state);