import { StyleSheet } from 'react-native';

export const helpStyles = StyleSheet.create({
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
