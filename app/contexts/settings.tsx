import createContextHook from '@nkzw/create-context-hook';
import { useState, useEffect } from 'react';
import AsyncStorage from '@react-native-async-storage/async-storage';

export type AppSettings = {
  wifiSSID: string;
  wifiPassword: string;
  timezone: string;
  weatherAPIKey: string;
  weatherLocation: string;
  brightness: number;
  selectedTheme: string;
  lastSyncedAt: number | null;
};

const SETTINGS_KEY = '@quil_app_settings';

const defaultSettings: AppSettings = {
  wifiSSID: '',
  wifiPassword: '',
  timezone: 'America/New_York',
  weatherAPIKey: '',
  weatherLocation: '',
  brightness: 128,
  selectedTheme: 'default',
  lastSyncedAt: null,
};

export const [SettingsProvider, useSettings] = createContextHook(() => {
  const [settings, setSettings] = useState<AppSettings>(defaultSettings);
  const [isLoading, setIsLoading] = useState<boolean>(true);

  useEffect(() => {
    loadSettings();
  }, []);

  const loadSettings = async () => {
    try {
      const stored = await AsyncStorage.getItem(SETTINGS_KEY);
      if (stored) {
        const parsed = JSON.parse(stored) as AppSettings;
        setSettings(parsed);
        console.log('[Settings] Loaded settings from storage');
      }
    } catch (err) {
      console.error('[Settings] Failed to load settings:', err);
    } finally {
      setIsLoading(false);
    }
  };

  const updateSettings = async (updates: Partial<AppSettings>) => {
    try {
      const newSettings = { ...settings, ...updates };
      setSettings(newSettings);
      await AsyncStorage.setItem(SETTINGS_KEY, JSON.stringify(newSettings));
      console.log('[Settings] Saved settings to storage');
    } catch (err) {
      console.error('[Settings] Failed to save settings:', err);
    }
  };

  const clearSettings = async () => {
    try {
      setSettings(defaultSettings);
      await AsyncStorage.removeItem(SETTINGS_KEY);
      console.log('[Settings] Cleared settings from storage');
    } catch (err) {
      console.error('[Settings] Failed to clear settings:', err);
    }
  };

  const markSynced = async () => {
    await updateSettings({ lastSyncedAt: Date.now() });
  };

  return {
    settings,
    isLoading,
    updateSettings,
    clearSettings,
    markSynced,
  };
});
