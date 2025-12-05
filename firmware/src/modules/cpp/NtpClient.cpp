#include "../h/NtpClient.h"
#include "config.h"
#include <WiFiUdp.h>
#include <NTPClient.h>

// WiFiUDP instance for NTP
static WiFiUDP ntpUDP;

// NTPClient instance (server, offset_seconds, update_interval_ms)
// IST = UTC+5:30 = 19800 seconds
static NTPClient timeClient(ntpUDP, NTP_SERVER, NTP_OFFSET_SEC, 60000);

static bool synced = false;
static int current_offset = NTP_OFFSET_SEC;

void NtpInit() {
  timeClient.begin();
  timeClient.update();
  synced = timeClient.isTimeSet();
}

void NtpUpdate() {
  if (timeClient.update()) {
    synced = true;
  }
}

void NtpSetTimezone(int offset_sec) {
  current_offset = offset_sec;
  timeClient.setTimeOffset(offset_sec);
  timeClient.forceUpdate();
}

String NtpGetTime() {
  if (!synced || !timeClient.isTimeSet()) return "--:--";
  
  int hr = timeClient.getHours();
  int mn = timeClient.getMinutes();
  
  char buf[16];
  snprintf(buf, sizeof(buf), "%02d:%02d", hr, mn);
  return String(buf);
}

String NtpGetDate() {
  if (!synced || !timeClient.isTimeSet()) return "----/--/--";
  
  time_t rawTime = timeClient.getEpochTime();
  struct tm* tmInfo = localtime(&rawTime);
  
  char buf[32];
  snprintf(buf, sizeof(buf), "%04d/%02d/%02d", 
           tmInfo->tm_year + 1900, 
           tmInfo->tm_mon + 1, 
           tmInfo->tm_mday);
  return String(buf);
}

String NtpGetDay() {
  if (!synced || !timeClient.isTimeSet()) return "---";
  
  time_t rawTime = timeClient.getEpochTime();
  struct tm* tmInfo = localtime(&rawTime);
  
  static const char* daynames[] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
  return String(daynames[tmInfo->tm_wday]);
}

int NtpGetHour() {
  if (!synced || !timeClient.isTimeSet()) return 0;
  return timeClient.getHours();
}

int NtpGetMinute() {
  if (!synced || !timeClient.isTimeSet()) return 0;
  return timeClient.getMinutes();
}

int NtpGetSecond() {
  if (!synced || !timeClient.isTimeSet()) return 0;
  return timeClient.getSeconds();
}

bool NtpIsSynced() {
  return synced && timeClient.isTimeSet();
}