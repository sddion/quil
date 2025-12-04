#include "../h/I2S.h"

#ifdef ESP32
#include <driver/i2s.h>
#include "pins.h"

bool hal_i2s_init_mic() {
  i2s_config_t cfg = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = 16000,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 4,
    .dma_buf_len = 1024
  };
  i2s_pin_config_t pins = {
    .bck_io_num = PIN_I2S_MIC_BCLK,
    .ws_io_num = PIN_I2S_MIC_WS,
    .data_out_num = I2S_PIN_NO_CHANGE,
    .data_in_num = PIN_I2S_MIC_DOUT
  };
  return i2s_driver_install(I2S_NUM_0, &cfg, 0, NULL) == ESP_OK &&
         i2s_set_pin(I2S_NUM_0, &pins) == ESP_OK;
}

bool hal_i2s_init_speaker() {
  i2s_config_t cfg = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
    .sample_rate = 16000,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 4,
    .dma_buf_len = 1024
  };
  i2s_pin_config_t pins = {
    .bck_io_num = PIN_I2S_SPK_BCLK,
    .ws_io_num = PIN_I2S_SPK_WS,
    .data_out_num = PIN_I2S_SPK_DIN,
    .data_in_num = I2S_PIN_NO_CHANGE
  };
  return i2s_driver_install(I2S_NUM_1, &cfg, 0, NULL) == ESP_OK &&
         i2s_set_pin(I2S_NUM_1, &pins) == ESP_OK;
}

size_t hal_i2s_read_mic(uint8_t* buffer, size_t len) {
  size_t read = 0;
  i2s_read(I2S_NUM_0, buffer, len, &read, portMAX_DELAY);
  return read;
}

size_t hal_i2s_write_speaker(const uint8_t* data, size_t len) {
  size_t written = 0;
  i2s_write(I2S_NUM_1, data, len, &written, portMAX_DELAY);
  return written;
}
#endif
