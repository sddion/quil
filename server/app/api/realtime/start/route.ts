import { type NextRequest, NextResponse } from "next/server"
import { loadMemory } from "@/lib/quil/memory"
import { QUIL_PERSONA, QUIL_VOICE } from "@/lib/quil/constants"

export const runtime = "edge" // Edge is good for low latency start

export async function POST(request: NextRequest) {
    try {
        const { userId_or_sessionId } = await request.json().catch(() => ({}))
        const sessionId = userId_or_sessionId || crypto.randomUUID()

        // 1. Load context
        let memory = await loadMemory(sessionId)

        // 2. Prepare instructions
        let instructions = QUIL_PERSONA
        if (memory) {
            instructions += `\n\nContext from previous conversations:\n${memory}`
        }

        // 3. Create Ephemeral Token from OpenAI
        const response = await fetch("https://api.openai.com/v1/realtime/sessions", {
            method: "POST",
            headers: {
                Authorization: `Bearer ${process.env.OPENAI_API_KEY}`,
                "Content-Type": "application/json",
            },
            body: JSON.stringify({
                model: "gpt-4o-realtime-preview-2024-12-17",
                voice: QUIL_VOICE,
                modalities: ["audio", "text"],
                instructions: instructions,
            }),
        })

        if (!response.ok) {
            const error = await response.text()
            console.error("OpenAI Session Error:", error)
            return NextResponse.json({ error: "Failed to start session" }, { status: 500 })
        }

        const data = await response.json()

        // 4. Return details to client
        return NextResponse.json({
            client_secret: data.client_secret.value,
            sessionId,
            memory, // Optional: return memory so client knows what's loaded
            instructions, // Return full instructions for client-side session.update
        })

    } catch (error) {
        console.error("Start API Error:", error)
        return NextResponse.json({ error: "Internal Server Error" }, { status: 500 })
    }
}
