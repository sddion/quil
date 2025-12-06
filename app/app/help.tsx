import React from 'react';
import {
  View,
  Text,
  StyleSheet,
  ScrollView,
  TouchableOpacity,
  Linking,
} from 'react-native';
import { SafeAreaView } from 'react-native-safe-area-context';
import { StatusBar } from 'expo-status-bar';
import { LinearGradient } from 'expo-linear-gradient';
import { router } from 'expo-router';
import {
  HelpCircle,
  Bluetooth,
  Wifi,
  Battery,
  Settings,
  AlertTriangle,
  ExternalLink,
  X,
} from 'lucide-react-native';
import { helpStyles as styles } from '@/styles/help';

export default function HelpScreen() {
  const handleOpenLink = (url: string) => {
    Linking.openURL(url);
  };

  return (
    <View style={styles.container}>
      <LinearGradient
        colors={['#0a0e27', '#1a1f3a']}
        style={StyleSheet.absoluteFillObject}
      />
      <StatusBar style="light" />
        <ScrollView style={styles.scrollView} contentContainerStyle={styles.scrollContent}>
          <View style={styles.section}>
            <View style={styles.sectionHeader}>
              <Bluetooth size={20} color="#00bfff" />
              <Text style={styles.sectionTitle}>Bluetooth Connection</Text>
            </View>
            <Text style={styles.sectionText}>
              1. Click &quot;Scan for Devices&quot; to search for nearby Quil robots{'\n'}
              2. Select your device from the list{'\n'}
              3. Wait for the connection to establish{'\n'}
              4. Once connected, you&apos;ll see device status and can configure settings
            </Text>
          </View>

          <View style={styles.section}>
            <View style={styles.sectionHeader}>
              <Wifi size={20} color="#00ff88" />
              <Text style={styles.sectionTitle}>WiFi Setup</Text>
            </View>
            <Text style={styles.sectionText}>
              Enter your WiFi network name (SSID) and password. After saving, Quil will attempt to connect to the network. Make sure your credentials are correct.
            </Text>
          </View>

          <View style={styles.section}>
            <View style={styles.sectionHeader}>
              <Settings size={20} color="#00bfff" />
              <Text style={styles.sectionTitle}>Configuration</Text>
            </View>
            <Text style={styles.sectionText}>
              • Timezone: Select your local timezone for accurate time display{'\n'}
              • Weather: Enter OpenWeatherMap API key and location{'\n'}
              • Brightness: Adjust display brightness (0-255){'\n'}
              • Theme: Choose from multiple display themes
            </Text>
          </View>

          <View style={styles.section}>
            <View style={styles.sectionHeader}>
              <Battery size={20} color="#00ff88" />
              <Text style={styles.sectionTitle}>Device Status</Text>
            </View>
            <Text style={styles.sectionText}>
              View real-time information about your Quil:{'\n'}
              • Battery level and charging status{'\n'}
              • Connected WiFi network{'\n'}
              • Current firmware version{'\n'}
              • Connection health indicator
            </Text>
          </View>

          <View style={styles.section}>
            <View style={styles.sectionHeader}>
              <HelpCircle size={20} color="#ff6b6b" />
              <Text style={styles.sectionTitle}>Troubleshooting</Text>
            </View>
            <Text style={styles.sectionText}>
              Connection Issues:{'\n'}
              • Ensure Bluetooth is enabled on your device{'\n'}
              • Move closer to your Quil robot{'\n'}
              • Try restarting the app{'\n'}
              • Restart your Quil device{'\n\n'}
              Configuration Not Saving:{'\n'}
              • Verify you&apos;re connected to the device{'\n'}
              • Check that all fields are filled correctly{'\n'}
              • Try disconnecting and reconnecting
            </Text>
          </View>

          <View style={styles.footer}>
            <Text style={styles.footerText}>Quil Control App v1.0.0</Text>
          </View>
        </ScrollView>
    </View>
  );
}