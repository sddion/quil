// Voice options for Quil AI assistant
export interface VoiceOption {
  id: string;
  name: string;
  gender: 'male' | 'female';
  description: string;
}

export const VOICE_OPTIONS: VoiceOption[] = [
  { id: 'shimmer', name: 'Shimmer', gender: 'female', description: 'Warm & friendly' },
  { id: 'nova', name: 'Nova', gender: 'female', description: 'Clear & expressive' },
  { id: 'echo', name: 'Echo', gender: 'male', description: 'Calm & conversational' },
  { id: 'onyx', name: 'Onyx', gender: 'male', description: 'Deep & confident' },
];

// Language options supported by OpenAI Realtime API
export interface LanguageOption {
  code: string;
  name: string;
  region?: string;
}

export const LANGUAGE_OPTIONS: LanguageOption[] = [
  { code: 'en', name: 'English', region: 'US' },
  { code: 'en-GB', name: 'English', region: 'UK' },
  { code: 'es', name: 'Spanish' },
  { code: 'fr', name: 'French' },
  { code: 'de', name: 'German' },
  { code: 'it', name: 'Italian' },
  { code: 'pt', name: 'Portuguese' },
  { code: 'ja', name: 'Japanese' },
  { code: 'ko', name: 'Korean' },
  { code: 'zh', name: 'Chinese' },
  { code: 'hi', name: 'Hindi' },
  { code: 'ar', name: 'Arabic' },
  { code: 'ru', name: 'Russian' },
  { code: 'nl', name: 'Dutch' },
  { code: 'pl', name: 'Polish' },
  { code: 'tr', name: 'Turkish' },
  { code: 'sv', name: 'Swedish' },
  { code: 'th', name: 'Thai' },
  { code: 'vi', name: 'Vietnamese' },
  { code: 'id', name: 'Indonesian' },
  { code: 'ta', name: 'Tamil' },
  { code: 'te', name: 'Telugu' },
  { code: 'bn', name: 'Bengali' },
  { code: 'uk', name: 'Ukrainian' },
  { code: 'el', name: 'Greek' },
  { code: 'he', name: 'Hebrew' },
  { code: 'cs', name: 'Czech' },
  { code: 'ro', name: 'Romanian' },
  { code: 'hu', name: 'Hungarian' },
];
