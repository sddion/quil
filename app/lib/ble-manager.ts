import { Platform, PermissionsAndroid } from 'react-native';
import { BleManager as NativeBleManager, Device, State } from 'react-native-ble-plx';

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
  private nativeManager: NativeBleManager | null = null;
  private connectedDevice: Device | null = null;
  private scanning = false;
  private statusListeners: ((status: DeviceStatus) => void)[] = [];

  // Web Bluetooth (for web platform)
  private webDevice: BluetoothDevice | null = null;
  private webServer: BluetoothRemoteGATTServer | null = null;
  private webStatusChar: BluetoothRemoteGATTCharacteristic | null = null;
  private webConfigChar: BluetoothRemoteGATTCharacteristic | null = null;

  async initialize(): Promise<void> {
    if (Platform.OS === 'web') {
      if (!navigator.bluetooth) {
        throw new Error('Web Bluetooth API is not available in this browser');
      }
      console.log('[BLE Manager] Initialized for Web Bluetooth');
      return;
    }

    // Native platform - create BleManager instance
    this.nativeManager = new NativeBleManager();
    
    // Wait for Bluetooth to be powered on
    const state = await this.nativeManager.state();
    if (state !== State.PoweredOn) {
      console.log('[BLE Manager] Waiting for Bluetooth to power on...');
      await new Promise<void>((resolve) => {
        const subscription = this.nativeManager!.onStateChange((newState) => {
          if (newState === State.PoweredOn) {
            subscription.remove();
            resolve();
          }
        }, true);
      });
    }
    
    console.log('[BLE Manager] Initialized for native Bluetooth');
  }

  async requestPermissions(): Promise<boolean> {
    if (Platform.OS === 'web') {
      if (!navigator.bluetooth) {
        throw new Error('Web Bluetooth API is not available');
      }
      return true;
    }

    if (Platform.OS === 'android') {
      const apiLevel = Platform.Version;
      
      if (apiLevel >= 31) {
        // Android 12+ (API 31+)
        const scanPermission = await PermissionsAndroid.request(
          PermissionsAndroid.PERMISSIONS.BLUETOOTH_SCAN,
          {
            title: 'Bluetooth Scan Permission',
            message: 'Quil needs Bluetooth scanning to find your device.',
            buttonPositive: 'Allow',
          }
        );
        const connectPermission = await PermissionsAndroid.request(
          PermissionsAndroid.PERMISSIONS.BLUETOOTH_CONNECT,
          {
            title: 'Bluetooth Connect Permission',
            message: 'Quil needs Bluetooth connection to communicate with your device.',
            buttonPositive: 'Allow',
          }
        );
        
        return (
          scanPermission === PermissionsAndroid.RESULTS.GRANTED &&
          connectPermission === PermissionsAndroid.RESULTS.GRANTED
        );
      } else {
        // Android < 12 - need fine location
        const locationPermission = await PermissionsAndroid.request(
          PermissionsAndroid.PERMISSIONS.ACCESS_FINE_LOCATION,
          {
            title: 'Location Permission',
            message: 'Quil needs location access to scan for Bluetooth devices.',
            buttonPositive: 'Allow',
          }
        );
        
        return locationPermission === PermissionsAndroid.RESULTS.GRANTED;
      }
    }

    // iOS doesn't need runtime permission requests for BLE
    return true;
  }

  async startScan(callback: (devices: BLEDevice[]) => void): Promise<void> {
    if (Platform.OS === 'web') {
      return this.startWebScan(callback);
    }

    if (!this.nativeManager) {
      throw new Error('BLE Manager not initialized');
    }

    this.scanning = true;
    const foundDevices: Map<string, BLEDevice> = new Map();
    
    console.log('[BLE Manager] Starting native scan for Quil devices...');

    this.nativeManager.startDeviceScan(
      [SERVICE_UUID], // Filter by service UUID
      { allowDuplicates: false },
      (error, device) => {
        if (error) {
          console.error('[BLE Manager] Scan error:', error);
          this.scanning = false;
          return;
        }

        if (device && device.name?.startsWith('Quil')) {
          const bleDevice: BLEDevice = {
            id: device.id,
            name: device.name || 'Unknown Device',
            rssi: device.rssi || -100,
          };
          foundDevices.set(device.id, bleDevice);
          callback(Array.from(foundDevices.values()));
          console.log('[BLE Manager] Found device:', bleDevice.name);
        }
      }
    );

    // Auto-stop after 10 seconds
    setTimeout(() => {
      if (this.scanning) {
        this.stopScan();
      }
    }, 10000);
  }

  private async startWebScan(callback: (devices: BLEDevice[]) => void): Promise<void> {
    if (!navigator.bluetooth) {
      throw new Error('Web Bluetooth API is not available');
    }

    this.scanning = true;
    console.log('[BLE Manager] Starting web scan for Quil devices...');

    try {
      const device = await navigator.bluetooth.requestDevice({
        filters: [{ namePrefix: 'Quil' }],
        optionalServices: [SERVICE_UUID]
      });

      if (device) {
        this.webDevice = device;
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
    
    if (Platform.OS !== 'web' && this.nativeManager) {
      this.nativeManager.stopDeviceScan();
    }
    
    console.log('[BLE Manager] Scan stopped');
  }

  async connect(deviceId: string): Promise<void> {
    if (Platform.OS === 'web') {
      return this.connectWeb(deviceId);
    }

    if (!this.nativeManager) {
      throw new Error('BLE Manager not initialized');
    }

    console.log('[BLE Manager] Connecting to', deviceId);

    // Connect to device
    this.connectedDevice = await this.nativeManager.connectToDevice(deviceId);
    console.log('[BLE Manager] Connected to device');

    // Discover services and characteristics
    await this.connectedDevice.discoverAllServicesAndCharacteristics();
    console.log('[BLE Manager] Discovered services and characteristics');

    // Subscribe to status notifications
    this.connectedDevice.monitorCharacteristicForService(
      SERVICE_UUID,
      STATUS_CHAR_UUID,
      (error, characteristic) => {
        if (error) {
          console.error('[BLE Manager] Monitor error:', error);
          return;
        }
        if (characteristic?.value) {
          try {
            const decoded = Buffer.from(characteristic.value, 'base64').toString('utf-8');
            const status = JSON.parse(decoded) as DeviceStatus;
            this.statusListeners.forEach(listener => listener(status));
          } catch (e) {
            console.error('[BLE Manager] Failed to parse status:', e);
          }
        }
      }
    );

    // Monitor disconnection
    this.nativeManager.onDeviceDisconnected(deviceId, (error, device) => {
      console.log('[BLE Manager] Device disconnected');
      this.connectedDevice = null;
    });

    console.log('[BLE Manager] Connected successfully');
  }

  private async connectWeb(deviceId: string): Promise<void> {
    if (!this.webDevice) {
      throw new Error('No device available to connect');
    }

    console.log('[BLE Manager] Connecting to web device', deviceId);

    if (!this.webDevice.gatt) {
      throw new Error('Device does not support GATT');
    }

    this.webServer = await this.webDevice.gatt.connect();
    console.log('[BLE Manager] Connected to GATT server');

    const service = await this.webServer.getPrimaryService(SERVICE_UUID);
    this.webConfigChar = await service.getCharacteristic(CONFIG_CHAR_UUID);
    this.webStatusChar = await service.getCharacteristic(STATUS_CHAR_UUID);
    
    await this.webStatusChar.startNotifications();
    this.webStatusChar.addEventListener('characteristicvaluechanged', (event: Event) => {
      const characteristic = event.target as BluetoothRemoteGATTCharacteristic;
      const value = characteristic.value;
      if (value) {
        const decoder = new TextDecoder();
        const jsonString = decoder.decode(value);
        const status = JSON.parse(jsonString) as DeviceStatus;
        this.statusListeners.forEach(listener => listener(status));
      }
    });

    this.webDevice.addEventListener('gattserverdisconnected', () => {
      console.log('[BLE Manager] Web device disconnected');
      this.cleanup();
    });

    console.log('[BLE Manager] Web connected successfully');
  }

  async disconnect(): Promise<void> {
    console.log('[BLE Manager] Disconnecting');
    
    if (Platform.OS === 'web') {
      if (this.webServer?.connected) {
        this.webServer.disconnect();
      }
      this.cleanup();
      return;
    }

    if (this.connectedDevice) {
      await this.connectedDevice.cancelConnection();
      this.connectedDevice = null;
    }
  }

  private cleanup(): void {
    this.webDevice = null;
    this.webServer = null;
    this.webStatusChar = null;
    this.webConfigChar = null;
    this.statusListeners = [];
  }

  isConnected(): boolean {
    if (Platform.OS === 'web') {
      return this.webServer?.connected || false;
    }
    return this.connectedDevice !== null;
  }

  async writeCharacteristic(
    serviceUUID: string,
    characteristicUUID: string,
    data: string
  ): Promise<void> {
    if (Platform.OS === 'web') {
      if (!this.webServer?.connected || !this.webConfigChar) {
        throw new Error('Device not connected');
      }
      const encoder = new TextEncoder();
      await this.webConfigChar.writeValue(encoder.encode(data));
      console.log('[BLE Manager] Web write successful');
      return;
    }

    if (!this.connectedDevice) {
      throw new Error('Device not connected');
    }

    const base64Data = Buffer.from(data, 'utf-8').toString('base64');
    await this.connectedDevice.writeCharacteristicWithResponseForService(
      SERVICE_UUID,
      CONFIG_CHAR_UUID,
      base64Data
    );
    console.log('[BLE Manager] Native write successful');
  }

  async readCharacteristic(
    serviceUUID: string,
    characteristicUUID: string
  ): Promise<string> {
    if (Platform.OS === 'web') {
      if (!this.webServer?.connected || !this.webStatusChar) {
        throw new Error('Device not connected');
      }
      const value = await this.webStatusChar.readValue();
      const decoder = new TextDecoder();
      return decoder.decode(value);
    }

    if (!this.connectedDevice) {
      throw new Error('Device not connected');
    }

    const characteristic = await this.connectedDevice.readCharacteristicForService(
      SERVICE_UUID,
      STATUS_CHAR_UUID
    );
    
    if (characteristic.value) {
      return Buffer.from(characteristic.value, 'base64').toString('utf-8');
    }
    
    return '{}';
  }

  onStatusUpdate(callback: (status: DeviceStatus) => void): void {
    this.statusListeners.push(callback);
  }
}

export const bleManager = new BLEManager();
