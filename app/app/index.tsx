import React, { useState, useEffect, useRef } from 'react';
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
import { useUpdate } from '@/utils/UpdateContext';
import { downloadAndInstallUpdate, DownloadProgress } from '@/utils/updater';
import { VOICE_OPTIONS, LANGUAGE_OPTIONS } from '@/constants/voice';
import { TIMEZONE_DATA, TIMEZONE_REGIONS, POPULAR_TIMEZONES, getTimezoneLabel } from '@/constants/timezone';
import { Mic, Globe, Clock } from 'lucide-react-native';
import { homeStyles as styles } from '@/styles/index';
import {
  Connection,
  DeviceStatus,
  QuickActions,
} from '@/components/home';


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
  const { updateInfo } = useUpdate();

  const [showTimezoneModal, setShowTimezoneModal] = useState<boolean>(false);
  const [showVoiceModal, setShowVoiceModal] = useState<boolean>(false);
  const [showLanguageModal, setShowLanguageModal] = useState<boolean>(false);
  const [isSaving, setIsSaving] = useState<boolean>(false);
  const [notificationAnim] = useState(new Animated.Value(-100));
  const [updateProgress, setUpdateProgress] = useState<DownloadProgress | null>(null);
  const [updateStatus, setUpdateStatus] = useState<string | null>(null);
  const [isUpdating, setIsUpdating] = useState<boolean>(false);
  const lastConnectedDeviceRef = useRef<string | null>(null);

  const wifiSSID = settings.wifiSSID;
  const wifiPassword = settings.wifiPassword;
  const timezone = settings.timezone;
  const weatherAPIKey = settings.weatherAPIKey;
  const weatherLocation = settings.weatherLocation;
  const brightness = settings.brightness;
  const selectedTheme = settings.selectedTheme;
  const voiceId = settings.voiceId;
  const language = settings.language;

  const setWifiSSID = (value: string) => updateSettings({ wifiSSID: value });
  const setWifiPassword = (value: string) => updateSettings({ wifiPassword: value });
  const setTimezone = (value: string) => updateSettings({ timezone: value });
  const setWeatherAPIKey = (value: string) => updateSettings({ weatherAPIKey: value });
  const setWeatherLocation = (value: string) => updateSettings({ weatherLocation: value });
  const setBrightness = (value: number) => updateSettings({ brightness: value });
  const setSelectedTheme = (value: string) => updateSettings({ selectedTheme: value });
  const setVoiceId = (value: string) => updateSettings({ voiceId: value });
  const setLanguage = (value: string) => updateSettings({ language: value });

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
      // Only show notification once per device connection
      if (lastConnectedDeviceRef.current !== connectedDevice.id) {
        lastConnectedDeviceRef.current = connectedDevice.id;
        saveDevice(connectedDevice.id, connectedDevice.name);
        showNotification('success', 'Connected', `Connected to ${connectedDevice.name}`);
      }
    } else if (connectionState === 'disconnected') {
      lastConnectedDeviceRef.current = null;
    }
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [connectionState, connectedDevice?.id]);

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

  const handleUpdate = async () => {
    if (!updateInfo || isUpdating) return;
    
    setIsUpdating(true);
    setUpdateProgress(null);
    setUpdateStatus('Preparing...');
    
    await downloadAndInstallUpdate(
      updateInfo,
      (progress) => setUpdateProgress(progress),
      (status) => setUpdateStatus(status),
      () => {
        showNotification('warning', 'Permission Required', 'Please enable install from unknown sources');
      }
    );
    
    setIsUpdating(false);
    setUpdateProgress(null);
    setUpdateStatus(null);
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
          {updateInfo && (
            <TouchableOpacity
              style={styles.updateBanner}
              onPress={handleUpdate}
              disabled={isUpdating}
            >
              <View style={styles.updateBannerContent}>
                <Download size={20} color="#00ff88" />
                <View style={styles.updateBannerText}>
                  <Text style={styles.updateBannerTitle}>
                    {isUpdating ? (updateStatus || 'Updating...') : `Update Available: ${updateInfo.tag_name}`}
                  </Text>
                  {isUpdating && updateProgress ? (
                    <Text style={styles.updateBannerSubtitle}>
                      {Math.round(updateProgress.progress * 100)}% downloaded
                    </Text>
                  ) : (
                    <Text style={styles.updateBannerSubtitle}>Tap to download and install</Text>
                  )}
                </View>
                {isUpdating && <ActivityIndicator size="small" color="#00ff88" />}
              </View>
              {isUpdating && updateProgress && (
                <View style={styles.updateProgressBar}>
                  <View style={[styles.updateProgressFill, { width: `${updateProgress.progress * 100}%` }]} />
                </View>
              )}
            </TouchableOpacity>
          )}
          <Connection
            connectionState={connectionState}
            connectedDevice={connectedDevice}
            devices={devices}
            error={error}
            retryCount={retryCount}
            maxRetries={maxRetries}
            connectionHealth={connectionHealth}
            isConnected={isConnected}
            onConnect={handleConnect}
            onDisconnect={handleDisconnect}
            onStartScan={startScan}
          />

          {isConnected && deviceStatus && (
            <>
              <DeviceStatus deviceStatus={deviceStatus} />
              <QuickActions onQuickAction={handleQuickAction} />
            </>
          )}

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
                  <View style={styles.inputGroup}>
                    <View style={styles.inputLabelRow}>
                      <Clock size={16} color="#00bfff" />
                      <Text style={styles.inputLabel}>Timezone</Text>
                    </View>
                    <TouchableOpacity
                      style={[styles.pickerButton, !isConnected && styles.disabledInput]}
                      onPress={() => isConnected && setShowTimezoneModal(!showTimezoneModal)}
                      disabled={!isConnected}
                    >
                      <Text style={styles.pickerButtonText}>{getTimezoneLabel(timezone)}</Text>
                    </TouchableOpacity>
                    {showTimezoneModal && (
                      <ScrollView style={styles.pickerOptionsScroll} nestedScrollEnabled>
                        <Text style={styles.pickerSectionHeader}>Popular Timezones</Text>
                        {POPULAR_TIMEZONES.map((tz) => (
                          <TouchableOpacity
                            key={tz.value}
                            style={styles.pickerOption}
                            onPress={() => {
                              setTimezone(tz.value);
                              setShowTimezoneModal(false);
                            }}
                          >
                            <View>
                              <Text style={styles.pickerOptionText}>{tz.label}</Text>
                              <Text style={styles.pickerOptionSubtext}>{tz.value} • {tz.offsetStr}</Text>
                            </View>
                            {tz.value === timezone && (
                              <View style={styles.pickerOptionSelected} />
                            )}
                          </TouchableOpacity>
                        ))}
                        {TIMEZONE_REGIONS.filter(r => !['Other', 'Etc'].includes(r)).map((region) => (
                          <View key={region}>
                            <Text style={styles.pickerSectionHeader}>{region}</Text>
                            {TIMEZONE_DATA.regions[region].map((tz) => (
                              <TouchableOpacity
                                key={tz.value}
                                style={styles.pickerOption}
                                onPress={() => {
                                  setTimezone(tz.value);
                                  setShowTimezoneModal(false);
                                }}
                              >
                                <View>
                                  <Text style={styles.pickerOptionText}>{tz.label}</Text>
                                  <Text style={styles.pickerOptionSubtext}>{tz.value} • {tz.offsetStr}</Text>
                                </View>
                                {tz.value === timezone && (
                                  <View style={styles.pickerOptionSelected} />
                                )}
                              </TouchableOpacity>
                            ))}
                          </View>
                        ))}
                      </ScrollView>
                    )}
                  </View>
                </View>
              </View>

              <View style={styles.section}>
                <Text style={styles.sectionTitle}>VOICE SETTINGS</Text>
                <View style={styles.formCard}>
                  <View style={styles.inputGroup}>
                    <View style={styles.inputLabelRow}>
                      <Mic size={16} color="#00bfff" />
                      <Text style={styles.inputLabel}>Voice</Text>
                    </View>
                    <TouchableOpacity
                      style={[styles.pickerButton, !isConnected && styles.disabledInput]}
                      onPress={() => isConnected && setShowVoiceModal(!showVoiceModal)}
                      disabled={!isConnected}
                    >
                      <Text style={styles.pickerButtonText}>
                        {VOICE_OPTIONS.find(v => v.id === voiceId)?.name ?? 'Shimmer'} - {VOICE_OPTIONS.find(v => v.id === voiceId)?.description}
                      </Text>
                    </TouchableOpacity>
                    {showVoiceModal && (
                      <View style={styles.pickerOptions}>
                        {VOICE_OPTIONS.map((voice) => (
                          <TouchableOpacity
                            key={voice.id}
                            style={styles.pickerOption}
                            onPress={() => {
                              setVoiceId(voice.id);
                              setShowVoiceModal(false);
                            }}
                          >
                            <View>
                              <Text style={styles.pickerOptionText}>{voice.name}</Text>
                              <Text style={styles.pickerOptionSubtext}>{voice.gender} • {voice.description}</Text>
                            </View>
                            {voice.id === voiceId && (
                              <View style={styles.pickerOptionSelected} />
                            )}
                          </TouchableOpacity>
                        ))}
                      </View>
                    )}
                  </View>
                  <View style={styles.inputGroup}>
                    <View style={styles.inputLabelRow}>
                      <Globe size={16} color="#00bfff" />
                      <Text style={styles.inputLabel}>Language</Text>
                    </View>
                    <TouchableOpacity
                      style={[styles.pickerButton, !isConnected && styles.disabledInput]}
                      onPress={() => isConnected && setShowLanguageModal(!showLanguageModal)}
                      disabled={!isConnected}
                    >
                      <Text style={styles.pickerButtonText}>
                        {LANGUAGE_OPTIONS.find(l => l.code === language)?.name ?? 'English'}
                        {LANGUAGE_OPTIONS.find(l => l.code === language)?.region ? ` (${LANGUAGE_OPTIONS.find(l => l.code === language)?.region})` : ''}
                      </Text>
                    </TouchableOpacity>
                    {showLanguageModal && (
                      <ScrollView style={styles.pickerOptionsScroll} nestedScrollEnabled>
                        {LANGUAGE_OPTIONS.map((lang) => (
                          <TouchableOpacity
                            key={lang.code}
                            style={styles.pickerOption}
                            onPress={() => {
                              setLanguage(lang.code);
                              setShowLanguageModal(false);
                            }}
                          >
                            <Text style={styles.pickerOptionText}>
                              {lang.name}{lang.region ? ` (${lang.region})` : ''}
                            </Text>
                            {lang.code === language && (
                              <View style={styles.pickerOptionSelected} />
                            )}
                          </TouchableOpacity>
                        ))}
                      </ScrollView>
                    )}
                  </View>
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


          <View style={styles.bottomSpacer} />
        </ScrollView>
      </SafeAreaView>
    </View>
  );
}
