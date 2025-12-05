#include "CompactTheme.h"
#include <Adafruit_GFX.h>
#include "../assets/fonts/Org_01.h"

void CompactThemeRender(int hour, int minute, const char* dateStr, const char* dayStr, 
                       uint8_t batteryPct, int rssi, bool wifiConnected, bool btConnected,
                       uint8_t weatherCode, const char* tempStr, const char* condStr) {
  Adafruit_SSD1306& display = DisplayGetDisplay();
  display.clearDisplay();
  
  // WiFi + Bluetooth at top left
  if (wifiConnected) {
    StatusIconsDrawWifi(2, 2, rssi);
  }
  StatusIconsDrawBluetooth(25, 2, btConnected);
  
  // Battery at top right
  if (BatteryIsConnected()) {
    StatusIconsDrawBattery(103, 1, batteryPct);
  }
  
  // Big time with Org_01 font
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(4);
  display.setFont(&Org_01);
  
  char timeStr[6];
  snprintf(timeStr, sizeof(timeStr), "%02d:%02d", hour, minute);
  display.setCursor(7, 42);
  display.print(timeStr);
  
  // Date text below time
  int year = 0, month = 0, day = 0;
  sscanf(dateStr, "%d/%d/%d", &year, &month, &day);
  
  const char* monthNamesShort[] = {"", "Jan", "Feb", "Mar", "Apr", "May", "Jun", 
                                    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
  
  display.setTextSize(1);
  display.setCursor(22, 56);
  if (month >= 1 && month <= 12) {
    display.print(day);
    display.print(" ");
    display.print(monthNamesShort[month]);
    display.print(" ");
    display.print(dayStr);
  }
  
  // Weather icon at bottom right
  StatusIconsDrawWeather(106, 29, weatherCode);
  
  // Temperature text
  display.setCursor(99, 56);
  display.print(tempStr);
  
  display.setFont();  // Reset to default font
  DisplayUpdate();
}
