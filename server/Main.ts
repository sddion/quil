/// <reference lib="deno.ns" />

// Quil Server - Main Entry Point
// Deno WebSocket server for ESP32 voice-to-voice AI

import { ServerHost, ServerPort, IsDevMode, OpenAiApiKey } from "./lib/Config.ts";
import { HandleEsp32Connection, GetActiveSessionCount } from "./api/Esp32Handler.ts";

function GenerateSessionId(): string {
    return crypto.randomUUID();
}

function HandleWebSocketUpgrade(Request: Request): Response {
    const { socket: Ws, response: UpgradeResponse } = Deno.upgradeWebSocket(Request);
    const SessionId = GenerateSessionId();

    Ws.onopen = () => {
        HandleEsp32Connection(Ws, SessionId);
    };

    return UpgradeResponse;
}

function HandleHttpRequest(Request: Request): Response {
    const Url = new URL(Request.url);
    const Path = Url.pathname;

    // Health check endpoint
    if (Path === "/health" || Path === "/") {
        return new Response(JSON.stringify({
            Status: "ok",
            Service: "Quil Voice Server",
            ActiveSessions: GetActiveSessionCount(),
            Timestamp: new Date().toISOString(),
        }), {
            status: 200,
            headers: { "Content-Type": "application/json" },
        });
    }

    // WebSocket upgrade for ESP32
    if (Path === "/ws" || Path === "/esp32") {
        const UpgradeHeader = Request.headers.get("upgrade");
        if (UpgradeHeader?.toLowerCase() === "websocket") {
            return HandleWebSocketUpgrade(Request);
        }
        return new Response("WebSocket upgrade required", { status: 426 });
    }

    // 404 for unknown paths
    return new Response("Not Found", { status: 404 });
}

function ValidateConfig(): boolean {
    if (!OpenAiApiKey) {
        console.error("[Server] OPENAI_API_KEY is not set!");
        return false;
    }
    return true;
}

function StartServer(): void {
    if (!ValidateConfig()) {
        throw new Error("Server configuration invalid. Check environment variables.");
    }

    console.log("╔════════════════════════════════════════╗");
    console.log("║       🤖 Quil Voice Server             ║");
    console.log("╠════════════════════════════════════════╣");
    console.log(`║  Host: ${ServerHost.padEnd(30)} ║`);
    console.log(`║  Port: ${String(ServerPort).padEnd(30)} ║`);
    console.log(`║  Mode: ${(IsDevMode ? "Development" : "Production").padEnd(30)} ║`);
    console.log("╚════════════════════════════════════════╝");
    console.log("");
    console.log(`[Server] WebSocket endpoint: ws://${ServerHost}:${ServerPort}/ws`);
    console.log("[Server] Ready for ESP32 connections...");

    Deno.serve({
        hostname: ServerHost,
        port: ServerPort,
    }, HandleHttpRequest);
}

StartServer();
