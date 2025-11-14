/*
 * Parola WiFi Matrix Display Controller - Modified for Persistent WiFi
 *
 * Changes: ESP will NEVER enter AP mode after initial setup
 *          AP mode only activates if default credentials "..." are present
 *          During WiFi outages, device waits and retries indefinitely
 */

#include <ArduinoOTA.h>
#include <EEPROM.h>
#include <ESP8266HTTPUpdateServer.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <MD_MAX72xx.h>
#include <ArduinoOTA.h>
#include <MD_Parola.h>
#include <NTPClient.h>
#include <SPI.h>
#include <WiFiUdp.h>
#include <map>
#include <ESP8266HTTPClient.h>

#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 4
#define CLK_PIN D5
#define DATA_PIN D7
#define CS_PIN D8

MD_Parola myDisplay =
  MD_Parola(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);
ESP8266HTTPUpdateServer httpUpdater;

textEffect_t effects[] = {
  PA_NO_EFFECT,
  PA_PRINT,
  PA_SCROLL_UP,
  PA_SCROLL_DOWN,
  PA_SCROLL_LEFT,
  PA_SCROLL_RIGHT,
  PA_SLICE,
  PA_MESH,
  PA_FADE,
  PA_DISSOLVE,
  PA_BLINDS,
  PA_RANDOM,
  PA_WIPE,
  PA_WIPE_CURSOR,
  PA_SCAN_HORIZ,
  PA_SCAN_HORIZX,
  PA_SCAN_VERT,
  PA_SCAN_VERTX,
  PA_OPENING,
  PA_OPENING_CURSOR,
  PA_CLOSING,
  PA_CLOSING_CURSOR,
  PA_SCROLL_UP_LEFT,
  PA_SCROLL_UP_RIGHT,
  PA_SCROLL_DOWN_LEFT,
  PA_SCROLL_DOWN_RIGHT,
  PA_GROW_UP,
  PA_GROW_DOWN
};

const char * effect_names[] = {
  "No Effect",
  "Print",
  "Scroll Up",
  "Scroll Down",
  "Scroll Left",
  "Scroll Right",
  "Slice",
  "Mesh",
  "Fade",
  "Dissolve",
  "Blinds",
  "Random",
  "Wipe",
  "Wipe Cursor",
  "Scan Horiz",
  "Scan HorizX",
  "Scan Vert",
  "Scan VertX",
  "Opening",
  "Opening Cursor",
  "Closing",
  "Closing Cursor",
  "Scroll Up Left",
  "Scroll Up Right",
  "Scroll Down Left",
  "Scroll Down Right",
  "Grow Up",
  "Grow Down"
};

const char * effect_icons[] = {
  "block",
  "print",
  "arrow_upward",
  "arrow_downward",
  "arrow_back",
  "arrow_forward",
  "animation",
  "grid_on",
  "blur_on",
  "grain",
  "view_week",
  "shuffle",
  "waterfall_chart",
  "keyboard_tab",
  "deselect",
  "table_chart",
  "height",
  "align_vertical_bottom",
  "open_in_full",
  "keyboard_return",
  "fullscreen_exit",
  "undo",
  "north_east",
  "north_west",
  "south_east",
  "south_west",
  "vertical_align_top",
  "vertical_align_bottom"
};

#define NUM_EFFECTS (sizeof(effects) / sizeof(effects[0]))
#define EEPROM_SIZE 256
#define EEPROM_ADDR_BRIGHTNESS 0
#define EEPROM_ADDR_SPEED 1
#define EEPROM_ADDR_EFFECT 2
#define EEPROM_ADDR_MESSAGE 4
#define EEPROM_ADDR_WIFI_SSID 68
#define EEPROM_ADDR_WIFI_PASS 132

struct Settings {
  uint8_t brightness, speed, effect_idx;
  char message[64];
  char wifi_ssid[64];
  char wifi_password[64];
};
Settings settings;

const char * ssid = "...";
const char * password = "...";
bool apMode = false;
const char * adminUser = "admin";
const char * adminPass = "admin@parola";
ESP8266WebServer server(80);
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 19800, 60000);
bool wifiConnected = true;
bool displayDisabled = false;
unsigned long lastWifiCheck = 0;
const unsigned long wifiRetryInterval = 15000;

