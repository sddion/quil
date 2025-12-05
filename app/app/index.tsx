import React, { useState, useEffect } from 'react';
import {
  View,
  Text,
  StyleSheet,
  ScrollView,
  TextInput,
  TouchableOpacity,
  Alert,
  ActivityIndicator,
  Image,
  Animated,
} from 'react-native';
import { SafeAreaView } from 'react-native-safe-area-context';
import { StatusBar } from 'expo-status-bar';
import { LinearGradient } from 'expo-linear-gradient';
import { router } from 'expo-router';
import AsyncStorage from '@react-native-async-storage/async-storage';
import {
  Bluetooth,
  BluetoothConnected,
  BluetoothSearching,
  Battery,
  Wifi,
  PackageCheck,
  Clock,
  CloudSun,
  RotateCcw,
  Download,
  Sun,
  MapPin,
  Key,
  AlertTriangle,
  Menu,
  HelpCircle,
} from 'lucide-react-native';
import { useBLE } from '@/hooks/use-ble';
import type { BLEDevice } from '@/lib/ble-manager';
import { useSettings } from '@/contexts/settings';
import { useDevices } from '@/contexts/devices';
import { useNotifications } from '@/contexts/notifications';

const TIMEZONES = [
  'America/New_York',
  'America/Chicago',
  'America/Denver',
  'America/Los_Angeles',
  'Europe/London',
  'Europe/Paris',
  'Asia/Tokyo',
  'Asia/Shanghai',
  'Australia/Sydney',
];

const ONBOARDING_KEY = '@quil_onboarding_completed';

