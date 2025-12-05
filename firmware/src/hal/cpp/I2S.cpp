#include "../h/I2S.h"
#include <driver/i2s.h>
#include "pins.h"

static uint32_t CurrentMicRate = I2S_SAMPLE_RATE_MIC;
static uint32_t CurrentSpeakerRate = I2S_SAMPLE_RATE_SPEAKER;

bool I2SInitMic() {
  i2s_config_t Cfg = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = CurrentMicRate,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,  // PCM16 for OpenAI
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 8,
    .dma_buf_len = 1024,
    .use_apll = false,
    .tx_desc_auto_clear = false,
    .fixed_mclk = 0
  };
  
  i2s_pin_config_t Pins = {
    .bck_io_num = PIN_I2S_MIC_BCLK,
    .ws_io_num = PIN_I2S_MIC_WS,
    .data_out_num = I2S_PIN_NO_CHANGE,
    .data_in_num = PIN_I2S_MIC_DOUT
  };
  
  esp_err_t Result = i2s_driver_install(I2S_NUM_0, &Cfg, 0, NULL);
  if (Result != ESP_OK) {
    Serial.printf("[I2S] Mic driver install failed: %d\n", Result);
    return false;
  }
  
  Result = i2s_set_pin(I2S_NUM_0, &Pins);
  if (Result != ESP_OK) {
    Serial.printf("[I2S] Mic pin config failed: %d\n", Result);
    return false;
  }
  
  Serial.printf("[I2S] Mic initialized at %d Hz\n", CurrentMicRate);
  return true;
}

bool I2SInitSpeaker() {
  i2s_config_t Cfg = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
    .sample_rate = CurrentSpeakerRate,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,  // PCM16 from OpenAI
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 8,
    .dma_buf_len = 1024,
    .use_apll = false,
    .tx_desc_auto_clear = true,
    .fixed_mclk = 0
  };
  
  i2s_pin_config_t Pins = {
    .bck_io_num = PIN_I2S_SPK_BCLK,
    .ws_io_num = PIN_I2S_SPK_WS,
    .data_out_num = PIN_I2S_SPK_DIN,
    .data_in_num = I2S_PIN_NO_CHANGE
  };
  
  esp_err_t Result = i2s_driver_install(I2S_NUM_1, &Cfg, 0, NULL);
  if (Result != ESP_OK) {
    Serial.printf("[I2S] Speaker driver install failed: %d\n", Result);
    return false;
  }
  
  Result = i2s_set_pin(I2S_NUM_1, &Pins);
  if (Result != ESP_OK) {
    Serial.printf("[I2S] Speaker pin config failed: %d\n", Result);
    return false;
  }
  
  Serial.printf("[I2S] Speaker initialized at %d Hz\n", CurrentSpeakerRate);
  return true;
}

size_t I2SReadMic(uint8_t* Buffer, size_t Length) {
  size_t BytesRead = 0;
  esp_err_t Result = i2s_read(I2S_NUM_0, Buffer, Length, &BytesRead, 100 / portTICK_PERIOD_MS);
  
  if (Result != ESP_OK) {
    return 0;
  }
  
  return BytesRead;
}

size_t I2SWriteSpeaker(const uint8_t* Data, size_t Length) {
  size_t BytesWritten = 0;
  esp_err_t Result = i2s_write(I2S_NUM_1, Data, Length, &BytesWritten, 100 / portTICK_PERIOD_MS);
  
  if (Result != ESP_OK) {
    return 0;
  }
  
  return BytesWritten;
}

bool I2SSetSampleRate(uint32_t MicRate, uint32_t SpeakerRate) {
  bool Success = true;
  
  if (MicRate != CurrentMicRate) {
    esp_err_t Result = i2s_set_sample_rates(I2S_NUM_0, MicRate);
    if (Result == ESP_OK) {
      CurrentMicRate = MicRate;
      Serial.printf("[I2S] Mic sample rate changed to %d Hz\n", MicRate);
    } else {
      Success = false;
    }
  }
  
  if (SpeakerRate != CurrentSpeakerRate) {
    esp_err_t Result = i2s_set_sample_rates(I2S_NUM_1, SpeakerRate);
    if (Result == ESP_OK) {
      CurrentSpeakerRate = SpeakerRate;
      Serial.printf("[I2S] Speaker sample rate changed to %d Hz\n", SpeakerRate);
    } else {
      Success = false;
    }
  }
  
  return Success;
}