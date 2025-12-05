import { type NextRequest, NextResponse } from "next/server"
import { saveMemory } from "@/lib/quil/memory"

export const runtime = "edge"

export async function POST(request: NextRequest) {
    try {
        const { sessionId, summary } = await request.json()

        if (!sessionId || !summary) {
            return NextResponse.json({ error: "Missing sessionId or summary" }, { status: 400 })
        }

        await saveMemory(sessionId, summary)

        return NextResponse.json({ success: true })
    } catch (error) {
        return NextResponse.json({ error: "Internal Server Error" }, { status: 500 })
    }
}
