#include "ntp_client.h"
#include "config.h"
#include <WiFiUdp.h>
#include <NTPClient.h>

// WiFiUDP instance for NTP
static WiFiUDP ntpUDP;

// NTPClient instance (server, offset_seconds, update_interval_ms)
// IST = UTC+5:30 = 19800 seconds
static NTPClient timeClient(ntpUDP, NTP_SERVER, NTP_OFFSET_SEC, 60000);

static bool synced = false;

void ntp_init() {
  timeClient.begin();
  timeClient.update();
  synced = timeClient.isTimeSet();
}

void ntp_update() {
  if (timeClient.update()) {
    synced = true;
  }
}

String ntp_get_time() {
  if (!synced || !timeClient.isTimeSet()) return "--:--:--";
  
  int hr = timeClient.getHours();
  int mn = timeClient.getMinutes();
  int sc = timeClient.getSeconds();
  
  char buf[16];
  snprintf(buf, sizeof(buf), "%02d:%02d:%02d", hr, mn, sc);
  return String(buf);
}

String ntp_get_date() {
  if (!synced || !timeClient.isTimeSet()) return "----/--/--";
  
  time_t rawTime = timeClient.getEpochTime();
  struct tm* tmInfo = localtime(&rawTime);
  
  char buf[16];
  snprintf(buf, sizeof(buf), "%04d/%02d/%02d", 
           tmInfo->tm_year + 1900, 
           tmInfo->tm_mon + 1, 
           tmInfo->tm_mday);
  return String(buf);
}

String ntp_get_day() {
  if (!synced || !timeClient.isTimeSet()) return "---";
  
  time_t rawTime = timeClient.getEpochTime();
  struct tm* tmInfo = localtime(&rawTime);
  
  static const char* daynames[] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
  return String(daynames[tmInfo->tm_wday]);
}

int ntp_get_hour() {
  if (!synced || !timeClient.isTimeSet()) return 0;
  return timeClient.getHours();
}

int ntp_get_minute() {
  if (!synced || !timeClient.isTimeSet()) return 0;
  return timeClient.getMinutes();
}

int ntp_get_second() {
  if (!synced || !timeClient.isTimeSet()) return 0;
  return timeClient.getSeconds();
}

bool ntp_is_synced() {
  return synced && timeClient.isTimeSet();
}