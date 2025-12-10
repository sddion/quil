export interface IVadConfig {
    Threshold: number;       // RMS Threshold (0.0 to 1.0)
    SilenceDurationMs: number; // How long silence must persist to commit
    PrefixPaddingMs: number;   // How much audio before trigger to keep
}

export type VadState = "silence" | "speech";

export class EnergyVad {
    private Config: IVadConfig;
    private buffer: Uint8Array[] = [];
    private state: VadState = "silence";
    private silenceStartTime: number = 0;
    private speechStartTime: number = 0;
    
    // RMS Calculation Constants
    private readonly MAX_AMPLITUDE = 32768; // 16-bit PCM

    // Event callbacks
    public onSpeechStart?: () => void;
    public onSpeechEnd?: (AudioBuffer: Uint8Array) => void;

    constructor(Config: IVadConfig) {
        this.Config = Config;
    }

    public Process(Chunk: Uint8Array): void {
        const rms = this.CalculateRMS(Chunk);
        
        // Debug
        // if (rms > 0.01) console.log(`VAD RMS: ${rms.toFixed(4)}`);

        if (this.state === "silence") {
            if (rms > this.Config.Threshold) {
                this.state = "speech";
                this.speechStartTime = Date.now();
                console.log("[VAD] Speech Detected");
                if (this.onSpeechStart) this.onSpeechStart();
                this.buffer.push(Chunk); // Start collecting
            } else {
                // Buffer pre-roll (prefix padding)
                // Keep only last N ms
                // Simplified: Just keep small buffer for now or implement ring buffer
                // For simplicity, we won't do deep prefix padding in this first version
            }
        } else {
            // In Speech
            this.buffer.push(Chunk);

            if (rms < this.Config.Threshold) {
                if (this.silenceStartTime === 0) {
                    this.silenceStartTime = Date.now();
                } else {
                    const silenceDuration = Date.now() - this.silenceStartTime;
                    if (silenceDuration > this.Config.SilenceDurationMs) {
                        this.Commit();
                    }
                }
            } else {
                // Still speaking, reset silence timer
                this.silenceStartTime = 0;
            }
        }
    }

    private Commit(): void {
        console.log(`[VAD] Speech Committed (${this.buffer.length} chunks)`);
        
        // Merge chunks
        const totalSize = this.buffer.reduce((acc, val) => acc + val.length, 0);
        const merged = new Uint8Array(totalSize);
        let offset = 0;
        for (const chunk of this.buffer) {
            merged.set(chunk, offset);
            offset += chunk.length;
        }

        if (this.onSpeechEnd) this.onSpeechEnd(merged);

        // Reset
        this.state = "silence";
        this.buffer = [];
        this.silenceStartTime = 0;
    }

    private CalculateRMS(Chunk: Uint8Array): number {
        // PCM16 Little Endian
        let sum = 0;
        const numSamples = Chunk.length / 2;
        
        // Use DataView for safe reading
        const view = new DataView(Chunk.buffer, Chunk.byteOffset, Chunk.byteLength);

        for (let i = 0; i < numSamples; i++) {
            const val = view.getInt16(i * 2, true); // Little Endian
            const normalized = val / this.MAX_AMPLITUDE;
            sum += normalized * normalized;
        }

        return Math.sqrt(sum / numSamples);
    }
}
