# Quil Bridge App

> Host-side application for OpenAI API integration

## Overview

The bridge app connects Quil ESP32 firmware to OpenAI APIs:
- Receives audio stream via Serial/WebSocket
- Transcribes speech with Whisper
- Generates responses with GPT-4o-mini
- Synthesizes speech with TTS
- Sends audio back to ESP32

## Requirements

```bash
npm install openai serialport
# or
pip install openai pyserial
```

## Environment Variables

```bash
OPENAI_API_KEY=sk-...
SERIAL_PORT=/dev/ttyUSB0  # or COM3 on Windows
BAUD_RATE=115200
```

## Protocol

### ESP32 → Bridge

**Wake Event:**
```json
{"event":"wake_quil"}
```

**Audio Stream:**
Binary PCM data (16kHz, 16-bit)

### Bridge → ESP32

**Audio Response:**
Binary PCM/WAV data

## Implementation (Node.js)

```javascript
const { SerialPort } = require('serialport');
const OpenAI = require('openai');

const openai = new OpenAI({ apiKey: process.env.OPENAI_API_KEY });
const port = new SerialPort({ path: '/dev/ttyUSB0', baudRate: 115200 });

let audioBuffer = [];

port.on('data', async (data) => {
  // Check for wake event
  const str = data.toString();
  if (str.includes('wake_quil')) {
    console.log('Wake word detected');
    audioBuffer = [];
    return;
  }
  
  // Accumulate audio
  audioBuffer.push(...data);
  
  // When buffer full, process
  if (audioBuffer.length > 32000) {  // ~2s at 16kHz
    await processAudio(Buffer.from(audioBuffer));
    audioBuffer = [];
  }
});

async function processAudio(audioData) {
  // 1. Transcribe
  const transcription = await openai.audio.transcriptions.create({
    file: audioData,
    model: 'whisper-1'
  });
  
  // 2. Generate response
  const response = await openai.chat.completions.create({
    model: 'gpt-4o-mini',
    messages: [
      { role: 'system', content: 'You are Quil, a helpful desktop companion.' },
      { role: 'user', content: transcription.text }
    ]
  });
  
  // 3. Synthesize speech
  const speech = await openai.audio.speech.create({
    model: 'tts-1',
    voice: 'verse',
    input: response.choices[0].message.content
  });
  
  // 4. Send to ESP32
  const buffer = Buffer.from(await speech.arrayBuffer());
  port.write(buffer);
}
```

## Implementation (Python)

```python
import serial
import openai
import os

openai.api_key = os.getenv('OPENAI_API_KEY')
ser = serial.Serial('/dev/ttyUSB0', 115200)

audio_buffer = bytearray()

while True:
    data = ser.read(256)
    
    # Check for wake event
    if b'wake_quil' in data:
        print('Wake detected')
        audio_buffer = bytearray()
        continue
    
    # Accumulate audio
    audio_buffer.extend(data)
    
    # Process when buffer full
    if len(audio_buffer) > 32000:
        process_audio(bytes(audio_buffer))
        audio_buffer = bytearray()

def process_audio(audio_data):
    # 1. Transcribe
    transcription = openai.Audio.transcribe('whisper-1', audio_data)
    
    # 2. Generate response
    response = openai.ChatCompletion.create(
        model='gpt-4o-mini',
        messages=[
            {'role': 'system', 'content': 'You are Quil, a helpful companion.'},
            {'role': 'user', 'content': transcription['text']}
        ]
    )
    
    # 3. Synthesize speech
    speech = openai.Audio.create_speech(
        model='tts-1',
        voice='verse',
        input=response['choices'][0]['message']['content']
    )
    
    # 4. Send to ESP32
    ser.write(speech)
```

## Usage

1. Upload firmware to ESP32
2. Start bridge app:
   ```bash
   node bridge.js
   # or
   python bridge.py
   ```
3. Say "Hey Quil" to wake
4. Speak your query
5. Wait for audio response

## Troubleshooting

**No wake detection:**
- Check serial monitor for audio levels
- Adjust threshold: `wake_set_threshold(300.0f)`

**No audio streaming:**
- Verify I2S mic connections
- Check ESP32 pins in `pins.h`

**No response playback:**
- Verify I2S speaker connections
- Check serial data flow

## Development

- Test with mock serial: `socat -d -d pty,raw,echo=0 pty,raw,echo=0`
- Use audio file instead of mic for testing
- Log API responses for debugging
