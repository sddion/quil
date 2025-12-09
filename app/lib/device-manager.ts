/**
 * Quil Device Manager - HTTP-based device communication
 * Connects to Quil device over WiFi instead of Bluetooth
 */
import * as Network from 'expo-network';

export type DeviceStatus = {
  battery: number;
  wifiConnected: boolean;
  wifiSSID: string;
  firmwareVersion: string;
  timezone: number;
  brightness: number;
  theme: number;
  serverUrl?: string;
};

export type DeviceConfig = {
  ssid?: string;
  password?: string;
  tz?: number;
  brightness?: number;
  theme?: number;
  wk?: string; // weather api key
  wl?: string; // weather location
  server_url?: string; // WebSocket server URL
};

export type ConnectionState = 'disconnected' | 'scanning' | 'connecting' | 'connected';

const DEFAULT_DEVICE_IP = 'quil.local';
const API_TIMEOUT = 3000; // Reduced timeout for faster scanning

class DeviceManager {
  private deviceIp: string | null = null;
  private isConnected = false;
  private statusListeners: ((status: DeviceStatus) => void)[] = [];
  private pollInterval: ReturnType<typeof setInterval> | null = null;

  /**
   * Discover device on local network
   * Tries mDNS first, then common IPs, then performs a subnet scan
   */
  async discoverDevice(): Promise<string | null> {
    console.log('[Device] Discovering device on network...');
    
    // 1. Try mDNS hostname and common defaults first (fastest)
    const priorityHostnames = [
      'quil.local',
      '192.168.4.1', // ESP32 AP default
    ];

    for (const host of priorityHostnames) {
      if (await this.checkDevice(host)) {
        return host;
      }
    }

    // 2. Perform subnet scan
    console.log('[Device] Performing subnet scan...');
    try {
      const ip = await Network.getIpAddressAsync();
      if (!ip || ip === '0.0.0.0') {
        console.log('[Device] Could not determine local IP');
        return null;
      }

      console.log(`[Device] Local IP: ${ip}`);
      const subnet = ip.substring(0, ip.lastIndexOf('.'));
      
      // Create batches of IPs to scan to avoid overwhelming the network/thread
      const BATCH_SIZE = 20;
      const foundDevice = await this.scanSubnet(subnet, BATCH_SIZE);
      
      if (foundDevice) {
        return foundDevice;
      }

    } catch (e) {
      console.error('[Device] Subnet scan failed:', e);
    }

    console.log('[Device] Device not found on network');
    return null;
  }

  private async scanSubnet(subnet: string, batchSize: number): Promise<string | null> {
    for (let i = 1; i < 255; i += batchSize) {
      const batch: Promise<string | null>[] = [];
      for (let j = 0; j < batchSize && (i + j) < 255; j++) {
        const host = `${subnet}.${i + j}`;
        batch.push(this.checkDevice(host).then(found => found ? host : null));
      }

      const results = await Promise.all(batch);
      const found = results.find(ip => ip !== null);
      if (found) {
        console.log(`[Device] Found device at ${found}`);
        return found;
      }
    }
    return null;
  }

  private async checkDevice(host: string): Promise<boolean> {
    try {
      // Use efficient timeout for check
      const controller = new AbortController();
      const timeout = setTimeout(() => controller.abort(), 1500);

      const response = await fetch(`http://${host}/api/status`, {
        method: 'GET',
        signal: controller.signal,
      }).catch(() => null);

      clearTimeout(timeout);

      if (response && response.ok) {
        console.log(`[Device] Found device at ${host}`);
        return true;
      }
    } catch {
      // Ignore errors
    }
    return false;
  }

  /**
   * Connect to device at specified IP
   */
  async connect(ip?: string): Promise<boolean> {
    const targetIp = ip || await this.discoverDevice();
    
    if (!targetIp) {
      console.log('[Device] No device found to connect');
      return false;
    }

    try {
      const response = await this.fetchWithTimeout(`http://${targetIp}/api/status`, {
        method: 'GET',
      });

      if (response.ok) {
        this.deviceIp = targetIp;
        this.isConnected = true;
        console.log(`[Device] Connected to ${targetIp}`);
        
        // Start polling for status updates
        this.startStatusPolling();
        return true;
      }
    } catch (error) {
      console.error(`[Device] Connection failed to ${targetIp}:`, error);
    }

    return false;
  }

  /**
   * Disconnect from device
   */
  disconnect(): void {
    this.stopStatusPolling();
    this.deviceIp = null;
    this.isConnected = false;
    console.log('[Device] Disconnected');
  }

  /**
   * Get current connection state
   */
  getConnectionState(): ConnectionState {
    if (this.isConnected && this.deviceIp) return 'connected';
    return 'disconnected';
  }

  /**
   * Get device IP address
   */
  getDeviceIp(): string | null {
    return this.deviceIp;
  }

  /**
   * Send configuration to device
   */
  async sendConfig(config: DeviceConfig): Promise<boolean> {
    if (!this.deviceIp) {
      console.log('[Device] Not connected');
      return false;
    }

    try {
      const response = await this.fetchWithTimeout(`http://${this.deviceIp}/api/config`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify(config),
      });

      const result = await response.json();
      console.log('[Device] Config sent:', result);
      return result.success === true;
    } catch (error) {
      console.error('[Device] Failed to send config:', error);
      return false;
    }
  }

  /**
   * Get device status
   */
  async getStatus(): Promise<DeviceStatus | null> {
    if (!this.deviceIp) return null;

    try {
      const response = await this.fetchWithTimeout(`http://${this.deviceIp}/api/status`, {
        method: 'GET',
      });

      if (response.ok) {
        return await response.json();
      }
    } catch (error) {
      console.error('[Device] Failed to get status:', error);
    }

    return null;
  }

  /**
   * Send command to device (restart, reset, etc)
   */
  async sendCommand(cmd: string): Promise<boolean> {
    if (!this.deviceIp) return false;

    try {
      const response = await this.fetchWithTimeout(`http://${this.deviceIp}/api/config`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ cmd }),
      });

      return response.ok;
    } catch (error) {
      console.error('[Device] Command failed:', error);
      return false;
    }
  }

  /**
   * Subscribe to status updates
   */
  onStatusUpdate(callback: (status: DeviceStatus) => void): () => void {
    this.statusListeners.push(callback);
    return () => {
      this.statusListeners = this.statusListeners.filter(cb => cb !== callback);
    };
  }

  private startStatusPolling(): void {
    this.stopStatusPolling();
    
    this.pollInterval = setInterval(async () => {
      const status = await this.getStatus();
      if (status) {
        this.statusListeners.forEach(cb => cb(status));
      } else {
        // Connection lost
        this.isConnected = false;
        this.stopStatusPolling();
      }
    }, 5000);
  }

  private stopStatusPolling(): void {
    if (this.pollInterval) {
      clearInterval(this.pollInterval);
      this.pollInterval = null;
    }
  }

  private async fetchWithTimeout(url: string, options: RequestInit): Promise<Response> {
    const controller = new AbortController();
    const timeout = setTimeout(() => controller.abort(), API_TIMEOUT);

    try {
      const response = await fetch(url, {
        ...options,
        signal: controller.signal,
      });
      return response;
    } finally {
      clearTimeout(timeout);
    }
  }
}

// Singleton instance
export const deviceManager = new DeviceManager();
