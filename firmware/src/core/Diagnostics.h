#pragma once
#include <Arduino.h>

void diag_init();
void diag_update();
uint32_t diag_free_heap();
unsigned long diag_uptime();
