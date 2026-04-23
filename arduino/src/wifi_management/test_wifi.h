
#ifndef TEST_WIFI_H
#define TEST_WIFI_H
#include <Arduino.h>
#include <WiFi.h>
#include "../../arduino_secrets.h"
#include "../csv_management/csvToJson.h"
#include "../logger/logger.h" 

bool disconnectWiFi();
bool sendToServer(const char* test_id, bool productA_pressed, bool productB_pressed, unsigned long timestamp);
void logWiFiStatus();
bool isWiFiConnecting();

#endif