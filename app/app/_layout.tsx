// template
import { QueryClient, QueryClientProvider } from "@tanstack/react-query";
import { Stack } from "expo-router";
import * as SplashScreen from "expo-splash-screen";
import React, { useEffect } from "react";
import { GestureHandlerRootView } from "react-native-gesture-handler";
import { BLEProvider } from "@/hooks/use-ble";
import { SettingsProvider } from "@/contexts/settings";
import { NotificationsProvider } from "@/contexts/notifications";
import { UpdateProvider } from "@/utils/UpdateContext";

SplashScreen.preventAutoHideAsync();

const queryClient = new QueryClient();

function RootLayoutNav() {
  return (
    <Stack 
      screenOptions={{ 
        headerShown: false,
        contentStyle: { backgroundColor: '#0a0e27' },
        animation: 'slide_from_right'
      }}
    >
      <Stack.Screen name="index" />
      <Stack.Screen name="onboarding" options={{ presentation: 'modal' }} />
      <Stack.Screen name="+not-found" />
    </Stack>
  );
}

export default function RootLayout() {
  useEffect(() => {
    SplashScreen.hideAsync();
  }, []);

  return (
    <QueryClientProvider client={queryClient}>
      <GestureHandlerRootView>
        <UpdateProvider>
          <NotificationsProvider>
            <SettingsProvider>
              <BLEProvider>
                <RootLayoutNav />
              </BLEProvider>
            </SettingsProvider>
          </NotificationsProvider>
        </UpdateProvider>
      </GestureHandlerRootView>
    </QueryClientProvider>
  );
}
