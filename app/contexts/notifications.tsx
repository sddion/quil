import createContextHook from '@nkzw/create-context-hook';
import { useState, useCallback } from 'react';
import * as Haptics from 'expo-haptics';

export type NotificationType = 'success' | 'error' | 'warning' | 'info';

export type Notification = {
  id: string;
  type: NotificationType;
  title: string;
  message: string;
  timestamp: number;
  duration: number;
};

export const [NotificationsProvider, useNotifications] = createContextHook(() => {
  const [notifications, setNotifications] = useState<Notification[]>([]);
  const [currentNotification, setCurrentNotification] = useState<Notification | null>(null);

  const showNotification = useCallback((
    type: NotificationType,
    title: string,
    message: string,
    duration: number = 3000
  ) => {
    const notification: Notification = {
      id: `${Date.now()}-${Math.random()}`,
      type,
      title,
      message,
      timestamp: Date.now(),
      duration,
    };

    switch (type) {
      case 'success':
        Haptics.notificationAsync(Haptics.NotificationFeedbackType.Success);
        break;
      case 'error':
        Haptics.notificationAsync(Haptics.NotificationFeedbackType.Error);
        break;
      case 'warning':
        Haptics.notificationAsync(Haptics.NotificationFeedbackType.Warning);
        break;
      default:
        Haptics.impactAsync(Haptics.ImpactFeedbackStyle.Light);
    }

    setNotifications(prev => [...prev, notification]);
    setCurrentNotification(notification);

    setTimeout(() => {
      setCurrentNotification(null);
    }, duration);

    console.log(`[Notification] ${type}: ${title} - ${message}`);
  }, []);

  const dismissNotification = useCallback((id: string) => {
    setNotifications(prev => prev.filter(n => n.id !== id));
    setCurrentNotification(null);
  }, []);

  const clearAll = useCallback(() => {
    setNotifications([]);
    setCurrentNotification(null);
  }, []);

  return {
    notifications,
    currentNotification,
    showNotification,
    dismissNotification,
    clearAll,
  };
});
