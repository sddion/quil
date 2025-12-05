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

      <SafeAreaView style={styles.safeArea} edges={['top']}>
        <View style={styles.header}>
          <Text style={styles.title}>Help & Support</Text>
          <TouchableOpacity onPress={() => router.back()} style={styles.closeButton}>
            <X size={24} color="#fff" />
          </TouchableOpacity>
        </View>

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
            <View style={styles.note}>
              <AlertTriangle size={16} color="#FFD700" />
              <Text style={styles.noteText}>
                Web Bluetooth only works in Chrome, Edge, and Opera browsers
              </Text>
            </View>
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
              • Try refreshing the page{'\n'}
              • Restart your Quil device{'\n\n'}
              Configuration Not Saving:{'\n'}
              • Verify you&apos;re connected to the device{'\n'}
              • Check that all fields are filled correctly{'\n'}
              • Try disconnecting and reconnecting
            </Text>
          </View>

          <View style={styles.section}>
            <View style={styles.sectionHeader}>
              <ExternalLink size={20} color="#00bfff" />
              <Text style={styles.sectionTitle}>Resources</Text>
            </View>
            <TouchableOpacity
              style={styles.link}
              onPress={() => handleOpenLink('https://github.com/WebBluetoothCG/web-bluetooth')}
            >
              <Text style={styles.linkText}>Web Bluetooth API Documentation</Text>
              <ExternalLink size={16} color="#00bfff" />
            </TouchableOpacity>
            <TouchableOpacity
              style={styles.link}
              onPress={() => handleOpenLink('https://openweathermap.org/api')}
            >
              <Text style={styles.linkText}>OpenWeatherMap API</Text>
              <ExternalLink size={16} color="#00bfff" />
            </TouchableOpacity>
          </View>

          <View style={styles.footer}>
            <Text style={styles.footerText}>Quil Control App v1.0.0</Text>
            <Text style={styles.footerSubtext}>
              For additional support, please visit our website
            </Text>
          </View>
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
    alignItems: 'center',
    justifyContent: 'space-between',
    paddingHorizontal: 24,
    paddingVertical: 16,
    borderBottomWidth: 1,
    borderBottomColor: 'rgba(255,255,255,0.1)',
  },
  title: {
    fontSize: 24,
    fontWeight: '700' as const,
    color: '#fff',
  },
  closeButton: {
    padding: 8,
  },
  scrollView: {
    flex: 1,
  },
  scrollContent: {
    padding: 24,
  },
  section: {
    marginBottom: 32,
  },
  sectionHeader: {
    flexDirection: 'row',
    alignItems: 'center',
    gap: 12,
    marginBottom: 12,
  },
  sectionTitle: {
    fontSize: 18,
    fontWeight: '700' as const,
    color: '#fff',
  },
  sectionText: {
    fontSize: 15,
    color: '#999',
    lineHeight: 24,
  },
  note: {
    flexDirection: 'row',
    alignItems: 'center',
    gap: 8,
    marginTop: 12,
    padding: 12,
    backgroundColor: 'rgba(255,215,0,0.1)',
    borderRadius: 8,
    borderWidth: 1,
    borderColor: 'rgba(255,215,0,0.3)',
  },
  noteText: {
    flex: 1,
    fontSize: 13,
    color: '#FFD700',
    lineHeight: 18,
  },
  link: {
    flexDirection: 'row',
    alignItems: 'center',
    justifyContent: 'space-between',
    padding: 16,
    backgroundColor: 'rgba(0,191,255,0.1)',
    borderRadius: 12,
    borderWidth: 1,
    borderColor: 'rgba(0,191,255,0.3)',
    marginBottom: 12,
  },
  linkText: {
    fontSize: 15,
    color: '#00bfff',
    fontWeight: '600' as const,
  },
  footer: {
    alignItems: 'center',
    paddingVertical: 32,
  },
  footerText: {
    fontSize: 14,
    color: '#666',
    fontWeight: '600' as const,
    marginBottom: 4,
  },
  footerSubtext: {
    fontSize: 13,
    color: '#555',
  },
});
