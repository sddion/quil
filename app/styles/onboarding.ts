import { StyleSheet, Dimensions } from 'react-native';

const { width } = Dimensions.get('window');

export const onboardingStyles = StyleSheet.create({
  container: {
    flex: 1,
    backgroundColor: '#0a0e27',
  },
  safeArea: {
    flex: 1,
  },
  skipButton: {
    alignSelf: 'flex-end',
    padding: 16,
    margin: 8,
  },
  skipText: {
    fontSize: 16,
    color: '#999',
    fontWeight: '600' as const,
  },
  content: {
    flex: 1,
    alignItems: 'center',
    justifyContent: 'center',
    paddingHorizontal: 32,
  },
  iconContainer: {
    width: 160,
    height: 160,
    borderRadius: 80,
    alignItems: 'center',
    justifyContent: 'center',
    marginBottom: 48,
  },
  title: {
    fontSize: 32,
    fontWeight: '700' as const,
    color: '#fff',
    textAlign: 'center',
    marginBottom: 16,
  },
  description: {
    fontSize: 16,
    color: '#999',
    textAlign: 'center',
    lineHeight: 24,
    maxWidth: width - 80,
  },
  footer: {
    paddingHorizontal: 32,
    paddingBottom: 32,
    gap: 24,
  },
  pagination: {
    flexDirection: 'row',
    justifyContent: 'center',
    gap: 8,
  },
  dot: {
    width: 8,
    height: 8,
    borderRadius: 4,
    backgroundColor: 'rgba(255,255,255,0.2)',
  },
  dotActive: {
    width: 24,
  },
  button: {
    paddingVertical: 16,
    borderRadius: 12,
    alignItems: 'center',
  },
  buttonText: {
    fontSize: 18,
    fontWeight: '700' as const,
    color: '#0a0e27',
    letterSpacing: 1,
  },
});
