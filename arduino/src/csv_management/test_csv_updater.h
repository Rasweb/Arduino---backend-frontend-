#ifndef TEST_CSV_UPDATER_H
#define TEST_CSV_UPDATER_H

#include <WiFi.h>
#include <Arduino.h>
#include "../wifi_management/test_wifi.h"
#include "../flash_management/flash-conf.h"
#include "../csv_management/csv_parser.h"
#include "../hardware_management/LED.h"
#include "../state_management/test_states.h"
#include "../types/state_types.h"
#include "../../arduino_secrets.h"
#include "../logger/logger.h"

bool markTestAsSent(int lineNumber);
bool markTestAsSentInCSV(int lineNumber);
void updateSentStatus(int lineNumber);

#endif