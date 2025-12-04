#pragma once
#include <Arduino.h>

void bridge_init();
void bridge_send_audio(const uint8_t* data, size_t len);
void bridge_send_command(const char* cmd);
void bridge_handle_response();
bool bridge_has_response();
