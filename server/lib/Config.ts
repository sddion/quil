/// <reference lib="deno.ns" />

// Quil Server Configuration

export const OpenAiApiKey: string = Deno.env.get("OPENAI_API_KEY") ?? "";
export const OpenAiRealtimeUrl: string = "wss://api.openai.com/v1/realtime";
export const OpenAiModel: string = "gpt-4o-mini-realtime-preview-2024-12-17";

export const ServerHost: string = Deno.env.get("HOST") ?? "0.0.0.0";
export const ServerPort: number = parseInt(Deno.env.get("PORT") ?? "8000");
export const IsDevMode: boolean = Deno.env.get("DEV_MODE") === "true";

export const SupabaseUrl: string = Deno.env.get("SUPABASE_URL") ?? "";
export const SupabaseAnonKey: string = Deno.env.get("SUPABASE_ANON_KEY") ?? "";

// Audio Configuration
export const AudioSampleRate: number = 24000; // OpenAI Realtime uses 24kHz
export const AudioChannels: number = 1;
export const AudioBitsPerSample: number = 16;
export const FrameSize: number = 960; // 40ms at 24kHz (960 samples)

// WebSocket Configuration
export const WsPingInterval: number = 30000; // 30 seconds
export const WsReconnectDelay: number = 3000; // 3 seconds

// Quil Persona
export const QuilPersona: string = `You are Quil, a warm and friendly voice companion. You have a gentle, supportive personality and genuinely care about the person you're speaking with.

Key traits:
- Always introduce yourself as "Quil" when appropriate
- Speak naturally and conversationally, like a caring friend
- Be encouraging and positive, but authentic
- Listen actively and remember what the user shares
- Keep responses concise for voice interaction (1-3 sentences usually)
- Use a warm, friendly tone - never robotic or clinical

You love helping people think through their day, offering a listening ear, or just having a nice chat. You're curious about the person you're talking to and enjoy learning about their interests and experiences.`;

export const QuilVoice: string = "alloy";

// Server VAD Configuration
export interface IVadConfig {
    Type: "server_vad";
    Threshold: number;
    PrefixPaddingMs: number;
    SilenceDurationMs: number;
}

export const VadConfig: IVadConfig = {
    Type: "server_vad",
    Threshold: 0.4,
    PrefixPaddingMs: 400,
    SilenceDurationMs: 1000,
};
