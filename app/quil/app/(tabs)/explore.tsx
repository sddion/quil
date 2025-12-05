import React, { useState, useEffect } from 'react';
import { StyleSheet, View, ScrollView, TextInput, Pressable, Alert } from 'react-native';
import { Image } from 'expo-image';
import { ThemedText } from '@/components/themed-text';
import { ThemedView } from '@/components/themed-view';
import { IconSymbol } from '@/components/ui/icon-symbol';
import { Colors } from '@/constants/theme';
import { useColorScheme } from '@/hooks/use-color-scheme';
import { useBLE, QuilConfig } from '@/hooks/use-ble';

const TIMEZONES = [
  { label: 'UTC (0)', value: 0 },
  { label: 'IST - India (UTC+5:30)', value: 19800 },
  { label: 'CST - China (UTC+8)', value: 28800 },
  { label: 'JST - Japan (UTC+9)', value: 32400 },
  { label: 'EST - US East (UTC-5)', value: -18000 },
  { label: 'CST - US Central (UTC-6)', value: -21600 },
  { label: 'MST - US Mountain (UTC-7)', value: -25200 },
  { label: 'PST - US West (UTC-8)', value: -28800 },
  { label: 'CET - Central Europe (UTC+1)', value: 3600 },
  { label: 'EET - Eastern Europe (UTC+2)', value: 7200 },
  { label: 'AEST - Australia East (UTC+10)', value: 36000 },
];

