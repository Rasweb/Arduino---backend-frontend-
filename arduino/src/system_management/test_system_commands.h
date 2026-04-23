#ifndef TEST_SYSTEM_COMMANDS_H
#define TEST_SYSTEM_COMMANDS_H
#include "../csv_management/csvToJson.h"
#include "../csv_management/csv_parser.h"
#include "../hardware_management/LED.h"
#include "../state_management/test_global_variables.h"
#include <Arduino.h>
#include <WiFi.h>
#include "../flash_management/flash-conf.h"
#include "../wifi_management/test_wifi.h"
#include "../state_management/test_states.h"
#include "../csv_management/test_csv_scanner.h"
#include "../sync_management/test_sync_manager.h"
#include "../types/state_types.h"
#include "../logger/logger.h"

void handleIncomingData();

#endif