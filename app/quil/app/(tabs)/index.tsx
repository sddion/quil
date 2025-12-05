import React, { useEffect } from 'react';
import { StyleSheet, View, ScrollView, Pressable, ActivityIndicator } from 'react-native';
import { ThemedText } from '@/components/themed-text';
import { ThemedView } from '@/components/themed-view';
import { IconSymbol } from '@/components/ui/icon-symbol';
import { Colors } from '@/constants/theme';
import { useColorScheme } from '@/hooks/use-color-scheme';
import { useBLE } from '@/hooks/use-ble';

export default function DashboardScreen() {
  const colorScheme = useColorScheme() ?? 'dark';
  const colors = Colors[colorScheme];

  const {
    connectionState,
    devices,
    quilStatus,
    error,
    startScan,
    stopScan,
    connectToDevice,
    disconnect,
    sendCommand,
  } = useBLE();

  const isConnected = connectionState === 'connected';
  const isScanning = connectionState === 'scanning';
  const isConnecting = connectionState === 'connecting';

  const handleSyncTime = () => sendCommand('sync_time');
  const handleUpdateWeather = () => sendCommand('update_weather');
  const handleRestart = () => sendCommand('restart');
  const handleCheckUpdates = () => sendCommand('check_updates');

  return (
    <ThemedView style={styles.container}>
      <ScrollView style={styles.scrollView} contentContainerStyle={styles.scrollContent}>
        {/* Header */}
        <View style={styles.header}>
          <ThemedText style={styles.title}>Quil</ThemedText>
          <ThemedText style={styles.subtitle}>Device Dashboard</ThemedText>
        </View>

        {/* Connection Card */}
        <View style={[styles.card, { backgroundColor: colors.card, borderColor: colors.cardBorder }]}>
          <View style={styles.cardHeader}>
            <IconSymbol name="antenna.radiowaves.left.and.right" size={24} color={colors.tint} />
            <ThemedText style={styles.cardTitle}>Connection</ThemedText>
          </View>

          {!isConnected ? (
            <View style={styles.connectionSection}>
              {isScanning ? (
                <>
                  <ActivityIndicator color={colors.tint} style={{ marginBottom: 12 }} />
                  <ThemedText style={styles.scanningText}>Scanning for Quil devices...</ThemedText>
                  {devices.map((device) => (
                    <Pressable
                      key={device.id}
                      style={[styles.deviceItem, { backgroundColor: colors.inputBackground, borderColor: colors.inputBorder }]}
                      onPress={() => connectToDevice(device.id)}
                    >
                      <View style={styles.deviceInfo}>
                        <ThemedText style={styles.deviceName}>{device.name}</ThemedText>
                        <ThemedText style={styles.deviceRssi}>Signal: {device.rssi} dBm</ThemedText>
                      </View>
                      <IconSymbol name="chevron.right" size={16} color={colors.icon} />
                    </Pressable>
                  ))}
                  <Pressable style={[styles.scanButton, { borderColor: colors.cardBorder }]} onPress={stopScan}>
                    <ThemedText style={styles.scanButtonText}>Stop Scanning</ThemedText>
                  </Pressable>
                </>
              ) : isConnecting ? (
                <>
                  <ActivityIndicator color={colors.tint} style={{ marginBottom: 12 }} />
                  <ThemedText style={styles.scanningText}>Connecting...</ThemedText>
                </>
              ) : (
                <>
                  <View style={[styles.statusIndicator, { backgroundColor: colors.danger }]} />
                  <ThemedText style={styles.disconnectedText}>Not Connected</ThemedText>
                  <Pressable style={[styles.scanButton, { backgroundColor: colors.tint }]} onPress={startScan}>
                    <IconSymbol name="magnifyingglass" size={18} color="#000" />
                    <ThemedText style={styles.scanButtonTextDark}>Scan for Devices</ThemedText>
                  </Pressable>
                </>
              )}
              {error && <ThemedText style={[styles.errorText, { color: colors.danger }]}>{error}</ThemedText>}
            </View>
          ) : (
            <View style={styles.connectedSection}>
              <View style={styles.connectedHeader}>
                <View style={[styles.statusIndicator, { backgroundColor: colors.success }]} />
                <ThemedText style={styles.connectedText}>Connected to Quil</ThemedText>
              </View>
              <Pressable style={[styles.disconnectButton, { borderColor: colors.danger }]} onPress={disconnect}>
                <ThemedText style={[styles.disconnectText, { color: colors.danger }]}>Disconnect</ThemedText>
              </Pressable>
            </View>
          )}
        </View>

        {/* Device Status Card - Only show when connected */}
        {isConnected && quilStatus && (
          <View style={[styles.card, { backgroundColor: colors.card, borderColor: colors.cardBorder }]}>
            <View style={styles.cardHeader}>
              <IconSymbol name="info.circle.fill" size={24} color={colors.tint} />
              <ThemedText style={styles.cardTitle}>Device Status</ThemedText>
            </View>
            <View style={styles.statusGrid}>
              <View style={styles.statusItem}>
                <ThemedText style={styles.statusLabel}>Battery</ThemedText>
                <View style={styles.statusValue}>
                  <IconSymbol
                    name={quilStatus.battery > 20 ? "battery.100" : "battery.25"}
                    size={18}
                    color={quilStatus.battery > 20 ? colors.success : colors.danger}
                  />
                  <ThemedText style={[styles.statusText, { color: quilStatus.battery > 20 ? colors.success : colors.danger }]}>
                    {quilStatus.battery}%
                  </ThemedText>
                </View>
              </View>
              <View style={styles.statusItem}>
                <ThemedText style={styles.statusLabel}>WiFi</ThemedText>
                <View style={styles.statusValue}>
                  <IconSymbol
                    name={quilStatus.wifiConnected ? "wifi" : "wifi.slash"}
                    size={18}
                    color={quilStatus.wifiConnected ? colors.success : colors.danger}
                  />
                  <ThemedText style={styles.statusText}>
                    {quilStatus.wifiConnected ? quilStatus.wifiSsid || 'Connected' : 'Disconnected'}
                  </ThemedText>
                </View>
              </View>
              <View style={styles.statusItem}>
                <ThemedText style={styles.statusLabel}>Brightness</ThemedText>
                <ThemedText style={styles.statusText}>{Math.round((quilStatus.brightness / 255) * 100)}%</ThemedText>
              </View>
              <View style={styles.statusItem}>
                <ThemedText style={styles.statusLabel}>Firmware</ThemedText>
                <ThemedText style={styles.statusText}>{quilStatus.firmwareVersion}</ThemedText>
              </View>
            </View>
          </View>
        )}

        {/* Quick Actions Card - Only show when connected */}
        {isConnected && (
          <View style={[styles.card, { backgroundColor: colors.card, borderColor: colors.cardBorder }]}>
            <View style={styles.cardHeader}>
              <IconSymbol name="bolt.fill" size={24} color={colors.tint} />
              <ThemedText style={styles.cardTitle}>Quick Actions</ThemedText>
            </View>
            <View style={styles.actionsGrid}>
              <Pressable
                style={[styles.actionButton, { backgroundColor: colors.inputBackground, borderColor: colors.inputBorder }]}
                onPress={handleSyncTime}
              >
                <IconSymbol name="clock.fill" size={28} color={colors.tint} />
                <ThemedText style={styles.actionText}>Sync Time</ThemedText>
              </Pressable>
              <Pressable
                style={[styles.actionButton, { backgroundColor: colors.inputBackground, borderColor: colors.inputBorder }]}
                onPress={handleUpdateWeather}
              >
                <IconSymbol name="cloud.fill" size={28} color={colors.tint} />
                <ThemedText style={styles.actionText}>Update Weather</ThemedText>
              </Pressable>
              <Pressable
                style={[styles.actionButton, { backgroundColor: colors.inputBackground, borderColor: colors.inputBorder }]}
                onPress={handleRestart}
              >
                <IconSymbol name="arrow.clockwise" size={28} color={colors.tint} />
                <ThemedText style={styles.actionText}>Restart Device</ThemedText>
              </Pressable>
              <Pressable
                style={[styles.actionButton, { backgroundColor: colors.inputBackground, borderColor: colors.inputBorder }]}
                onPress={handleCheckUpdates}
              >
                <IconSymbol name="arrow.down.circle.fill" size={28} color={colors.tint} />
                <ThemedText style={styles.actionText}>Check Updates</ThemedText>
              </Pressable>
            </View>
          </View>
        )}

        {/* Placeholder when not connected */}
        {!isConnected && (
          <View style={[styles.card, styles.placeholderCard, { backgroundColor: colors.card, borderColor: colors.cardBorder }]}>
            <IconSymbol name="antenna.radiowaves.left.and.right" size={48} color={colors.icon} style={{ opacity: 0.3 }} />
            <ThemedText style={styles.placeholderText}>Connect to a Quil device to view status and controls</ThemedText>
          </View>
        )}
      </ScrollView>
    </ThemedView>
  );
}

