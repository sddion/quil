#pragma once
#include <Arduino.h>

void I2CInit();
bool I2CWrite(uint8_t addr, uint8_t reg, uint8_t val);
uint8_t I2CRead(uint8_t addr, uint8_t reg);
bool I2CWriteBytes(uint8_t addr, uint8_t* data, size_t len);