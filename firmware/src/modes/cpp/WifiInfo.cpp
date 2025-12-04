#include "../h/WifiInfo.h"
#include "hal/h/Display.h"
#include "modules/h/WifiManager.h"

void WifiInfoInit() {}

void WifiInfoUpdate() {}

void WifiInfoRender() {
  DisplayClear();
  String ip = WifiGetIp();
  int rssi = WifiGetRssi();
  DisplayText("WiFi Info", 20, 0);
  DisplayText(ip.c_str(), 10, 20);
  char buf[16];
  sprintf(buf, "RSSI: %d", rssi);
  DisplayText(buf, 10, 40);
  DisplayUpdate();
}