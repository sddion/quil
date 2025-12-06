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

// Voice Options
export interface IVoiceOption {
    Id: string;
    Name: string;
    Gender: "male" | "female";
    Description: string;
}

export const VoiceOptions: IVoiceOption[] = [
    { Id: "shimmer", Name: "Shimmer", Gender: "female", Description: "Warm & friendly" },
    { Id: "nova", Name: "Nova", Gender: "female", Description: "Clear & expressive" },
    { Id: "echo", Name: "Echo", Gender: "male", Description: "Calm & conversational" },
    { Id: "onyx", Name: "Onyx", Gender: "male", Description: "Deep & confident" },
];

export const DefaultVoice: string = "shimmer"; // Best for Quil's warm, caring personality

// Supported Languages
export interface ILanguageOption {
    Code: string;
    Name: string;
    Region?: string;
}

export const SupportedLanguages: ILanguageOption[] = [
    { Code: "en", Name: "English", Region: "US" },
    { Code: "en-GB", Name: "English", Region: "UK" },
    { Code: "en-AU", Name: "English", Region: "Australia" },
    { Code: "es", Name: "Spanish", Region: "Spain" },
    { Code: "es-MX", Name: "Spanish", Region: "Mexico" },
    { Code: "fr", Name: "French", Region: "France" },
    { Code: "fr-CA", Name: "French", Region: "Canada" },
    { Code: "de", Name: "German" },
    { Code: "it", Name: "Italian" },
    { Code: "pt", Name: "Portuguese", Region: "Brazil" },
    { Code: "pt-PT", Name: "Portuguese", Region: "Portugal" },
    { Code: "ja", Name: "Japanese" },
    { Code: "ko", Name: "Korean" },
    { Code: "zh", Name: "Chinese", Region: "Mandarin" },
    { Code: "hi", Name: "Hindi" },
    { Code: "ar", Name: "Arabic" },
    { Code: "ru", Name: "Russian" },
    { Code: "nl", Name: "Dutch" },
    { Code: "pl", Name: "Polish" },
    { Code: "tr", Name: "Turkish" },
    { Code: "sv", Name: "Swedish" },
    { Code: "da", Name: "Danish" },
    { Code: "no", Name: "Norwegian" },
    { Code: "fi", Name: "Finnish" },
    { Code: "th", Name: "Thai" },
    { Code: "vi", Name: "Vietnamese" },
    { Code: "id", Name: "Indonesian" },
    { Code: "ms", Name: "Malay" },
    { Code: "ta", Name: "Tamil" },
    { Code: "te", Name: "Telugu" },
    { Code: "bn", Name: "Bengali" },
    { Code: "uk", Name: "Ukrainian" },
    { Code: "el", Name: "Greek" },
    { Code: "he", Name: "Hebrew" },
    { Code: "cs", Name: "Czech" },
    { Code: "ro", Name: "Romanian" },
    { Code: "hu", Name: "Hungarian" },
];

export const DefaultLanguage: string = "en";

// Quil Persona (language-aware)
export function GetQuilPersona(language: string): string {
    const languageName = SupportedLanguages.find(l => l.Code === language)?.Name ?? "English";
    
    return `You are Quil, a warm and friendly voice companion. You have a gentle, supportive personality and genuinely care about the person you're speaking with.

Key traits:
- Always introduce yourself as "Quil" when appropriate
- Speak naturally and conversationally, like a caring friend
- Be encouraging and positive, but authentic
- Listen actively and remember what the user shares
- Keep responses concise for voice interaction (1-3 sentences usually)
- Use a warm, friendly tone - never robotic or clinical
- IMPORTANT: Always respond in ${languageName}. The user prefers to communicate in ${languageName}.

You love helping people think through their day, offering a listening ear, or just having a nice chat. You're curious about the person you're talking to and enjoy learning about their interests and experiences.`;
}

export const QuilVoice: string = DefaultVoice;

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

