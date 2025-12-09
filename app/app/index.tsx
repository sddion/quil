import React, { useState, useEffect, useRef, useCallback } from 'react';
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
import { useKeepAwake } from 'expo-keep-awake';
import AsyncStorage from '@react-native-async-storage/async-storage';
import {
  Download,
  Sun,
  MapPin,
  Key,
  AlertTriangle,
} from 'lucide-react-native';
import { useSettings } from '@/contexts/settings';
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
import { useDevice } from '@/hooks/use-device';


const ONBOARDING_KEY = '@quil_onboarding_completed';

export default function HomeScreen() {
  useKeepAwake();
  const {
    connectionState,
    deviceStatus,
    deviceIp,
    error,
    isConnected,
    connect,
    disconnect,
    sendConfig: sendConfiguration,
    sendCommand,
    discover,
  } = useDevice();

  const { settings, updateSettings } = useSettings();
  const { showNotification, currentNotification } = useNotifications();
  const { updateInfo } = useUpdate();

  const [showTimezoneModal, setShowTimezoneModal] = useState<boolean>(false);
  const [showVoiceModal, setShowVoiceModal] = useState<boolean>(false);
  const [showLanguageModal, setShowLanguageModal] = useState<boolean>(false);
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
    if (connectionState === 'connected' && deviceIp) {
      // Only show notification once per device connection
      if (lastConnectedDeviceRef.current !== deviceIp) {
        lastConnectedDeviceRef.current = deviceIp;
        showNotification('success', 'Connected', `Connected to Quil (${deviceIp})`);
      }
    } else if (connectionState === 'disconnected') {
      lastConnectedDeviceRef.current = null;
    }
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [connectionState, deviceIp]);

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

  const syncSettingsToDevice = useCallback(async () => {
    if (!isConnected) return;

    const themeMap: Record<string, number> = { default: 0, compact: 1, modern: 2, minimal: 3, retro: 4 };

    const config = {
      ssid: settings.wifiSSID,
      password: settings.wifiPassword,
      tz: parseInt(settings.timezone) || 0,
      brightness: settings.brightness,
      theme: themeMap[settings.selectedTheme] ?? 0,
      wk: settings.weatherAPIKey,
      wl: settings.weatherLocation,
    };

    await sendConfiguration(config);
  }, [isConnected, settings, sendConfiguration]);

  // Debounced auto-sync when settings change
  useEffect(() => {
    if (!isConnected) return;
    const debounceTimer = setTimeout(() => {
      syncSettingsToDevice();
    }, 1000); // Wait 1 second after last change before syncing
    return () => clearTimeout(debounceTimer);
  }, [isConnected, settings, syncSettingsToDevice]);

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
            deviceIp={deviceIp}
            error={error}
            isConnected={isConnected}
            onConnect={connect}
            onDisconnect={disconnect}
            onDiscover={discover}
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
                    source={require('@/assets/images/Default.png')}
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
                    source={require('@/assets/images/Compact.png')}
                    style={styles.themePreviewImage}
                    resizeMode="cover"
                  />
                </View>
                <Text style={styles.themeCardTitle}>Compact</Text>
                <Text style={styles.themeCardDesc}>Minimalist view</Text>
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


          <View style={styles.bottomSpacer} />
        </ScrollView>
      </SafeAreaView>
    </View>
  );
}
