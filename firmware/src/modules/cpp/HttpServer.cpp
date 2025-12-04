#include "../h/HttpServer.h"

#include <WebServer.h>
#include "../h/ConfigStore.h"

static WebServer server(80);

const char html[] PROGMEM = R"HTML(
<!DOCTYPE html>
<html><head><title>Quil Setup</title>
<style>body{font-family:Arial;margin:40px;background:#1a1a1a;color:#fff}
input,select,button{padding:10px;margin:5px;width:220px}
label{display:block;margin-top:10px;color:#aaa;font-size:12px}
input[type=range]{width:220px}
.btn-reset{background:#d32f2f;margin-top:30px}</style></head>
<body><h1>Quil Config</h1>
<form id="f">
<input id="s" placeholder="WiFi SSID">
<input id="p" type="password" placeholder="Password">
<label>Timezone Offset (seconds from UTC)</label>
<select id="tz">
<option value="0">UTC (0)</option>
<option value="19800">IST - India (UTC+5:30)</option>
<option value="28800">CST - China (UTC+8)</option>
<option value="32400">JST - Japan (UTC+9)</option>
<option value="-18000">EST - US East (UTC-5)</option>
<option value="-21600">CST - US Central (UTC-6)</option>
<option value="-25200">MST - US Mountain (UTC-7)</option>
<option value="-28800">PST - US West (UTC-8)</option>
<option value="3600">CET - Central Europe (UTC+1)</option>
<option value="7200">EET - Eastern Europe (UTC+2)</option>
<option value="36000">AEST - Australia East (UTC+10)</option>
</select>
<input id="wk" placeholder="Weather API Key">
<input id="wl" placeholder="Weather Location">
<label>Display Brightness: <span id="cv">128</span></label>
<input type="range" id="c" min="0" max="255" value="128">
<button type="submit">Save</button></form>
<button class="btn-reset" onclick="factoryReset()">Factory Reset</button>
<script>
document.getElementById('c').oninput=function(e){document.getElementById('cv').textContent=e.target.value};
document.getElementById('f').onsubmit=function(e){e.preventDefault();
fetch('/save',{method:'POST',body:JSON.stringify({s:document.getElementById('s').value,
p:document.getElementById('p').value,
tz:document.getElementById('tz').value,
wk:document.getElementById('wk').value,
wl:document.getElementById('wl').value,
c:document.getElementById('c').value})}).then(function(r){alert('Saved! Restarting...')})};
function factoryReset(){if(confirm('Reset all settings? Device will restart.')){
fetch('/reset',{method:'POST'}).then(function(r){alert('Reset complete! Restarting...')})}}</script>
</body></html>
)HTML";

void handle_root() {
  server.send(200, "text/html", html);
}

void handle_save() {
  String body = server.arg("plain");
  int s_idx = body.indexOf("\"s\":\"") + 5;
  int s_end = body.indexOf("\"", s_idx);
  int p_idx = body.indexOf("\"p\":\"") + 5;
  int p_end = body.indexOf("\"", p_idx);
  
  int tz_idx = body.indexOf("\"tz\":\"") + 6;
  int tz_end = body.indexOf("\"", tz_idx);
  
  int wk_idx = body.indexOf("\"wk\":\"") + 6;
  int wk_end = body.indexOf("\"", wk_idx);
  int wl_idx = body.indexOf("\"wl\":\"") + 6;
  int wl_end = body.indexOf("\"", wl_idx);

  int c_idx = body.indexOf("\"c\":\"") + 5;
  int c_end = body.indexOf("\"", c_idx);
  
  String ssid = body.substring(s_idx, s_end);
  String pass = body.substring(p_idx, p_end);
  String tz_str = body.substring(tz_idx, tz_end);
  String wk = body.substring(wk_idx, wk_end);
  String wl = body.substring(wl_idx, wl_end);
  String c_str = body.substring(c_idx, c_end);
  
  config_save_wifi(ssid.c_str(), pass.c_str());
  if (tz_str.length() > 0) {
    int tz_offset = tz_str.toInt();
    config_save_timezone(tz_offset);
  }
  if (c_str.length() > 0) {
    uint8_t contrast = c_str.toInt();
    config_save_contrast(contrast);
  }
  config_save_weather(wk.c_str(), wl.c_str());
  server.send(200, "text/plain", "OK");
  delay(1000);
  ESP.restart();
}

void handle_reset() {
  config_clear();
  server.send(200, "text/plain", "OK");
  delay(1000);
  ESP.restart();
}

void http_init() {
  server.on("/", handle_root);
  server.on("/save", HTTP_POST, handle_save);
  server.on("/reset", HTTP_POST, handle_reset);
  server.begin();
  Serial.println("[HTTP] Server started");
}

void http_handle() {
  server.handleClient();
}

void http_stop() {
  server.stop();
  Serial.println("[HTTP] Server stopped");
}

bool http_is_running() {
  return true; // Server always runs after init
}
