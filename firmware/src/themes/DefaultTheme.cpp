#include "DefaultTheme.h"
#include <Adafruit_GFX.h>
#include "assets/fonts/Org_01.h"

void DefaultThemeRender(int hour, int minute, const char* dateStr, const char* dayStr, 
                       uint8_t batteryPct, int rssi, bool wifiConnected, bool btConnected,
                       uint8_t weatherCode, const char* tempStr, const char* condStr) {
  Adafruit_SSD1306& display = DisplayGetDisplay();
  display.clearDisplay();
  
  // Status icons (top row)
  if (wifiConnected) {
    StatusIconsDrawWifi(4, 1, rssi);
  }
  StatusIconsDrawBluetooth(27, 1, btConnected);
  if (BatteryIsConnected()) {
    StatusIconsDrawBattery(48, 1, batteryPct);
  }
  
  StatusIconsDrawWeather(108, 1, weatherCode);
  
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(80, 2);
  display.print(condStr);
  display.setCursor(80,11);
  display.print(tempStr);
  
  // Separator lines
  display.drawLine(0, 18, 107, 18, 1);
  display.drawLine(0, 19, 96, 19, 1);
  
  // Big time display with Org_01 font
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(5);
  display.setTextWrap(false);
  display.setFont(&Org_01);
  
  char hourStr[3], minStr[3];
  snprintf(hourStr, sizeof(hourStr), "%02d", hour);
  snprintf(minStr, sizeof(minStr), "%02d", minute);
  
  display.setCursor(4, 45);
  display.print(hourStr);
  
  display.setTextSize(3);
  display.setFont();  // Reset to default for colon
  display.setCursor(57, 27);
  display.print(":");
  
  display.setTextSize(5);
  display.setFont(&Org_01);
  display.setCursor(69, 45);
  display.print(minStr);
  
  display.setFont();  // Reset to default for rest
  
  // Date bars
  int year = 0, month = 0, day = 0;
  sscanf(dateStr, "%d/%d/%d", &year, &month, &day);
  
  const char* monthNames[] = {"", "JAN", "FEB", "MAR", "APR", "MAY", "JUN", 
                               "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"};
  
  // Left date bar: MONTH + DAY
  display.fillRect(4, 53, 64, 9, 1);
  display.setTextColor(SSD1306_BLACK);
  display.setTextSize(1);
  display.setCursor(5, 54);
  if (month >= 1 && month <= 12) {
    display.print(monthNames[month]);
  }
  display.setCursor(56, 54);
  char dayNumStr[3];
  snprintf(dayNumStr, sizeof(dayNumStr), "%02d", day);
  display.print(dayNumStr);
  
  // Right date bar: YEAR + DAY_NAME
  display.fillRect(71, 53, 53, 9, 1);
  display.setCursor(76, 54);
  display.print(year);
  display.setCursor(104, 54);
  display.print(dayStr);
  
  DisplayUpdate();
}
