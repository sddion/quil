import React from 'react';
import { View, Text, TouchableOpacity, ActivityIndicator, TextInput } from 'react-native';
import {
  Wifi,
  WifiOff,
  Search,
  AlertTriangle,
  CheckCircle,
} from 'lucide-react-native';
import type { ConnectionState } from '@/lib/device-manager';
import { homeStyles as styles } from '@/styles/index';

export interface ConnectionProps {
  connectionState: ConnectionState;
  deviceIp: string | null;
  error: string | null;
  isConnected: boolean;
  onConnect: (ip?: string) => void;
  onDisconnect: () => void;
  onDiscover: () => void;
}

export function Connection({
  connectionState,
  deviceIp,
  error,
  isConnected,
  onConnect,
  onDisconnect,
  onDiscover,
}: ConnectionProps) {
  const [manualIp, setManualIp] = React.useState('');

  return (
    <View style={styles.section}>
      <Text style={styles.sectionTitle}>CONNECTION</Text>
      <View style={styles.connectionCard}>
        <View style={styles.connectionHeader}>
          {connectionState === 'connected' && (
            <CheckCircle size={24} color="#00ff88" />
          )}
          {connectionState === 'connecting' && (
            <ActivityIndicator size="small" color="#00bfff" />
          )}
          {connectionState === 'scanning' && (
            <Search size={24} color="#00bfff" />
          )}
          {connectionState === 'disconnected' && (
            <WifiOff size={24} color="#666" />
          )}
          <View style={styles.connectionInfo}>
            <Text style={styles.connectionStatus}>
              {connectionState === 'connected' && 'Connected via WiFi'}
              {connectionState === 'connecting' && 'Connecting...'}
              {connectionState === 'scanning' && 'Scanning network...'}
              {connectionState === 'disconnected' && 'Disconnected'}
            </Text>
            {deviceIp && (
              <Text style={styles.deviceName}>{deviceIp}</Text>
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

        {!isConnected && connectionState !== 'scanning' && connectionState !== 'connecting' && (
          <>
            <TouchableOpacity
              style={styles.scanButton}
              onPress={() => onConnect()}
            >
              <Wifi size={20} color="#00bfff" />
              <Text style={styles.scanButtonText}>Find Quil on Network</Text>
            </TouchableOpacity>

            <View style={{ marginTop: 12 }}>
              <Text style={[styles.deviceListTitle, { marginBottom: 8 }]}>Or enter IP manually:</Text>
              <View style={{ flexDirection: 'row', gap: 8 }}>
                <TextInput
                  style={[styles.input, { flex: 1, color: '#fff' }]}
                  placeholder="192.168.1.x"
                  placeholderTextColor="#666"
                  value={manualIp}
                  onChangeText={setManualIp}
                  keyboardType="numeric"
                />
                <TouchableOpacity
                  style={[styles.scanButton, { flex: 0, paddingHorizontal: 16 }]}
                  onPress={() => onConnect(manualIp)}
                  disabled={!manualIp}
                >
                  <Text style={styles.scanButtonText}>Connect</Text>
                </TouchableOpacity>
              </View>
            </View>
          </>
        )}
      </View>
    </View>
  );
}
