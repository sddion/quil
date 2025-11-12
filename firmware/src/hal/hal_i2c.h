#pragma once
#include <Arduino.h>

void hal_i2c_init();
bool hal_i2c_write(uint8_t addr, uint8_t reg, uint8_t val);
uint8_t hal_i2c_read(uint8_t addr, uint8_t reg);
bool hal_i2c_write_bytes(uint8_t addr, uint8_t* data, size_t len);
