#include "../h/RealtimeVoice.h"
#include "hal/h/I2S.h"
#include "pins.h"
#include <WiFi.h>
#include <WebSocketsClient.h>
#include <ArduinoJson.h>

// Configuration
static const uint32_t SAMPLE_RATE = 24000;  // OpenAI Realtime uses 24kHz
static const size_t AUDIO_BUFFER_SIZE = 1024;
static const size_t PLAYBACK_BUFFER_SIZE = 8192;

// State
static WebSocketsClient WsClient;
static bool IsConnected = false;
static bool IsListening = false;
static bool IsPlaying = false;
static uint8_t Volume = 100;

// Audio buffers
static uint8_t MicBuffer[AUDIO_BUFFER_SIZE];
static uint8_t PlaybackBuffer[PLAYBACK_BUFFER_SIZE];
static size_t PlaybackWritePos = 0;
static size_t PlaybackReadPos = 0;

// Timing
static unsigned long LastPingTime = 0;
static const unsigned long PING_INTERVAL = 30000;

// Forward declarations
static void OnWsEvent(WStype_t Type, uint8_t* Payload, size_t Length);
static void ProcessAudioChunk(uint8_t* Data, size_t Length);
static void StreamMicData();
static void PlayBufferedAudio();
static void SendInstruction(const char* Msg);

void RealtimeVoiceInit() {
  // Configure I2S for 24kHz (OpenAI Realtime API requirement)
  // Note: I2S will be reconfigured at the appropriate sample rate
  Serial.println("[RealtimeVoice] Initialized");
  IsConnected = false;
  IsListening = false;
  Volume = 100;
}

bool RealtimeVoiceConnect(const char* ServerUrl) {
  Serial.print("[RealtimeVoice] Connecting to: ");
  Serial.println(ServerUrl);
  
  // Parse URL (expects format: ws://host:port/path or wss://...)
  String Url = String(ServerUrl);
  bool UseSSL = Url.startsWith("wss://");
  
  // Remove protocol
  if (UseSSL) {
    Url = Url.substring(6);
  } else if (Url.startsWith("ws://")) {
    Url = Url.substring(5);
  }
  
  // Extract host, port, path
  int ColonIdx = Url.indexOf(':');
  int SlashIdx = Url.indexOf('/');
  
  String Host;
  uint16_t Port;
  String Path = "/ws";
  
  if (ColonIdx > 0 && (SlashIdx < 0 || ColonIdx < SlashIdx)) {
    Host = Url.substring(0, ColonIdx);
    if (SlashIdx > 0) {
      Port = Url.substring(ColonIdx + 1, SlashIdx).toInt();
      Path = Url.substring(SlashIdx);
    } else {
      Port = Url.substring(ColonIdx + 1).toInt();
    }
  } else if (SlashIdx > 0) {
    Host = Url.substring(0, SlashIdx);
    Port = UseSSL ? 443 : 80;
    Path = Url.substring(SlashIdx);
  } else {
    Host = Url;
    Port = UseSSL ? 443 : 80;
  }
  
  Serial.printf("[RealtimeVoice] Host: %s, Port: %d, Path: %s\n", 
                Host.c_str(), Port, Path.c_str());
  
  // Setup WebSocket
  WsClient.onEvent(OnWsEvent);
  WsClient.setReconnectInterval(5000);
  
  if (UseSSL) {
    WsClient.beginSSL(Host.c_str(), Port, Path.c_str());
  } else {
    WsClient.begin(Host.c_str(), Port, Path.c_str());
  }
  
  return true;
}

void RealtimeVoiceDisconnect() {
  WsClient.disconnect();
  IsConnected = false;
  IsListening = false;
  Serial.println("[RealtimeVoice] Disconnected");
}

bool RealtimeVoiceIsConnected() {
  return IsConnected;
}

void RealtimeVoiceLoop() {
  WsClient.loop();
  
  // Stream mic data if listening
  if (IsConnected && IsListening) {
    StreamMicData();
  }
  
  // Play buffered audio
  if (PlaybackWritePos > PlaybackReadPos) {
    PlayBufferedAudio();
  }
  
  // Periodic ping
  if (IsConnected && (millis() - LastPingTime > PING_INTERVAL)) {
    SendInstruction("ping");
    LastPingTime = millis();
  }
}

void RealtimeVoiceStartListening() {
  if (!IsConnected) return;
  IsListening = true;
  Serial.println("[RealtimeVoice] Started listening");
}

void RealtimeVoiceStopListening() {
  IsListening = false;
  Serial.println("[RealtimeVoice] Stopped listening");
}

