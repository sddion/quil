import { type NextRequest, NextResponse } from "next/server"
import { loadMemory } from "@/lib/quil/memory"

export const runtime = "edge"

export async function GET(request: NextRequest) {
    const { searchParams } = new URL(request.url)
    const sessionId = searchParams.get("sessionId")

    if (!sessionId) {
        return NextResponse.json({ error: "Missing sessionId" }, { status: 400 })
    }

    const summary = await loadMemory(sessionId)

    return NextResponse.json({ summary })
}
