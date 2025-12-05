/// <reference lib="deno.ns" />

// Audio Utilities for ESP32
// Handles base64 encoding/decoding and PCM audio processing

export function EncodeBase64(Data: Uint8Array): string {
    let Binary = "";
    const Bytes = new Uint8Array(Data);
    for (let I = 0; I < Bytes.byteLength; I++) {
        Binary += String.fromCharCode(Bytes[I]);
    }
    return btoa(Binary);
}

export function DecodeBase64(Base64String: string): Uint8Array {
    const Binary = atob(Base64String);
    const Bytes = new Uint8Array(Binary.length);
    for (let I = 0; I < Binary.length; I++) {
        Bytes[I] = Binary.charCodeAt(I);
    }
    return Bytes;
}

// Chunk audio data for ESP32-friendly transmission (1024 bytes per chunk)
export function ChunkAudioData(Data: Uint8Array, ChunkSize: number = 1024): Uint8Array[] {
    const Chunks: Uint8Array[] = [];
    for (let I = 0; I < Data.length; I += ChunkSize) {
        const End = Math.min(I + ChunkSize, Data.length);
        Chunks.push(Data.slice(I, End));
    }
    return Chunks;
}