bool RealtimeVoiceIsListening() {
  return IsListening;
}

void RealtimeVoiceEndOfSpeech() {
  if (!IsConnected) return;
  SendInstruction("end_of_speech");
  Serial.println("[RealtimeVoice] End of speech signaled");
}

void RealtimeVoiceInterrupt() {
  if (!IsConnected) return;
  
  // Clear playback buffer
  PlaybackWritePos = 0;
  PlaybackReadPos = 0;
  
  // Send interrupt with approximate audio position
  StaticJsonDocument<128> Doc;
  Doc["type"] = "instruction";
  Doc["msg"] = "INTERRUPT";
  Doc["audio_end_ms"] = 0; // TODO: Track actual playback position
  
  String Json;
  serializeJson(Doc, Json);
  WsClient.sendTXT(Json);
  
  Serial.println("[RealtimeVoice] Interrupted");
}

void RealtimeVoiceSetVolume(uint8_t NewVolume) {
  Volume = NewVolume > 100 ? 100 : NewVolume;
}

uint8_t RealtimeVoiceGetVolume() {
  return Volume;
}

// WebSocket event handler
static void OnWsEvent(WStype_t Type, uint8_t* Payload, size_t Length) {
  switch (Type) {
    case WStype_DISCONNECTED:
      IsConnected = false;
      IsListening = false;
      Serial.println("[RealtimeVoice] WebSocket disconnected");
      break;
      
    case WStype_CONNECTED:
      IsConnected = true;
      LastPingTime = millis();
      Serial.println("[RealtimeVoice] WebSocket connected");
      break;
      
    case WStype_TEXT: {
      // Parse JSON message from server
      StaticJsonDocument<512> Doc;
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
      } else if (MsgType && strcmp(MsgType, "pong") == 0) {
        // Ping response received
      } else if (MsgType && strcmp(MsgType, "error") == 0) {
        const char* ErrorMsg = Doc["message"];
        Serial.printf("[RealtimeVoice] Error: %s\n", ErrorMsg ? ErrorMsg : "Unknown");
      }
      break;
    }
      
    case WStype_BIN:
      // Binary audio data from server - add to playback buffer
      ProcessAudioChunk(Payload, Length);
      break;
      
    case WStype_PING:
      // Auto-handled by library
      break;
      
    case WStype_PONG:
      break;
      
    case WStype_ERROR:
      Serial.println("[RealtimeVoice] WebSocket error");
      break;
      
    default:
      break;
  }
}

// Process incoming audio chunk from server
static void ProcessAudioChunk(uint8_t* Data, size_t Length) {
  // Apply volume scaling
  int16_t* Samples = (int16_t*)Data;
  size_t SampleCount = Length / 2;
  
  for (size_t I = 0; I < SampleCount; I++) {
    int32_t Sample = Samples[I];
    Sample = (Sample * Volume) / 100;
    Samples[I] = (int16_t)Sample;
  }
  
  // Add to circular playback buffer
  size_t SpaceAvailable = PLAYBACK_BUFFER_SIZE - PlaybackWritePos;
  size_t ToCopy = Length < SpaceAvailable ? Length : SpaceAvailable;
  
  if (ToCopy > 0) {
    memcpy(PlaybackBuffer + PlaybackWritePos, Data, ToCopy);
    PlaybackWritePos += ToCopy;
  }
}

// Stream microphone data to server
static void StreamMicData() {
  size_t BytesRead = I2SReadMic(MicBuffer, AUDIO_BUFFER_SIZE);
  
  if (BytesRead > 0) {
    // Send raw PCM data as binary WebSocket message
    WsClient.sendBIN(MicBuffer, BytesRead);
  }
}

// Play buffered audio through speaker
static void PlayBufferedAudio() {
  size_t Available = PlaybackWritePos - PlaybackReadPos;
  
  if (Available >= 512) {
    size_t ToPlay = Available < 1024 ? Available : 1024;
    I2SWriteSpeaker(PlaybackBuffer + PlaybackReadPos, ToPlay);
    PlaybackReadPos += ToPlay;
    
    // Reset buffer positions when fully played
    if (PlaybackReadPos >= PlaybackWritePos) {
      PlaybackWritePos = 0;
      PlaybackReadPos = 0;
    }
  }
}

// Send instruction message to server
static void SendInstruction(const char* Msg) {
  StaticJsonDocument<128> Doc;
  Doc["type"] = "instruction";
  Doc["msg"] = Msg;
  
  String Json;
  serializeJson(Doc, Json);
  WsClient.sendTXT(Json);
}