export default function HomeScreen() {
  const {
    connectionState,
    devices,
    connectedDevice,
    deviceStatus,
    error,
    retryCount,
    maxRetries,
    connectionHealth,
    startScan,
    connect,
    disconnect,
    sendConfiguration,
    sendCommand,
  } = useBLE();

  const { settings, updateSettings, markSynced } = useSettings();
  const { saveDevice } = useDevices();
  const { showNotification, currentNotification } = useNotifications();

  const [showTimezoneModal, setShowTimezoneModal] = useState<boolean>(false);
  const [isSaving, setIsSaving] = useState<boolean>(false);
  const [notificationAnim] = useState(new Animated.Value(-100));

  const wifiSSID = settings.wifiSSID;
  const wifiPassword = settings.wifiPassword;
  const timezone = settings.timezone;
  const weatherAPIKey = settings.weatherAPIKey;
  const weatherLocation = settings.weatherLocation;
  const brightness = settings.brightness;
  const selectedTheme = settings.selectedTheme;

  const setWifiSSID = (value: string) => updateSettings({ wifiSSID: value });
  const setWifiPassword = (value: string) => updateSettings({ wifiPassword: value });
  const setTimezone = (value: string) => updateSettings({ timezone: value });
  const setWeatherAPIKey = (value: string) => updateSettings({ weatherAPIKey: value });
  const setWeatherLocation = (value: string) => updateSettings({ weatherLocation: value });
  const setBrightness = (value: number) => updateSettings({ brightness: value });
  const setSelectedTheme = (value: string) => updateSettings({ selectedTheme: value });

  useEffect(() => {
    checkOnboarding();
  }, []);

  useEffect(() => {
    if (settings) {
      console.log('[Home] Settings loaded');
    }
  }, [settings]);

  useEffect(() => {
    if (connectionState === 'connected' && connectedDevice) {
      saveDevice(connectedDevice.id, connectedDevice.name);
      showNotification('success', 'Connected', `Connected to ${connectedDevice.name}`);
    }
  }, [connectionState, connectedDevice, saveDevice, showNotification]);

  useEffect(() => {
    if (currentNotification) {
      Animated.sequence([
        Animated.timing(notificationAnim, {
          toValue: 0,
          duration: 300,
          useNativeDriver: true,
        }),
        Animated.delay(currentNotification.duration - 600),
        Animated.timing(notificationAnim, {
          toValue: -100,
          duration: 300,
          useNativeDriver: true,
        }),
      ]).start();
    }
  }, [currentNotification, notificationAnim]);

  const checkOnboarding = async () => {
    try {
      const completed = await AsyncStorage.getItem(ONBOARDING_KEY);
      if (!completed) {
        router.push('/onboarding');
      }
    } catch (err) {
      console.error('[Home] Failed to check onboarding:', err);
    }
  };



  const handleConnect = async (device: BLEDevice) => {
    showNotification('info', 'Connecting', `Connecting to ${device.name}...`);
    await connect(device);
  };

  const handleDisconnect = async () => {
    showNotification('info', 'Disconnecting', 'Disconnecting from device...');
    await disconnect();
  };

  const handleQuickAction = async (action: string) => {
    if (connectionState !== 'connected') return;

    const success = await sendCommand(action);
    if (success) {
      showNotification('success', 'Command Sent', `${action} executed successfully`);
    } else {
      showNotification('error', 'Command Failed', `Failed to execute ${action}`);
    }
  };

  const handleFactoryReset = () => {
    if (connectionState !== 'connected') return;

    Alert.alert(
      'Factory Reset',
      'Are you sure you want to reset Quil to factory defaults? This action cannot be undone.',
      [
        { text: 'Cancel', style: 'cancel' },
        {
          text: 'Reset',
          style: 'destructive',
          onPress: async () => {
            const success = await sendCommand('factory_reset');
            if (success) {
              showNotification('success', 'Factory Reset', 'Device reset initiated');
            } else {
              showNotification('error', 'Reset Failed', 'Failed to send reset command');
            }
          },
        },
      ]
    );
  };

  const handleSaveConfiguration = async () => {
    if (connectionState !== 'connected') return;

    setIsSaving(true);

    const config = {
      wifi: {
        ssid: settings.wifiSSID,
        password: settings.wifiPassword,
      },
      timezone: settings.timezone,
      weather: {
        apiKey: settings.weatherAPIKey,
        location: settings.weatherLocation,
      },
      display: {
        brightness: settings.brightness,
      },
      theme: settings.selectedTheme,
    };

    const success = await sendConfiguration(config);

    setIsSaving(false);

    if (success) {
      await markSynced();
      showNotification('success', 'Configuration Saved', 'Settings synced to device');
    } else {
      showNotification('error', 'Save Failed', 'Failed to save configuration');
    }
  };

  const isConnected = connectionState === 'connected';

  return (
    <View style={styles.container}>
      <LinearGradient
        colors={['#0a0e27', '#1a1f3a']}
        style={StyleSheet.absoluteFillObject}
      />
      <StatusBar style="light" />

      <SafeAreaView style={styles.safeArea} edges={['top']}>
        {currentNotification && (
          <Animated.View
            style={[
              styles.notificationBar,
              {
                backgroundColor:
                  currentNotification.type === 'success'
                    ? 'rgba(0,255,136,0.9)'
                    : currentNotification.type === 'error'
                      ? 'rgba(255,68,68,0.9)'
                      : currentNotification.type === 'warning'
                        ? 'rgba(255,215,0,0.9)'
                        : 'rgba(0,191,255,0.9)',
                transform: [{ translateY: notificationAnim }],
              },
            ]}
          >
            <Text style={styles.notificationTitle}>{currentNotification.title}</Text>
            <Text style={styles.notificationMessage}>{currentNotification.message}</Text>
          </Animated.View>
        )}

        <View style={styles.header}>
          <View>
            <Text style={styles.logo}>QUIL</Text>
            <Text style={styles.subtitle}>ROBOT CONTROL</Text>
          </View>
          <View style={styles.headerActions}>
            <TouchableOpacity
              style={styles.headerButton}
              onPress={() => router.push('/devices')}
            >
              <Menu size={24} color="#00bfff" />
            </TouchableOpacity>
            <TouchableOpacity
              style={styles.headerButton}
              onPress={() => router.push('/help')}
            >
              <HelpCircle size={24} color="#00bfff" />
            </TouchableOpacity>
          </View>
        </View>

        <ScrollView
          style={styles.scrollView}
          contentContainerStyle={styles.scrollContent}
          showsVerticalScrollIndicator={false}
        >
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
                    {connectionState === 'connecting' && 'Connecting...'}
                    {connectionState === 'scanning' && 'Scanning...'}
                    {connectionState === 'disconnected' && 'Disconnected'}
                  </Text>
                  {connectedDevice && (
                    <Text style={styles.deviceName}>{connectedDevice.name}</Text>
                  )}
                </View>
                {isConnected && (
                  <TouchableOpacity
                    style={styles.disconnectButton}
                    onPress={handleDisconnect}
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
                  onPress={startScan}
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
                      onPress={() => handleConnect(device)}
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

          {isConnected && deviceStatus && (
            <>
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

              <View style={styles.section}>
                <Text style={styles.sectionTitle}>QUICK ACTIONS</Text>
                <View style={styles.actionsGrid}>
                  <TouchableOpacity
                    style={styles.actionButton}
                    onPress={() => handleQuickAction('Sync Time')}
                  >
                    <Clock size={24} color="#00bfff" />
                    <Text style={styles.actionButtonText}>Sync Time</Text>
                  </TouchableOpacity>
                  <TouchableOpacity
                    style={styles.actionButton}
                    onPress={() => handleQuickAction('Update Weather')}
                  >
                    <CloudSun size={24} color="#00bfff" />
                    <Text style={styles.actionButtonText}>Update Weather</Text>
                  </TouchableOpacity>
                  <TouchableOpacity
                    style={styles.actionButton}
                    onPress={() => handleQuickAction('Restart')}
                  >
                    <RotateCcw size={24} color="#00bfff" />
                    <Text style={styles.actionButtonText}>Restart</Text>
                  </TouchableOpacity>
                  <TouchableOpacity
                    style={styles.actionButton}
                    onPress={() => handleQuickAction('Check Updates')}
                  >
                    <Download size={24} color="#00bfff" />
                    <Text style={styles.actionButtonText}>Check Updates</Text>
                  </TouchableOpacity>
                </View>
              </View>

              <View style={styles.section}>
                <Text style={styles.sectionTitle}>WIFI CONFIGURATION</Text>
                <View style={styles.formCard}>
                  <View style={styles.inputGroup}>
                    <Text style={styles.inputLabel}>Network SSID</Text>
                    <TextInput
                      style={styles.input}
                      value={wifiSSID}
                      onChangeText={setWifiSSID}
                      placeholder="Enter WiFi network name"
                      placeholderTextColor="#666"
                      editable={isConnected}
                    />
                  </View>
                  <View style={styles.inputGroup}>
                    <Text style={styles.inputLabel}>Password</Text>
                    <TextInput
                      style={styles.input}
                      value={wifiPassword}
                      onChangeText={setWifiPassword}
                      placeholder="Enter WiFi password"
                      placeholderTextColor="#666"
                      secureTextEntry
                      editable={isConnected}
                    />
                  </View>
                </View>
              </View>

              <View style={styles.section}>
                <Text style={styles.sectionTitle}>TIMEZONE</Text>
                <View style={styles.formCard}>
                  <TouchableOpacity
                    style={[styles.pickerButton, !isConnected && styles.disabledInput]}
                    onPress={() => isConnected && setShowTimezoneModal(!showTimezoneModal)}
                    disabled={!isConnected}
                  >
                    <Text style={styles.pickerButtonText}>{timezone}</Text>
                  </TouchableOpacity>
                  {showTimezoneModal && (
                    <View style={styles.pickerOptions}>
                      {TIMEZONES.map((tz) => (
                        <TouchableOpacity
                          key={tz}
                          style={styles.pickerOption}
                          onPress={() => {
                            setTimezone(tz);
                            setShowTimezoneModal(false);
                          }}
                        >
                          <Text style={styles.pickerOptionText}>{tz}</Text>
                          {tz === timezone && (
                            <View style={styles.pickerOptionSelected} />
                          )}
                        </TouchableOpacity>
                      ))}
                    </View>
                  )}
                </View>
              </View>

              <View style={styles.section}>
                <Text style={styles.sectionTitle}>WEATHER SERVICE</Text>
                <View style={styles.formCard}>
                  <View style={styles.inputGroup}>
                    <View style={styles.inputLabelRow}>
                      <Key size={16} color="#00bfff" />
                      <Text style={styles.inputLabel}>API Key</Text>
                    </View>
                    <TextInput
                      style={styles.input}
                      value={weatherAPIKey}
                      onChangeText={setWeatherAPIKey}
                      placeholder="Enter weather API key"
                      placeholderTextColor="#666"
                      editable={isConnected}
                    />
                  </View>
                  <View style={styles.inputGroup}>
                    <View style={styles.inputLabelRow}>
                      <MapPin size={16} color="#00bfff" />
                      <Text style={styles.inputLabel}>Location</Text>
                    </View>
                    <TextInput
                      style={styles.input}
                      value={weatherLocation}
                      onChangeText={setWeatherLocation}
                      placeholder="e.g., New York, NY"
                      placeholderTextColor="#666"
                      editable={isConnected}
                    />
                  </View>
                </View>
              </View>

              <View style={styles.section}>
                <Text style={styles.sectionTitle}>DISPLAY CONTROL</Text>
                <View style={styles.formCard}>
                  <View style={styles.sliderContainer}>
                    <View style={styles.sliderHeader}>
                      <Sun size={20} color="#00bfff" />
                      <Text style={styles.inputLabel}>Brightness</Text>
                      <Text style={styles.sliderValue}>{brightness}</Text>
                    </View>
                    <View style={styles.sliderTrack}>
                      <View
                        style={[
                          styles.sliderFill,
                          { width: `${(brightness / 255) * 100}%` },
                        ]}
                      />
                    </View>
                    <View style={styles.sliderButtons}>
                      <TouchableOpacity
                        style={styles.sliderButton}
                        onPress={() => setBrightness(Math.max(0, brightness - 25))}
                        disabled={!isConnected}
                      >
                        <Text style={styles.sliderButtonText}>-</Text>
                      </TouchableOpacity>
                      <TouchableOpacity
                        style={styles.sliderButton}
                        onPress={() => setBrightness(Math.min(255, brightness + 25))}
                        disabled={!isConnected}
                      >
                        <Text style={styles.sliderButtonText}>+</Text>
                      </TouchableOpacity>
                    </View>
                  </View>
                </View>
              </View>

              <View style={styles.section}>
                <Text style={styles.sectionTitle}>THEME SELECTION</Text>
                <ScrollView
                  horizontal
                  showsHorizontalScrollIndicator={false}
                  contentContainerStyle={styles.themeScrollContent}
                >
                  <TouchableOpacity
                    style={[
                      styles.themeCard,
                      selectedTheme === 'default' && styles.themeCardSelected,
                    ]}
                    onPress={() => setSelectedTheme('default')}
                    disabled={!isConnected}
                  >
                    <View style={styles.themePreview}>
                      <Image
                        source={{ uri: 'https://pub-e001eb4506b145aa938b5d3badbff6a5.r2.dev/attachments/y0ywgb7h01g8or3wtzf7f' }}
                        style={styles.themePreviewImage}
                        resizeMode="cover"
                      />
                    </View>
                    <Text style={styles.themeCardTitle}>Default</Text>
                    <Text style={styles.themeCardDesc}>Full status display</Text>
                  </TouchableOpacity>
                  <TouchableOpacity
                    style={[
                      styles.themeCard,
                      selectedTheme === 'compact' && styles.themeCardSelected,
                    ]}
                    onPress={() => setSelectedTheme('compact')}
                    disabled={!isConnected}
                  >
                    <View style={styles.themePreview}>
                      <Image
                        source={{ uri: 'https://pub-e001eb4506b145aa938b5d3badbff6a5.r2.dev/attachments/s0p8ue3infjjvl1pzktil' }}
                        style={styles.themePreviewImage}
                        resizeMode="cover"
                      />
                    </View>
                    <Text style={styles.themeCardTitle}>Compact</Text>
                    <Text style={styles.themeCardDesc}>Minimalist view</Text>
                  </TouchableOpacity>
                  <TouchableOpacity
                    style={[
                      styles.themeCard,
                      selectedTheme === 'modern' && styles.themeCardSelected,
                    ]}
                    onPress={() => setSelectedTheme('modern')}
                    disabled={!isConnected}
                  >
                    <View style={styles.themePreview}>
                      <View style={styles.themePreviewModern}>
                        <View style={styles.modernBlock1} />
                        <View style={styles.modernBlock2} />
                      </View>
                    </View>
                    <Text style={styles.themeCardTitle}>Modern</Text>
                    <Text style={styles.themeCardDesc}>Card-based layout</Text>
                  </TouchableOpacity>
                  <TouchableOpacity
                    style={[
                      styles.themeCard,
                      selectedTheme === 'minimal' && styles.themeCardSelected,
                    ]}
                    onPress={() => setSelectedTheme('minimal')}
                    disabled={!isConnected}
                  >
                    <View style={styles.themePreview}>
                      <View style={styles.themePreviewMinimal}>
                        <View style={styles.minimalLine} />
                        <View style={[styles.minimalLine, { width: '60%' }]} />
                      </View>
                    </View>
                    <Text style={styles.themeCardTitle}>Minimal</Text>
                    <Text style={styles.themeCardDesc}>Clean & simple</Text>
                  </TouchableOpacity>
                  <TouchableOpacity
                    style={[
                      styles.themeCard,
                      selectedTheme === 'retro' && styles.themeCardSelected,
                    ]}
                    onPress={() => setSelectedTheme('retro')}
                    disabled={!isConnected}
                  >
                    <View style={styles.themePreview}>
                      <View style={styles.themePreviewRetro}>
                        <View style={styles.retroGrid} />
                      </View>
                    </View>
                    <Text style={styles.themeCardTitle}>Retro</Text>
                    <Text style={styles.themeCardDesc}>Pixelated style</Text>
                  </TouchableOpacity>
                </ScrollView>
              </View>

              <View style={styles.section}>
                <Text style={styles.sectionTitle}>DANGER ZONE</Text>
                <View style={styles.dangerCard}>
                  <TouchableOpacity
                    style={[styles.dangerButton, !isConnected && styles.disabledButton]}
                    onPress={handleFactoryReset}
                    disabled={!isConnected}
                  >
                    <AlertTriangle size={20} color="#ff4444" />
                    <Text style={styles.dangerButtonText}>Factory Reset</Text>
                  </TouchableOpacity>
                  <Text style={styles.dangerWarning}>
                    This will erase all settings and configurations
                  </Text>
                </View>
              </View>

              <TouchableOpacity
                style={[styles.saveButton, !isConnected && styles.disabledButton]}
                onPress={handleSaveConfiguration}
                disabled={!isConnected || isSaving}
              >
                {isSaving ? (
                  <ActivityIndicator size="small" color="#0a0e27" />
                ) : (
                  <>
                    <Download size={20} color="#0a0e27" />
                    <Text style={styles.saveButtonText}>Save Configuration</Text>
                  </>
                )}
              </TouchableOpacity>
            </>
          )}

          <View style={styles.bottomSpacer} />
        </ScrollView>
      </SafeAreaView>
    </View>
  );
}

const styles = StyleSheet.create({
  container: {
    flex: 1,
    backgroundColor: '#0a0e27',
  },
  safeArea: {
    flex: 1,
  },
  header: {
    flexDirection: 'row',
    justifyContent: 'space-between',
    alignItems: 'center',
    paddingHorizontal: 24,
    paddingTop: 20,
    paddingBottom: 16,
    borderBottomWidth: 1,
    borderBottomColor: 'rgba(255,255,255,0.1)',
  },
  logo: {
    fontSize: 32,
    fontWeight: '700' as const,
    color: '#00ff88',
    letterSpacing: 4,
  },
  subtitle: {
    fontSize: 11,
    fontWeight: '600' as const,
    color: '#00bfff',
    letterSpacing: 2,
    marginTop: 2,
  },
  scrollView: {
    flex: 1,
  },
  scrollContent: {
    padding: 24,
  },
  section: {
    marginBottom: 24,
  },
  sectionTitle: {
    fontSize: 11,
    fontWeight: '700' as const,
    color: '#00bfff',
    letterSpacing: 1.5,
    marginBottom: 12,
  },
  connectionCard: {
    backgroundColor: 'rgba(255,255,255,0.05)',
    borderRadius: 12,
    padding: 16,
    borderWidth: 1,
    borderColor: 'rgba(255,255,255,0.1)',
  },
  connectionHeader: {
    flexDirection: 'row',
    alignItems: 'center',
    gap: 12,
  },
  connectionInfo: {
    flex: 1,
  },
  connectionStatus: {
    fontSize: 16,
    fontWeight: '600' as const,
    color: '#fff',
  },
  deviceName: {
    fontSize: 13,
    color: '#999',
    marginTop: 2,
    fontFamily: 'monospace',
  },
  disconnectButton: {
    backgroundColor: 'rgba(255,68,68,0.2)',
    paddingHorizontal: 12,
    paddingVertical: 6,
    borderRadius: 6,
    borderWidth: 1,
    borderColor: '#ff4444',
  },
  disconnectButtonText: {
    color: '#ff4444',
    fontSize: 12,
    fontWeight: '600' as const,
  },
  errorBanner: {
    flexDirection: 'row',
    alignItems: 'center',
    gap: 8,
    marginTop: 12,
    padding: 12,
    backgroundColor: 'rgba(255,68,68,0.1)',
    borderRadius: 8,
    borderWidth: 1,
    borderColor: 'rgba(255,68,68,0.3)',
  },
  errorText: {
    flex: 1,
    color: '#ff4444',
    fontSize: 13,
  },
  deviceList: {
    marginTop: 16,
    paddingTop: 16,
    borderTopWidth: 1,
    borderTopColor: 'rgba(255,255,255,0.1)',
  },
  deviceListTitle: {
    fontSize: 13,
    fontWeight: '600' as const,
    color: '#999',
    marginBottom: 8,
  },
  deviceItem: {
    flexDirection: 'row',
    alignItems: 'center',
    justifyContent: 'space-between',
    padding: 12,
    backgroundColor: 'rgba(255,255,255,0.05)',
    borderRadius: 8,
    marginBottom: 8,
    borderWidth: 1,
    borderColor: 'rgba(0,191,255,0.3)',
  },
  deviceItemContent: {
    flex: 1,
  },
  deviceItemName: {
    fontSize: 15,
    fontWeight: '600' as const,
    color: '#fff',
  },
  deviceItemId: {
    fontSize: 12,
    color: '#666',
    marginTop: 2,
    fontFamily: 'monospace',
  },
  deviceItemRSSI: {
    fontSize: 12,
    color: '#00bfff',
    fontFamily: 'monospace',
  },
  statusCard: {
    backgroundColor: 'rgba(255,255,255,0.05)',
    borderRadius: 12,
    padding: 16,
    borderWidth: 1,
    borderColor: 'rgba(255,255,255,0.1)',
    gap: 12,
  },
  statusRow: {
    flexDirection: 'row',
    alignItems: 'center',
    gap: 12,
  },
  statusLabel: {
    flex: 1,
    fontSize: 14,
    color: '#999',
  },
  statusValue: {
    fontSize: 14,
    fontWeight: '600' as const,
    color: '#fff',
    fontFamily: 'monospace',
  },
  actionsGrid: {
    flexDirection: 'row',
    flexWrap: 'wrap',
    gap: 12,
  },
  actionButton: {
    flex: 1,
    minWidth: '45%',
    backgroundColor: 'rgba(0,191,255,0.1)',
    padding: 16,
    borderRadius: 12,
    alignItems: 'center',
    gap: 8,
    borderWidth: 1,
    borderColor: 'rgba(0,191,255,0.3)',
  },
  actionButtonText: {
    color: '#00bfff',
    fontSize: 13,
    fontWeight: '600' as const,
  },
  formCard: {
    backgroundColor: 'rgba(255,255,255,0.05)',
    borderRadius: 12,
    padding: 16,
    borderWidth: 1,
    borderColor: 'rgba(255,255,255,0.1)',
    gap: 16,
  },
  inputGroup: {
    gap: 8,
  },
  inputLabel: {
    fontSize: 13,
    fontWeight: '600' as const,
    color: '#999',
    marginLeft: 4,
  },
  inputLabelRow: {
    flexDirection: 'row',
    alignItems: 'center',
    gap: 8,
  },
  input: {
    backgroundColor: 'rgba(0,0,0,0.3)',
    borderWidth: 1,
    borderColor: 'rgba(255,255,255,0.2)',
    borderRadius: 8,
    padding: 12,
    color: '#fff',
    fontSize: 14,
    fontFamily: 'monospace',
  },
  disabledInput: {
    opacity: 0.5,
  },
  pickerButton: {
    backgroundColor: 'rgba(0,0,0,0.3)',
    borderWidth: 1,
    borderColor: 'rgba(255,255,255,0.2)',
    borderRadius: 8,
    padding: 12,
  },
  pickerButtonText: {
    color: '#fff',
    fontSize: 14,
    fontFamily: 'monospace',
  },
  pickerOptions: {
    backgroundColor: 'rgba(0,0,0,0.5)',
    borderRadius: 8,
    overflow: 'hidden',
    borderWidth: 1,
    borderColor: 'rgba(255,255,255,0.2)',
  },
  pickerOption: {
    padding: 12,
    flexDirection: 'row',
    justifyContent: 'space-between',
    alignItems: 'center',
    borderBottomWidth: 1,
    borderBottomColor: 'rgba(255,255,255,0.1)',
  },
  pickerOptionText: {
    color: '#fff',
    fontSize: 13,
    fontFamily: 'monospace',
  },
  pickerOptionSelected: {
    width: 8,
    height: 8,
    borderRadius: 4,
    backgroundColor: '#00ff88',
  },
  sliderContainer: {
    gap: 12,
  },
  sliderHeader: {
    flexDirection: 'row',
    alignItems: 'center',
    gap: 8,
  },
  sliderValue: {
    marginLeft: 'auto',
    fontSize: 16,
    fontWeight: '700' as const,
    color: '#00bfff',
    fontFamily: 'monospace',
  },
  sliderTrack: {
    height: 8,
    backgroundColor: 'rgba(0,0,0,0.3)',
    borderRadius: 4,
    overflow: 'hidden',
  },
  sliderFill: {
    height: '100%',
    backgroundColor: '#00bfff',
  },
  sliderButtons: {
    flexDirection: 'row',
    gap: 12,
  },
  sliderButton: {
    flex: 1,
    backgroundColor: 'rgba(0,191,255,0.2)',
    padding: 12,
    borderRadius: 8,
    alignItems: 'center',
    borderWidth: 1,
    borderColor: 'rgba(0,191,255,0.3)',
  },
  sliderButtonText: {
    color: '#00bfff',
    fontSize: 20,
    fontWeight: '700' as const,
  },
  themeScrollContent: {
    gap: 12,
    paddingRight: 24,
  },
  themeCard: {
    width: 140,
    backgroundColor: 'rgba(255,255,255,0.05)',
    borderRadius: 12,
    padding: 16,
    borderWidth: 2,
    borderColor: 'rgba(255,255,255,0.1)',
    alignItems: 'center',
    gap: 12,
  },
  themeCardSelected: {
    borderColor: '#00ff88',
    backgroundColor: 'rgba(0,255,136,0.1)',
  },
  themePreview: {
    width: '100%',
    height: 80,
    backgroundColor: 'rgba(0,0,0,0.3)',
    borderRadius: 8,
    overflow: 'hidden',
    justifyContent: 'center',
    alignItems: 'center',
  },
  themePreviewImage: {
    width: '100%',
    height: '100%',
  },
  themePreviewModern: {
    width: '100%',
    height: '100%',
    padding: 8,
    gap: 6,
  },
  modernBlock1: {
    flex: 1,
    backgroundColor: 'rgba(0,191,255,0.4)',
    borderRadius: 4,
  },
  modernBlock2: {
    flex: 1,
    backgroundColor: 'rgba(0,255,136,0.4)',
    borderRadius: 4,
  },
  themePreviewMinimal: {
    width: '100%',
    height: '100%',
    padding: 12,
    justifyContent: 'center',
    gap: 8,
  },
  minimalLine: {
    height: 3,
    backgroundColor: 'rgba(255,255,255,0.6)',
    borderRadius: 2,
  },
  themePreviewRetro: {
    width: '100%',
    height: '100%',
    padding: 4,
  },
  retroGrid: {
    width: '100%',
    height: '100%',
    backgroundColor: 'rgba(0,255,136,0.2)',
    borderWidth: 2,
    borderColor: 'rgba(0,255,136,0.6)',
    borderRadius: 2,
  },
  themeCardTitle: {
    fontSize: 14,
    fontWeight: '700' as const,
    color: '#fff',
  },
  themeCardDesc: {
    fontSize: 11,
    color: '#999',
  },
  dangerCard: {
    backgroundColor: 'rgba(255,68,68,0.05)',
    borderRadius: 12,
    padding: 16,
    borderWidth: 1,
    borderColor: 'rgba(255,68,68,0.3)',
    gap: 12,
  },
  dangerButton: {
    flexDirection: 'row',
    alignItems: 'center',
    justifyContent: 'center',
    gap: 8,
    backgroundColor: 'rgba(255,68,68,0.2)',
    padding: 14,
    borderRadius: 8,
    borderWidth: 1,
    borderColor: '#ff4444',
  },
  dangerButtonText: {
    color: '#ff4444',
    fontSize: 14,
    fontWeight: '700' as const,
  },
  dangerWarning: {
    fontSize: 12,
    color: '#ff6b6b',
    textAlign: 'center',
  },
  saveButton: {
    flexDirection: 'row',
    alignItems: 'center',
    justifyContent: 'center',
    gap: 8,
    backgroundColor: '#00ff88',
    padding: 16,
    borderRadius: 12,
    marginTop: 8,
  },
  saveButtonText: {
    color: '#0a0e27',
    fontSize: 16,
    fontWeight: '700' as const,
    letterSpacing: 1,
  },
  disabledButton: {
    opacity: 0.5,
  },
  scanButton: {
    marginTop: 16,
    flexDirection: 'row',
    alignItems: 'center',
    justifyContent: 'center',
    gap: 8,
    backgroundColor: 'rgba(0,191,255,0.1)',
    padding: 14,
    borderRadius: 8,
    borderWidth: 1,
    borderColor: 'rgba(0,191,255,0.3)',
  },
  scanButtonText: {
    color: '#00bfff',
    fontSize: 14,
    fontWeight: '600' as const,
  },
  bottomSpacer: {
    height: 40,
  },
  notificationBar: {
    position: 'absolute' as const,
    top: 0,
    left: 0,
    right: 0,
    padding: 16,
    zIndex: 1000,
  },
  notificationTitle: {
    fontSize: 14,
    fontWeight: '700' as const,
    color: '#0a0e27',
    marginBottom: 4,
  },
  notificationMessage: {
    fontSize: 13,
    color: '#0a0e27',
  },
  headerActions: {
    flexDirection: 'row',
    gap: 12,
  },
  headerButton: {
    padding: 8,
  },
});
