#include "ntp_client.h"
#include "config.h"
#include <time.h>

static bool synced = false;
static unsigned long last_sync = 0;

void ntp_init() {
  configTime(NTP_OFFSET_SEC, 0, NTP_SERVER);
}

void ntp_update() {
  unsigned long now = millis();
  if (now - last_sync < NTP_UPDATE_MS) return;
  last_sync = now;
  time_t t = time(nullptr);
  synced = (t > 100000);
}

String ntp_get_time() {
  if (!synced) return "--:--:--";
  time_t now = time(nullptr);
  struct tm* t = localtime(&now);
  char buf[32];
  strftime(buf, sizeof(buf), "%H:%M:%S", t);
  return String(buf);
}

String ntp_get_date() {
  if (!synced) return "----/--/--";
  time_t now = time(nullptr);
  struct tm* t = localtime(&now);
  char buf[32];
  strftime(buf, sizeof(buf), "%Y/%m/%d", t);
  return String(buf);
}

String ntp_get_day() {
  if (!synced) return "---";
  time_t now = time(nullptr);
  struct tm* t = localtime(&now);
  char buf[16];
  strftime(buf, sizeof(buf), "%a", t);  // Short day name
  return String(buf);
}

int ntp_get_hour() {
  if (!synced) return 0;
  time_t now = time(nullptr);
  struct tm* t = localtime(&now);
  return t->tm_hour;
}

int ntp_get_minute() {
  if (!synced) return 0;
  time_t now = time(nullptr);
  struct tm* t = localtime(&now);
  return t->tm_min;
}

int ntp_get_second() {
  if (!synced) return 0;
  time_t now = time(nullptr);
  struct tm* t = localtime(&now);
  return t->tm_sec;
}

bool ntp_is_synced() {
  return synced;
}
