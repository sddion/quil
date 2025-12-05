import { useState, useEffect, useCallback, useRef } from 'react';
import { Platform, PermissionsAndroid } from 'react-native';
import { BleManager, Device, State } from 'react-native-ble-plx';

// Quil BLE Service UUIDs
const QUIL_SERVICE_UUID = '4fafc201-1fb5-459e-8fcc-c5c9c331914b';
const CONFIG_CHAR_UUID = 'beb5483e-36e1-4688-b7f5-ea07361b26a8';
const STATUS_CHAR_UUID = 'beb5483e-36e1-4688-b7f5-ea07361b26a9';

export interface QuilDevice {
    id: string;
    name: string;
    rssi: number;
    isConnected: boolean;
}

export interface QuilStatus {
    battery: number;
    wifiConnected: boolean;
    wifiSsid: string;
    timezone: number;
    brightness: number;
    firmwareVersion: string;
}

export interface QuilConfig {
    ssid: string;
    password: string;
    timezone: number;
    weatherApiKey: string;
    weatherLocation: string;
    brightness: number;
}

type ConnectionState = 'disconnected' | 'scanning' | 'connecting' | 'connected';

export function useBLE() {
    const managerRef = useRef<BleManager | null>(null);
    const [connectionState, setConnectionState] = useState<ConnectionState>('disconnected');
    const [devices, setDevices] = useState<QuilDevice[]>([]);
    const [connectedDevice, setConnectedDevice] = useState<Device | null>(null);
    const [quilStatus, setQuilStatus] = useState<QuilStatus | null>(null);
    const [error, setError] = useState<string | null>(null);

    useEffect(() => {
        managerRef.current = new BleManager();

        const subscription = managerRef.current.onStateChange((state) => {
            if (state === State.PoweredOn) {
                console.log('[BLE] Bluetooth powered on');
            } else if (state === State.PoweredOff) {
                setError('Bluetooth is turned off');
                setConnectionState('disconnected');
            }
        }, true);

        return () => {
            subscription.remove();
            managerRef.current?.destroy();
        };
    }, []);

    const requestPermissions = async (): Promise<boolean> => {
        if (Platform.OS === 'android') {
            const granted = await PermissionsAndroid.requestMultiple([
                PermissionsAndroid.PERMISSIONS.BLUETOOTH_SCAN,
                PermissionsAndroid.PERMISSIONS.BLUETOOTH_CONNECT,
                PermissionsAndroid.PERMISSIONS.ACCESS_FINE_LOCATION,
            ]);
            return Object.values(granted).every(
                (status) => status === PermissionsAndroid.RESULTS.GRANTED
            );
        }
        return true;
    };

    const startScan = useCallback(async () => {
        if (!managerRef.current) return;

        const hasPermissions = await requestPermissions();
        if (!hasPermissions) {
            setError('Bluetooth permissions not granted');
            return;
        }

        setDevices([]);
        setError(null);
        setConnectionState('scanning');

        managerRef.current.startDeviceScan(
            [QUIL_SERVICE_UUID],
            { allowDuplicates: false },
            (err, device) => {
                if (err) {
                    setError(err.message);
                    setConnectionState('disconnected');
                    return;
                }

                if (device && device.name?.includes('Quil')) {
                    setDevices((prev) => {
                        const exists = prev.find((d) => d.id === device.id);
                        if (exists) return prev;
                        return [
                            ...prev,
                            {
                                id: device.id,
                                name: device.name || 'Quil Device',
                                rssi: device.rssi || -100,
                                isConnected: false,
                            },
                        ];
                    });
                }
            }
        );

        // Stop scanning after 10 seconds
        setTimeout(() => {
            managerRef.current?.stopDeviceScan();
            setConnectionState((prev) => (prev === 'scanning' ? 'disconnected' : prev));
        }, 10000);
    }, []);

    const stopScan = useCallback(() => {
        managerRef.current?.stopDeviceScan();
        setConnectionState('disconnected');
    }, []);

    const connectToDevice = useCallback(async (deviceId: string) => {
        if (!managerRef.current) return;

        try {
            setConnectionState('connecting');
            managerRef.current.stopDeviceScan();

            const device = await managerRef.current.connectToDevice(deviceId);
            await device.discoverAllServicesAndCharacteristics();

            setConnectedDevice(device);
            setConnectionState('connected');

            // Subscribe to status updates
            device.monitorCharacteristicForService(
                QUIL_SERVICE_UUID,
                STATUS_CHAR_UUID,
                (err, characteristic) => {
                    if (err) {
                        console.error('[BLE] Status monitor error:', err);
                        return;
                    }
                    if (characteristic?.value) {
                        const decoded = Buffer.from(characteristic.value, 'base64').toString('utf-8');
                        try {
                            const status = JSON.parse(decoded) as QuilStatus;
                            setQuilStatus(status);
                        } catch (e) {
                            console.error('[BLE] Failed to parse status:', e);
                        }
                    }
                }
            );

            // Monitor disconnection
            device.onDisconnected(() => {
                setConnectedDevice(null);
                setConnectionState('disconnected');
                setQuilStatus(null);
            });
        } catch (err: any) {
            setError(err.message || 'Failed to connect');
            setConnectionState('disconnected');
        }
    }, []);

    const disconnect = useCallback(async () => {
        if (connectedDevice) {
            await connectedDevice.cancelConnection();
            setConnectedDevice(null);
            setConnectionState('disconnected');
            setQuilStatus(null);
        }
    }, [connectedDevice]);

    const sendConfig = useCallback(async (config: QuilConfig): Promise<boolean> => {
        if (!connectedDevice) {
            setError('No device connected');
            return false;
        }

        try {
            const configJson = JSON.stringify(config);
            const base64Config = Buffer.from(configJson).toString('base64');

            await connectedDevice.writeCharacteristicWithResponseForService(
                QUIL_SERVICE_UUID,
                CONFIG_CHAR_UUID,
                base64Config
            );

            return true;
        } catch (err: any) {
            setError(err.message || 'Failed to send config');
            return false;
        }
    }, [connectedDevice]);

    const sendCommand = useCallback(async (command: string): Promise<boolean> => {
        if (!connectedDevice) {
            setError('No device connected');
            return false;
        }

        try {
            const cmdJson = JSON.stringify({ cmd: command });
            const base64Cmd = Buffer.from(cmdJson).toString('base64');

            await connectedDevice.writeCharacteristicWithResponseForService(
                QUIL_SERVICE_UUID,
                CONFIG_CHAR_UUID,
                base64Cmd
            );

            return true;
        } catch (err: any) {
            setError(err.message || 'Failed to send command');
            return false;
        }
    }, [connectedDevice]);

    return {
        connectionState,
        devices,
        quilStatus,
        error,
        startScan,
        stopScan,
        connectToDevice,
        disconnect,
        sendConfig,
        sendCommand,
    };
}
