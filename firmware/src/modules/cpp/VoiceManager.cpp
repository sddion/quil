#include "../h/VoiceManager.h"
#ifdef ESP32
#include "hal/h/I2S.h"

static bool listening = false;
static uint8_t audio_buffer[512];
static size_t buffer_pos = 0;

void voice_init() {
  hal_i2s_init_mic();
  hal_i2s_init_speaker();
  listening = false;
}

void voice_start_listening() {
  listening = true;
  buffer_pos = 0;
}

void voice_stop_listening() {
  listening = false;
}

bool voice_is_listening() {
  return listening;
}

size_t voice_read_buffer(uint8_t* buf, size_t len) {
  if (!listening) return 0;
  return hal_i2s_read_mic(buf, len);
}

void voice_play_response(const uint8_t* data, size_t len) {
  hal_i2s_write_speaker(data, len);
}

float voice_get_rms() {
  if (buffer_pos < 100) return 0.0f;
  float sum = 0;
  for (size_t i = 0; i < buffer_pos; i += 2) {
    int16_t sample = (audio_buffer[i+1] << 8) | audio_buffer[i];
    sum += sample * sample;
  }
  return sqrt(sum / (buffer_pos / 2));
}
#else
void voice_init() {}
void voice_start_listening() {}
void voice_stop_listening() {}
bool voice_is_listening() { return false; }
size_t voice_read_buffer(uint8_t* buf, size_t len) { return 0; }
void voice_play_response(const uint8_t* data, size_t len) {}
float voice_get_rms() { return 0.0f; }
#endif
