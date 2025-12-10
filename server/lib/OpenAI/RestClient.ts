import { OpenAiApiKey } from "../Config.ts";
import { EncodeBase64, DecodeBase64 } from "../audio/AudioUtils.ts";

export class RestClient {
    public async SendAudioInteraction(AudioData: Uint8Array, Voice: string = "alloy", Instructions: string = ""): Promise<Uint8Array | null> {
        const Base64Audio = EncodeBase64(AudioData);

        const Payload = {
            model: "gpt-4o-audio-preview", // Force audio preview model
            modalities: ["text", "audio"],
            audio: {
                voice: Voice,
                format: "pcm16"
            },
            messages: [
                {
                    role: "system",
                    content: Instructions
                },
                {
                    role: "user",
                    content: [
                        { type: "text", text: "Listen to this audio and respond." },
                        {
                            type: "input_audio",
                            input_audio: {
                                data: Base64Audio,
                                format: "pcm16"
                            }
                        }
                    ]
                }
            ]
        };

        try {
            console.log("[RestClient] Sending request to OpenAI...");
            const StartTime = Date.now();
            
            const Response = await fetch("https://api.openai.com/v1/chat/completions", {
                method: "POST",
                headers: {
                    "Content-Type": "application/json",
                    "Authorization": `Bearer ${OpenAiApiKey}`
                },
                body: JSON.stringify(Payload)
            });

            if (!Response.ok) {
                const ErrorText = await Response.text();
                console.error("[RestClient] API Error:", Response.status, ErrorText);
                return null;
            }

            const Json = await Response.json();
            const Duration = Date.now() - StartTime;
            console.log(`[RestClient] Response received in ${Duration}ms`);

            // Extract Audio
            const Message = Json.choices[0].message;
            if (Message.audio && Message.audio.data) {
                console.log(`[RestClient] Audio ID: ${Message.audio.id}`);
                const AudioBuffer = DecodeBase64(Message.audio.data);
                return AudioBuffer;
            } else {
                console.warn("[RestClient] No audio data in response");
                return null;
            }

        } catch (Error) {
            console.error("[RestClient] Network Error:", Error);
            return null;
        }
    }
}
