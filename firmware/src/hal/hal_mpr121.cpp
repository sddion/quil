#include "hal_mpr121.h"
#include "hal_i2c.h"
#include "config.h"

#define MPR121_TOUCHSTATUS_L 0x00
#define MPR121_TOUCHSTATUS_H 0x01
#define MPR121_SOFTRESET 0x80
#define MPR121_ECR 0x5E

bool hal_mpr121_init() {
  hal_i2c_write(MPR121_ADDR, MPR121_SOFTRESET, 0x63);
  delay(1);
  hal_i2c_write(MPR121_ADDR, MPR121_ECR, 0x8F);
  return true;
}

uint16_t hal_mpr121_read_touched() {
  uint8_t lo = hal_i2c_read(MPR121_ADDR, MPR121_TOUCHSTATUS_L);
  uint8_t hi = hal_i2c_read(MPR121_ADDR, MPR121_TOUCHSTATUS_H);
  return ((uint16_t)hi << 8) | lo;
}
