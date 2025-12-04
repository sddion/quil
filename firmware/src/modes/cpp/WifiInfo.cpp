#include "../h/WifiInfo.h"
#include "hal/h/Display.h"
#include "modules/h/WifiManager.h"

void mode_wifi_init() {}

void mode_wifi_update() {}

void mode_wifi_render() {
  hal_display_clear();
  String ip = wifi_get_ip();
  int rssi = wifi_get_rssi();
  hal_display_text("WiFi Info", 20, 0);
  hal_display_text(ip.c_str(), 10, 20);
  char buf[16];
  sprintf(buf, "RSSI: %d", rssi);
  hal_display_text(buf, 10, 40);
  hal_display_update();
}