// TRACK LOGIN ATTEMPTS
struct LoginAttempt {
  uint8_t attempts;
  unsigned long lastAttemptTime;
  unsigned long lastSeen;
};

std::map < String, LoginAttempt > loginAttempts;
const int MAX_ATTEMPTS = 3;
const unsigned long BLOCK_TIME = 30000;

String sessionToken = "";
unsigned long sessionTimestamp = 0;
const unsigned long SESSION_TIMEOUT = 10 * 60 * 1000UL; // 10 minutes

// IP cleanup timing control
unsigned long lastCleanupTime = 0;
const unsigned long CLEANUP_INTERVAL = 60000;
const unsigned long ONE_WEEK_MS = 604800000UL; // 7 days in milliseconds

String lastClock = "";
String lastClockUI = "";

String lastTimeStr = "";
String lastDateStr = "";
String lastDayStr = "";

bool waitingForEffectFinish = false;

unsigned long lastInfoChange = 0;
const unsigned long INFO_DISPLAY_MS = 2200;

enum InfoMode { MODE_TIME, MODE_DATE, MODE_DAY };
InfoMode infoMode = MODE_TIME;


bool isAuthorized() {
  if (!server.hasHeader("Authorization"))
    return false;
  if (millis() - sessionTimestamp > SESSION_TIMEOUT)
    return false;
  String header = server.header("Authorization");
  bool ok = header == "Bearer " + sessionToken;
  if (ok)
    sessionTimestamp = millis();
  return ok;
}

String sanitizeMsg(String msg) {
  String o = "";
  for (uint8_t i = 0; i < msg.length() && o.length() < 63; i++) {
    char c = msg.charAt(i);
    if (c >= 32 && c <= 126 && c != '<' && c != '>' && c != '\"' &&
      c != '\'')
      o += c;
  }
  return o;
}
void cleanupOldIPs() {
  unsigned long now = millis();
  for (auto it = loginAttempts.begin(); it != loginAttempts.end();) {
    if (now - it -> second.lastSeen > ONE_WEEK_MS) {
      it = loginAttempts.erase(it);
    } else {
      ++it;
    }
  }
}
void updateClock() {
  if (timeClient.update()) {
    time_t rawTime = timeClient.getEpochTime();
    struct tm *tmInfo = localtime(&rawTime);

    // Day name
    static const char* daynames[] = { "Sun", "Mon", "Tue", "Wed", "Thur", "Fri", "Sat" };
    String dayStr = daynames[tmInfo->tm_wday];

    // Date in "DD MMM" format, e.g. "15 JUL"
    static const char* monthnames[] = {
    "JAN", "FEB", "MAR", "APR", "MAY", "JUNE",
    "JULY", "AUG", "SEP", "OCT", "NOV", "DEC"
    };
    int mo = tmInfo->tm_mon;
    int da = tmInfo->tm_mday;
    char dateBuf[12];
    snprintf(dateBuf, sizeof(dateBuf), "%d %s", da, monthnames[mo]);

    // Time in 12h format with AM/PM
    int hr = timeClient.getHours();
    int mn = timeClient.getMinutes();
    char ap = (hr < 12) ? 'A' : 'P';
    int dhr = hr % 12; if (dhr == 0) dhr = 12;
    char timeBuf[10];
    snprintf(timeBuf, sizeof(timeBuf), "%d:%02d %c", dhr, mn, ap);

    lastDayStr = dayStr;
    lastDateStr = String(dateBuf);
    lastTimeStr = String(timeBuf);

    char uiBuf[32];
    snprintf(uiBuf, sizeof(uiBuf), "%s, %s %s", dayStr.c_str(), dateBuf, timeBuf);
    lastClockUI = String(uiBuf);
  }
}
void forceShowDefaultMessage() {
    String msg = String(settings.message);
    msg.trim();
    myDisplay.displayClear();
    myDisplay.setIntensity(map(settings.brightness, 1, 15, 1, 15));
    myDisplay.setSpeed(map(settings.speed, 1, 10, 100, 10));
    textEffect_t eff = effects[settings.effect_idx < NUM_EFFECTS ? settings.effect_idx : 0];
    if (msg.length() == 0) {
        const char* displayText;
        if (infoMode == MODE_TIME)      { displayText = lastTimeStr.c_str();      waitingForEffectFinish = true; }
        else if (infoMode == MODE_DATE) { displayText = lastDateStr.c_str();      waitingForEffectFinish = true; }
        else                           { displayText = lastDayStr.c_str();       waitingForEffectFinish = true; }
        myDisplay.displayText(displayText, PA_CENTER, myDisplay.getSpeed(), 1000, eff, eff);
    } else {
        waitingForEffectFinish = false;
        myDisplay.displayText(settings.message, PA_LEFT, myDisplay.getSpeed(), 0, eff, eff);
    }
}
void applySettings() {
  myDisplay.setIntensity(map(settings.brightness, 1, 15, 1, 15));
  myDisplay.setSpeed(map(settings.speed, 1, 10, 100, 10));
  forceShowDefaultMessage();
}
void saveSettingsToEEPROM() {
  EEPROM.write(EEPROM_ADDR_BRIGHTNESS, settings.brightness);
  EEPROM.write(EEPROM_ADDR_SPEED, settings.speed);
  EEPROM.write(EEPROM_ADDR_EFFECT, settings.effect_idx);
  for (uint8_t i = 0; i < sizeof(settings.message); i++) {
    EEPROM.write(EEPROM_ADDR_MESSAGE + i, settings.message[i]);
    if (settings.message[i] == '\0')
      break;
  }
  for (uint8_t i = 0; i < sizeof(settings.wifi_ssid); i++) {
    EEPROM.write(EEPROM_ADDR_WIFI_SSID + i, settings.wifi_ssid[i]);
    if (settings.wifi_ssid[i] == '\0')
      break;
  }
  for (uint8_t i = 0; i < sizeof(settings.wifi_password); i++) {
    EEPROM.write(EEPROM_ADDR_WIFI_PASS + i, settings.wifi_password[i]);
    if (settings.wifi_password[i] == '\0')
      break;
  }
  EEPROM.commit();
}

