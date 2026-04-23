#ifndef TEST_SYSTEM_CORE_H
#define TEST_SYSTEM_CORE_H

#include "test_system_commands.h"
#include "test_system_helpers.h"
#include "../csv_management/csvToJson.h"
#include "../hardware_management/LED.h"
#include "../../arduino_secrets.h"
#include "../button_management/test_button_handlers.h"
#include <Arduino.h>
#include <WiFi.h>
#include "../flash_management/flash-conf.h"
#include "../wifi_management/test_wifi.h"
#include "../state_management/test_states.h"
#include "../csv_management/test_csv_scanner.h"
#include "../sync_management/test_sync_manager.h"
#include "../logger/logger.h"

void testSystem_init(int btnPin1, int btnPin2, int btnPin3, int ledPin);
void testSystem_update();

#endif