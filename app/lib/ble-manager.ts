import { Platform } from 'react-native';

export type BLEDevice = {
  id: string;
  name: string;
  rssi: number;
};

export type DeviceStatus = {
  battery: number;
  wifiSSID: string;
  firmwareVersion: string;
};

export type ConnectionState = 'disconnected' | 'scanning' | 'connecting' | 'connected';

const SERVICE_UUID = '4fafc201-1fb5-459e-8fcc-c5c9c331914b';
const CONFIG_CHAR_UUID = 'beb5483e-36e1-4688-b7f5-ea07361b26a8';
const STATUS_CHAR_UUID = 'beb5483e-36e1-4688-b7f5-ea07361b26a9';

class BLEManager {
  private device: BluetoothDevice | null = null;
  private server: BluetoothRemoteGATTServer | null = null;
  private statusCharacteristic: BluetoothRemoteGATTCharacteristic | null = null;
  private configCharacteristic: BluetoothRemoteGATTCharacteristic | null = null;
  private scanning = false;
  private statusListeners: ((status: DeviceStatus) => void)[] = [];

  async initialize(): Promise<void> {
    if (Platform.OS !== 'web') {
      console.log('[BLE Manager] Native Bluetooth requires custom development build');
      console.log('[BLE Manager] This app is currently configured for web Bluetooth only');
      return;
    }

    if (!navigator.bluetooth) {
      throw new Error('Web Bluetooth API is not available in this browser');
    }

    console.log('[BLE Manager] Initialized for Web Bluetooth');
  }

  async requestPermissions(): Promise<boolean> {
    if (Platform.OS !== 'web') {
      throw new Error('Bluetooth is not available in Expo Go. Please use a custom development build.');
    }

    if (!navigator.bluetooth) {
      throw new Error('Web Bluetooth API is not available in this browser');
    }

    return true;
  }

  async startScan(callback: (devices: BLEDevice[]) => void): Promise<void> {
    if (Platform.OS !== 'web') {
      throw new Error('Bluetooth is not available in Expo Go. Please use a custom development build.');
    }

    if (!navigator.bluetooth) {
      throw new Error('Web Bluetooth API is not available in this browser');
    }

    this.scanning = true;
    console.log('[BLE Manager] Starting scan for Quil devices...');

    try {
      const device = await navigator.bluetooth.requestDevice({
        filters: [{ namePrefix: 'Quil' }],
        optionalServices: [SERVICE_UUID]
      });

      if (device) {
        this.device = device;
        const bleDevice: BLEDevice = {
          id: device.id,
          name: device.name || 'Unknown Device',
          rssi: -60,
        };
        callback([bleDevice]);
        console.log('[BLE Manager] Found device:', bleDevice.name);
      }
    } catch (err) {
      if (err instanceof Error && err.name === 'NotFoundError') {
        console.log('[BLE Manager] No device selected');
        callback([]);
      } else {
        throw err;
      }
    } finally {
      this.scanning = false;
    }
  }

  stopScan(): void {
    this.scanning = false;
    console.log('[BLE Manager] Scan stopped');
  }

  async connect(deviceId: string): Promise<void> {
    if (Platform.OS !== 'web') {
      throw new Error('Bluetooth is not available in Expo Go. Please use a custom development build.');
    }

    if (!this.device) {
      throw new Error('No device available to connect');
    }

    console.log('[BLE Manager] Connecting to', deviceId);

    if (!this.device.gatt) {
      throw new Error('Device does not support GATT');
    }

    this.server = await this.device.gatt.connect();
    console.log('[BLE Manager] Connected to GATT server');

    const service = await this.server.getPrimaryService(SERVICE_UUID);
    console.log('[BLE Manager] Got primary service');

    this.configCharacteristic = await service.getCharacteristic(CONFIG_CHAR_UUID);
    this.statusCharacteristic = await service.getCharacteristic(STATUS_CHAR_UUID);
    console.log('[BLE Manager] Got characteristics');

    await this.statusCharacteristic.startNotifications();
    this.statusCharacteristic.addEventListener('characteristicvaluechanged', (event: Event) => {
      const characteristic = event.target as BluetoothRemoteGATTCharacteristic;
      const value = characteristic.value;
      if (value) {
        const decoder = new TextDecoder();
        const jsonString = decoder.decode(value);
        const status = JSON.parse(jsonString) as DeviceStatus;
        this.statusListeners.forEach(listener => listener(status));
      }
    });

    this.device.addEventListener('gattserverdisconnected', () => {
      console.log('[BLE Manager] Device disconnected');
      this.cleanup();
    });

    console.log('[BLE Manager] Connected successfully');
  }

  async disconnect(): Promise<void> {
    console.log('[BLE Manager] Disconnecting');
    
    if (this.server && this.server.connected) {
      this.server.disconnect();
    }
    
    this.cleanup();
  }

  private cleanup(): void {
    this.device = null;
    this.server = null;
    this.statusCharacteristic = null;
    this.configCharacteristic = null;
    this.statusListeners = [];
  }

  isConnected(): boolean {
    return this.server?.connected || false;
  }

  async writeCharacteristic(
    serviceUUID: string,
    characteristicUUID: string,
    data: string
  ): Promise<void> {
    if (!this.server?.connected) {
      throw new Error('Device not connected');
    }

    if (!this.configCharacteristic) {
      throw new Error('Config characteristic not available');
    }

    console.log('[BLE Manager] Writing to characteristic', characteristicUUID);
    console.log('[BLE Manager] Data:', data);

    const encoder = new TextEncoder();
    const dataArray = encoder.encode(data);
    
    await this.configCharacteristic.writeValue(dataArray);
    console.log('[BLE Manager] Write successful');
  }

  async readCharacteristic(
    serviceUUID: string,
    characteristicUUID: string
  ): Promise<string> {
    if (!this.server?.connected) {
      throw new Error('Device not connected');
    }

    if (!this.statusCharacteristic) {
      throw new Error('Status characteristic not available');
    }

    console.log('[BLE Manager] Reading characteristic', characteristicUUID);
    
    const value = await this.statusCharacteristic.readValue();
    const decoder = new TextDecoder();
    const jsonString = decoder.decode(value);
    
    console.log('[BLE Manager] Read:', jsonString);
    return jsonString;
  }

  onStatusUpdate(callback: (status: DeviceStatus) => void): void {
    this.statusListeners.push(callback);
  }


}

export const bleManager = new BLEManager();
