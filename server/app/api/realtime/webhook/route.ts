import { type NextRequest, NextResponse } from "next/server"
import { saveMemory, generateSessionSummary } from "@/lib/quil/memory"

export const runtime = "edge"

export async function POST(request: NextRequest) {
    try {
        const body = await request.json()
        const { event, sessionId, transcript } = body

        // We primarily care about session ending or explicit save requests
        if (event === "session.ended" || event === "save_memory") {
            if (!transcript || !sessionId) {
                return NextResponse.json({ error: "Missing transcript or sessionId" }, { status: 400 })
            }

            // Generate summary
            const summary = await generateSessionSummary(transcript)

            if (summary) {
                await saveMemory(sessionId, summary)
                console.log(`Saved memory for session ${sessionId}: ${summary}`)
                return NextResponse.json({ success: true, summary })
            } else {
                console.log("No summary generated (content too short?)")
                return NextResponse.json({ success: false, reason: "skipped_summary" })
            }
        }

        // Acknowledge other events (like 'session.started') without action
        return NextResponse.json({ received: true })

    } catch (error) {
        console.error("Webhook Error:", error)
        return NextResponse.json({ error: "Internal Server Error" }, { status: 500 })
    }
}
