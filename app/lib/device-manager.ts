/**
 * Quil Device Manager - HTTP-based device communication
 * Connects to Quil device over WiFi instead of Bluetooth
 */

export type DeviceStatus = {
  battery: number;
  wifiConnected: boolean;
  wifiSSID: string;
  firmwareVersion: string;
  timezone: number;
  brightness: number;
  theme: number;
};

export type DeviceConfig = {
  ssid?: string;
  password?: string;
  tz?: number;
  brightness?: number;
  theme?: number;
  wk?: string; // weather api key
  wl?: string; // weather location
};

export type ConnectionState = 'disconnected' | 'scanning' | 'connecting' | 'connected';

const DEFAULT_DEVICE_IP = 'quil.local';
const API_TIMEOUT = 5000;

class DeviceManager {
  private deviceIp: string | null = null;
  private isConnected = false;
  private statusListeners: ((status: DeviceStatus) => void)[] = [];
  private pollInterval: ReturnType<typeof setInterval> | null = null;

  /**
   * Discover device on local network
   * Tries mDNS first, then falls back to common IPs
   */
  async discoverDevice(): Promise<string | null> {
    console.log('[Device] Discovering device on network...');
    
    // Try mDNS hostname first
    const hostnames = [
      'quil.local',
      '192.168.1.1', // Common router IP range
      '192.168.4.1', // ESP32 AP default
    ];

    for (const host of hostnames) {
      try {
        const response = await this.fetchWithTimeout(`http://${host}/api/status`, {
          method: 'GET',
        });
        if (response.ok) {
          console.log(`[Device] Found device at ${host}`);
          return host;
        }
      } catch {
        // Continue to next hostname
      }
    }

    console.log('[Device] Device not found on network');
    return null;
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
      console.error('[Device] Connection failed:', error);
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
