#pragma once
#include <Arduino.h>
#include <WebSocketsClient.h> // Required for RealtimeVoice public interface if types are exposed, or forward declare

// --- Audio Manager ---
// Combines VoiceManager, WakeManager, AudioMemoryBuffer, and RealtimeVoice

// Core Audio Functions
void AudioInit();
void AudioStartListening();
void AudioStopListening();
bool AudioIsListening();
size_t AudioReadBuffer(uint8_t* buf, size_t len);
void AudioPlayResponse(const uint8_t* data, size_t len);
float AudioGetRms();

// Audio Memory Buffer Class
class AudioMemoryBuffer {
private:
    static const int BUFFER_SIZE = 16384; 
    int16_t* buffer;
    int writeIndex = 0;
    int readIndex = 0; 
    int samplesAvailable = 0;

public:
    AudioMemoryBuffer();
    bool init();
    bool write(const int16_t* data, int length);
    bool read(int16_t* data, int length);
    int available() const;
    void clear();
    int size() const { return BUFFER_SIZE; }
};

// Wake Word Detection
void WakeInit(); // Might be internal to AudioInit
bool WakeDetect();
void WakeSetThreshold(float thresh);
float WakeGetConfidence();
float WakeGetAmbientNoise();
float WakeGetThreshold();

// Realtime Voice AI
void RealtimeVoiceInit(); // Might be internal
bool RealtimeVoiceConnect(const char* ServerUrl);
void RealtimeVoiceDisconnect();
bool RealtimeVoiceIsConnected();
void RealtimeVoiceLoop();
void RealtimeVoiceStartListening();
void RealtimeVoiceStopListening();
bool RealtimeVoiceIsListening();
void RealtimeVoiceEndOfSpeech();
void RealtimeVoiceInterrupt();
void RealtimeVoiceSetVolume(uint8_t Volume);
uint8_t RealtimeVoiceGetVolume();
