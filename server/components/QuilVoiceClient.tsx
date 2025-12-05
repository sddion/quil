"use client"

import { useState, useRef, useCallback, useEffect } from "react"
import { Button } from "@/components/ui/button"
import { Card, CardContent, CardHeader, CardTitle, CardDescription } from "@/components/ui/card"
import { Mic, MicOff, Volume2, Loader2, Phone, PhoneOff } from "lucide-react"

interface QuilVoiceClientProps { }

type ConnectionStatus = "disconnected" | "connecting" | "connected" | "error"

interface TranscriptItem {
  role: "user" | "assistant"
  text: string
}

export function QuilVoiceClient({ }: QuilVoiceClientProps) {
  const [status, setStatus] = useState<ConnectionStatus>("disconnected")
  const [isMuted, setIsMuted] = useState(false)
  const [isSpeaking, setIsSpeaking] = useState(false)
  const [transcript, setTranscript] = useState<TranscriptItem[]>([])
  const [sessionId, setSessionId] = useState<string>("")
  const [error, setError] = useState<string>("")

  const pcRef = useRef<RTCPeerConnection | null>(null)
  const dcRef = useRef<RTCDataChannel | null>(null)
  const audioRef = useRef<HTMLAudioElement | null>(null)
  const mediaStreamRef = useRef<MediaStream | null>(null)
  const currentTranscriptRef = useRef<{ user: string; assistant: string }>({ user: "", assistant: "" })
  const currentInstructionsRef = useRef<string>("")

  // Handle data channel messages (events from OpenAI)
  const handleDataChannelMessage = useCallback((event: MessageEvent) => {
    try {
      const data = JSON.parse(event.data)
      console.log("Received event:", data.type)

      switch (data.type) {
        case "session.created":
        case "session.updated":
          console.log("Session event:", data.type)
          break

        case "response.audio_transcript.delta":
          if (data.delta) {
            currentTranscriptRef.current.assistant += data.delta
            setTranscript((prev) => {
              const newTranscript = [...prev]
              const lastItem = newTranscript[newTranscript.length - 1]
              if (lastItem && lastItem.role === "assistant") {
                lastItem.text = currentTranscriptRef.current.assistant
              } else {
                newTranscript.push({ role: "assistant", text: currentTranscriptRef.current.assistant })
              }
              return newTranscript
            })
          }
          break

        case "response.audio_transcript.done":
          // Reset for next response
          currentTranscriptRef.current.assistant = ""
          break

        case "response.audio.started":
          setIsSpeaking(true)
          break

        case "response.audio.done":
        case "response.done":
          if (data.type === "response.done") {
            console.log("Response done:", JSON.stringify(data.response, null, 2))
          }
          setIsSpeaking(false)
          break

        case "conversation.item.input_audio_transcription.completed":
          if (data.transcript) {
            setTranscript((prev) => [...prev, { role: "user", text: data.transcript }])
          }
          break

        case "input_audio_buffer.speech_started":
          // User started speaking - stop any ongoing response
          setIsSpeaking(false)
          break

        case "error":
          console.error("Realtime error:", data.error)
          setError(data.error?.message || "An error occurred")
          break
      }
    } catch (e) {
      console.error("Failed to parse message:", e)
    }
  }, [])

  // Connect using WebRTC
  const connect = useCallback(async () => {
    setStatus("connecting")
    setError("")
    setTranscript([])

    try {
      // 1. Get ephemeral token from our server
      const tokenResponse = await fetch("/api/realtime/start", {
        method: "POST",
      })

      if (!tokenResponse.ok) {
        throw new Error("Failed to get ephemeral token")
      }

      const { client_secret, sessionId: newSessionId, memory, instructions } = await tokenResponse.json()
      setSessionId(newSessionId)
      if (instructions) {
        currentInstructionsRef.current = instructions
      }

      if (memory) {
        // Optionally show memory to user or just log it
        console.log("Loaded memory context:", memory)
      }

      // 2. Create peer connection
      const pc = new RTCPeerConnection()
      pcRef.current = pc

      // Set up audio element for playback
      const audio = document.createElement("audio")
      audio.autoplay = true
      audioRef.current = audio

      pc.ontrack = (e) => {
        audio.srcObject = e.streams[0]
      }

      // Get microphone access
      const stream = await navigator.mediaDevices.getUserMedia({
        audio: true,
      })
      mediaStreamRef.current = stream

      // Add audio track to peer connection
      stream.getTracks().forEach((track) => {
        pc.addTrack(track, stream)
      })

      // Set up data channel for events
      const dc = pc.createDataChannel("oai-events")
      dcRef.current = dc

      dc.onopen = () => {
        setStatus("connected")
        // Clear buffer
        currentTranscriptRef.current = { user: "", assistant: "" }

        // Configure session explicitly to ensure audio output
        const sessionUpdate = {
          type: "session.update",
          session: {
            modalities: ["audio", "text"],
            voice: "alloy",
            instructions: currentInstructionsRef.current || "You are a helpful assistant.",
            input_audio_transcription: {
              model: "whisper-1",
            },
            turn_detection: {
              type: "server_vad",
            },
          },
        }
        dc.send(JSON.stringify(sessionUpdate))
      }

      dc.onmessage = handleDataChannelMessage

      // 3. Create and set local description (SDP offer)
      const offer = await pc.createOffer()
      await pc.setLocalDescription(offer)

      // 4. Send SDP to OpenAI directly with ephemeral token
      const baseUrl = "https://api.openai.com/v1/realtime"
      const model = "gpt-4o-realtime-preview-2024-12-17"

      const sdpResponse = await fetch(`${baseUrl}?model=${model}`, {
        method: "POST",
        body: offer.sdp,
        headers: {
          Authorization: `Bearer ${client_secret}`,
          "Content-Type": "application/sdp",
        },
      })

      if (!sdpResponse.ok) {
        throw new Error(`OpenAI connection failed: ${sdpResponse.status}`)
      }

      const answerSdp = await sdpResponse.text()

      // 5. Set remote description
      await pc.setRemoteDescription({
        type: "answer",
        sdp: answerSdp,
      })

      // Handle connection state changes
      pc.onconnectionstatechange = () => {
        if (pc.connectionState === "disconnected" || pc.connectionState === "failed") {
          // Only update status if we aren't already handling a disconnect
          setStatus((prev) => prev === "connected" ? "disconnected" : prev)
          setIsSpeaking(false)
        }
      }

    } catch (e) {
      console.error("Connection error:", e)
      setStatus("error")
      setError(e instanceof Error ? e.message : "Failed to connect")
      cleanup()
    }
  }, [handleDataChannelMessage])

  // Cleanup function
  const cleanup = useCallback(() => {
    if (mediaStreamRef.current) {
      mediaStreamRef.current.getTracks().forEach((track) => track.stop())
      mediaStreamRef.current = null
    }
    if (dcRef.current) {
      dcRef.current.close()
      dcRef.current = null
    }
    if (pcRef.current) {
      pcRef.current.close()
      pcRef.current = null
    }
    if (audioRef.current) {
      audioRef.current.srcObject = null
      audioRef.current = null
    }
  }, [])

  // Disconnect
  const disconnect = useCallback(async () => {
    // Save memory summary if we have transcript
    if (sessionId && transcript.length > 0) {
      try {
        const fullTranscript = transcript
          .map((t) => `${t.role === "user" ? "User" : "Quil"}: ${t.text}`)
          .join("\n")

        await fetch("/api/realtime/webhook", {
          method: "POST",
          headers: { "Content-Type": "application/json" },
          body: JSON.stringify({
            event: "save_memory",
            sessionId,
            transcript: fullTranscript,
          }),
        })
      } catch (e) {
        console.error("Failed to save memory:", e)
      }
    }

    cleanup()
    setStatus("disconnected")
    setIsSpeaking(false)
  }, [sessionId, transcript, cleanup])

  // Toggle mute
  const toggleMute = useCallback(() => {
    if (mediaStreamRef.current) {
      const audioTrack = mediaStreamRef.current.getAudioTracks()[0]
      if (audioTrack) {
        audioTrack.enabled = !audioTrack.enabled
        setIsMuted(!audioTrack.enabled)
      }
    }
  }, [])

  // Send a text message via data channel
  const sendTextMessage = useCallback((text: string) => {
    if (dcRef.current && dcRef.current.readyState === "open") {
      dcRef.current.send(
        JSON.stringify({
          type: "conversation.item.create",
          item: {
            type: "message",
            role: "user",
            content: [{ type: "input_text", text }],
          },
        }),
      )
      dcRef.current.send(JSON.stringify({ type: "response.create" }))
    }
  }, [])

  // Cleanup on unmount
  useEffect(() => {
    return () => {
      cleanup()
    }
  }, [cleanup])

  return (
    <Card className="w-full max-w-2xl mx-auto">
      <CardHeader className="text-center">
        <CardTitle className="text-2xl">Meet Quil</CardTitle>
        <CardDescription>Your friendly voice companion. Click connect and start talking!</CardDescription>
      </CardHeader>
      <CardContent className="space-y-6">
        {/* Status indicator */}
        <div className="flex items-center justify-center gap-2">
          <div
            className={`w-3 h-3 rounded-full ${status === "connected"
              ? "bg-green-500"
              : status === "connecting"
                ? "bg-yellow-500 animate-pulse"
                : status === "error"
                  ? "bg-red-500"
                  : "bg-muted-foreground/30"
              }`}
          />
          <span className="text-sm text-muted-foreground capitalize">{status}</span>
        </div>

        {error && <div className="p-3 bg-destructive/10 text-destructive rounded-lg text-sm text-center">{error}</div>}

        {/* Control buttons */}
        <div className="flex justify-center gap-4">
          {status === "disconnected" || status === "error" ? (
            <Button onClick={connect} size="lg" className="gap-2">
              <Phone className="h-4 w-4" />
              Connect to Quil
            </Button>
          ) : status === "connecting" ? (
            <Button disabled size="lg">
              <Loader2 className="mr-2 h-4 w-4 animate-spin" />
              Connecting...
            </Button>
          ) : (
            <>
              <Button onClick={toggleMute} variant={isMuted ? "destructive" : "secondary"} size="lg">
                {isMuted ? (
                  <>
                    <MicOff className="mr-2 h-4 w-4" />
                    Unmute
                  </>
                ) : (
                  <>
                    <Mic className="mr-2 h-4 w-4" />
                    Mute
                  </>
                )}
              </Button>
              <Button onClick={disconnect} variant="destructive" size="lg" className="gap-2">
                <PhoneOff className="h-4 w-4" />
                End Call
              </Button>
            </>
          )}
        </div>

        {/* Speaking indicator */}
        {isSpeaking && (
          <div className="flex items-center justify-center gap-2 text-primary">
            <Volume2 className="h-5 w-5 animate-pulse" />
            <span className="text-sm">Quil is speaking...</span>
          </div>
        )}

        {/* Recording indicator */}
        {status === "connected" && !isMuted && !isSpeaking && (
          <div className="flex items-center justify-center gap-2 text-muted-foreground">
            <div className="w-2 h-2 bg-red-500 rounded-full animate-pulse" />
            <span className="text-sm">Listening...</span>
          </div>
        )}

        {/* Transcript */}
        {transcript.length > 0 && (
          <div className="mt-6 p-4 bg-muted/50 rounded-lg max-h-64 overflow-y-auto">
            <h3 className="text-sm font-medium mb-2">Conversation</h3>
            <div className="space-y-2">
              {transcript.map((item, i) => (
                <p
                  key={i}
                  className={`text-sm ${item.role === "assistant" ? "text-primary" : "text-muted-foreground"}`}
                >
                  <span className="font-medium">{item.role === "assistant" ? "Quil" : "You"}:</span> {item.text}
                </p>
              ))}
            </div>
          </div>
        )}

        {sessionId && <p className="text-xs text-center text-muted-foreground">Session: {sessionId.slice(0, 8)}...</p>}
      </CardContent>
    </Card>
  )
}