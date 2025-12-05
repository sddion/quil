# Quil Voice Server

Deno-based WebSocket server that bridges ESP32 devices to OpenAI Realtime API
for voice-to-voice AI interactions.

## Architecture

```
ESP32 (mic) → WebSocket → Server → OpenAI Realtime API → Server → WebSocket → ESP32 (speaker)
```

## Structure

```
server/
├── Main.ts              # HTTP/WebSocket entry point
├── api/
│   └── Esp32Handler.ts  # ESP32 connection manager
└── lib/
    ├── Config.ts        # Configuration constants
    ├── OpenAI/
    │   └── RealtimeClient.ts  # OpenAI Realtime WebSocket client
    └── audio/
        └── AudioUtils.ts      # Base64 encoding, audio chunking
```

## Setup

1. **Install Deno**
   ```bash
   curl -fsSL https://deno.land/install.sh | sh
   ```

2. **Configure environment**
   ```bash
   cp .env.example .env
   # Edit .env and add your OPENAI_API_KEY
   ```

3. **Run development server**
   ```bash
   deno task Dev
   ```

## Endpoints

| Path  | Type      | Description                       |
| ----- | --------- | --------------------------------- |
| `/`   | HTTP      | Health check, returns JSON status |
| `/ws` | WebSocket | ESP32 realtime voice connection   |

## Environment Variables

| Variable         | Required | Description                             |
| ---------------- | -------- | --------------------------------------- |
| `OPENAI_API_KEY` | Yes      | Your OpenAI API key                     |
| `HOST`           | No       | Server host (default: `0.0.0.0`)        |
| `PORT`           | No       | Server port (default: `8000`)           |
| `DEV_MODE`       | No       | Enable debug logging (default: `false`) |

## Protocol

### ESP32 → Server

| Type    | Format                                         |
| ------- | ---------------------------------------------- |
| Audio   | Binary (raw PCM16 24kHz)                       |
| Control | `{"type":"instruction","msg":"end_of_speech"}` |

### Server → ESP32

| Type   | Format                                        |
| ------ | --------------------------------------------- |
| Audio  | Binary (PCM16 chunks)                         |
| Status | `{"type":"server","msg":"RESPONSE.COMPLETE"}` |

## Deployment

Deploy to Vercel:

```bash
deno task Deploy
```

Or use the Vercel CLI:

```bash
vercel --prod
```
