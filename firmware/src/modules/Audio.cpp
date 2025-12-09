#include "Audio.h"
#include "hal/h/I2S.h"
#include "config.h"
#include "ConfigStore.h"
#include "pins.h" 
#include <WiFi.h>
#include <WebSocketsClient.h>
#include <ArduinoJson.h>

// =======================
// Core Audio (VoiceManager)
// =======================

static bool audio_listening = false;
static float last_rms = 0.0f;

void AudioInit() {
  I2SInitMic();
  I2SInitSpeaker();
  audio_listening = false;
  WakeInit();
  RealtimeVoiceInit();
}

void AudioStartListening() {
  audio_listening = true;
}

void AudioStopListening() {
  audio_listening = false;
}

bool AudioIsListening() {
  return audio_listening;
}

size_t AudioReadBuffer(uint8_t* buf, size_t len) {
  if (!audio_listening) return 0;
  
  size_t bytesRead = I2SReadMic(buf, len);
  
  // Calculate RMS
  if (bytesRead >= 100) {
    float sum = 0;
    for (size_t i = 0; i < bytesRead - 1; i += 2) {
      int16_t sample = (buf[i+1] << 8) | buf[i];
      sum += (float)sample * sample;
    }
    last_rms = sqrt(sum / (bytesRead / 2));
  }
  
  return bytesRead;
}

void AudioPlayResponse(const uint8_t* data, size_t len) {
  I2SWriteSpeaker(data, len);
}

float AudioGetRms() {
  return last_rms;
}

// =======================
// Audio Memory Buffer
// =======================

// Note: buffer implementation moved here
AudioMemoryBuffer::AudioMemoryBuffer() {
    buffer = NULL;
}

bool AudioMemoryBuffer::init() {
    if (buffer != NULL) free(buffer);
    
    if (psramFound()) {
        buffer = (int16_t*)ps_malloc(BUFFER_SIZE * sizeof(int16_t));
    } else {
        buffer = (int16_t*)malloc(BUFFER_SIZE * sizeof(int16_t));
    }
    
    if (!buffer) {
        Serial.println("[AudioBuffer] Failed to allocate memory");
        return false;
    }
    
    clear();
    Serial.printf("[AudioBuffer] Allocated %d bytes\n", BUFFER_SIZE * sizeof(int16_t));
    return true;
}

bool AudioMemoryBuffer::write(const int16_t* data, int length) {
    if (!buffer || !data) return false;
    
    if (samplesAvailable + length > BUFFER_SIZE) {
        return false; 
    }
    
    for (int i = 0; i < length; i++) {
        buffer[writeIndex] = data[i];
        writeIndex = (writeIndex + 1) % BUFFER_SIZE;
    }
    samplesAvailable += length;
    return true;
}

bool AudioMemoryBuffer::read(int16_t* data, int length) {
    if (!buffer || !data) return false;

    if (samplesAvailable < length) {
        return false; 
    }
    
    for (int i = 0; i < length; i++) {
        data[i] = buffer[readIndex];
        readIndex = (readIndex + 1) % BUFFER_SIZE;
    }
    samplesAvailable -= length;
    return true;
}

int AudioMemoryBuffer::available() const {
    if (!buffer) return 0;
    return samplesAvailable;
}

void AudioMemoryBuffer::clear() {
    if (!buffer) return;
    writeIndex = 0;
    readIndex = 0;
    samplesAvailable = 0;
    memset(buffer, 0, BUFFER_SIZE * sizeof(int16_t));
}

// =======================
// Wake Word Detection
// =======================

static const float MIN_THRESHOLD = 300.0f;        
static const float THRESHOLD_MULTIPLIER = 2.5f;   
static const int REQUIRED_FRAMES = 3;             
static const float AMBIENT_DECAY = 0.98f;         
static const float AMBIENT_RISE = 0.05f;          

static float ambientNoise = 200.0f;              
static float adaptiveThreshold = 500.0f;          
static int consecutiveFrames = 0;                 
static float wakeLastConfidence = 0.0f;               
static bool calibrated = false;                   
static int calibrationFrames = 0;                 
static const int CALIBRATION_FRAMES = 20;         

void WakeInit() {
  ambientNoise = 200.0f;
  adaptiveThreshold = 500.0f;
  consecutiveFrames = 0;
  wakeLastConfidence = 0.0f;
  calibrated = false;
  calibrationFrames = 0;
}

