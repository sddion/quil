# Voice & LLM Integration - Phase 8

## Architecture

```
[ESP32 Firmware]
├── I2S Mic (INMP441) → voice_manager
├── Wake Word Detection → wake_manager
├── Audio Streaming → llm_bridge (Serial/WebSocket)
└── I2S Speaker (MAX98357A) ← Response playback

[Bridge App] (Node.js/Python - separate)
├── Receives audio stream from ESP32
├── OpenAI Whisper → Speech-to-Text
├── OpenAI GPT-4o-mini → Generate response
├── OpenAI TTS → Text-to-Speech
└── Sends audio response back to ESP32
```

## New Modules

### voice_manager.{h,cpp}
Manages I2S audio capture and playback:
- `voice_init()` - Initialize mic and speaker
- `voice_start_listening()` - Begin audio capture
- `voice_stop_listening()` - Stop capture
- `voice_is_listening()` - Check capture state
- `voice_read_buffer()` - Read mic data
- `voice_play_response()` - Play audio response
- `voice_get_rms()` - Get audio level (VAD)

### wake_manager.{h,cpp}
Wake word detection ("Hey Quil"):
- `wake_init()` - Initialize detector
- `wake_detect()` - Check for wake phrase (RMS-based)
- `wake_set_threshold()` - Adjust sensitivity
- `wake_get_confidence()` - Get detection confidence

### llm_bridge.{h,cpp}
Communication with host bridge app:
- `bridge_init()` - Initialize serial bridge
- `bridge_send_audio()` - Stream audio to host
- `bridge_send_command()` - Send control signals
- `bridge_handle_response()` - Receive AI responses
- `bridge_has_response()` - Check response ready

## Main Loop Integration

```cpp
// Wake word detection
if (wake_detect()) {
  bridge_send_command("wake_quil");
  voice_start_listening();
}

// Audio streaming
if (voice_is_listening()) {
  uint8_t audio[256];
  size_t len = voice_read_buffer(audio, sizeof(audio));
  if (len > 0) {
    bridge_send_audio(audio, len);
  }
}

// Response handling
bridge_handle_response();
```

## Wake Word Detection

**Simple RMS-based approach:**
1. Monitor audio RMS level
2. When RMS > threshold (500.0f default)
3. Trigger wake event
4. Send `{"event":"wake_quil"}` to bridge

**Future enhancement:** MFCC fingerprint matching

## Communication Protocol

### ESP32 → Bridge

**Wake event:**
```json
{"event":"wake_quil"}
```

**Audio stream:**
Binary data via Serial.write()

### Bridge → ESP32

**Audio response:**
Binary PCM/WAV data via Serial

## Bridge App Implementation (Future)

### Speech-to-Text (Whisper)
```javascript
const transcription = await openai.audio.transcriptions.create({
  file: audioFile,
  model: "whisper-1"
});
```

### Text Generation (GPT-4o-mini)
```javascript
const response = await openai.chat.completions.create({
  model: "gpt-4o-mini",
  messages: [
    { role: "system", content: "You are Quil, a friendly desktop AI companion." },
    { role: "user", content: transcription.text }
  ]
});
```

### Text-to-Speech
```javascript
const speech = await openai.audio.speech.create({
  model: "gpt-4o-mini-tts",
  voice: "verse",
  input: response.choices[0].message.content
});
```

## Mode Integration

### mode_chat.cpp
Updated to use voice_manager:
- `mode_chat_init()` → calls `voice_init()`
- `mode_chat_start_listen()` → calls `voice_start_listening()`
- `mode_chat_toggle_mute()` → toggles voice capture
- Display shows real-time listening state

## Performance

- **Buffer size:** 512 bytes audio buffer
- **Sample rate:** 16 kHz (I2S config)
- **Bits per sample:** 32-bit (mic), 16-bit (speaker)
- **Latency:** ~100ms mic → bridge
- **Wake detection:** RMS calculation every loop cycle

## Testing

1. **Wake word test:**
   - Speak "Hey Quil" near mic
   - Check serial: `{"event":"wake_quil"}`
   
2. **Audio streaming:**
   - Monitor serial for audio data after wake
   - Verify continuous stream while listening

3. **Response playback:**
   - Bridge sends audio via serial
   - Speaker should play response

## Configuration

**Wake threshold:**
```cpp
wake_set_threshold(500.0f);  // Adjust sensitivity
```

**Audio buffer:**
Defined in `voice_manager.cpp`:
```cpp
static uint8_t audio_buffer[512];
```

## ESP8266 Support

Voice features disabled on ESP8266 (no I2S):
- All functions return empty/false
- No audio capture or playback
- Bridge still functional for future use

## Security Notes

- Audio streamed unencrypted over serial (local only)
- Consider encryption for network-based bridge
- API keys stored in bridge app, not on ESP32

## Future Enhancements

1. **Advanced wake word:** MFCC fingerprint matching
2. **VAD improvement:** Energy-based voice activity detection
3. **Noise reduction:** Pre-processing before streaming
4. **Multiple wake words:** Support custom phrases
5. **Local TTS:** On-device synthesis for common phrases