export default function SettingsScreen() {
  const colorScheme = useColorScheme() ?? 'dark';
  const colors = Colors[colorScheme];

  const { connectionState, quilStatus, sendConfig, sendCommand } = useBLE();
  const isConnected = connectionState === 'connected';

  const [ssid, setSsid] = useState('');
  const [password, setPassword] = useState('');
  const [timezone, setTimezone] = useState(19800);
  const [weatherApiKey, setWeatherApiKey] = useState('');
  const [weatherLocation, setWeatherLocation] = useState('');
  const [brightness, setBrightness] = useState(128);
  const [theme, setTheme] = useState(0);
  const [showTimezones, setShowTimezones] = useState(false);
  const [isSaving, setIsSaving] = useState(false);

  // Load current config when connected
  useEffect(() => {
    if (quilStatus) {
      if (quilStatus.wifiSsid) setSsid(quilStatus.wifiSsid);
      setTimezone(quilStatus.timezone);
      setBrightness(quilStatus.brightness);
      setTheme(quilStatus.theme);
    }
  }, [quilStatus]);

  const handleSave = async () => {
    if (!isConnected) {
      Alert.alert('Not Connected', 'Please connect to a Quil device first.');
      return;
    }

    setIsSaving(true);
    const config: QuilConfig = {
      ssid,
      password,
      timezone,
      weatherApiKey,
      weatherLocation,
      brightness,
      theme,
    };

    const success = await sendConfig(config);
    setIsSaving(false);

    if (success) {
      Alert.alert('Settings Saved', 'Configuration has been sent to the device.');
    } else {
      Alert.alert('Error', 'Failed to save settings. Please try again.');
    }
  };

  const handleFactoryReset = () => {
    if (!isConnected) {
      Alert.alert('Not Connected', 'Please connect to a Quil device first.');
      return;
    }

    Alert.alert(
      'Factory Reset',
      'This will erase all settings and return the device to factory defaults. Are you sure?',
      [
        { text: 'Cancel', style: 'cancel' },
        {
          text: 'Reset',
          style: 'destructive',
          onPress: async () => {
            const success = await sendCommand('reset');
            if (success) {
              Alert.alert('Reset Complete', 'Device will restart with factory settings.');
            }
          }
        },
      ]
    );
  };

  const selectedTimezone = TIMEZONES.find(tz => tz.value === timezone);

  return (
    <ThemedView style={styles.container}>
      <ScrollView style={styles.scrollView} contentContainerStyle={styles.scrollContent}>
        {/* Header */}
        <View style={styles.header}>
          <ThemedText style={styles.title}>Settings</ThemedText>
          <ThemedText style={styles.subtitle}>Configure your Quil device</ThemedText>
        </View>

        {/* Connection Status Banner */}
        {!isConnected && (
          <View style={[styles.banner, { backgroundColor: 'rgba(255, 71, 87, 0.15)', borderColor: colors.danger }]}>
            <IconSymbol name="exclamationmark.triangle.fill" size={18} color={colors.danger} />
            <ThemedText style={[styles.bannerText, { color: colors.danger }]}>
              Not connected to device. Go to Dashboard to connect.
            </ThemedText>
          </View>
        )}

        {/* WiFi Section */}
        <View style={[styles.section, { backgroundColor: colors.card, borderColor: colors.cardBorder, opacity: isConnected ? 1 : 0.5 }]}>
          <View style={styles.sectionHeader}>
            <IconSymbol name="wifi" size={22} color={colors.tint} />
            <ThemedText style={styles.sectionTitle}>WiFi Network</ThemedText>
          </View>
          <View style={styles.inputGroup}>
            <ThemedText style={styles.inputLabel}>Network Name (SSID)</ThemedText>
            <TextInput
              style={[styles.input, { backgroundColor: colors.inputBackground, borderColor: colors.inputBorder, color: colors.text }]}
              value={ssid}
              onChangeText={setSsid}
              placeholder="Enter WiFi name"
              placeholderTextColor={colors.icon}
              editable={isConnected}
            />
          </View>
          <View style={styles.inputGroup}>
            <ThemedText style={styles.inputLabel}>Password</ThemedText>
            <TextInput
              style={[styles.input, { backgroundColor: colors.inputBackground, borderColor: colors.inputBorder, color: colors.text }]}
              value={password}
              onChangeText={setPassword}
              placeholder="Enter WiFi password"
              placeholderTextColor={colors.icon}
              secureTextEntry
              editable={isConnected}
            />
          </View>
        </View>

        {/* Timezone Section */}
        <View style={[styles.section, { backgroundColor: colors.card, borderColor: colors.cardBorder, opacity: isConnected ? 1 : 0.5 }]}>
          <View style={styles.sectionHeader}>
            <IconSymbol name="globe" size={22} color={colors.tint} />
            <ThemedText style={styles.sectionTitle}>Timezone</ThemedText>
          </View>
          <Pressable
            style={[styles.dropdown, { backgroundColor: colors.inputBackground, borderColor: colors.inputBorder }]}
            onPress={() => isConnected && setShowTimezones(!showTimezones)}
            disabled={!isConnected}
          >
            <ThemedText>{selectedTimezone?.label || 'Select timezone'}</ThemedText>
            <IconSymbol name={showTimezones ? 'chevron.up' : 'chevron.down'} size={16} color={colors.icon} />
          </Pressable>
          {showTimezones && (
            <View style={[styles.dropdownList, { backgroundColor: colors.inputBackground, borderColor: colors.inputBorder }]}>
              {TIMEZONES.map((tz) => (
                <Pressable
                  key={tz.value}
                  style={[styles.dropdownItem, timezone === tz.value && { backgroundColor: colors.cardBorder }]}
                  onPress={() => {
                    setTimezone(tz.value);
                    setShowTimezones(false);
                  }}
                >
                  <ThemedText style={[styles.dropdownItemText, timezone === tz.value && { color: colors.tint }]}>
                    {tz.label}
                  </ThemedText>
                </Pressable>
              ))}
            </View>
          )}
        </View>

        {/* Weather Section */}
        <View style={[styles.section, { backgroundColor: colors.card, borderColor: colors.cardBorder, opacity: isConnected ? 1 : 0.5 }]}>
          <View style={styles.sectionHeader}>
            <IconSymbol name="cloud.fill" size={22} color={colors.tint} />
            <ThemedText style={styles.sectionTitle}>Weather</ThemedText>
          </View>
          <View style={styles.inputGroup}>
            <ThemedText style={styles.inputLabel}>API Key</ThemedText>
            <TextInput
              style={[styles.input, { backgroundColor: colors.inputBackground, borderColor: colors.inputBorder, color: colors.text }]}
              value={weatherApiKey}
              onChangeText={setWeatherApiKey}
              placeholder="Enter OpenWeatherMap API key"
              placeholderTextColor={colors.icon}
              editable={isConnected}
            />
          </View>
          <View style={styles.inputGroup}>
            <ThemedText style={styles.inputLabel}>Location</ThemedText>
            <TextInput
              style={[styles.input, { backgroundColor: colors.inputBackground, borderColor: colors.inputBorder, color: colors.text }]}
              value={weatherLocation}
              onChangeText={setWeatherLocation}
              placeholder="e.g., Mumbai, IN"
              placeholderTextColor={colors.icon}
              editable={isConnected}
            />
          </View>
        </View>

        {/* Theme Section */}
        <View style={[styles.section, { backgroundColor: colors.card, borderColor: colors.cardBorder, opacity: isConnected ? 1 : 0.5 }]}>
          <View style={styles.sectionHeader}>
            <IconSymbol name="paintbrush.fill" size={22} color={colors.tint} />
            <ThemedText style={styles.sectionTitle}>Theme</ThemedText>
          </View>
          <View style={styles.themeGrid}>
            <Pressable
              style={[styles.themeOption, { backgroundColor: colors.inputBackground, borderColor: theme === 0 ? colors.tint : colors.inputBorder }]}
              onPress={() => isConnected && setTheme(0)}
            >
              <Image source={require('@/assets/images/Default.png')} style={styles.themePreview} contentFit="contain" />
              <ThemedText style={[styles.themeName, theme === 0 && { color: colors.tint }]}>Default</ThemedText>
            </Pressable>
            <Pressable
              style={[styles.themeOption, { backgroundColor: colors.inputBackground, borderColor: theme === 1 ? colors.tint : colors.inputBorder }]}
              onPress={() => isConnected && setTheme(1)}
            >
              <Image source={require('@/assets/images/Compact.png')} style={styles.themePreview} contentFit="contain" />
              <ThemedText style={[styles.themeName, theme === 1 && { color: colors.tint }]}>Compact</ThemedText>
            </Pressable>
          </View>
        </View>

        {/* Display Section */}
        <View style={[styles.section, { backgroundColor: colors.card, borderColor: colors.cardBorder, opacity: isConnected ? 1 : 0.5 }]}>
          <View style={styles.sectionHeader}>
            <IconSymbol name="sun.max.fill" size={22} color={colors.tint} />
            <ThemedText style={styles.sectionTitle}>Display</ThemedText>
          </View>
          <View style={styles.inputGroup}>
            <View style={styles.sliderHeader}>
              <ThemedText style={styles.inputLabel}>Brightness</ThemedText>
              <ThemedText style={styles.sliderValue}>{Math.round((brightness / 255) * 100)}%</ThemedText>
            </View>
            <View style={styles.sliderContainer}>
              <Pressable
                style={[styles.sliderButton, { backgroundColor: colors.inputBackground, borderColor: colors.inputBorder }]}
                onPress={() => isConnected && setBrightness(Math.max(0, brightness - 25))}
                disabled={!isConnected}
              >
                <IconSymbol name="minus" size={16} color={colors.text} />
              </Pressable>
              <View style={[styles.sliderTrack, { backgroundColor: colors.inputBackground }]}>
                <View style={[styles.sliderFill, { width: `${(brightness / 255) * 100}%`, backgroundColor: colors.tint }]} />
              </View>
              <Pressable
                style={[styles.sliderButton, { backgroundColor: colors.inputBackground, borderColor: colors.inputBorder }]}
                onPress={() => isConnected && setBrightness(Math.min(255, brightness + 25))}
                disabled={!isConnected}
              >
                <IconSymbol name="plus" size={16} color={colors.text} />
              </Pressable>
            </View>
          </View>
        </View>

        {/* Save Button */}
        <Pressable
          style={[styles.saveButton, { backgroundColor: isConnected ? colors.tint : colors.cardBorder }]}
          onPress={handleSave}
          disabled={!isConnected || isSaving}
        >
          <IconSymbol name="checkmark.circle.fill" size={20} color={isConnected ? "#000" : colors.icon} />
          <ThemedText style={[styles.saveButtonText, { color: isConnected ? '#000' : colors.icon }]}>
            {isSaving ? 'Saving...' : 'Save Configuration'}
          </ThemedText>
        </Pressable>

        {/* Danger Zone */}
        <View style={[styles.section, styles.dangerSection, { backgroundColor: 'rgba(255, 71, 87, 0.1)', borderColor: colors.danger, opacity: isConnected ? 1 : 0.5 }]}>
          <View style={styles.sectionHeader}>
            <IconSymbol name="exclamationmark.triangle.fill" size={22} color={colors.danger} />
            <ThemedText style={[styles.sectionTitle, { color: colors.danger }]}>Danger Zone</ThemedText>
          </View>
          <ThemedText style={styles.dangerText}>
            Factory reset will erase all settings including WiFi credentials, timezone, and weather configuration.
          </ThemedText>
          <Pressable
            style={[styles.dangerButton, { borderColor: colors.danger }]}
            onPress={handleFactoryReset}
            disabled={!isConnected}
          >
            <IconSymbol name="trash.fill" size={18} color={colors.danger} />
            <ThemedText style={[styles.dangerButtonText, { color: colors.danger }]}>Factory Reset</ThemedText>
          </Pressable>
        </View>
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
    paddingBottom: 40,
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
  banner: {
    flexDirection: 'row',
    alignItems: 'center',
    gap: 10,
    padding: 14,
    borderRadius: 12,
    borderWidth: 1,
    marginBottom: 16,
  },
  bannerText: {
    flex: 1,
    fontSize: 13,
    fontWeight: '500',
  },
  section: {
    borderRadius: 16,
    padding: 20,
    marginBottom: 16,
    borderWidth: 1,
  },
  sectionHeader: {
    flexDirection: 'row',
    alignItems: 'center',
    marginBottom: 16,
    gap: 10,
  },
  sectionTitle: {
    fontSize: 18,
    fontWeight: '600',
  },
  inputGroup: {
    marginBottom: 12,
  },
  inputLabel: {
    fontSize: 13,
    opacity: 0.6,
    marginBottom: 6,
  },
  input: {
    borderWidth: 1,
    borderRadius: 10,
    padding: 14,
    fontSize: 15,
  },
  dropdown: {
    flexDirection: 'row',
    justifyContent: 'space-between',
    alignItems: 'center',
    borderWidth: 1,
    borderRadius: 10,
    padding: 14,
  },
  dropdownList: {
    marginTop: 8,
    borderWidth: 1,
    borderRadius: 10,
    overflow: 'hidden',
  },
  dropdownItem: {
    padding: 14,
  },
  dropdownItemText: {
    fontSize: 14,
  },
  sliderHeader: {
    flexDirection: 'row',
    justifyContent: 'space-between',
    alignItems: 'center',
    marginBottom: 12,
  },
  sliderValue: {
    fontSize: 14,
    fontWeight: '600',
  },
  sliderContainer: {
    flexDirection: 'row',
    alignItems: 'center',
    gap: 12,
  },
  sliderButton: {
    width: 40,
    height: 40,
    borderRadius: 10,
    borderWidth: 1,
    justifyContent: 'center',
    alignItems: 'center',
  },
  sliderTrack: {
    flex: 1,
    height: 8,
    borderRadius: 4,
    overflow: 'hidden',
  },
  sliderFill: {
    height: '100%',
    borderRadius: 4,
  },
  saveButton: {
    flexDirection: 'row',
    alignItems: 'center',
    justifyContent: 'center',
    gap: 8,
    padding: 16,
    borderRadius: 12,
    marginBottom: 16,
  },
  saveButtonText: {
    fontSize: 16,
    fontWeight: '600',
  },
  dangerSection: {
    marginTop: 16,
  },
  dangerText: {
    fontSize: 13,
    opacity: 0.8,
    marginBottom: 16,
    lineHeight: 18,
  },
  dangerButton: {
    flexDirection: 'row',
    alignItems: 'center',
    justifyContent: 'center',
    gap: 8,
    padding: 14,
    borderRadius: 10,
    borderWidth: 1,
  },
  dangerButtonText: {
    fontSize: 14,
    fontWeight: '600',
  },
  themeGrid: {
    flexDirection: 'row',
    gap: 12,
  },
  themeOption: {
    flex: 1,
    borderRadius: 12,
    borderWidth: 2,
    padding: 8,
    alignItems: 'center',
    gap: 8,
  },
  themePreview: {
    width: '100%',
    height: 80,
    borderRadius: 8,
  },
  themeName: {
    fontSize: 14,
    fontWeight: '500',
  },
});
