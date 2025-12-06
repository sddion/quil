import React from 'react';
import { View, Text, TouchableOpacity } from 'react-native';
import { Clock, CloudSun, RotateCcw, Download } from 'lucide-react-native';
import { homeStyles as styles } from '@/styles/index';

export interface QuickActionsProps {
  onQuickAction: (action: string) => void;
}

export function QuickActions({ onQuickAction }: QuickActionsProps) {
  return (
    <View style={styles.section}>
      <Text style={styles.sectionTitle}>QUICK ACTIONS</Text>
      <View style={styles.actionsGrid}>
        <TouchableOpacity
          style={styles.actionButton}
          onPress={() => onQuickAction('Sync Time')}
        >
          <Clock size={24} color="#00bfff" />
          <Text style={styles.actionButtonText}>Sync Time</Text>
        </TouchableOpacity>
        <TouchableOpacity
          style={styles.actionButton}
          onPress={() => onQuickAction('Update Weather')}
        >
          <CloudSun size={24} color="#00bfff" />
          <Text style={styles.actionButtonText}>Update Weather</Text>
        </TouchableOpacity>
        <TouchableOpacity
          style={styles.actionButton}
          onPress={() => onQuickAction('Restart')}
        >
          <RotateCcw size={24} color="#00bfff" />
          <Text style={styles.actionButtonText}>Restart</Text>
        </TouchableOpacity>
        <TouchableOpacity
          style={styles.actionButton}
          onPress={() => onQuickAction('Check Updates')}
        >
          <Download size={24} color="#00bfff" />
          <Text style={styles.actionButtonText}>Check Updates</Text>
        </TouchableOpacity>
      </View>
    </View>
  );
}
