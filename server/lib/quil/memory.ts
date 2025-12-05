import { createClient } from "@/lib/supabase/server"

export interface SessionMemory {
  id: string
  session_id: string
  summary: string
  created_at: string
  updated_at: string
}

// Load the most recent memory summary for a session
export async function loadMemory(sessionId: string): Promise<string | null> {
  const supabase = await createClient()

  const { data, error } = await supabase
    .from("session_memories")
    .select("summary")
    .eq("session_id", sessionId)
    .order("updated_at", { ascending: false })
    .limit(1)
    .maybeSingle() // Use maybeSingle() instead of single() to handle 0 rows gracefully

  if (error || !data) {
    return null
  }

  return data.summary
}

// Save or update memory summary for a session
export async function saveMemory(sessionId: string, summary: string): Promise<void> {
  const supabase = await createClient()

  // Upsert: update if exists, insert if not
  const { error } = await supabase.from("session_memories").upsert(
    {
      session_id: sessionId,
      summary,
      updated_at: new Date().toISOString(),
    },
    {
      onConflict: "session_id",
    },
  )

  if (error) {
    console.error("Failed to save memory:", error)
  }
}

// Generate a brief summary from conversation transcript
export function generateSummaryPrompt(transcript: string): string {
  return `Summarize this conversation in 1-3 sentences, focusing on key topics discussed and any personal details shared by the user. Keep it brief`
}

export async function generateSessionSummary(transcript: string): Promise<string | null> {
  if (!transcript || transcript.length < 50) return null

  try {
    const response = await fetch("https://api.openai.com/v1/chat/completions", {
      method: "POST",
      headers: {
        "Content-Type": "application/json",
        Authorization: `Bearer ${process.env.OPENAI_API_KEY}`,
      },
      body: JSON.stringify({
        model: "gpt-4o-mini",
        messages: [
          {
            role: "system",
            content: "You are a helpful assistant that summarizes conversations.",
          },
          {
            role: "user",
            content: generateSummaryPrompt(transcript),
          },
        ],
      }),
    })

    if (!response.ok) {
      console.error("Summary generation failed:", await response.text())
      return null
    }

    const data = await response.json()
    return data.choices[0]?.message?.content || null
  } catch (error) {
    console.error("Error generating summary:", error)
    return null
  }
}
