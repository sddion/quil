#pragma once

// I2C for Display (SSD1306 OLED)
#define PIN_I2C_SDA 21
#define PIN_I2C_SCL 22

// I2S Microphone (INMP441)
#define PIN_I2S_MIC_BCLK 32   // SCK
#define PIN_I2S_MIC_WS   25   // WS
#define PIN_I2S_MIC_DOUT 33   // SD

// I2S Speaker/Amp (MAX98357A)
#define PIN_I2S_SPK_WS   26   // LRCLK
#define PIN_I2S_SPK_BCLK 27   // BCLK
#define PIN_I2S_SPK_DIN  23   // DIN

// Status LED
#define PIN_LED 4

// UART2 (optional)
#define PIN_UART2_TX 17
#define PIN_UART2_RX 16

// Native ESP32 Capacitive Touch (T6)
#define PIN_TOUCH 14  // GPIO14 - T6 native touch pin