const styles = StyleSheet.create({
  container: {
    flex: 1,
  },
  scrollView: {
    flex: 1,
  },
  scrollContent: {
    padding: 20,
    paddingTop: 60,
  },
  header: {
    marginBottom: 24,
  },
  title: {
    fontSize: 36,
    fontWeight: '700',
    letterSpacing: -1,
  },
  subtitle: {
    fontSize: 16,
    opacity: 0.6,
    marginTop: 4,
  },
  card: {
    borderRadius: 16,
    padding: 20,
    marginBottom: 16,
    borderWidth: 1,
  },
  cardHeader: {
    flexDirection: 'row',
    alignItems: 'center',
    marginBottom: 16,
    gap: 10,
  },
  cardTitle: {
    fontSize: 18,
    fontWeight: '600',
  },
  connectionSection: {
    alignItems: 'center',
  },
  statusIndicator: {
    width: 12,
    height: 12,
    borderRadius: 6,
    marginBottom: 8,
  },
  disconnectedText: {
    fontSize: 16,
    opacity: 0.6,
    marginBottom: 16,
  },
  scanningText: {
    fontSize: 14,
    opacity: 0.6,
    marginBottom: 12,
  },
  scanButton: {
    flexDirection: 'row',
    alignItems: 'center',
    justifyContent: 'center',
    gap: 8,
    paddingVertical: 12,
    paddingHorizontal: 24,
    borderRadius: 10,
    borderWidth: 1,
  },
  scanButtonText: {
    fontSize: 14,
    fontWeight: '500',
  },
  scanButtonTextDark: {
    fontSize: 14,
    fontWeight: '600',
    color: '#000',
  },
  deviceItem: {
    width: '100%',
    flexDirection: 'row',
    alignItems: 'center',
    justifyContent: 'space-between',
    padding: 14,
    borderRadius: 10,
    borderWidth: 1,
    marginBottom: 8,
  },
  deviceInfo: {
    flex: 1,
  },
  deviceName: {
    fontSize: 15,
    fontWeight: '500',
  },
  deviceRssi: {
    fontSize: 12,
    opacity: 0.6,
    marginTop: 2,
  },
  connectedSection: {
    alignItems: 'center',
  },
  connectedHeader: {
    flexDirection: 'row',
    alignItems: 'center',
    gap: 8,
    marginBottom: 12,
  },
  connectedText: {
    fontSize: 16,
    fontWeight: '500',
  },
  disconnectButton: {
    paddingVertical: 10,
    paddingHorizontal: 20,
    borderRadius: 8,
    borderWidth: 1,
  },
  disconnectText: {
    fontSize: 14,
    fontWeight: '500',
  },
  errorText: {
    fontSize: 13,
    marginTop: 12,
    textAlign: 'center',
  },
  statusGrid: {
    gap: 12,
  },
  statusItem: {
    flexDirection: 'row',
    justifyContent: 'space-between',
    alignItems: 'center',
  },
  statusLabel: {
    fontSize: 14,
    opacity: 0.6,
  },
  statusValue: {
    flexDirection: 'row',
    alignItems: 'center',
    gap: 8,
  },
  statusText: {
    fontSize: 14,
    fontWeight: '500',
  },
  actionsGrid: {
    flexDirection: 'row',
    flexWrap: 'wrap',
    gap: 12,
  },
  actionButton: {
    width: '47%',
    padding: 16,
    borderRadius: 12,
    alignItems: 'center',
    gap: 8,
    borderWidth: 1,
  },
  actionText: {
    fontSize: 12,
    fontWeight: '500',
    textAlign: 'center',
  },
  placeholderCard: {
    alignItems: 'center',
    paddingVertical: 40,
  },
  placeholderText: {
    fontSize: 14,
    opacity: 0.5,
    textAlign: 'center',
    marginTop: 16,
    paddingHorizontal: 20,
  },
});