bool hasActiveInternet() {
    WiFiClient client;
    HTTPClient http;
    http.begin(client, "http://clients3.google.com/generate_204");
    int code = http.GET();
    http.end();
    return (code == 204);
}
void loadSettingsFromEEPROM() {
  uint8_t b = EEPROM.read(EEPROM_ADDR_BRIGHTNESS);
  uint8_t s = EEPROM.read(EEPROM_ADDR_SPEED);
  uint8_t e = EEPROM.read(EEPROM_ADDR_EFFECT);
  if (b >= 1 && b <= 15)
    settings.brightness = b;
  if (s >= 1 && s <= 10)
    settings.speed = s;
  if (e < NUM_EFFECTS)
    settings.effect_idx = e;
  for (uint8_t i = 0; i < sizeof(settings.message); i++)
    settings.message[i] = EEPROM.read(EEPROM_ADDR_MESSAGE + i);
  settings.message[sizeof(settings.message) - 1] = '\0';

  // Load WiFi credentials
  for (uint8_t i = 0; i < sizeof(settings.wifi_ssid); i++)
    settings.wifi_ssid[i] = EEPROM.read(EEPROM_ADDR_WIFI_SSID + i);
  settings.wifi_ssid[sizeof(settings.wifi_ssid) - 1] = '\0';

  for (uint8_t i = 0; i < sizeof(settings.wifi_password); i++)
    settings.wifi_password[i] = EEPROM.read(EEPROM_ADDR_WIFI_PASS + i);
  settings.wifi_password[sizeof(settings.wifi_password) - 1] = '\0';

  // Don't overwrite empty EEPROM with defaults - leave empty to preserve saved credentials
  // Only set to empty string if EEPROM contains garbage (0xFF)
  if (settings.wifi_ssid[0] == 0xFF) {
    settings.wifi_ssid[0] = '\0';
  }
  if (settings.wifi_password[0] == 0xFF) {
    settings.wifi_password[0] = '\0';
  }
}

