#pragma once
#include <Arduino.h>

void http_init();
void http_handle();
void http_stop();
bool http_is_running();
