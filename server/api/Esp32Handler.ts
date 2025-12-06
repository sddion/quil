/// <reference lib="deno.ns" />

// ESP32 WebSocket Handler
// Manages WebSocket connections from ESP32 devices and bridges to OpenAI Realtime API

import { RealtimeClient, type IRealtimeEvent } from "../lib/OpenAI/RealtimeClient.ts";
import { EncodeBase64, DecodeBase64, ChunkAudioData } from "../lib/audio/AudioUtils.ts";
import { GetQuilPersona, DefaultVoice, DefaultLanguage, VoiceOptions, VadConfig, IsDevMode } from "../lib/Config.ts";

export interface IEsp32Session {
    Ws: WebSocket;
    OpenAiClient: RealtimeClient;
    CurrentItemId: string | null;
    IsActive: boolean;
    Voice: string;
    Language: string;
    IsConfigured: boolean;
}

const ActiveSessions: Map<string, IEsp32Session> = new Map();

export function HandleEsp32Connection(Ws: WebSocket, SessionId: string): void {
    console.log(`[ESP32] New connection: ${SessionId}`);

    const OpenAiClient = new RealtimeClient();
    const Session: IEsp32Session = {
        Ws: Ws,
        OpenAiClient: OpenAiClient,
        CurrentItemId: null,
        IsActive: true,
        Voice: DefaultVoice,
        Language: DefaultLanguage,
        IsConfigured: false,
    };

    ActiveSessions.set(SessionId, Session);

    // Setup OpenAI event handlers
    SetupOpenAiHandlers(Session, SessionId);

    // Setup ESP32 WebSocket handlers
    SetupEsp32Handlers(Session, SessionId);

    // Send ready message - wait for config before connecting to OpenAI
    SendToEsp32(Ws, { 
        type: "ready", 
        message: "Send config to start session",
        defaultVoice: DefaultVoice,
        defaultLanguage: DefaultLanguage,
        voices: VoiceOptions,
    });
}

async function ConnectToOpenAI(Session: IEsp32Session, SessionId: string): Promise<void> {
    const { Ws, OpenAiClient, Voice, Language } = Session;
    
    // Validate voice
    const validVoice = VoiceOptions.find(v => v.Id === Voice)?.Id ?? DefaultVoice;
    
    try {
        await OpenAiClient.Connect({
            Model: "gpt-4o-mini-realtime-preview-2024-12-17",
            Voice: validVoice,
            Instructions: GetQuilPersona(Language),
            InputAudioTranscription: { Model: "whisper-1" },
            TurnDetection: {
                Type: "server_vad",
                Threshold: VadConfig.Threshold,
                PrefixPaddingMs: VadConfig.PrefixPaddingMs,
                SilenceDurationMs: VadConfig.SilenceDurationMs,
            },
        });
        console.log(`[ESP32] Session ${SessionId} connected to OpenAI (voice: ${validVoice}, lang: ${Language})`);

        // Send auth confirmation to ESP32
        SendToEsp32(Ws, { type: "auth", status: "connected", voice: validVoice, language: Language });
        Session.IsConfigured = true;
    } catch (Error) {
        console.error(`[ESP32] Failed to connect to OpenAI:`, Error);
        SendToEsp32(Ws, { type: "error", message: "Failed to connect to AI" });
        CleanupSession(SessionId);
    }
}

