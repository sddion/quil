import React from 'react';
import { View, Text, TouchableOpacity, ActivityIndicator } from 'react-native';
import {
  Bluetooth,
  BluetoothConnected,
  BluetoothSearching,
  AlertTriangle,
} from 'lucide-react-native';
import type { BLEDevice, ConnectionState } from '@/lib/ble-manager';
import { homeStyles as styles } from '@/styles/index';

export interface ConnectionProps {
  connectionState: ConnectionState;
  connectedDevice: BLEDevice | null; 
  devices: BLEDevice[];
  error: string | null;
  retryCount: number;
  maxRetries: number;
  connectionHealth: 'good' | 'warning' | 'error';
  isConnected: boolean;
  onConnect: (device: BLEDevice) => void;
  onDisconnect: () => void;
  onStartScan: () => void;
}

export function Connection({
  connectionState,
  connectedDevice,
  devices,
  error,
  retryCount,
  maxRetries,
  connectionHealth,
  isConnected,
  onConnect,
  onDisconnect,
  onStartScan,
}: ConnectionProps) {
  return (
    <View style={styles.section}>
      <Text style={styles.sectionTitle}>CONNECTION</Text>
      <View style={styles.connectionCard}>
        <View style={styles.connectionHeader}>
          {connectionState === 'connected' && (
            <BluetoothConnected size={24} color="#00ff88" />
          )}
          {connectionState === 'connecting' && (
            <ActivityIndicator size="small" color="#00bfff" />
          )}
          {connectionState === 'scanning' && (
            <BluetoothSearching size={24} color="#00bfff" />
          )}
          {connectionState === 'disconnected' && (
            <Bluetooth size={24} color="#666" />
          )}
          <View style={styles.connectionInfo}>
            <Text style={styles.connectionStatus}>
              {connectionState === 'connected' && 'Connected'}
              {connectionState === 'connecting' && `Connecting... (${retryCount}/${maxRetries})`}
              {connectionState === 'scanning' && 'Scanning...'}
              {connectionState === 'disconnected' && 'Disconnected'}
            </Text>
            {connectedDevice && (
              <Text style={styles.deviceName}>{connectedDevice.name}</Text>
            )}
            {isConnected && connectionHealth && (
              <View style={styles.connectionHealthRow}>
                <View style={[
                  styles.healthIndicator,
                  { backgroundColor: connectionHealth === 'good' ? '#00ff88' : connectionHealth === 'warning' ? '#ffaa00' : '#ff4444' }
                ]} />
                <Text style={styles.healthText}>
                  {connectionHealth === 'good' ? 'Stable' : connectionHealth === 'warning' ? 'Fair' : 'Weak'}
                </Text>
              </View>
            )}
          </View>
          {isConnected && (
            <TouchableOpacity
              style={styles.disconnectButton}
              onPress={onDisconnect}
            >
              <Text style={styles.disconnectButtonText}>Disconnect</Text>
            </TouchableOpacity>
          )}
        </View>

        {error && (
          <View style={styles.errorBanner}>
            <AlertTriangle size={16} color="#ff4444" />
            <Text style={styles.errorText}>{error}</Text>
          </View>
        )}

        {!isConnected && connectionState !== 'scanning' && devices.length === 0 && (
          <TouchableOpacity
            style={styles.scanButton}
            onPress={onStartScan}
          >
            <BluetoothSearching size={20} color="#00bfff" />
            <Text style={styles.scanButtonText}>Scan for Devices</Text>
          </TouchableOpacity>
        )}

        {!isConnected && devices.length > 0 && (
          <View style={styles.deviceList}>
            <Text style={styles.deviceListTitle}>Available Devices</Text>
            {devices.map((device) => (
              <TouchableOpacity
                key={device.id}
                style={styles.deviceItem}
                onPress={() => onConnect(device)}
              >
                <View style={styles.deviceItemContent}>
                  <Text style={styles.deviceItemName}>{device.name}</Text>
                  <Text style={styles.deviceItemId}>{device.id}</Text>
                </View>
                <Text style={styles.deviceItemRSSI}>{device.rssi} dBm</Text>
              </TouchableOpacity>
            ))}
          </View>
        )}
      </View>
    </View>
  );
}
