#pragma once
#include <Arduino.h>

bool I2SInitMic();
bool I2SInitSpeaker();
size_t I2SReadMic(uint8_t* buffer, size_t len);
size_t I2SWriteSpeaker(const uint8_t* data, size_t len);