import React from 'react';
import { View, Text } from 'react-native';
import { Battery, Wifi, PackageCheck } from 'lucide-react-native';
import type { DeviceStatus } from '@/lib/device-manager';
import { homeStyles as styles } from '@/styles/index';

export interface DeviceStatusProps {
  deviceStatus: DeviceStatus;
}

export function DeviceStatus({ deviceStatus }: DeviceStatusProps) {
  return (
    <View style={styles.section}>
      <Text style={styles.sectionTitle}>DEVICE STATUS</Text>
      <View style={styles.statusCard}>
        <View style={styles.statusRow}>
          <Battery size={20} color="#00ff88" />
          <Text style={styles.statusLabel}>Battery</Text>
          <Text style={styles.statusValue}>{deviceStatus.battery}%</Text>
        </View>
        <View style={styles.statusRow}>
          <Wifi size={20} color="#00bfff" />
          <Text style={styles.statusLabel}>WiFi</Text>
          <Text style={styles.statusValue}>{deviceStatus.wifiSSID}</Text>
        </View>
        <View style={styles.statusRow}>
          <PackageCheck size={20} color="#ff6b6b" />
          <Text style={styles.statusLabel}>Firmware</Text>
          <Text style={styles.statusValue}>{deviceStatus.firmwareVersion}</Text>
        </View>
      </View>
    </View>
  );
}