bool WakeDetect() {
  if (!AudioIsListening()) return false;
  
  float rms = AudioGetRms();
  wakeLastConfidence = rms;
  
  if (!calibrated) {
    ambientNoise = (ambientNoise * calibrationFrames + rms) / (calibrationFrames + 1);
    calibrationFrames++;
    if (calibrationFrames >= CALIBRATION_FRAMES) {
      calibrated = true;
      adaptiveThreshold = max(MIN_THRESHOLD, ambientNoise * THRESHOLD_MULTIPLIER);
      Serial.printf("[Wake] Calibrated: ambient=%.0f, threshold=%.0f\n", ambientNoise, adaptiveThreshold);
    }
    return false;
  }
  
  adaptiveThreshold = max(MIN_THRESHOLD, ambientNoise * THRESHOLD_MULTIPLIER);
  
  if (rms > adaptiveThreshold) {
    consecutiveFrames++;
    if (consecutiveFrames >= REQUIRED_FRAMES) {
      consecutiveFrames = 0;
      return true;  
    }
  } else {
    consecutiveFrames = 0;
    
    if (rms < ambientNoise) {
      ambientNoise = ambientNoise * AMBIENT_DECAY + rms * (1.0f - AMBIENT_DECAY);
    } else if (rms < adaptiveThreshold * 0.7f) {
      ambientNoise = ambientNoise * (1.0f - AMBIENT_RISE) + rms * AMBIENT_RISE;
    }
  }
  
  return false;
}

void WakeSetThreshold(float thresh) {
  adaptiveThreshold = max(thresh, MIN_THRESHOLD);
}

float WakeGetConfidence() {
  return wakeLastConfidence;
}

float WakeGetAmbientNoise() {
  return ambientNoise;
}

float WakeGetThreshold() {
  return adaptiveThreshold;
}

// =======================
// Realtime Voice AI
// =======================

static const size_t AUDIO_BUFFER_SIZE = 1024; 

static WebSocketsClient WsClient;
static bool rt_IsConnected = false;
static bool rt_IsListening = false;
static uint8_t rt_Volume = 100;

static uint8_t MicBuffer[AUDIO_BUFFER_SIZE];
static AudioMemoryBuffer PlaybackBuffer;       
static int16_t TempPlaybackChunk[512];         

static unsigned long LastPingTime = 0;
static const unsigned long PING_INTERVAL = 30000;

static void OnWsEvent(WStype_t Type, uint8_t* Payload, size_t Length);
static void ProcessAudioChunk(uint8_t* Data, size_t Length);
static void StreamMicData();
static void PlayBufferedAudio();
static void SendInstruction(const char* Msg);

void RealtimeVoiceInit() {
  Serial.println("[RealtimeVoice] Initialized");
  if (!PlaybackBuffer.init()) {
    Serial.println("[RealtimeVoice] Buffer init failed!");
  }
  rt_IsConnected = false;
  rt_IsListening = false;
  rt_Volume = 100;
}
bool RealtimeVoiceConnect(const char* ServerUrl) {
  static char loadUrl[64];
  if (!ConfigLoadServerUrl(loadUrl)) {
    Serial.println("[RealtimeVoice] No server URL configured");
    return false;
  }
  
  Serial.print("[RealtimeVoice] Connecting to: ");
  Serial.println(loadUrl);
  
  static char Host[64];
  static char Path[32];
  uint16_t Port = 8000;
  bool UseSSL = false;
  
  const char* p = loadUrl;
  
  if (strncmp(p, "wss://", 6) == 0) {
    UseSSL = true;
    p += 6;
    Port = 443;
  } else if (strncmp(p, "ws://", 5) == 0) {
    UseSSL = false;
    p += 5;
    Port = 80;
  }
  
  const char* pathStart = strchr(p, '/');
  const char* portStart = strchr(p, ':');
  
  size_t hostLen;
  if (portStart && (!pathStart || portStart < pathStart)) {
    hostLen = portStart - p;
    Port = atoi(portStart + 1);
  } else if (pathStart) {
    hostLen = pathStart - p;
  } else {
    hostLen = strlen(p);
  }
  hostLen = min(hostLen, sizeof(Host) - 1);
  memcpy(Host, p, hostLen);
  Host[hostLen] = '\0';
  
  if (pathStart) {
    strncpy(Path, pathStart, sizeof(Path) - 1);
    Path[sizeof(Path) - 1] = '\0';
  } else {
    strcpy(Path, "/ws");
  }
  
  Serial.printf("[RealtimeVoice] Host: %s, Port: %d, Path: %s, SSL: %s\n", Host, Port, Path, UseSSL ? "Yes" : "No");
  
  WsClient.onEvent(OnWsEvent);
  WsClient.setReconnectInterval(5000);
  WsClient.enableHeartbeat(15000, 5000, 2); 
  
  if (UseSSL) {
    WsClient.beginSSL(Host, Port, Path);
  } else {
    WsClient.begin(Host, Port, Path);
  }
  return true;
}

void RealtimeVoiceDisconnect() {
  WsClient.disconnect();
  rt_IsConnected = false;
  rt_IsListening = false;
  PlaybackBuffer.clear();
  Serial.println("[RealtimeVoice] Disconnected");
}

bool RealtimeVoiceIsConnected() {
  return rt_IsConnected;
}

void RealtimeVoiceLoop() {
  WsClient.loop();
  
  if (rt_IsConnected && rt_IsListening) {
    StreamMicData();
  }
  
  PlayBufferedAudio();
  
  if (rt_IsConnected && (millis() - LastPingTime > PING_INTERVAL)) {
    SendInstruction("ping");
    LastPingTime = millis();
  }
}

