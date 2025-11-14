#include "http_server.h"

#ifdef ESP32
#include <WebServer.h>
#include "config_store.h"

static WebServer server(80);

const char html[] PROGMEM = R"(
<!DOCTYPE html>
<html><head><title>Quil Setup</title>
<style>body{font-family:Arial;margin:40px;background:#1a1a1a;color:#fff}
input,button{padding:10px;margin:5px;width:200px}</style></head>
<body><h1>Quil Config</h1>
<form id="f"><input id="s" placeholder="WiFi SSID">
<input id="p" type="password" placeholder="Password">
<input id="wk" placeholder="Weather API Key">
<input id="wl" placeholder="Weather Location">
<button type="submit">Save</button></form>
<script>document.getElementById('f').onsubmit=e=>{e.preventDefault();
fetch('/save',{method:'POST',body:JSON.stringify({s:document.getElementById('s').value,
p:document.getElementById('p').value,
wk:document.getElementById('wk').value,
wl:document.getElementById('wl').value})}).then(r=>alert('Saved'))}</script>
</body></html>
)";

void handle_root() {
  server.send(200, "text/html", html);
}

void handle_save() {
  String body = server.arg("plain");
  int s_idx = body.indexOf("\"s\":\"") + 5;
  int s_end = body.indexOf("\"", s_idx);
  int p_idx = body.indexOf("\"p\":\"") + 5;
  int p_end = body.indexOf("\"", p_idx);
  
  int wk_idx = body.indexOf("\"wk\":\"") + 6;
  int wk_end = body.indexOf("\"", wk_idx);
  int wl_idx = body.indexOf("\"wl\":\"") + 6;
  int wl_end = body.indexOf("\"", wl_idx);

  String ssid = body.substring(s_idx, s_end);
  String pass = body.substring(p_idx, p_end);
  String wk = body.substring(wk_idx, wk_end);
  String wl = body.substring(wl_idx, wl_end);
  
  config_save_wifi(ssid.c_str(), pass.c_str());
  config_save_weather(wk.c_str(), wl.c_str());
  server.send(200, "text/plain", "OK");
  delay(1000);
  ESP.restart();
}

void http_init() {
  server.on("/", handle_root);
  server.on("/save", HTTP_POST, handle_save);
  server.begin();
}

void http_handle() {
  server.handleClient();
}

void http_stop() {
  server.stop();
}

#elif defined(ESP8266)
#include <ESP8266WebServer.h>
#include "config_store.h"

static ESP8266WebServer server(80);

const char html[] PROGMEM = R"(
<!DOCTYPE html>
<html><head><title>Quil Setup</title>
<style>body{font-family:Arial;margin:40px;background:#1a1a1a;color:#fff}
input,button{padding:10px;margin:5px;width:200px}</style></head>
<body><h1>Quil Config</h1>
<form id="f"><input id="s" placeholder="WiFi SSID">
<input id="p" type="password" placeholder="Password">
<input id="wk" placeholder="Weather API Key">
<input id="wl" placeholder="Weather Location">
<button type="submit">Save</button></form>
<script>document.getElementById('f').onsubmit=e=>{e.preventDefault();
fetch('/save',{method:'POST',body:JSON.stringify({s:document.getElementById('s').value,
p:document.getElementById('p').value,
wk:document.getElementById('wk').value,
wl:document.getElementById('wl').value})}).then(r=>alert('Saved'))}</script>
</body></html>
)";

void handle_root() {
  server.send(200, "text/html", html);
}

void handle_save() {
  String body = server.arg("plain");
  int s_idx = body.indexOf("\"s\":\"") + 5;
  int s_end = body.indexOf("\"", s_idx);
  int p_idx = body.indexOf("\"p\":\"") + 5;
  int p_end = body.indexOf("\"", p_idx);
  
  String ssid = body.substring(s_idx, s_end);
  String pass = body.substring(p_idx, p_end);
  
  int wk_idx = body.indexOf("\"wk\":\"") + 6;
  int wk_end = body.indexOf("\"", wk_idx);
  int wl_idx = body.indexOf("\"wl\":\"") + 6;
  int wl_end = body.indexOf("\"", wl_idx);

  String wk = body.substring(wk_idx, wk_end);
  String wl = body.substring(wl_idx, wl_end);

  config_save_wifi(ssid.c_str(), pass.c_str());
  config_save_weather(wk.c_str(), wl.c_str());
  server.send(200, "text/plain", "OK");
  delay(1000);
  ESP.restart();
}

void http_init() {
  server.on("/", handle_root);
  server.on("/save", HTTP_POST, handle_save);
  server.begin();
}

void http_handle() {
  server.handleClient();
}

void http_stop() {
  server.stop();
}
#endif
