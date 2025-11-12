#pragma once

#ifdef ESP32
  #define PIN_I2C_SDA 21
  #define PIN_I2C_SCL 22
  #define PIN_I2S_MIC_WS 25
  #define PIN_I2S_MIC_BCLK 26
  #define PIN_I2S_MIC_DOUT 27
  #define PIN_I2S_SPK_WS 32
  #define PIN_I2S_SPK_BCLK 33
  #define PIN_I2S_SPK_DIN 19
  #define PIN_LED 4
  #define PIN_UART2_TX 17
  #define PIN_UART2_RX 16
#elif defined(ESP8266)
  #define PIN_I2C_SDA D2
  #define PIN_I2C_SCL D1
  #define PIN_LED D4
#endif