void RealtimeVoiceStartListening() {
  if (!rt_IsConnected) return;
  rt_IsListening = true;
  Serial.println("[RealtimeVoice] Started listening");
}

void RealtimeVoiceStopListening() {
  rt_IsListening = false;
  Serial.println("[RealtimeVoice] Stopped listening");
}

bool RealtimeVoiceIsListening() {
  return rt_IsListening;
}

void RealtimeVoiceEndOfSpeech() {
  if (!rt_IsConnected) return;
  SendInstruction("end_of_speech");
  Serial.println("[RealtimeVoice] End of speech signaled");
}

void RealtimeVoiceInterrupt() {
  if (!rt_IsConnected) return;
  
  PlaybackBuffer.clear();
  
  JsonDocument Doc;
  Doc["type"] = "instruction";
  Doc["msg"] = "INTERRUPT";
  Doc["audio_end_ms"] = 0; 
  
  String Json;
  serializeJson(Doc, Json);
  WsClient.sendTXT(Json);
  
  Serial.println("[RealtimeVoice] Interrupted");
}

void RealtimeVoiceSetVolume(uint8_t NewVolume) {
  rt_Volume = NewVolume > 100 ? 100 : NewVolume;
}

uint8_t RealtimeVoiceGetVolume() {
  return rt_Volume;
}

static void OnWsEvent(WStype_t Type, uint8_t* Payload, size_t Length) {
  switch (Type) {
    case WStype_DISCONNECTED:
      rt_IsConnected = false;
      rt_IsListening = false;
      Serial.println("[RealtimeVoice] WebSocket disconnected");
      break;
      
    case WStype_CONNECTED: {
      rt_IsConnected = true;
      LastPingTime = millis();
      PlaybackBuffer.clear(); 
      Serial.println("[RealtimeVoice] WebSocket connected");
      
      JsonDocument configDoc;
      configDoc["type"] = "config";
      configDoc["voice"] = "coral";  
      configDoc["language"] = "en";  
      
      String configJson;
      serializeJson(configDoc, configJson);
      WsClient.sendTXT(configJson);
      Serial.println("[RealtimeVoice] Sent config to server");
      break;
    }
      
    case WStype_TEXT: {
      JsonDocument Doc;
      DeserializationError Error = deserializeJson(Doc, Payload, Length);
      
      if (Error) {
        Serial.println("[RealtimeVoice] JSON parse error");
        return;
      }
      
      const char* MsgType = Doc["type"];
      const char* Msg = Doc["msg"];
      
      if (MsgType && strcmp(MsgType, "auth") == 0) {
        Serial.println("[RealtimeVoice] Authenticated with server");
      } else if (MsgType && strcmp(MsgType, "server") == 0) {
        if (Msg && strcmp(Msg, "RESPONSE.COMPLETE") == 0) {
          Serial.println("[RealtimeVoice] AI response complete");
        } else if (Msg && strcmp(Msg, "AUDIO.COMMITTED") == 0) {
          Serial.println("[RealtimeVoice] Audio committed");
        }
      } else if (MsgType && strcmp(MsgType, "error") == 0) {
        const char* ErrorMsg = Doc["message"];
        Serial.printf("[RealtimeVoice] Error: %s\n", ErrorMsg ? ErrorMsg : "Unknown");
      }
      break;
    }
      
    case WStype_BIN:
      ProcessAudioChunk(Payload, Length);
      break;
      
    case WStype_PING:
    case WStype_PONG:
      break;
      
    case WStype_ERROR:
      Serial.println("[RealtimeVoice] WebSocket error");
      break;
      
    default:
      break;
  }
}

static void ProcessAudioChunk(uint8_t* Data, size_t Length) {
  if (Length % 2 != 0) {
    return;
  }

  int16_t* Samples = (int16_t*)Data;
  size_t SampleCount = Length / 2;
  
  for (size_t I = 0; I < SampleCount; I++) {
    int32_t Sample = Samples[I];
    Sample = (Sample * rt_Volume) / 100;
    Samples[I] = (int16_t)Sample;
  }
  
  if (!PlaybackBuffer.write(Samples, SampleCount)) {
    Serial.println("[RealtimeVoice] Playback buffer overflow");
  }
}

static void StreamMicData() {
  size_t BytesRead = I2SReadMic(MicBuffer, AUDIO_BUFFER_SIZE);
  
  if (BytesRead > 0) {
    WsClient.sendBIN(MicBuffer, BytesRead);
  }
}

static void PlayBufferedAudio() {
  const int ChunkSamples = 512; 
  
  if (PlaybackBuffer.available() >= ChunkSamples) {
    if (PlaybackBuffer.read(TempPlaybackChunk, ChunkSamples)) {
       I2SWriteSpeaker((uint8_t*)TempPlaybackChunk, ChunkSamples * 2);
    }
  }
}

static void SendInstruction(const char* Msg) {
  JsonDocument Doc;
  Doc["type"] = "instruction";
  Doc["msg"] = Msg;
  
  String Json;
  serializeJson(Doc, Json);
  WsClient.sendTXT(Json);
}
