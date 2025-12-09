import createContextHook from '@nkzw/create-context-hook';
import { useCallback, useEffect, useState, useRef } from 'react';
import { Alert, Linking, Platform } from 'react-native';
import { bleManager, type BLEDevice, type ConnectionState, type DeviceStatus } from '@/lib/ble-manager';
import * as Haptics from 'expo-haptics';

export const [BLEProvider, useBLE] = createContextHook(() => {
  const [connectionState, setConnectionState] = useState<ConnectionState>('disconnected');
  const [devices, setDevices] = useState<BLEDevice[]>([]);
  const [connectedDevice, setConnectedDevice] = useState<BLEDevice | null>(null);
  const [deviceStatus, setDeviceStatus] = useState<DeviceStatus | null>(null);
  const [error, setError] = useState<string | null>(null);
  const [retryCount, setRetryCount] = useState<number>(0);
  const [lastConnectionCheck, setLastConnectionCheck] = useState<number>(Date.now());
  const connectionCheckInterval = useRef<number | null>(null);
  const maxRetries = 3;

  useEffect(() => {
    const init = async () => {
      try {
        await bleManager.initialize();
        bleManager.onStatusUpdate((status: DeviceStatus) => {
          console.log('[BLE Hook] Status update:', status);
          setDeviceStatus(status);
          setLastConnectionCheck(Date.now());
        });
      } catch (err) {
        const errorMessage = err instanceof Error ? err.message : 'Failed to initialize Bluetooth';
        console.error('[BLE Hook] Initialization error:', errorMessage);
        setError(errorMessage);
      }
    };
    init();
    
    return () => {
      if (connectionCheckInterval.current) {
        clearInterval(connectionCheckInterval.current);
      }
    };
  }, []);

  const startScan = useCallback(async () => {
    try {
      setError(null);
      
      // Check if Bluetooth is enabled, prompt to turn on if off
      const isBluetoothOn = await bleManager.checkAndEnableBluetooth();
      if (!isBluetoothOn) {
        Alert.alert(
          'Bluetooth Required',
          'Bluetooth is turned off. Please enable Bluetooth to scan for devices.',
          [
            { text: 'Cancel', style: 'cancel' },
            { 
              text: 'Open Settings', 
              onPress: () => {
                if (Platform.OS === 'ios') {
                  Linking.openURL('App-Prefs:Bluetooth');
                } else {
                  Linking.sendIntent('android.settings.BLUETOOTH_SETTINGS');
                }
              }
            }
          ]
        );
        return;
      }
      
      // Request permissions (handles both native and web)
      const hasPermission = await bleManager.requestPermissions();
      if (!hasPermission) {
        setError('Bluetooth permissions denied. Please allow Bluetooth access in Settings.');
        return;
      }
      
      setConnectionState('scanning');
      setDevices([]);
      
      await bleManager.startScan((foundDevices: BLEDevice[]) => {
        setDevices(foundDevices);
      });
    } catch (err) {
      setError(err instanceof Error ? err.message : 'Failed to start scan');
      setConnectionState('disconnected');
      console.error('[BLE Hook] Scan error:', err);
    }
  }, []);

  const stopScan = useCallback(() => {
    bleManager.stopScan();
    if (connectionState === 'scanning') {
      setConnectionState('disconnected');
    }
  }, [connectionState]);

  const startConnectionMonitoring = useCallback(() => {
    if (connectionCheckInterval.current) {
      clearInterval(connectionCheckInterval.current);
    }
    
    connectionCheckInterval.current = setInterval(() => {
      if (!bleManager.isConnected()) {
        console.log('[BLE Hook] Connection lost');
        setConnectionState('disconnected');
        setConnectedDevice(null);
        setDeviceStatus(null);
        Haptics.notificationAsync(Haptics.NotificationFeedbackType.Error);
        if (connectionCheckInterval.current) {
          clearInterval(connectionCheckInterval.current);
          connectionCheckInterval.current = null;
        }
      } else {
        setLastConnectionCheck(Date.now());
      }
    }, 5000) as unknown as number;
  }, []);
  
  const stopConnectionMonitoring = useCallback(() => {
    if (connectionCheckInterval.current) {
      clearInterval(connectionCheckInterval.current);
      connectionCheckInterval.current = null;
    }
  }, []);

  const connect = useCallback(async (device: BLEDevice, attemptNumber: number = 0) => {
    try {
      setError(null);
      setConnectionState('connecting');
      bleManager.stopScan();
      
      await bleManager.connect(device.id);
      
      setConnectedDevice(device);
      setConnectionState('connected');
      setRetryCount(0);
      
      Haptics.notificationAsync(Haptics.NotificationFeedbackType.Success);
      
      // Device status will come via notifications, no need to read immediately
      startConnectionMonitoring();
      
      console.log('[BLE Hook] Connected to:', device.name);
    } catch (err) {
      const errorMessage = err instanceof Error ? err.message : 'Failed to connect';
      
      if (attemptNumber < maxRetries) {
        console.log(`[BLE Hook] Connection failed, retry ${attemptNumber + 1}/${maxRetries}`);
        setRetryCount(attemptNumber + 1);
        await new Promise(resolve => setTimeout(resolve, 2000));
        return connect(device, attemptNumber + 1);
      }
      
      setError(errorMessage);
      setConnectionState('disconnected');
      setConnectedDevice(null);
      setRetryCount(0);
      Haptics.notificationAsync(Haptics.NotificationFeedbackType.Error);
      console.error('[BLE Hook] Connection error:', err);
    }
  }, [startConnectionMonitoring]);

  const disconnect = useCallback(async () => {
    try {
      stopConnectionMonitoring();
      await bleManager.disconnect();
      setConnectedDevice(null);
      setDeviceStatus(null);
      setConnectionState('disconnected');
      setRetryCount(0);
      Haptics.notificationAsync(Haptics.NotificationFeedbackType.Warning);
      console.log('[BLE Hook] Disconnected');
    } catch (err) {
      setError(err instanceof Error ? err.message : 'Failed to disconnect');
      console.error('[BLE Hook] Disconnect error:', err);
    }
  }, [stopConnectionMonitoring]);

  const sendConfiguration = useCallback(async (config: Record<string, unknown>) => {
    try {
      setError(null);
      const jsonData = JSON.stringify(config);
      await bleManager.writeCharacteristic('', '', jsonData);
      console.log('[BLE Hook] Configuration sent:', config);
      return true;
    } catch (err) {
      setError(err instanceof Error ? err.message : 'Failed to send configuration');
      console.error('[BLE Hook] Send config error:', err);
      return false;
    }
  }, []);

  const refreshStatus = useCallback(async () => {
    try {
      if (connectionState !== 'connected') return;
      const statusData = await bleManager.readCharacteristic('', '');
      const status = JSON.parse(statusData) as DeviceStatus;
      setDeviceStatus(status);
    } catch (err) {
      console.error('[BLE Hook] Refresh status error:', err);
    }
  }, [connectionState]);

  const sendCommand = useCallback(async (command: string) => {
    try {
      setError(null);
      await bleManager.writeCharacteristic('', '', JSON.stringify({ command }));
      console.log('[BLE Hook] Command sent:', command);
      return true;
    } catch (err) {
      setError(err instanceof Error ? err.message : 'Failed to send command');
      console.error('[BLE Hook] Send command error:', err);
      return false;
    }
  }, []);

  
  const getConnectionHealth = useCallback((): 'good' | 'warning' | 'error' => {
    const timeSinceCheck = Date.now() - lastConnectionCheck;
    if (timeSinceCheck < 10000) return 'good';
    if (timeSinceCheck < 30000) return 'warning';
    return 'error';
  }, [lastConnectionCheck]);

  return {
    connectionState,
    devices,
    connectedDevice,
    deviceStatus,
    error,
    retryCount,
    maxRetries,
    connectionHealth: getConnectionHealth(),
    lastConnectionCheck,
    startScan,
    stopScan,
    connect,
    disconnect,
    sendConfiguration,
    refreshStatus,
    sendCommand,
  };
});
