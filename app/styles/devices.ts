import { StyleSheet } from 'react-native';

export const devicesStyles = StyleSheet.create({
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
  sectionTitle: {
    fontSize: 11,
    fontWeight: '700' as const,
    color: '#00bfff',
    letterSpacing: 1.5,
    marginBottom: 12,
  },
  deviceCard: {
    backgroundColor: 'rgba(255,255,255,0.05)',
    borderRadius: 12,
    padding: 16,
    borderWidth: 1,
    borderColor: 'rgba(255,255,255,0.1)',
    marginBottom: 12,
    flexDirection: 'row',
    alignItems: 'center',
  },
  deviceInfo: {
    flex: 1,
  },
  deviceHeader: {
    flexDirection: 'row',
    alignItems: 'center',
    gap: 8,
    marginBottom: 8,
  },
  deviceName: {
    fontSize: 16,
    fontWeight: '600' as const,
    color: '#fff',
  },
  deviceOriginalName: {
    fontSize: 13,
    color: '#666',
  },
  deviceMeta: {
    flexDirection: 'row',
    alignItems: 'center',
    gap: 6,
  },
  deviceMetaText: {
    fontSize: 12,
    color: '#666',
  },
  connectedBadge: {
    marginTop: 8,
    backgroundColor: 'rgba(0,255,136,0.2)',
    paddingHorizontal: 8,
    paddingVertical: 4,
    borderRadius: 4,
    alignSelf: 'flex-start',
  },
  connectedText: {
    fontSize: 11,
    fontWeight: '600' as const,
    color: '#00ff88',
  },
  deviceActions: {
    flexDirection: 'row',
    alignItems: 'center',
    gap: 12,
  },
  actionButton: {
    padding: 8,
  },
  editContainer: {
    flex: 1,
    flexDirection: 'row',
    gap: 8,
    alignItems: 'center',
  },
  editInput: {
    flex: 1,
    backgroundColor: 'rgba(0,0,0,0.3)',
    borderWidth: 1,
    borderColor: 'rgba(255,255,255,0.2)',
    borderRadius: 8,
    padding: 10,
    color: '#fff',
    fontSize: 14,
  },
  saveButton: {
    backgroundColor: '#00ff88',
    paddingHorizontal: 16,
    paddingVertical: 8,
    borderRadius: 6,
  },
  saveButtonText: {
    color: '#0a0e27',
    fontSize: 13,
    fontWeight: '600' as const,
  },
  cancelButton: {
    paddingHorizontal: 12,
    paddingVertical: 8,
  },
  cancelButtonText: {
    color: '#666',
    fontSize: 13,
    fontWeight: '600' as const,
  },
  emptyState: {
    alignItems: 'center',
    justifyContent: 'center',
    paddingVertical: 60,
  },
  emptyText: {
    fontSize: 18,
    fontWeight: '600' as const,
    color: '#999',
    marginBottom: 8,
  },
  emptySubtext: {
    fontSize: 14,
    color: '#666',
  },
});