function SetupOpenAiHandlers(Session: IEsp32Session, SessionId: string): void {
    const { OpenAiClient, Ws } = Session;

    // Session created
    OpenAiClient.On("session.created", () => {
        console.log(`[ESP32] Session ${SessionId}: OpenAI session created`);
    });

    // Handle audio response from OpenAI (uses lowercase 'delta')
    OpenAiClient.On("response.audio.delta", (Event: IRealtimeEvent) => {
        const Delta = Event.delta as string | undefined;
        if (Delta) {
            // Decode base64 audio and send to ESP32 in chunks
            const AudioData = DecodeBase64(Delta);
            const Chunks = ChunkAudioData(AudioData, 1024);
            for (const Chunk of Chunks) {
                if (Session.IsActive && Ws.readyState === WebSocket.OPEN) {
                    Ws.send(Chunk);
                }
            }
        }
    });

    // Track current response item for interruption
    OpenAiClient.On("response.output_item.added", (Event: IRealtimeEvent) => {
        const Item = Event.item as { id?: string } | undefined;
        if (Item?.id) {
            Session.CurrentItemId = Item.id;
        }
    });

    // Response complete
    OpenAiClient.On("response.done", () => {
        SendToEsp32(Ws, { type: "server", msg: "RESPONSE.COMPLETE" });
    });

    // Audio buffer committed (user finished speaking)
    OpenAiClient.On("input_audio_buffer.committed", () => {
        SendToEsp32(Ws, { type: "server", msg: "AUDIO.COMMITTED" });
    });

    // User transcription complete
    OpenAiClient.On("conversation.item.input_audio_transcription.completed", (Event: IRealtimeEvent) => {
        const Transcript = Event.transcript as string | undefined;
        if (Transcript && IsDevMode) {
            console.log(`[ESP32] User said: ${Transcript}`);
        }
    });

    // AI response transcription
    OpenAiClient.On("response.audio_transcript.done", (Event: IRealtimeEvent) => {
        const Transcript = Event.transcript as string | undefined;
        if (Transcript && IsDevMode) {
            console.log(`[ESP32] Quil said: ${Transcript}`);
        }
    });

    // Handle errors
    OpenAiClient.On("error", (Event: IRealtimeEvent) => {
        console.error(`[ESP32] OpenAI error:`, Event);
        SendToEsp32(Ws, { type: "error", message: "AI processing error" });
    });

    // Handle close
    OpenAiClient.On("close", () => {
        console.log(`[ESP32] Session ${SessionId}: OpenAI connection closed`);
        CleanupSession(SessionId);
    });
}

function SetupEsp32Handlers(Session: IEsp32Session, SessionId: string): void {
    const { Ws, OpenAiClient } = Session;

    Ws.onmessage = (Event: MessageEvent) => {
        if (!Session.IsActive) return;

        // Binary data = audio from ESP32 microphone
        if (Event.data instanceof ArrayBuffer || Event.data instanceof Uint8Array) {
            const AudioData = Event.data instanceof ArrayBuffer
                ? new Uint8Array(Event.data)
                : Event.data;
            const Base64Audio = EncodeBase64(AudioData);
            OpenAiClient.SendAudio(Base64Audio);
            return;
        }

        // Text data = control messages from ESP32
        try {
            const Message = JSON.parse(Event.data as string);
            HandleEsp32Message(Session, Message, SessionId);
        } catch {
            console.error(`[ESP32] Invalid message from ${SessionId}:`, Event.data);
        }
    };

    Ws.onerror = (Error: Event) => {
        console.error(`[ESP32] WebSocket error for ${SessionId}:`, Error);
    };

    Ws.onclose = () => {
        console.log(`[ESP32] Connection closed: ${SessionId}`);
        CleanupSession(SessionId);
    };
}

function HandleEsp32Message(Session: IEsp32Session, Message: Record<string, unknown>, SessionId: string): void {
    const { OpenAiClient } = Session;
    const MsgType = Message.type as string;
    const Msg = Message.msg as string;

    // Handle config message - sets voice/language and connects to OpenAI
    if (MsgType === "config") {
        const voice = (Message.voice as string) ?? DefaultVoice;
        const language = (Message.language as string) ?? DefaultLanguage;
        
        Session.Voice = voice;
        Session.Language = language;
        
        if (!Session.IsConfigured) {
            console.log(`[ESP32] Session ${SessionId} config: voice=${voice}, language=${language}`);
            ConnectToOpenAI(Session, SessionId);
        }
        return;
    }

    // Handle instruction messages
    if (MsgType === "instruction") {
        switch (Msg) {
            case "end_of_speech":
                // Manual VAD: user finished speaking
                OpenAiClient.CommitAudio();
                OpenAiClient.CreateResponse();
                OpenAiClient.ClearAudioBuffer();
                break;

            case "INTERRUPT": {
                // User interrupted AI response
                const AudioEndMs = (Message.audio_end_ms ?? 0) as number;
                if (Session.CurrentItemId) {
                    OpenAiClient.TruncateResponse(Session.CurrentItemId, 0, AudioEndMs);
                }
                OpenAiClient.ClearAudioBuffer();
                break;
            }

            case "ping":
                SendToEsp32(Session.Ws, { type: "pong" });
                break;
        }
    }
}

function SendToEsp32(Ws: WebSocket, Data: Record<string, unknown>): void {
    if (Ws.readyState === WebSocket.OPEN) {
        Ws.send(JSON.stringify(Data));
    }
}

function CleanupSession(SessionId: string): void {
    const Session = ActiveSessions.get(SessionId);
    if (Session) {
        Session.IsActive = false;
        Session.OpenAiClient.Disconnect();
        ActiveSessions.delete(SessionId);
        console.log(`[ESP32] Session ${SessionId} cleaned up`);
    }
}

export function GetActiveSessionCount(): number {
    return ActiveSessions.size;
}
