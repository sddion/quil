import { useState, useCallback, useEffect } from 'react';
import { deviceManager, DeviceStatus, DeviceConfig, ConnectionState } from '@/lib/device-manager';

export function useDevice() {
  const [connectionState, setConnectionState] = useState<ConnectionState>('disconnected');
  const [deviceStatus, setDeviceStatus] = useState<DeviceStatus | null>(null);
  const [deviceIp, setDeviceIp] = useState<string | null>(null);
  const [error, setError] = useState<string | null>(null);
  const [isConnecting, setIsConnecting] = useState(false);

  // Subscribe to status updates
  useEffect(() => {
    const unsubscribe = deviceManager.onStatusUpdate((status) => {
      setDeviceStatus(status);
    });

    return unsubscribe;
  }, []);

  // Connect to device
  const connect = useCallback(async (ip?: string) => {
    setError(null);
    setIsConnecting(true);
    setConnectionState('connecting');

    try {
      const success = await deviceManager.connect(ip);
      if (success) {
        setConnectionState('connected');
        setDeviceIp(deviceManager.getDeviceIp());
        
        // Get initial status
        const status = await deviceManager.getStatus();
        if (status) setDeviceStatus(status);
      } else {
        setConnectionState('disconnected');
        setError('Could not find Quil on network. Make sure you\'re on the same WiFi.');
      }
    } catch (err) {
      setConnectionState('disconnected');
      setError(err instanceof Error ? err.message : 'Connection failed');
    } finally {
      setIsConnecting(false);
    }
  }, []);

  // Disconnect from device
  const disconnect = useCallback(() => {
    deviceManager.disconnect();
    setConnectionState('disconnected');
    setDeviceStatus(null);
    setDeviceIp(null);
  }, []);

  // Send configuration
  const sendConfig = useCallback(async (config: DeviceConfig): Promise<boolean> => {
    const success = await deviceManager.sendConfig(config);
    if (success) {
      // Refresh status after config change
      const status = await deviceManager.getStatus();
      if (status) setDeviceStatus(status);
    }
    return success;
  }, []);

  // Send command
  const sendCommand = useCallback(async (cmd: string): Promise<boolean> => {
    return deviceManager.sendCommand(cmd);
  }, []);

  // Discover device (scan network)
  const discover = useCallback(async (): Promise<string | null> => {
    setConnectionState('scanning');
    const ip = await deviceManager.discoverDevice();
    setConnectionState('disconnected');
    return ip;
  }, []);

  return {
    connectionState,
    deviceStatus,
    deviceIp,
    error,
    isConnecting,
    isConnected: connectionState === 'connected',
    connect,
    disconnect,
    sendConfig,
    sendCommand,
    discover,
  };
}
