"use client"

import { useState } from "react"
import { QuilVoiceClient } from "@/components/QuilVoiceClient"
import { Button } from "@/components/ui/button"
import { Card, CardContent, CardDescription, CardHeader, CardTitle } from "@/components/ui/card"

export default function Home() {
  const [isReady, setIsReady] = useState(false)

  if (!isReady) {
    return (
      <main className="min-h-screen flex items-center justify-center p-6 bg-linear-to-b from-background to-muted/20">
        <Card className="w-full max-w-md">
          <CardHeader className="text-center">
            <CardTitle className="text-3xl">Quil</CardTitle>
            <CardDescription>Your friendly voice companion</CardDescription>
          </CardHeader>
          <CardContent className="space-y-4">
            <p className="text-sm text-muted-foreground">
              Connect to Quil and start talking. Your voice conversations are private and secure.
            </p>
            <Button onClick={() => setIsReady(true)} className="w-full">
              Start Talking
            </Button>
          </CardContent>
        </Card>
      </main>
    )
  }

  return (
    <main className="min-h-screen flex items-center justify-center p-6 bg-linear-to-b from-background to-muted/20">
      <QuilVoiceClient />
    </main>
  )
}