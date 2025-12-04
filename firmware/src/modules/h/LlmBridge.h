#pragma once
#include <Arduino.h>

void BridgeInit();
void BridgeSendAudio(const uint8_t* data, size_t len);
void BridgeSendCommand(const char* cmd);
void BridgeHandleResponse();
bool bridge_has_response();