void setup() {
  Serial.begin(9600);
  delay(100);
  EEPROM.begin(EEPROM_SIZE);
  loadSettingsFromEEPROM();
  myDisplay.begin();

  // Determine WiFi credentials to use
  const char* connectSSID = ssid;
  const char* connectPassword = password;
  bool hasValidCredentials = false;

  // Check if saved WiFi credentials exist and are valid (not default "...")
  if (strlen(settings.wifi_ssid) > 0 && strcmp(settings.wifi_ssid, "...") != 0) {
    connectSSID = settings.wifi_ssid;
    connectPassword = settings.wifi_password;
    hasValidCredentials = true;
    Serial.print("Using saved WiFi credentials: ");
    Serial.println(connectSSID);
  } 
  // Check if default credentials are valid (not "...")
  else if (strcmp(ssid, "...") != 0) {
    hasValidCredentials = true;
    Serial.print("Using default WiFi credentials: ");
    Serial.println(connectSSID);
  }

  // CRITICAL: Only enter AP mode if NO valid credentials exist (still at default "...")
  if (!hasValidCredentials) {
    Serial.println("No WiFi credentials configured (default '...' detected).");
    Serial.println("Starting AP mode for initial setup.");
    apMode = true;
  } else {
    // Valid credentials exist - NEVER use AP mode, even if connection fails
    apMode = false;
    Serial.println("Valid WiFi credentials found. AP mode disabled.");
  }

  if (!apMode) {
    // Station mode - attempt to connect with persistent retry
    WiFi.mode(WIFI_STA);
    WiFi.begin(connectSSID, connectPassword);
    WiFi.setAutoReconnect(true);
    WiFi.persistent(true);

    #ifdef ESP8266
    WiFi.setSleepMode(WIFI_NONE_SLEEP);
    #endif

    unsigned long startAttemptTime = millis();
    Serial.print("Connecting to WiFi");

    // Initial connection attempt (20 seconds)
    while (WiFi.status() != WL_CONNECTED &&
      millis() - startAttemptTime < 20000) {
      delay(100);
      Serial.print(".");
      yield();
    }

    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\nWiFi connected. IP: " + WiFi.localIP().toString());
    } else {
      // Connection failed but credentials exist - stay in STA mode and keep retrying
      Serial.println("\nInitial WiFi connection failed.");
      Serial.println("Device will continue attempting to connect in background.");
      Serial.println("Display disabled until WiFi is restored.");
      wifiConnected = false;
      displayDisabled = true;
      myDisplay.displayClear();
      myDisplay.displaySuspend(true);
    }
  }

  // Only start AP mode if no valid credentials exist
  if (apMode) {
    WiFi.mode(WIFI_AP);
    WiFi.softAP("Parola", "parola123");
    Serial.print("AP Mode active for initial setup. IP: ");
    Serial.println(WiFi.softAPIP());
  }

  ArduinoOTA.setHostname("Parola_");
  ArduinoOTA.begin();
  timeClient.begin();
  updateClock();

  server.on("/", HTTP_GET,
    []() {
      server.send_P(200, "text/html", index_html);
    });
  server.on("/status", HTTP_GET, []() {
    if (!isAuthorized()) {
      server.send(403, "text/plain", "Auth");
      return;
    }
    updateClock();
    String json = "{";
    json += "\"brightness\":" + String(settings.brightness) + ",";
    json += "\"speed\":" + String(settings.speed) + ",";
    json += "\"effect\":" + String(settings.effect_idx) + ",";
    json += "\"message\":\"" +
      String(sanitizeMsg(String(settings.message))) + "\",";
    json += "\"ip\":\"" + (apMode ? WiFi.softAPIP().toString() : WiFi.localIP().toString()) + "\",";
    json += "\"curtime\":\"" + lastClockUI + "\",";
    json += "\"ssid\":\"" + (apMode ? String("Parola") : String(WiFi.SSID())) + "\",";
    json += "\"apMode\":" + String(apMode ? "true" : "false") + ",";
    unsigned long sec = millis() / 1000;
    unsigned long hrs = sec / 3600;
    unsigned long min = (sec % 3600) / 60;
    unsigned long s = sec % 60;
    char uptime_str[24];
    snprintf(uptime_str, sizeof(uptime_str), "%02luh %02lum %02lus", hrs,
      min, s);
    json += "\"uptime\":\"";
    json += uptime_str;
    json += "\"";
    json += "}";
    server.send(200, "application/json", json);
  });

  server.on("/login", HTTP_POST, []() {
    String clientIP = server.client().remoteIP().toString();

    LoginAttempt & attempt = loginAttempts[clientIP];
    attempt.lastSeen = millis();
    if (millis() - attempt.lastAttemptTime < BLOCK_TIME &&
      attempt.attempts >= MAX_ATTEMPTS) {
      server.send(429, "text/plain", "Too many attempts. Try later.");
      return;
    }

    if (!server.hasArg("username") || !server.hasArg("password")) {
      server.send(400, "text/plain", "Missing credentials");
      return;
    }

    String username = server.arg("username");
    String pass = server.arg("password");

    if (username == adminUser && pass == adminPass) {
      sessionToken = String(random(0x0FFFFFFF), HEX);
      sessionTimestamp = millis();
      attempt.attempts = 0;
      attempt.lastSeen = millis();
      server.send(200, "application/json",
        "{\"token\": \"" + sessionToken + "\"}");
    } else {
      attempt.attempts++;
      attempt.lastAttemptTime = millis();
      server.send(401, "text/plain", "Invalid login");
    }
  });

  server.on("/resetAll", HTTP_POST, []() {
    if (!isAuthorized()) {
      server.send(403, "text/plain", "Auth");
      return;
    }

    for (int i = 0; i < EEPROM_SIZE; i++) {
      EEPROM.write(i, 0);
    }
    EEPROM.commit();

    server.send(200, "text/plain", "Reset OK");
    delay(500);
    ESP.restart(); // Optional: reboot after reset
  });

  server.on("/setBright", HTTP_POST, []() {
    if (!isAuthorized()) {
      server.send(403, "text/plain", "Auth");
      return;
    }
    if (server.hasArg("value")) {
      settings.brightness = constrain(server.arg("value").toInt(), 1, 15);
      applySettings();
      saveSettingsToEEPROM();
      server.send(200, "text/plain", "OK");
    } else
      server.send(400, "text/plain", "Missing value");
  });
  server.on("/setSpeed", HTTP_POST, []() {
    if (!isAuthorized()) {
      server.send(403, "text/plain", "Auth");
      return;
    }
    if (server.hasArg("value")) {
      settings.speed = constrain(server.arg("value").toInt(), 1, 10);
      applySettings();
      saveSettingsToEEPROM();
      server.send(200, "text/plain", "OK");
    } else
      server.send(400, "text/plain", "Missing value");
  });
  server.on("/setEffect", HTTP_POST, []() {
    if (!isAuthorized()) {
      server.send(403, "text/plain", "Auth");
      return;
    }
    if (server.hasArg("value")) {
      settings.effect_idx =
        constrain(server.arg("value").toInt(), 0, NUM_EFFECTS - 1);
      applySettings();
      saveSettingsToEEPROM();
      server.send(200, "text/plain", "OK");
    } else
      server.send(400, "text/plain", "Missing value");
  });
  server.on("/setWifi", HTTP_POST, []() {
    if (!isAuthorized()) {
      server.send(403, "text/plain", "Auth");
      return;
    }
    if (server.hasArg("ssid")) {
      String newSSID = server.arg("ssid");
      String newPassword = server.arg("password");

      if (newSSID.length() > 0 && newSSID.length() < 64) {
        newSSID.toCharArray(settings.wifi_ssid, 64);
        newPassword.toCharArray(settings.wifi_password, 64);
        saveSettingsToEEPROM();
        server.send(200, "text/plain", "OK");
        delay(1000);
        ESP.restart();
      } else {
        server.send(400, "text/plain", "Invalid SSID");
      }
    } else {
      server.send(400, "text/plain", "Missing SSID");
    }
  });
  server.on("/setMessage", HTTP_POST, []() {
    if (!isAuthorized()) {
      server.send(403, "text/plain", "Auth");
      return;
    }
    if (server.hasArg("value")) {
      String msg = sanitizeMsg(server.arg("value"));
      memset(settings.message, 0, sizeof(settings.message));
      msg.toCharArray(settings.message, sizeof(settings.message));
      myDisplay.setTextBuffer(settings.message);
      saveSettingsToEEPROM();
      server.send(200, "text/plain", "OK");
    } else
      server.send(400, "text/plain", "Missing value");
  });
  server.onNotFound([]() {
    server.sendHeader("Location", "/", true);
    server.send(302, "text/plain", "Not found, redirecting...");
  });
  httpUpdater.setup( & server, "/update", adminUser, adminPass);
  server.begin();
  applySettings();
}
unsigned long lastNtpSync = 0;
void loop() {
  server.handleClient();
  ArduinoOTA.handle();

  if (millis() - lastCleanupTime > CLEANUP_INTERVAL) {
    cleanupOldIPs();
    lastCleanupTime = millis();
  }
  unsigned long now = millis();
  
  // WiFi AND Internet watchdog/reconnect logic (only runs when NOT in AP mode)
  if (!apMode && !displayDisabled) {
    static bool internetConnected = true;
    if (now - lastWifiCheck > wifiRetryInterval) {
      lastWifiCheck = now;
      bool wifiOk = (WiFi.status() == WL_CONNECTED);
      bool inetOk = wifiOk && hasActiveInternet();

      if (!wifiOk || !inetOk) {
        // If this is a new disconnect/internet-down, deactivate the display
        if (wifiConnected || internetConnected) {
          Serial.println("WiFi/Internet lost. Turning off display.");
          wifiConnected = wifiOk;
          internetConnected = inetOk;
          displayDisabled = true;
          myDisplay.displayClear();
          myDisplay.displaySuspend(true);
        }
        // Try to reconnect WiFi if needed
        if (!wifiOk) {
          Serial.println("Attempting WiFi reconnection...");
          WiFi.disconnect();
          const char* reconnectSSID = (strlen(settings.wifi_ssid) > 0 && strcmp(settings.wifi_ssid, "...") != 0) ? settings.wifi_ssid : ssid;
          const char* reconnectPassword = (strlen(settings.wifi_ssid) > 0 && strcmp(settings.wifi_ssid, "...") != 0) ? settings.wifi_password : password;
          WiFi.begin(reconnectSSID, reconnectPassword);
        }
      } else {
        if (!wifiConnected || !internetConnected) {
          Serial.println("WiFi/Internet Restored.");
          wifiConnected = true;
          internetConnected = true;
          displayDisabled = false;
          myDisplay.displayClear();
          myDisplay.displaySuspend(false);
          applySettings();
          updateClock();
        }
      }
    }
  }
  
  // Handle initial connection attempts when display is disabled at startup
  if (!apMode && displayDisabled && WiFi.status() == WL_CONNECTED) {
    Serial.println("WiFi connected during background retry.");
    Serial.println("IP: " + WiFi.localIP().toString());
    wifiConnected = true;
    displayDisabled = false;
    myDisplay.displayClear();
    myDisplay.displaySuspend(false);
    applySettings();
    updateClock();
  }
  
  if (!displayDisabled) {
    if (now - lastNtpSync > 40000UL || lastNtpSync == 0) {
      lastNtpSync = now;
      updateClock();
    }

    String msg = String(settings.message);
    msg.trim();
    if (msg.length() == 0) {
      if (waitingForEffectFinish) {
        if (myDisplay.displayAnimate()) {
          infoMode = static_cast<InfoMode>((infoMode + 1) % 3);
          lastInfoChange = now;
          forceShowDefaultMessage();
        }
      } else {
        if (now - lastInfoChange >= INFO_DISPLAY_MS) {
          infoMode = static_cast<InfoMode>((infoMode + 1) % 3);
          lastInfoChange = now;
          forceShowDefaultMessage();
        }
        if (myDisplay.displayAnimate()) {
          myDisplay.displayReset();
        }
      }
    } else {
      if (myDisplay.displayAnimate()) {
        myDisplay.displayReset();
      }
    }
  }
}