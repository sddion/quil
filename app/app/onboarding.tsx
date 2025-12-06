import React, { useState } from 'react';
import {
  View,
  Text,
  StyleSheet,
  TouchableOpacity,
  Dimensions,
} from 'react-native';
import { SafeAreaView } from 'react-native-safe-area-context';
import { StatusBar } from 'expo-status-bar';
import { LinearGradient } from 'expo-linear-gradient';
import { router } from 'expo-router';
import AsyncStorage from '@react-native-async-storage/async-storage';
import {
  Bluetooth,
  Settings,
  RefreshCw,
  CheckCircle,
} from 'lucide-react-native';
import { onboardingStyles as styles } from '@/styles/onboarding';

const { width } = Dimensions.get('window');

const ONBOARDING_KEY = '@quil_onboarding_completed';

const slides = [
  {
    icon: Bluetooth,
    title: 'Connect via Bluetooth',
    description: 'Scan and connect to your Quil robot using Bluetooth Low Energy.',
    color: '#00bfff',
  },
  {
    icon: Settings,
    title: 'Configure Settings',
    description: 'Set up WiFi, timezone, weather, display brightness, and choose from multiple themes for your Quil.',
    color: '#00ff88',
  },
  {
    icon: RefreshCw,
    title: 'Quick Actions',
    description: 'Sync time, update weather, restart device, or check for firmware updates with a single tap.',
    color: '#FFD700',
  },
  {
    icon: CheckCircle,
    title: 'Ready to Go!',
    description: 'Your Quil dashboard is ready. Start by scanning for nearby devices.',
    color: '#ff6b6b',
  },
];

export default function OnboardingScreen() {
  const [currentSlide, setCurrentSlide] = useState<number>(0);

  const handleNext = () => {
    if (currentSlide < slides.length - 1) {
      setCurrentSlide(currentSlide + 1);
    }
  };

  const handleSkip = async () => {
    await AsyncStorage.setItem(ONBOARDING_KEY, 'true');
    router.replace('/');
  };

  const handleFinish = async () => {
    await AsyncStorage.setItem(ONBOARDING_KEY, 'true');
    router.replace('/');
  };

  const slide = slides[currentSlide];
  const Icon = slide.icon;
  const isLastSlide = currentSlide === slides.length - 1;

  return (
    <View style={styles.container}>
      <LinearGradient
        colors={['#0a0e27', '#1a1f3a']}
        style={StyleSheet.absoluteFillObject}
      />
      <StatusBar style="light" />

      <SafeAreaView style={styles.safeArea}>
        <TouchableOpacity style={styles.skipButton} onPress={handleSkip}>
          <Text style={styles.skipText}>Skip</Text>
        </TouchableOpacity>

        <View style={styles.content}>
          <View style={[styles.iconContainer, { backgroundColor: `${slide.color}20` }]}>
            <Icon size={80} color={slide.color} />
          </View>

          <Text style={styles.title}>{slide.title}</Text>
          <Text style={styles.description}>{slide.description}</Text>
        </View>

        <View style={styles.footer}>
          <View style={styles.pagination}>
            {slides.map((_, index) => (
              <View
                key={index}
                style={[
                  styles.dot,
                  currentSlide === index && styles.dotActive,
                  currentSlide === index && { backgroundColor: slide.color },
                ]}
              />
            ))}
          </View>

          <TouchableOpacity
            style={[styles.button, { backgroundColor: slide.color }]}
            onPress={isLastSlide ? handleFinish : handleNext}
          >
            <Text style={styles.buttonText}>
              {isLastSlide ? 'Get Started' : 'Next'}
            </Text>
          </TouchableOpacity>
        </View>
      </SafeAreaView>
    </View>
  );
}