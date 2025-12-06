import React, { useState } from 'react';
import {
  View,
  Text,
  ScrollView,
  TouchableOpacity,
  TextInput,
  Alert,
  StyleSheet,
} from 'react-native';
import { StatusBar } from 'expo-status-bar';
import { LinearGradient } from 'expo-linear-gradient';
import { router } from 'expo-router';
import {
  Star,
  Edit2,
  Trash2,
  Clock,
  ChevronRight,
  X,
} from 'lucide-react-native';
import { useDevices } from '@/contexts/devices';
import { useBLE } from '@/hooks/use-ble';
import { useNotifications } from '@/contexts/notifications';
import { devicesStyles as styles } from '@/styles/devices';

export default function DevicesScreen() {
  const { savedDevices, toggleFavorite, updateDeviceName, removeDevice } = useDevices();
  const { connect, connectedDevice } = useBLE();
  const { showNotification } = useNotifications();
  const [editingId, setEditingId] = useState<string | null>(null);
  const [editName, setEditName] = useState<string>('');

  const handleEdit = (id: string, currentName: string) => {
    setEditingId(id);
    setEditName(currentName);
  };

  const handleSaveName = async (id: string) => {
    if (editName.trim()) {
      await updateDeviceName(id, editName.trim());
      showNotification('success', 'Device Updated', 'Custom name saved');
    }
    setEditingId(null);
    setEditName('');
  };

  const handleDelete = (id: string, name: string) => {
    Alert.alert(
      'Remove Device',
      `Are you sure you want to remove ${name}?`,
      [
        { text: 'Cancel', style: 'cancel' },
        {
          text: 'Remove',
          style: 'destructive',
          onPress: async () => {
            await removeDevice(id);
            showNotification('success', 'Device Removed', `${name} has been removed`);
          },
        },
      ]
    );
  };

  const handleConnect = async (device: { id: string; name: string; customName?: string }) => {
    const deviceToConnect = {
      id: device.id,
      name: device.customName || device.name,
      rssi: -60,
    };
    await connect(deviceToConnect);
    showNotification('info', 'Connecting', `Connecting to ${deviceToConnect.name}...`);
    router.back();
  };

  const favorites = savedDevices.filter(d => d.isFavorite);
  const recent = savedDevices
    .filter(d => !d.isFavorite)
    .sort((a, b) => b.lastConnectedAt - a.lastConnectedAt);

  return (
    <View style={styles.container}>
      <LinearGradient
        colors={['#0a0e27', '#1a1f3a']}
        style={StyleSheet.absoluteFillObject}
      />
      <StatusBar style="light" />

        <ScrollView style={styles.scrollView} contentContainerStyle={styles.scrollContent}>
          {favorites.length > 0 && (
            <View style={styles.section}>
              <Text style={styles.sectionTitle}>FAVORITES</Text>
              {favorites.map(device => (
                <View key={device.id} style={styles.deviceCard}>
                  {editingId === device.id ? (
                    <View style={styles.editContainer}>
                      <TextInput
                        style={styles.editInput}
                        value={editName}
                        onChangeText={setEditName}
                        placeholder="Enter custom name"
                        placeholderTextColor="#666"
                        autoFocus
                      />
                      <TouchableOpacity
                        style={styles.saveButton}
                        onPress={() => handleSaveName(device.id)}
                      >
                        <Text style={styles.saveButtonText}>Save</Text>
                      </TouchableOpacity>
                      <TouchableOpacity
                        style={styles.cancelButton}
                        onPress={() => setEditingId(null)}
                      >
                        <Text style={styles.cancelButtonText}>Cancel</Text>
                      </TouchableOpacity>
                    </View>
                  ) : (
                    <>
                      <TouchableOpacity
                        style={styles.deviceInfo}
                        onPress={() => handleConnect(device)}
                      >
                        <View style={styles.deviceHeader}>
                          <Text style={styles.deviceName}>
                            {device.customName || device.name}
                          </Text>
                          {device.customName && (
                            <Text style={styles.deviceOriginalName}>({device.name})</Text>
                          )}
                        </View>
                        <View style={styles.deviceMeta}>
                          <Clock size={14} color="#666" />
                          <Text style={styles.deviceMetaText}>
                            {new Date(device.lastConnectedAt).toLocaleDateString()}
                          </Text>
                          <Text style={styles.deviceMetaText}>·</Text>
                          <Text style={styles.deviceMetaText}>
                            {device.connectionCount} connections
                          </Text>
                        </View>
                        {connectedDevice?.id === device.id && (
                          <View style={styles.connectedBadge}>
                            <Text style={styles.connectedText}>Connected</Text>
                          </View>
                        )}
                      </TouchableOpacity>

                      <View style={styles.deviceActions}>
                        <TouchableOpacity
                          onPress={() => toggleFavorite(device.id)}
                          style={styles.actionButton}
                        >
                          <Star
                            size={20}
                            color="#FFD700"
                            fill="#FFD700"
                          />
                        </TouchableOpacity>
                        <TouchableOpacity
                          onPress={() => handleEdit(device.id, device.customName || device.name)}
                          style={styles.actionButton}
                        >
                          <Edit2 size={18} color="#00bfff" />
                        </TouchableOpacity>
                        <TouchableOpacity
                          onPress={() => handleDelete(device.id, device.customName || device.name)}
                          style={styles.actionButton}
                        >
                          <Trash2 size={18} color="#ff4444" />
                        </TouchableOpacity>
                        <ChevronRight size={20} color="#666" />
                      </View>
                    </>
                  )}
                </View>
              ))}
            </View>
          )}

          {recent.length > 0 && (
            <View style={styles.section}>
              <Text style={styles.sectionTitle}>RECENT DEVICES</Text>
              {recent.map(device => (
                <View key={device.id} style={styles.deviceCard}>
                  {editingId === device.id ? (
                    <View style={styles.editContainer}>
                      <TextInput
                        style={styles.editInput}
                        value={editName}
                        onChangeText={setEditName}
                        placeholder="Enter custom name"
                        placeholderTextColor="#666"
                        autoFocus
                      />
                      <TouchableOpacity
                        style={styles.saveButton}
                        onPress={() => handleSaveName(device.id)}
                      >
                        <Text style={styles.saveButtonText}>Save</Text>
                      </TouchableOpacity>
                      <TouchableOpacity
                        style={styles.cancelButton}
                        onPress={() => setEditingId(null)}
                      >
                        <Text style={styles.cancelButtonText}>Cancel</Text>
                      </TouchableOpacity>
                    </View>
                  ) : (
                    <>
                      <TouchableOpacity
                        style={styles.deviceInfo}
                        onPress={() => handleConnect(device)}
                      >
                        <View style={styles.deviceHeader}>
                          <Text style={styles.deviceName}>
                            {device.customName || device.name}
                          </Text>
                          {device.customName && (
                            <Text style={styles.deviceOriginalName}>({device.name})</Text>
                          )}
                        </View>
                        <View style={styles.deviceMeta}>
                          <Clock size={14} color="#666" />
                          <Text style={styles.deviceMetaText}>
                            {new Date(device.lastConnectedAt).toLocaleDateString()}
                          </Text>
                          <Text style={styles.deviceMetaText}>·</Text>
                          <Text style={styles.deviceMetaText}>
                            {device.connectionCount} connections
                          </Text>
                        </View>
                        {connectedDevice?.id === device.id && (
                          <View style={styles.connectedBadge}>
                            <Text style={styles.connectedText}>Connected</Text>
                          </View>
                        )}
                      </TouchableOpacity>

                      <View style={styles.deviceActions}>
                        <TouchableOpacity
                          onPress={() => toggleFavorite(device.id)}
                          style={styles.actionButton}
                        >
                          <Star
                            size={20}
                            color="#666"
                          />
                        </TouchableOpacity>
                        <TouchableOpacity
                          onPress={() => handleEdit(device.id, device.customName || device.name)}
                          style={styles.actionButton}
                        >
                          <Edit2 size={18} color="#00bfff" />
                        </TouchableOpacity>
                        <TouchableOpacity
                          onPress={() => handleDelete(device.id, device.customName || device.name)}
                          style={styles.actionButton}
                        >
                          <Trash2 size={18} color="#ff4444" />
                        </TouchableOpacity>
                        <ChevronRight size={20} color="#666" />
                      </View>
                    </>
                  )}
                </View>
              ))}
            </View>
          )}

          {savedDevices.length === 0 && (
            <View style={styles.emptyState}>
              <Text style={styles.emptyText}>No devices saved yet</Text>
              <Text style={styles.emptySubtext}>
                Connect to a device to see it here
              </Text>
            </View>
          )}
        </ScrollView>
    </View>
  );
}
