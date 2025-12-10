/// <reference lib="deno.ns" />

// OpenAI Realtime WebSocket Client
// Handles connection, message routing, and event handling
// Based on OpenAI Realtime API documentation

import { OpenAiApiKey, OpenAiRealtimeUrl, OpenAiModel } from "../Config.ts";

export interface ISessionConfig {
    Model: string;
    Voice: string;
    Instructions: string;
    InputAudioTranscription: { Model: string };
    TurnDetection: {
        Type: "server_vad";
        Threshold: number;
        PrefixPaddingMs: number;
        SilenceDurationMs: number;
    } | null;
}

// OpenAI event uses lowercase keys
export interface IRealtimeEvent {
    event_id?: string;
    type: string;
    [key: string]: unknown;
}

export type EventHandler = (Event: IRealtimeEvent) => void | Promise<void>;

export class RealtimeClient {
    private Socket: WebSocket | null = null;
    private EventHandlers: Map<string, EventHandler[]> = new Map();
    private IsConnected: boolean = false;
    private MessageQueue: IRealtimeEvent[] = [];

    constructor() { }

  public Connect(SessionConfig: Partial<ISessionConfig>): Promise<void> {
        const Url = `${OpenAiRealtimeUrl}?model=${SessionConfig.Model ?? OpenAiModel}`;

        return new Promise((Resolve, Reject) => {
            this.Socket = new WebSocket(Url, [
                "realtime",
                `openai-insecure-api-key.${OpenAiApiKey}`,
                "openai-beta.realtime-v1",
            ]);

            this.Socket.onopen = () => {
                console.log("[RealtimeClient] Connected to OpenAI");
                this.IsConnected = true;
                this.ConfigureSession(SessionConfig);
                this.ProcessMessageQueue();
                Resolve();
            };

            this.Socket.onmessage = (Event: MessageEvent) => {
                this.HandleMessage(Event);
            };

            this.Socket.onerror = (Error: Event) => {
                console.error("[RealtimeClient] WebSocket error:", Error);
                Reject(Error);
            };

            this.Socket.onclose = (Event: CloseEvent) => {
                console.log("[RealtimeClient] Connection closed:", Event.code, Event.reason);
                this.IsConnected = false;
                this.EmitEvent({ type: "close", code: Event.code, reason: Event.reason });
            };
        });
    }

    private ConfigureSession(Config: Partial<ISessionConfig>): void {
        // OpenAI expects lowercase keys in the session object
        const SessionUpdate = {
            type: "session.update",
            session: {
                modalities: ["audio", "text"],
                voice: Config.Voice ?? "alloy",
                instructions: Config.Instructions ?? "",
                input_audio_format: "pcm16",
                output_audio_format: "pcm16",
                // input_audio_transcription: {
                //     model: Config.InputAudioTranscription?.Model ?? "whisper-1"
                // },
                turn_detection: Config.TurnDetection ? {
                    type: Config.TurnDetection.Type,
                    threshold: Config.TurnDetection.Threshold,
                    prefix_padding_ms: Config.TurnDetection.PrefixPaddingMs,
                    silence_duration_ms: Config.TurnDetection.SilenceDurationMs,
                } : {
                    type: "server_vad",
                    threshold: 0.4,
                    prefix_padding_ms: 400,
                    silence_duration_ms: 1000,
                },
            },
        };
        this.SendRaw(SessionUpdate);
    }

    private HandleMessage(Event: MessageEvent): void {
        try {
            const Data: IRealtimeEvent = JSON.parse(Event.data as string);
            this.EmitEvent(Data);
        } catch (Error) {
            console.error("[RealtimeClient] Failed to parse message:", Error);
        }
    }

    private EmitEvent(Event: IRealtimeEvent): void {
        const EventType = Event.type;

        // Emit to specific handlers
        const Handlers = this.EventHandlers.get(EventType);
        if (Handlers) {
            for (const Handler of Handlers) {
                try {
                    Handler(Event);
                } catch (Error) {
                    console.error(`[RealtimeClient] Handler error for ${EventType}:`, Error);
                }
            }
        }

        // Emit to wildcard handlers
        const WildcardHandlers = this.EventHandlers.get("*");
        if (WildcardHandlers) {
            for (const Handler of WildcardHandlers) {
                try {
                    Handler(Event);
                } catch (Error) {
                    console.error("[RealtimeClient] Wildcard handler error:", Error);
                }
            }
        }
    }

    public On(EventType: string, Handler: EventHandler): void {
        if (!this.EventHandlers.has(EventType)) {
            this.EventHandlers.set(EventType, []);
        }
        this.EventHandlers.get(EventType)!.push(Handler);
    }

    public Off(EventType: string, Handler: EventHandler): void {
        const Handlers = this.EventHandlers.get(EventType);
        if (Handlers) {
            const Index = Handlers.indexOf(Handler);
            if (Index !== -1) {
                Handlers.splice(Index, 1);
            }
        }
    }

    private SendRaw(Data: unknown): void {
        if (!this.IsConnected || !this.Socket) {
            return;
        }
        try {
            this.Socket.send(JSON.stringify(Data));
        } catch (Error) {
            console.error("[RealtimeClient] Failed to send message:", Error);
        }
    }

    public Send(Event: IRealtimeEvent): void {
        if (!this.IsConnected || !this.Socket) {
            this.MessageQueue.push(Event);
            return;
        }

        try {
            this.Socket.send(JSON.stringify(Event));
        } catch (Error) {
            console.error("[RealtimeClient] Failed to send message:", Error);
        }
    }

    private ProcessMessageQueue(): void {
        while (this.MessageQueue.length > 0) {
            const Event = this.MessageQueue.shift();
            if (Event) {
                this.Send(Event);
            }
        }
    }

    public SendAudio(AudioData: string): void {
        // OpenAI Realtime API expects 'audio' key for input_audio_buffer.append
        this.SendRaw({
            type: "input_audio_buffer.append",
            audio: AudioData,
        });
    }

    public CommitAudio(): void {
        this.SendRaw({ type: "input_audio_buffer.commit" });
    }

    public ClearAudioBuffer(): void {
        this.SendRaw({ type: "input_audio_buffer.clear" });
    }

    public CreateResponse(): void {
        this.SendRaw({ type: "response.create" });
    }

    public CancelResponse(): void {
        this.SendRaw({ type: "response.cancel" });
    }

    public TruncateResponse(ItemId: string, ContentIndex: number, AudioEndMs: number): void {
        this.SendRaw({
            type: "conversation.item.truncate",
            item_id: ItemId,
            content_index: ContentIndex,
            audio_end_ms: AudioEndMs,
        });
    }

    public Disconnect(): void {
        if (this.Socket) {
            this.Socket.close();
            this.Socket = null;
        }
        this.IsConnected = false;
        this.MessageQueue = [];
        this.EventHandlers.clear();
    }

    public GetIsConnected(): boolean {
        return this.IsConnected;
    }
}

// Utility to generate unique IDs
export function GenerateEventId(Prefix: string = "evt_"): string {
    const Chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    let Result = Prefix;
    for (let I = 0; I < 16; I++) {
        Result += Chars.charAt(Math.floor(Math.random() * Chars.length));
    }
    return Result;
}
