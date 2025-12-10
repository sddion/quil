/// <reference lib="deno.ns" />

import { EnergyVad } from "../lib/audio/EnergyVad.ts";
import { RestClient } from "../lib/OpenAI/RestClient.ts";
import { ChunkAudioData } from "../lib/audio/AudioUtils.ts";
import { GetQuilPersona, DefaultVoice, DefaultLanguage, VadConfig } from "../lib/Config.ts";

export interface IEsp32Session {
    Ws: WebSocket;
    Vad: EnergyVad;
    RestClient: RestClient;
    IsActive: boolean;
    Voice: string;
    Language: string;
}

const ActiveSessions = new Map<string, IEsp32Session>();

export function HandleEsp32Connection(Ws: WebSocket): void {
    const SessionId = crypto.randomUUID();
    console.log(`[ESP32] New connection: ${SessionId}`);

    const Session: IEsp32Session = {
        Ws,
        Vad: new EnergyVad(VadConfig),
        RestClient: new RestClient(),
        IsActive: true,
        Voice: DefaultVoice,
        Language: DefaultLanguage,
    };

    ActiveSessions.set(SessionId, Session);

    // Setup VAD Handlers
    Session.Vad.onSpeechStart = () => {
        console.log(`[Session ${SessionId}] User started speaking...`);
    };

    Session.Vad.onSpeechEnd = async (AudioBuffer: Uint8Array) => {
        console.log(`[Session ${SessionId}] Speech processing... (${AudioBuffer.length} bytes)`);
        
        // Send to OpenAI
        const ResponseAudio = await Session.RestClient.SendAudioInteraction(AudioBuffer, Session.Voice, GetQuilPersona(Session.Language));
        
        if (ResponseAudio) {
            console.log(`[Session ${SessionId}] Playing response...`);
            // Stream back to ESP32
            // We need to chunk it because ESP32 buffer is small
             const Chunks = ChunkAudioData(ResponseAudio, 1024);
             for (const Chunk of Chunks) {
                 if (Ws.readyState === WebSocket.OPEN) {
                     Ws.send(Chunk);
                 }
             }
        } else {
            console.log(`[Session ${SessionId}] No audio response.`);
        }
    };

    // WebSocket Handlers
    Ws.onmessage = (Event: MessageEvent) => {
        try {
            if (typeof Event.data === "string") {
                const Message = JSON.parse(Event.data);
                if (Message.type === "config") {
                     Session.Voice = Message.voice || DefaultVoice;
                     Session.Language = Message.language || DefaultLanguage;
                     console.log(`[Session ${SessionId}] Configured: voice=${Session.Voice}, language=${Session.Language}`);
                     // Send Auth/Ready
                     Ws.send(JSON.stringify({ type: "auth", status: "connected", voice: Session.Voice, language: Session.Language }));
                } else if (Message.type === "instruction" && Message.msg === "ping") {
                    Ws.send(JSON.stringify({ type: "pong" }));
                }
            } else if (Event.data instanceof ArrayBuffer) {
                // Audio Data
                const Chunk = new Uint8Array(Event.data);
                Session.Vad.Process(Chunk);
            }
        } catch (Err) {
            console.error("Handler Error:", Err);
        }
    };

    Ws.onclose = () => {
        console.log(`[ESP32] Connection closed: ${SessionId}`);
        ActiveSessions.delete(SessionId);
    };

    Ws.onerror = (Err) => {
        console.error(`[ESP32] WebSocket error:`, Err);
    };

    // Send Ready
    Ws.send(JSON.stringify({ 
        type: "ready", 
        message: "REST API Mode Ready",
        defaultVoice: DefaultVoice 
    }));
}

export function GetActiveSessionCount(): number {
    return ActiveSessions.size;
}
