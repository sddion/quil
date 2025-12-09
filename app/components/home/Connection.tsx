import React from 'react';
import { View, Text, TouchableOpacity, ActivityIndicator, TextInput } from 'react-native';
import {
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
            <ActivityIndicator size="small" color="#00bfff" />
          )}
          {connectionState === 'disconnected' && (
            <WifiOff size={24} color="#666" />
          )}
          <View style={styles.connectionInfo}>
            <Text style={styles.connectionStatus}>
              {connectionState === 'connected' && 'Connected via WiFi'}
              {connectionState === 'connecting' && 'Connecting...'}
              {connectionState === 'scanning' && 'Scanning local network...'}
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
          <View style={{ marginTop: 4 }}>
            <Text style={[styles.deviceListTitle, { marginBottom: 8 }]}>Enter Device IP Address:</Text>
            <TextInput
              style={[styles.input, { color: '#fff', height: 40, paddingVertical: 8 }]}
              placeholder="192.168.x.x"
              placeholderTextColor="#666"
              value={manualIp}
              onChangeText={setManualIp}
              keyboardType="numeric"
              autoCapitalize="none"
              autoCorrect={false}
            />
            <TouchableOpacity
              style={[styles.scanButton, { marginTop: 8 }]}
              onPress={() => onConnect(manualIp.trim())}
              disabled={!manualIp.trim()}
            >
              <Text style={styles.scanButtonText}>Connect</Text>
            </TouchableOpacity>
          </View>
        )}
      </View>
    </View>
  );
}
