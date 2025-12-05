import createContextHook from '@nkzw/create-context-hook';
import { useState, useEffect } from 'react';
import AsyncStorage from '@react-native-async-storage/async-storage';

export type SavedDevice = {
  id: string;
  name: string;
  customName?: string;
  isFavorite: boolean;
  lastConnectedAt: number;
  connectionCount: number;
};

const DEVICES_KEY = '@quil_saved_devices';

export const [DevicesProvider, useDevices] = createContextHook(() => {
  const [savedDevices, setSavedDevices] = useState<SavedDevice[]>([]);
  const [isLoading, setIsLoading] = useState<boolean>(true);

  useEffect(() => {
    loadDevices();
  }, []);

  const loadDevices = async () => {
    try {
      const stored = await AsyncStorage.getItem(DEVICES_KEY);
      if (stored) {
        const parsed = JSON.parse(stored) as SavedDevice[];
        setSavedDevices(parsed);
        console.log('[Devices] Loaded devices from storage');
      }
    } catch (err) {
      console.error('[Devices] Failed to load devices:', err);
    } finally {
      setIsLoading(false);
    }
  };

  const saveDevice = async (id: string, name: string) => {
    try {
      const existing = savedDevices.find(d => d.id === id);
      let updated: SavedDevice[];
      
      if (existing) {
        updated = savedDevices.map(d =>
          d.id === id
            ? {
                ...d,
                lastConnectedAt: Date.now(),
                connectionCount: d.connectionCount + 1,
              }
            : d
        );
      } else {
        const newDevice: SavedDevice = {
          id,
          name,
          isFavorite: false,
          lastConnectedAt: Date.now(),
          connectionCount: 1,
        };
        updated = [...savedDevices, newDevice];
      }
      
      setSavedDevices(updated);
      await AsyncStorage.setItem(DEVICES_KEY, JSON.stringify(updated));
      console.log('[Devices] Saved device:', name);
    } catch (err) {
      console.error('[Devices] Failed to save device:', err);
    }
  };

  const updateDeviceName = async (id: string, customName: string) => {
    try {
      const updated = savedDevices.map(d =>
        d.id === id ? { ...d, customName } : d
      );
      setSavedDevices(updated);
      await AsyncStorage.setItem(DEVICES_KEY, JSON.stringify(updated));
      console.log('[Devices] Updated device name');
    } catch (err) {
      console.error('[Devices] Failed to update device name:', err);
    }
  };

  const toggleFavorite = async (id: string) => {
    try {
      const updated = savedDevices.map(d =>
        d.id === id ? { ...d, isFavorite: !d.isFavorite } : d
      );
      setSavedDevices(updated);
      await AsyncStorage.setItem(DEVICES_KEY, JSON.stringify(updated));
      console.log('[Devices] Toggled favorite');
    } catch (err) {
      console.error('[Devices] Failed to toggle favorite:', err);
    }
  };

  const removeDevice = async (id: string) => {
    try {
      const updated = savedDevices.filter(d => d.id !== id);
      setSavedDevices(updated);
      await AsyncStorage.setItem(DEVICES_KEY, JSON.stringify(updated));
      console.log('[Devices] Removed device');
    } catch (err) {
      console.error('[Devices] Failed to remove device:', err);
    }
  };

  const getDevice = (id: string): SavedDevice | undefined => {
    return savedDevices.find(d => d.id === id);
  };

  const getFavorites = (): SavedDevice[] => {
    return savedDevices
      .filter(d => d.isFavorite)
      .sort((a, b) => b.lastConnectedAt - a.lastConnectedAt);
  };

  const getRecentDevices = (limit: number = 5): SavedDevice[] => {
    return savedDevices
      .sort((a, b) => b.lastConnectedAt - a.lastConnectedAt)
      .slice(0, limit);
  };

  return {
    savedDevices,
    isLoading,
    saveDevice,
    updateDeviceName,
    toggleFavorite,
    removeDevice,
    getDevice,
    getFavorites,
    getRecentDevices,
  };
});
